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
#include  "MYDWT.h"
#include  "JY901S.h"
#include  "i2c.h"
#include  "CONTOL.h"
#include  "adc_m.h"
#include  "PID.h"
#include  "usart.h"
#include "usart_lcd.h"
#include "remote_contol.h"
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
float  TEXT_VELOCITY[1] = {20};
JY901S_AngleData IMU_DATA;
JY901S_Status    IMU_IIC_SATUS;
volatile int16_t Rotation = 0;//圈数
float   Last_Angle = 180;
uint16_t OFF_STAGE_CNT = 0;
uint16_t between_STAGE_COUNT = 0;
//RS485       	优先级    osPriorityBelowNormal       task02
//JY901   			优先级    osPriorityLow2              dautask
//ADC_UPDATE    优先级    osPriorityLow1        			StartTask03
//contol        优先级    osPriorityLow       				StartTask04



//自身函数定义
void TEXT_VELOCITY_ADD( void );
void TEXT_MOVE(void);
void RS485_TXVELOCITY(void);
void vPrintTaskInfo(uint8_t *pucName);
void RS485_TXCommand(void);
void RS485_TXDistance(void);
//宏定义
#define    ON     						1
#define    OFF    						0
int        TEXT_MODE=   			OFF;      //上车的时候给他关掉就好
int        JY901S_MODE=       OFF;      //陀螺仪部分


/* USER CODE END Variables */
/* Definitions for JY901 */
osThreadId_t JY901Handle;
const osThreadAttr_t JY901_attributes = {
  .name = "JY901",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
/* Definitions for RS485 */
osThreadId_t RS485Handle;
const osThreadAttr_t RS485_attributes = {
  .name = "RS485",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for ADC_UPDATE */
osThreadId_t ADC_UPDATEHandle;
const osThreadAttr_t ADC_UPDATE_attributes = {
  .name = "ADC_UPDATE",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow1,
};
/* Definitions for contol */
osThreadId_t contolHandle;
const osThreadAttr_t contol_attributes = {
  .name = "contol",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
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

void vJY901Task(void *argument);
void vRS485Task(void *argument);
void vADC_UPDATETask(void *argument);
void vcontolTask(void *argument);

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
  /* creation of JY901 */
  JY901Handle = osThreadNew(vJY901Task, NULL, &JY901_attributes);

  /* creation of RS485 */
  RS485Handle = osThreadNew(vRS485Task, NULL, &RS485_attributes);

  /* creation of ADC_UPDATE */
  ADC_UPDATEHandle = osThreadNew(vADC_UPDATETask, NULL, &ADC_UPDATE_attributes);

  /* creation of contol */
  contolHandle = osThreadNew(vcontolTask, NULL, &contol_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_vJY901Task */
/**
  * @brief  Function implementing the JY901 thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_vJY901Task */
void vJY901Task(void *argument)
{
  /* USER CODE BEGIN vJY901Task */
  /* Infinite loop */
  for(;;)
  {
		
	#if OFF  //任务监视窗口函数
	vPrintTaskInfo((uint8_t *)"JY901" );
	#endif
		
		
		IMU_IIC_SATUS = JY901S_ReadAngles(&hi2c3,&IMU_DATA);
//		if(IMU_IIC_SATUS  ==  JY901S_OK  &&   JY901S_MODE )
//		{	
//			float v = 0;
//			v = LV1_PID_BACK(&IMU_PID,IMU_DATA.yaw,180);
//			move( v/3,-v/3);
//			PrintAngles(&IMU_DATA);  //不是陀螺仪模式的时候就不打印了
//		}
		GET_VISUAL_DATA();
		Rotation +=  __fabs(IMU_DATA.yaw - Last_Angle ) > 280 ? (IMU_DATA.yaw > Last_Angle ? -1 : 1 ) :  0; 
		Last_Angle = IMU_DATA.yaw;
		
		if( LCD_MODE == Remote_mode ){ get_SBUS_data(); }
		

//			for (int i = 2;i <20 ;i++)
//			{
//				printf("%c",USART3_dma_buffer[i]);
//			}
//			printf("\n");
//			USART3_flag = 0;
  
		//本次跟上次相差300度代表多了一圈
		osDelay(6);
  }
  /* USER CODE END vJY901Task */
}

/* USER CODE BEGIN Header_vRS485Task */
/**
* @brief Function implementing the RS485 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vRS485Task */
void vRS485Task(void *argument)
{
  /* USER CODE BEGIN vRS485Task */
  /* Infinite loop */
  for(;;)
  {
    if(xSemaphoreTake(RS485SEHandle, portMAX_DELAY) == pdPASS )  //没有信号量我就把自己挂起来，让其他函数执行
    {
			
			if(TEXT_MODE)
				
				{
					TEXT_MOVE();
				}
			else
				{
						
					if(JY901S_MODE)
						{
							osDelay(1);
						}
						else 
						{
              switch (current_Send_frame_type)
              {
                case DATA_Frame:
                  RS485_TXVELOCITY(); //速度数据
                  break;
                case Distance_Frame:
                  RS485_TXDistance(); //位移数据
                  break;
                case Command_Frame:
                  RS485_TXCommand(); //指令数据
                  break;
              } 

						}
				
				}
    }
  }
  /* USER CODE END vRS485Task */
}

/* USER CODE BEGIN Header_vADC_UPDATETask */
/**
* @brief Function implementing the ADC_UPDATE thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vADC_UPDATETask */
void vADC_UPDATETask(void *argument)
{
  /* USER CODE BEGIN vADC_UPDATETask */
  /* Infinite loop */
  for(;;)
  {
	#if OFF  //任务监视窗口函数
	vPrintTaskInfo((uint8_t *)"ADC_UPDATE" );
	#endif
		
		
		ADC_Process();                   //ADC采样     0，7灰度 
		float aver_huidu = (L_huidu + R_huidu )/ 2.0f;
		
		

		//700等于252cm，
		roaming_veocity_STR = aver_huidu > 1900 ? 400 : aver_huidu/9.0f;//
		
		
		
		if( aver_huidu < 1800) { 		
			Enemy_search_mode = zizhuan_mode;   
		xuanzhuan_count = 1500;  }
		uint16_t R_value = 50; 
		uint16_t aver_huidu_value = 2900;
		uint16_t R_huidu_CALL = R_huidu *1.333f ;
		if  ( L_huidu <  R_huidu_CALL )
		{
				roaming_veocity_L = aver_huidu > aver_huidu_value ? 550 : (aver_huidu /6.5f); 
				roaming_R    = aver_huidu> aver_huidu_value ? R_value : (aver_huidu /110.0f);
				if(roaming_R < 10) roaming_R = 10;
				roaming_veocity_R = (roaming_veocity_L *roaming_R )/ (roaming_R+22.0f);	
		}else 
		{
				roaming_veocity_R = aver_huidu > aver_huidu_value ? 550 : (aver_huidu /7.0f); 
				roaming_R    = aver_huidu> aver_huidu_value ? R_value : (aver_huidu /110.0f);
				if(roaming_R < 10) roaming_R = 10;
				roaming_veocity_L = (roaming_veocity_R *roaming_R )/ (roaming_R+22.0f);	 
		}
		
		
		

		//假上台限速：路过出发区时降低直行速度防止撞墙
		if (CAR_STATUS == ON_STAGE && STAGE_UP_CONFIRMED == 0)
		{
			if (roaming_veocity_STR > 150) roaming_veocity_STR = 150;
		}
		if(1)  //记得改回1
			{
				//记得补偿左右灰度的差值
				if( (L_huidu < 650 &&  L_huidu > 300 ) && (R_huidu   < 700 &&  R_huidu  > 300 ) ) 
					{
				
						
						
						if(OFF_STAGE_CNT >= 100) //150*5 = 750ms后判断为台下
						{
							CAR_STATUS = OFF_STAGE ;
						}else 
						{
							OFF_STAGE_CNT ++;	
						}
					
					} //小于六百是台下
				else if (L_huidu >= 600  &&  R_huidu >= 700)
				{
					
						if( QIAN_L_JG <= 180 &&
							  QIAN_R_JG <= 180 &&  
							( L_JG <= 1000  ||  R_JG <=1000 )    &&
								L_JG_60    <= 600                  &&
								R_JG_60    <= 600                  
						  )
						{
								CAR_STATUS = OFF_STAGE; 
								if ( B_JG >= 700 )
								{
									START_ZONE_flag = 1; //可以直接上台
								}else
								{
									START_ZONE_flag = 2; //后退转90度后退800ms
								}
						}else 
						{
							CAR_STATUS = ON_STAGE ;
							OFF_STAGE_CNT = 0;
							
						}
					
						//左边黑色右边出发区
				}else   if(   (L_huidu < 650 &&  L_huidu > 300 )  
									 && (R_huidu >= 700)  
									)
				{
					
						if( QIAN_L_JG <= 300 &&
							  QIAN_R_JG <= 300 &&  
							( L_JG <= 1000  ||  R_JG <=1000 )    &&
								L_JG_60    <= 650                  &&
								R_JG_60    <= 650                  
						  )
						{
								CAR_STATUS = OFF_STAGE; 
							
								if ( B_JG >= 700 )
								{ 
									START_ZONE_flag = 1; //可以直接上台
								}else
								{
									START_ZONE_flag = 2; //后退转90度后退800ms
								} 
						}
						//右边黑色左边出发区
				}else   if(  (R_huidu   < 700 &&  R_huidu  > 300 ) 
									 &&(  L_huidu >= 600 )  
									)
				{
						if( QIAN_L_JG <= 180 &&
							  QIAN_R_JG <= 180 &&  
							( L_JG <= 1000  ||  R_JG <=1000 )    &&
								L_JG_60    <= 600                  &&
								R_JG_60    <= 600                  
						  )
						{  
								CAR_STATUS = OFF_STAGE; 
									START_ZONE_flag = 2; //后退转90度后退800ms  
						}
				}
				
				
				
			}
		
		if(ZHUIJI_Flag != 0 )
		{
			if(ZHUIJI_TIME > 0)
			{
			ZHUIJI_TIME -= 5;
			}else 
			{
				ZHUIJI_TIME =1500;
				ZHUIJI_Flag = 0;
			}
		}
		if( __fabs(IMU_DATA.roll ) >=15)
		{
				between_STAGE_COUNT++;
			  if(  between_STAGE_COUNT == 400 )
				{
					CAR_STATUS = BETWEEN_STAGE;
					between_STAGE_COUNT=0;
				}
		}else if(__fabs(IMU_DATA.roll ) <= 5  )
		{
					between_STAGE_COUNT=0;
		}
		//背后转向
		TURN_BACK_COUNT-=5;
		if(TURN_BACK_COUNT > 0)
		{
			TURN_BACK_ENABLE = 0;
		}else
		{     
			TURN_BACK_ENABLE= 1;
		}
		//旋转漫游的计时
		xuanzhuan_count -= 5;
		if(xuanzhuan_count > 0)
		{
			Enemy_search_mode = zizhuan_mode;
		}else
		{        
			Enemy_search_mode= pianzhuan_mode;
		}
		
		if(Enemy_search_scilence_CNT >= 0)
		{
			Enemy_search_scilence_CNT -= 5;
		}

			//上台状态确认：跟踪从台下(含过渡态)到台上的过渡
			static int prev_CAR_STATUS = ON_STAGE;
			if ((prev_CAR_STATUS == OFF_STAGE || prev_CAR_STATUS == BETWEEN_STAGE)
			    && CAR_STATUS == ON_STAGE && backing_attempt == 1)
			{
				STAGE_UP_CONFIRMED = 1;  //通过后退程序上台，确认真上台
			}
			if (prev_CAR_STATUS != OFF_STAGE && CAR_STATUS == OFF_STAGE)
			{
				STAGE_UP_CONFIRMED = 0;
				backing_attempt = 0;
			}
			prev_CAR_STATUS = CAR_STATUS;
		osDelay(5);
  }  
  /* USER CODE END vADC_UPDATETask */
}

/* USER CODE BEGIN Header_vcontolTask */
/**
* @brief Function implementing the contol thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vcontolTask */
void vcontolTask(void *argument)
{
  /* USER CODE BEGIN vcontolTask */
  /* Infinite loop */
  for(;;)
  {
		
	#if OFF  //任务监视窗口函数
	vPrintTaskInfo((uint8_t *)"contol" );
	#endif
		
				if(TEXT_MODE)
			{
				
					TEXT_VELOCITY_ADD();
				
			}
		else
			
			{
			    LCD_interact();
			}
  }
  /* USER CODE END vcontolTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


void TEXT_VELOCITY_ADD( void )
{
		for (int i =0; i<200; i++)
		{
		TEXT_VELOCITY[0] = i;
    osDelay(25);		
		}
		for (int i =200; i>0; i--)
		{
		TEXT_VELOCITY[0] = i;
    osDelay(25);
		}
		
		for (int i =0; i>-200; i--)
		{
		TEXT_VELOCITY[0] = i;
    osDelay(25);		
		}
		for (int i =-200; i<0; i++)
		{
		TEXT_VELOCITY[0] = i;
    osDelay(25);
		}
		
}


//调用这个的时候一定要把速度自增函数的开关打开

void TEXT_MOVE(void)
{
	  start_cycle = DWT->CYCCNT;
//		MASTER_SEND_Frame(Slave1_ID, TEXT_VELOCITY,1 ,DATA_Frame); 
//		Analyze_RS485_data();	

		
//		MASTER_SEND_Frame(Slave2_ID, TEXT_VELOCITY,1 ,DATA_Frame); 
//		Analyze_RS485_data();		

//		
//		MASTER_SEND_Frame(Slave3_ID, TEXT_VELOCITY,1 ,DATA_Frame); 
//		Analyze_RS485_data();			


//		MASTER_SEND_Frame(Slave4_ID, TEXT_VELOCITY,1 ,DATA_Frame); 
//		Analyze_RS485_data();	
	  end_cycle = DWT->CYCCNT;
//	  printf("%d\n", (end_cycle - start_cycle)/1000);
}
     
//发送速度帧
void RS485_TXVELOCITY(void)
{
	
		MASTER_SEND_Frame(Slave1_ID, target_velocity_1,1 ,DATA_Frame); 		

		
		MASTER_SEND_Frame(Slave2_ID, target_velocity_2,1 ,DATA_Frame); 

		 
		MASTER_SEND_Frame(Slave3_ID, target_velocity_3,1 ,DATA_Frame); 


		MASTER_SEND_Frame(Slave4_ID, target_velocity_4,1 ,DATA_Frame); 

}


//发送位移帧
void RS485_TXDistance(void)
{
	  start_cycle = DWT->CYCCNT;
    float  target_distance_1[1] = { RS485_Distance_Frame_Data.CM};
    float  target_distance_2[1] = { RS485_Distance_Frame_Data.CM};
    float  target_distance_3[1] = { RS485_Distance_Frame_Data.CM};
    float  target_distance_4[1] = { RS485_Distance_Frame_Data.CM};


    MASTER_SEND_Frame(Slave1_ID, target_distance_1,1 ,Distance_Frame); 
		Analyze_RS485_data();			
		delay_us(10);

		MASTER_SEND_Frame(Slave2_ID, target_distance_2,1 ,Distance_Frame); 
		Analyze_RS485_data();		  
		delay_us(10);

		MASTER_SEND_Frame(Slave3_ID, target_distance_3,1 ,Distance_Frame); 
		Analyze_RS485_data();			
		delay_us(10);

		MASTER_SEND_Frame(Slave4_ID, target_distance_4,1 ,Distance_Frame); 
		Analyze_RS485_data();    
		delay_us(10);		
	  end_cycle = DWT->CYCCNT;
//	  printf("%d\n", (end_cycle - start_cycle)/1000);
}

//发送命令帧
void RS485_TXCommand(void)
{
		start_cycle = DWT->CYCCNT;
	
    float   target_command[1] = {0};
    MASTER_SEND_Frame(Slave1_ID, target_command,0 ,Command_Frame); 
		Analyze_RS485_data();	
		delay_us(10);

		MASTER_SEND_Frame(Slave2_ID, target_command,0 ,Command_Frame); 
		Analyze_RS485_data();		  
		delay_us(10);

		MASTER_SEND_Frame(Slave3_ID, target_command,0 ,Command_Frame); 
		Analyze_RS485_data();			
		delay_us(10);
		MASTER_SEND_Frame(Slave4_ID, target_command,0 ,Command_Frame); 
		Analyze_RS485_data();    
		delay_us(10);
	  end_cycle = DWT->CYCCNT;

//	  printf("%d\n", (end_cycle - start_cycle)/1000);		
}



//打印函数状态信息
void vPrintTaskInfo(uint8_t *pucName)
{
    TaskHandle_t TaskHandle;
    TaskStatus_t TaskStatus;

    uint8_t aucTaskName[16];

    TaskHandle = xTaskGetHandle((const char *)pucName);

    vTaskGetInfo((TaskHandle_t)TaskHandle,       // 任务句柄
                 (TaskStatus_t *)&TaskStatus,     // 任务信息
                 (BaseType_t)pdTRUE,              // 允许统计任务堆栈历史最小剩余大小
                 (eTaskState)eInvalid);          // 函数自己获取任务运行状态

    printf("任务名称                : %s\r\n",TaskStatus.pcTaskName);
    printf("任务编号                : %d\r\n",(int)TaskStatus.xTaskNumber);
    printf("任务状态                : %d\r\n",TaskStatus.eCurrentState);
    printf("任务当前优先级          : %d\r\n",(int)TaskStatus.uxCurrentPriority);
    printf("任务基优先级            : %d\r\n",(int)TaskStatus.uxBasePriority);
    printf("任务栈基地址            : %#x\r\n",(int)TaskStatus.pxStackBase);
    printf("任务栈历史最高水位与栈顶距离 : %d\r\n",TaskStatus.usStackHighWaterMark);
    printf("\r\n");
}

/* USER CODE END Application */

