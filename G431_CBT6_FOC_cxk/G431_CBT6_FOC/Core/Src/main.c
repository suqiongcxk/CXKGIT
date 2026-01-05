/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "cordic.h"
#include "dma.h"
#include "fdcan.h"
#include "fmac.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FOC.h"
#include <stdlib.h>
#include "string.h"
#include "electricity.h"
#include "AS5600.h"
#include "User_define.h"
#include "DWT.h"
#include "string.h"
#include "AS5047P.h"
#include "RS485.h"
#include "lowpass_fillter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t uart_rx_dma_buffer[FLOAT_BUF_SIZE] = {0};
uint8_t uart_dma_buffer[FLOAT_BUF_SIZE] = {0};

float time, last_time;
float serial_targrt = 0.0f;
float serial_send_time = 0, serial_send_time_prev = 0;

volatile float adc_value[3] = {0, 0, 0};
float current_Now, current_Preview;
FOC_TxPacket send_data = {
    .foc_current = 0,
    .rx_target = 0,
    .adc_value1 = 0,
    .adc_value2 = 0,
    .now_angle = 0,
    .pass_time = 0,
    .tail = {0x00, 0x00, 0x80, 0x7f}};
FOC_PID data = {
    .foc_current = 0,
    .rx_target = 0,
    .now_angle = 0,
    .tail = {0x00, 0x00, 0x80, 0x7f}};
float v_now;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void if_serial_receive_data(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CORDIC_Init();
  MX_FDCAN1_Init();
  MX_FMAC_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_ADC2_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
//  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
//  HAL_ADCEx_InjectedStart(&hadc2);
//  HAL_TIM_Base_Start(&htim2);
//  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
//  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
//  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
//  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
//  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
//  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4); // 先把通道四先打开，才能触发ADC采样，ADC才能采到正确的初始值
//  Enable_DWT_CycleCounter();
//  calculate_variety();
//  electricity_init();
//  setMotor(motor_Pn, motor_DIR);
//  SPI_AS5047P_Init(&hspi1, GPIOB, GPIO_PIN_0); // AS5047P的CS引脚初始为高电平
//  RS485_RX_EN

	RS485_RX_EN
	HAL_TIM_Base_Start(&htim2);
	Enable_DWT_CycleCounter();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint32_t micros(void)
{
  return __HAL_TIM_GetCounter(&htim2);
}

// 该函数用于解析字符型数据
float fast_atof(const uint8_t *buffer, uint16_t len)
{
  // 单次遍历算法（无状态机）
  int integer = 0;
  int fraction = 0;
  int fraction_digits = 0;
  int is_negative = 0;
  int has_decimal = 0;

  // 使用寄存器变量加速
  register uint8_t *ptr = (uint8_t *)buffer;
  register uint16_t count = len;

  // 单次遍历处理所有字符
  while (count--)
  {
    register uint8_t c = *ptr++;

    if (c == '-')
      is_negative = 1;
    else if (c == '.')
      has_decimal = 1;
    else if (c >= '0' && c <= '9')
    {
      if (has_decimal)
      {
        fraction = fraction * 10 + (c - '0');
        fraction_digits++;
      }
      else
      {
        integer = integer * 10 + (c - '0');
      }
    }
  }

  // 预计算除法因子（避免循环内浮点除法）
  const float fraction_multipliers[7] = {
      0.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001};
  float multiplier = (fraction_digits > 6) ? fraction_multipliers[6] : fraction_multipliers[fraction_digits];

  // 3. 整数转浮点的汇编优化
  float result = (float)integer + (float)fraction * multiplier;
  return is_negative ? -result : result;
}
void if_serial_receive_data(void)
{
  if (rx_complete)
  {

    uint16_t len = rx_length;
    rx_complete = 0;

    // 解析数据
    serial_targrt = fast_atof((uint8_t *)uart_dma_buffer, len);
    // 1.5us结束解析
  }
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM8 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM8)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
