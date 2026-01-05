/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

typedef struct
  {
    float foc_current;
    float rx_target;
    float adc_value1;
    float adc_value2;
    float now_angle;
    float pass_time;
    uint8_t tail[4];
  } FOC_TxPacket;
typedef struct
  {
    float foc_current;
    float rx_target;
		float now_angle;
    uint8_t tail[4];
  } FOC_PID;
extern float current_Now,current_Preview;
  extern volatile float adc_value[3];
#define FLOAT_BUF_SIZE 16 
  extern uint8_t uart_rx_dma_buffer[FLOAT_BUF_SIZE];
  extern uint8_t uart_dma_buffer[FLOAT_BUF_SIZE];
  extern float time, last_time;
extern float serial_targrt ;
extern float serial_send_time , serial_send_time_prev ;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
  uint32_t micros(void);
	void if_serial_receive_data(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
