#include "FOC.h"
#include "PID.h"
#include "lowpass_fillter.h"
#include "electricity.h"
#include "User_define.h"
#include "AS5047P.h"
#include "const_sincos.h"

float Ualpha, Ubeta = 0, Ua = 0, Ub = 0, Uc = 0;
float zero_electric_angle = 0.0f;
int PP = 1, DIR = 1;
float pid_output;
float shaft_angle = 0, open_loop_timestamp = 0;
float current_opamp_gain, current_Rshunt_gain, voltage_power_supply_gain;
uint16_t quarter = 1024;
int16_t raw_angle;
float angle;
float v_fillter, v;

lowpass_fillter velocity_fillter = {.Ts = velocity_fillter_Ts, .timestamp_prev = 0.0f, .y_prev = 0.0f};
lowpass_fillter electricity_fillter = {.Ts = electricity_fillter_Ts, .timestamp_prev = 0.0f, .y_prev = 0.0f};
PID_Struct velocity_PID = {.kp = velocity_p, .ki = velocity_i, .kd = 0, .I = 0, .lim_output = velocity_lim_output, .output_ramp = velocity_output_ramp, .lim_ki = velocity_lim_I, .last_time = 0, .last_error = 0, .last_output = 0};
PID_Struct angle_PID = {.kp = angle_p, .ki = angle_i, .kd = 0, .I = 0, .lim_output = angle_lim_output, .output_ramp = angle_output_ramp, .lim_ki = angle_lim_I, .last_time = 0, .last_error = 0, .last_output = 0};
PID_Struct current_PID = {.kp = current_p, .ki = current_i, .kd = 0, .I = 0, .lim_output = current_lim_output, .output_ramp = current_output_ramp, .lim_ki = current_lim_I, .last_time = 0, .last_error = 0, .last_output = 0};

// 开环测电机专用
float _normalizeAngle(float angle);
float _electricalAngle1(float shaft_angle, int pole_pairs)
{
  return (shaft_angle * pole_pairs);
}
void velocityOpenloop(float target_velocity) // 开环测试程序
{
  float Ts = (micros() - open_loop_timestamp) * 1e-6f; // 计算上次到当前的间隔
  open_loop_timestamp = micros();                      // 更新全局时间
  if (Ts <= 0 || Ts > 0.5f)
    Ts = 1e-3f;
  // 通过开环速度乘以经过的时间，得出下一个电角度
  shaft_angle = _normalizeAngle(shaft_angle + target_velocity * Ts);
  // 最大只能设置为Uq = voltage_power_supply/2
  float Uq = voltage_power_supply / 15.0f;

  FOC_clark_park(Uq, _electricalAngle1(shaft_angle, 7));
}

// 归一化角度到 [0,2PI]
float _normalizeAngle(float angle)
{
  float a = fmod(angle, 2 * PI); // 取余运算可以用于归一化
  return a >= 0 ? a : (a + 2 * PI);
}

// 电角度求解
float _electricalAngle(float angle)
{
  return _normalizeAngle((float)(DIR * PP) * angle - zero_electric_angle);
}

// 此函数用于初始化参数，将浮点数除法换成浮点数乘法
void calculate_variety(void)
{
  current_opamp_gain = 1.0f / current_opamp;
  current_Rshunt_gain = 1.0f / current_Rshunt;
  voltage_power_supply_gain = 1.0f / voltage_power_supply;
}

// 获取正弦值函数
float find_sin_in_table(uint16_t rawangle)
{
  rawangle &= 0x3FFF;
  const int quadrant = rawangle >> 12;
  const int index = rawangle & 0x0FFF;
  switch (quadrant)
  {
  case 0: // 第一象限
    return sin_table[index];
  case 1: // 第二象限
    return sin_table[4095 - index];
  case 2: // 第三象限
    return -sin_table[index];
  case 3: // 第四象限
    return -sin_table[4095 - index];
  }
  return 0.0f;
}
// 获取正弦值函数
float find_cos_in_table(uint16_t rawangle)
{
  rawangle &= 0x3FFF;                  // 保留低14位 (0-16383)
  const int quadrant = rawangle >> 12; // 取高2位作为象限 (0-3)
  const int index = rawangle & 0x0FFF; // 取低12位作为索引 (0-4095)
  switch (quadrant)
  {
  case 0:
    return cos_table[index];
  case 1:
    return -cos_table[4095 - index];
  case 2:
    return -cos_table[index];
  case 3:
    return cos_table[4095 - index];
  }
  return 0.0f;
}

// 对电机进行初始化
void setMotor(int _PP, int _DIR)
{
  PP = _PP;
  DIR = _DIR;
  FOC_clark_park(2, _3PI_2); // 这两行通过给固定占空比产生固定磁场
  HAL_Delay(1000);           // 使转子被吸附，确保零点角度读取准确
  Sensor_AS5047P_Calculate_rotations(&hspi1, GPIOB, GPIO_PIN_0);
  raw_angle = AS5047P_get_rawdata(&hspi1, GPIOB, GPIO_PIN_0);
  float transmit_angle = raw_angle * (2.0f * PI) / 16384.0f;
  zero_electric_angle = _electricalAngle(transmit_angle);
  lastangle = transmit_angle;
  FOC_clark_park(0, _3PI_2); // 解除吸附
}

void setPwm(float Ua, float Ub, float Uc)
{

  // 限幅占空比
  float dc_a = _constrain(Ua, 0.0f, PWM_lim_compare); // 限幅到PWM_lim_compare是确保电流采样时,
  float dc_b = _constrain(Ub, 0.0f, PWM_lim_compare); // 防止因MOS开关产生干扰
  float dc_c = _constrain(Uc, 0.0f, PWM_lim_compare);

  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, dc_a * 8500);
  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, dc_b * 8500);
  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, dc_c * 8500);
}

void FOC_clark_park(float Uq, float angle_el)
{
  // SPWM
  /*Uq = _constrain(Uq, -voltage_power_supply / 2.0f, voltage_power_supply / 2.0f);
  // float Ud=0;

  // 帕克逆变换
  Ualpha = -Uq * sinf(angle_el);
  Ubeta = Uq * cosf(angle_el);

  // 克拉克逆变换
  Ua = Ualpha + voltage_power_supply / 2.0f;
  Ub = (_SQRT3 * Ubeta - Ualpha) / 2.0f + voltage_power_supply / 2.0f;
  Uc = (-Ualpha - _SQRT3 * Ubeta) / 2.0f + voltage_power_supply / 2.0f;
  setPwm(Ua / voltage_power_supply, Ub / voltage_power_supply, Uc / voltage_power_supply);*/

  // SVPWM
  if (Uq < 0)
  {
    angle_el += PI;
    Uq = -Uq;
  }
  angle_el = _normalizeAngle(angle_el + _PI_2);
  int sector = floor(angle_el * _3_PI) + 1;
  // calculate the duty cycles
  float T1 = _SQRT3 * sin(sector * _PI_3 - angle_el) * Uq * voltage_power_supply_gain;
  float T2 = _SQRT3 * sin(angle_el - (sector - 1.0) * _PI_3) * Uq * voltage_power_supply_gain;
  float T0 = 1 - T1 - T2;

  float Ta, Tb, Tc;
  switch (sector)
  {
  case 1:
    Ta = T1 + T2 + T0 / 2;
    Tb = T2 + T0 / 2;
    Tc = T0 / 2;
    break;
  case 2:
    Ta = T1 + T0 / 2;
    Tb = T1 + T2 + T0 / 2;
    Tc = T0 / 2;
    break;
  case 3:
    Ta = T0 / 2;
    Tb = T1 + T2 + T0 / 2;
    Tc = T2 + T0 / 2;
    break;
  case 4:
    Ta = T0 / 2;
    Tb = T1 + T0 / 2;
    Tc = T1 + T2 + T0 / 2;
    break;
  case 5:
    Ta = T2 + T0 / 2;
    Tb = T0 / 2;
    Tc = T1 + T2 + T0 / 2;
    break;
  case 6:
    Ta = T1 + T2 + T0 / 2;
    Tb = T0 / 2;
    Tc = T1 + T0 / 2;
    break;
  default:
    Ta = 0;
    Tb = 0;
    Tc = 0;
  }

  float Ua = Ta;
  float Ub = Tb;
  float Uc = Tc;

  setPwm(Ua, Ub, Uc);
}

float getVelocity_Fillter(void)
{
  v = Sensor_AS5047PgetVelocity();
  v_fillter = LowPass_Fillter(&velocity_fillter, v * DIR);
  return v_fillter;
}

float getCurrent_fillter(float angle)
{
  current_Preview = calculate_current(angle);
  current_Now = LowPass_Fillter(&electricity_fillter, current_Preview * DIR);
  return current_Now;
}

void FOC_setVelocity(float target)
{
  raw_angle = Sensor_AS5047P_Calculate_rotations(&hspi1, GPIOB, GPIO_PIN_0); // 这一行不仅得到了转换后的角度，还更新了圈数为计算速度做准备
  float transmit_angle = raw_angle * (2.0f * PI) / 16384.0f;
  //angle = if_angle_real(transmit_angle);
  angle = transmit_angle;
	float electrical_angle = _electricalAngle(transmit_angle);
  pid_output = PID(&velocity_PID, (target - getVelocity_Fillter()));
  FOC_clark_park(pid_output, electrical_angle);
}

void FOC_setAngle(float target)
{
  raw_angle = Sensor_AS5047P_Calculate_rotations(&hspi1, GPIOB, GPIO_PIN_0);
  float transmit_angle = raw_angle * (2.0f * PI) / 16384.0f;
  angle = transmit_angle;
  float electrical_angle = _electricalAngle(transmit_angle);
  FOC_clark_park(PID(&angle_PID, (target - DIR * Sensor_AS5047PgetTotalAngle())), electrical_angle);
}

void FOC_setCurrent(float target)
{
  raw_angle = AS5047P_get_rawdata_soon(&hspi1, GPIOB, GPIO_PIN_0);
  float transmit_angle = raw_angle * (2.0f * PI) / 16384.0f;
  angle = if_angle_real(transmit_angle);
  float electrical_angle = _electricalAngle(angle);
  pid_output = PID(&current_PID, -(target - DIR * getCurrent_fillter(electrical_angle)));
  FOC_clark_park(pid_output, electrical_angle);
}

void FOC_setCurrent_velocity_angle(float target)
{
  raw_angle = Sensor_AS5047P_Calculate_rotations(&hspi1, GPIOB, GPIO_PIN_0);
  float transmit_angle = raw_angle * (2.0f * PI) / 16384.0f;
  float electrical_angle = _electricalAngle(transmit_angle);
  angle = transmit_angle;
  FOC_clark_park(PID(&current_PID, (PID(&velocity_PID, (PID(&angle_PID, (target - DIR * Sensor_AS5047PgetTotalAngle())) - getVelocity_Fillter())) - DIR * getCurrent_fillter(raw_angle))), electrical_angle);
}

// 纯电流环，FOC_setCurrent里的(target-DIR*getCurrent_fillter())括号前要加负号
