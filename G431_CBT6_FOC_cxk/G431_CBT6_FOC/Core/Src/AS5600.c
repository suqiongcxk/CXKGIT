#include "main.h"
#include "i2c.h"
#include "AS5600.h"
#include <math.h>

// AS5600.C的变量名相比于AS5047P.C里的变量都多了个1
// 因为我移植移植完代码懒得改了，嘻

float angle_now1 = 0.0, angle_prev1 = 0.0f, vel_angle_prev1 = 0.0f;
int full_rotations1 = 0, full_prev_rotations1 = 0;
uint32_t angle_now_ts1 = 0, angle_prev_ts1 = 0;

void AS5600_Write_Reg(uint16_t reg, uint8_t *value)
{
    HAL_I2C_Mem_Write(&hi2c1, AS5600_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, value, 1, 50);
}

void AS5600_Write_Regs(uint16_t reg, uint8_t *value, uint8_t len)
{
    HAL_I2C_Mem_Write(&hi2c1, AS5600_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, value, len, 50);
}

void AS5600_Read_Reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    HAL_I2C_Mem_Read(&hi2c1, AS5600_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, buf, len, 50);
}

// 从编码器获取原始数据
int16_t getAngle_Without_track(void)
{
    uint8_t temp[DATA_SIZE] = {0};
    AS5600_Read_Reg(Angle_Hight_Register_Addr, temp, DATA_SIZE);
    int16_t in_angle = ((int16_t)temp[0] << 8) | (temp[1]);
    return in_angle;
}

// 这个函数用于计算并更新圈数
int16_t Sensor_AS5600_Calculate_rotations(void)
{
    int16_t val = getAngle_Without_track();
    angle_now1 = val * (2.0f * PI) / 4096.0f;
    float d_angle = angle_now1 - angle_prev1;
    // 圈数检测
    if (fabs(d_angle) > (0.8f * 2.0f * PI))
        full_rotations1 += (d_angle > 0) ? -1 : 1;
    angle_prev1 = angle_now1;
    return val;
}

// 搭配上一个函数，可以算出总共转了多少角度，用于角度环闭环
float Sensor_AS5600getTotalAngle(void)
{
    float angle_cd = full_rotations1 * (2.0f * PI) + angle_prev1;
    return angle_cd;
}

float Sensor_AS5600getVelocity(void)
{
    angle_now_ts1 = micros();
    // 计算采样时间
    float Ts = (angle_now_ts1 - angle_prev_ts1) * 1e-6;
    // 快速修复奇怪的情况
    if (Ts <= 0)
        Ts = 1e-3f;
    // 速度溢出
    float vel = ((float)(full_rotations1 - full_prev_rotations1) * 2.0f * PI + (angle_prev1 - vel_angle_prev1)) / Ts;
    // 更新变量
    vel_angle_prev1 = angle_prev1;
    full_prev_rotations1 = full_rotations1;
    angle_prev_ts1 = angle_now_ts1;
    return vel;
}
