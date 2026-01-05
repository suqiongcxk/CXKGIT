#include "AS5047P.h"

float angle_now = 0.0, angle_prev = 0.0f, vel_angle_prev = 0.0f;
int full_rotations = 0, full_prev_rotations = 0;
uint32_t angle_now_ts = 0, angle_prev_ts = 0;
float lastangle;

// AS5047P初始化
void SPI_AS5047P_Init(AS5047P_SPI spi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin)
{
  HAL_GPIO_WritePin(cs_gpio_port, cs_pin, GPIO_PIN_SET);
  // 上电后至少等待10ms
  HAL_Delay(10);
}

// 偶校验函数（16位）
uint8_t check_even_bit(uint16_t data)
{
  uint32_t even_parity = 0;
  for (int i = 0; i < 16; i++)
  {
    even_parity += (data >> i) & 0x01;
  }
  return (even_parity & 0x1);
}

// SPI发送接收函数
uint16_t SPI_Send_Receive_Data(AS5047P_SPI spi, uint16_t senddata, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin)
{
  uint16_t rx_data = 0;
  HAL_GPIO_WritePin(cs_gpio_port, cs_pin, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(spi, (uint8_t *)&senddata, (uint8_t *)&rx_data, 1, HAL_MAX_DELAY);

  HAL_GPIO_WritePin(cs_gpio_port, cs_pin, GPIO_PIN_SET);

  return rx_data;
}

//  获取AS54047P数据
uint16_t AS5047P_get_rawdata(AS5047P_SPI spi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin)
{
  uint16_t even = 0;
  uint16_t data, data_e = 0;
  uint16_t addr = ANGLECOM_REGISTER;

  addr |= 0x4000; // 读写标志位置1（读）
  even = check_even_bit(addr);
  addr = addr | (even << 15); // 添加偶校验位

  data = SPI_Send_Receive_Data(spi, addr, cs_gpio_port, cs_pin);
  data_e = data & 0x7FFF; // 清除最高位（PARD位）

  // 重新计算偶校验
  even = check_even_bit(data_e);

  return (even == (data >> 15)) ? data_e : 0xFFFF;
}

//  角度转换函数
float AS5047P_rawdata_to_angle(uint16_t data)
{
  float angle = data;
  angle = angle * 2.0f * PI / 16384.0f;
  return angle;
}

//  获取AS5047P的邪修方法,极致的读取速度,但无法处理报错
uint16_t AS5047P_get_rawdata_soon(AS5047P_SPI spi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin)
{

  uint16_t data, data_e = 0;

  data = SPI_Send_Receive_Data(spi, 0xFFFF, cs_gpio_port, cs_pin);
  data_e = data & 0x3FFF; // 清除最高位（PARD位）

  return  data_e;
}

// 这个函数用于计算并更新圈数
int16_t Sensor_AS5047P_Calculate_rotations(AS5047P_SPI spi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin)
{
  int16_t val = AS5047P_get_rawdata_soon(spi, cs_gpio_port, cs_pin);
  angle_now = val * (2.0f * PI) / 16384.0f;
  float d_angle = angle_now - angle_prev;
  // 圈数检测
  if (fabs(d_angle) > (0.8f * 2.0f * PI))
    full_rotations += (d_angle > 0) ? -1 : 1;
  angle_prev = angle_now;
  return val;
}

// 搭配上一个函数，可以算出总共转了多少角度，用于角度环闭环
float Sensor_AS5047PgetTotalAngle(void)
{
  float angle_cd = full_rotations * (2.0f * PI) + angle_prev;
  return angle_cd;
}

float Sensor_AS5047PgetVelocity(void)
{
  angle_now_ts = micros();
  // 计算采样时间
  float Ts = (angle_now_ts - angle_prev_ts) * 1e-6;
  // 快速修复奇怪的情况
  if (Ts <= 0)
    Ts = 1e-3f;
  // 速度溢出
  float vel = ((float)(full_rotations - full_prev_rotations) * 2.0f * PI + (angle_prev - vel_angle_prev)) / Ts;
  // 更新变量
  vel_angle_prev = angle_prev;
  full_prev_rotations = full_rotations;
  angle_prev_ts = angle_now_ts;
  return vel;
}

//此函数用于滤去编码器的毛刺，编码器之所以数据会受到影响是因为SPI通信协议容易受到MOS开关信号的影响
float if_angle_real (float nowangle)
{
  float derta_angle = nowangle - lastangle;
  if (derta_angle  <= 0){
    derta_angle = -derta_angle;
  }
  if (derta_angle <= 0.3f || derta_angle >= 5.9831853f)
  {
    lastangle = nowangle;
    return nowangle;
  }else
  {
    return lastangle;
  }
}
