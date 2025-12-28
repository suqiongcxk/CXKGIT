/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include  "stdio.h"
#include  "semphr.h"
#include  "tim.h"
#include  "RS485.h"
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
/* USER CODE BEGIN Variables */
//@@@@@@@@@@@@@@@@@优先级以及变量@@@@@@@@@@@@@@@@@@@@@@@   

float  slave_velocity_arr[4] = {0};


//RS485       优先级    osPriorityBelowNormal       task02
//zhuhanshu   优先级    osPriorityLow               dautask




/* USER CODE END Variables */
/* Definitions for zhuhanshu */
osThreadId_t zhuhanshuHandle;
const osThreadAttr_t zhuhanshu_attributes = {
  .name = "zhuhanshu",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for RS485 */
osThreadId_t RS485Handle;
const osThreadAttr_t RS485_attributes = {
  .name = "RS485",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for RS485_DMA */
osMessageQueueId_t RS485_DMAHandle;
const osMessageQueueAttr_t RS485_DMA_attributes = {
  .name = "RS485_DMA"
};
/* Definitions for Send_Again */
osMessageQueueId_t Send_AgainHandle;
const osMessageQueueAttr_t Send_Again_attributes = {
  .name = "Send_Again"
};
/* Definitions for RS485SE */
osSemaphoreId_t RS485SEHandle;
const osSemaphoreAttr_t RS485SE_attributes = {
  .name = "RS485SE"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of RS485SE */
  RS485SEHandle = osSemaphoreNew(1, 1, &RS485SE_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of RS485_DMA */
  RS485_DMAHandle = osMessageQueueNew (52, sizeof(uint8_t), &RS485_DMA_attributes);

  /* creation of Send_Again */
  Send_AgainHandle = osMessageQueueNew (8, sizeof(uint8_t), &Send_Again_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of zhuhanshu */
  zhuhanshuHandle = osThreadNew(StartDefaultTask, NULL, &zhuhanshu_attributes);

  /* creation of RS485 */
  RS485Handle = osThreadNew(StartTask02, NULL, &RS485_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
//		printf("2\n");
//		for( int i = 0; i <4; i++)
//		{
//		slave_velocity_arr[i] += 0.5;
//		}
//		printf("%d,%d,%d %d\n", Slave1_ACK_ERR_CONT,Slave2_ACK_ERR_CONT ,Slave3_ACK_ERR_CONT, Slave4_ACK_ERR_CONT); 
		Analyze_RS485_data();
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the RS485 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
	HAL_TIM_Base_Start_IT(&htim1);
  /* Infinite loop */
	float  AAA[1] = {0.90};
  for(;;)
  {
    if(xSemaphoreTake(RS485SEHandle, portMAX_DELAY) == pdPASS )  //没有信号量我就把自己挂起来，让其他函数执行
    {
			
			MASTER_SEND_DATA(Slave1_ID, AAA,1 );
//			 RS485_SET_MOTEOR_SPEED(target_velocity_1,
//			target_velocity_2,
//			target_velocity_3,
//			target_velocity_4);
//      printf("485\n");
    }
  }
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */





/* USER CODE END Application */

