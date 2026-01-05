#ifndef __AS5047P_H__
#define __AS5047P_H__

#include "stm32g4xx_hal.h"
#include "spi.h"
#include <math.h>

#define ANGLEUNC_REGISTER 0x3FFE // 没有动态角度误差补偿的角度信息
#define ANGLECOM_REGISTER 0x3FFF // 具有动态角度误差补偿的角度信息

#define PI 3.14159265358979f

extern float lastangle;
typedef SPI_HandleTypeDef *AS5047P_SPI;

void SPI_AS5047P_Init(AS5047P_SPI spi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin);
uint16_t AS5047P_get_rawdata(AS5047P_SPI spi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin);
float AS5047P_rawdata_to_angle(uint16_t data);
uint16_t AS5047P_get_rawdata_soon(AS5047P_SPI spi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin);
int16_t Sensor_AS5047P_Calculate_rotations(AS5047P_SPI spi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_pin);
float Sensor_AS5047PgetTotalAngle(void);
float Sensor_AS5047PgetVelocity(void);
float if_angle_real(float nowangle);

#endif
