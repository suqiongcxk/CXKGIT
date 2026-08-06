#include "remote_contol.h"
#include "CONTOL.h"
#include "JY901S.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "PID.h"
#include "RS485.h"
#include "math.h"
#include  "adc_m.h"
#include "stdio.h"
#include "math.h"






uint8_t sbus_buf[SBUS_FRAME_SIZE];
volatile uint8_t sbus_new_data = 0;
int16_t sbus_ch[16] = {0};  
char x = '\n';


#define   RE_LOW    4   //低挡位
#define   RE_mid    2		//中档位
#define   RE_hig    1   //高档位


int  Remote_V_Limit   = 0;
int  REMOTE_BASIC_V   = 0;
int  RE_turn_V   = 0;

int16_t sbus_ch_BAGIN[6] = {0};
int Calibrate_flag = 0;



//串口波特率跟停止位校验位要注意一下
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if ((huart == &huart5) &&
      (sbus_buf[0] == 0x0F) && 
      ((sbus_buf[24] == 0x00) || ((sbus_buf[24] & 0x0F) == 0x04))) {
    sbus_new_data = 1;
  }
  HAL_UART_Receive_IT(huart,sbus_buf, SBUS_FRAME_SIZE);
  
}



void get_SBUS_data(void)
{
	 if (sbus_new_data) {
      sbus_new_data = 0;
      
      sbus_ch[0]  = (sbus_buf[1]  | (sbus_buf[2]  << 8)) & 0x07FF;																							//右摇杆左到右  520-920-1320 
      sbus_ch[1]  = ((sbus_buf[2] >> 3) | (sbus_buf[3]  << 5)) & 0x07FF;          															//左摇杆前到后  1300-900-500
      sbus_ch[2]  = ((sbus_buf[3] >> 6) | (sbus_buf[4]  << 2) | (sbus_buf[5] << 10)) & 0x07FF;									//右摇杆前到后  1736-940-136
      sbus_ch[3]  = ((sbus_buf[5] >> 1) | (sbus_buf[6]  << 7)) & 0x07FF;          															//左摇杆左到右  600-1000-1400 
		  sbus_ch[4]  = ((sbus_buf[6] >> 4) | (sbus_buf[7]  << 4)) & 0x07FF;																				//左下到上 			1800-1000-200
      sbus_ch[5]  = ((sbus_buf[7] >> 7) | (sbus_buf[8]  << 1) | (sbus_buf[9] << 9)) & 0x07FF;										//右下到上 			1800-1000-200  
      }
//			printf( "%d ,%d ,%d ,%d ,%d ,%d\n",sbus_ch[0],sbus_ch[1],sbus_ch[2],sbus_ch[3],sbus_ch[4],sbus_ch[5]);
			if( Calibrate_flag <10)
			{
				for ( int i = 0; i< 6; i++ )
				{
					sbus_ch_BAGIN[i]+= sbus_ch[i];
				}
				Calibrate_flag++;
			}else 
			{
				if( Calibrate_flag == 10)
				{
						for ( int i = 0; i< 6; i++ )
					{
						sbus_ch_BAGIN[i]/= 10;
					}
					Calibrate_flag=100;
				}

			}
}




//遥控运动
void  REMOTE_MOVE( void  )
{
		switch (sbus_ch[5] )   //档位切换
		{
			case 1800:Remote_V_Limit = RE_LOW; break ;//最下
			case 1000:Remote_V_Limit = RE_mid; break ;//中间
			case 200 :Remote_V_Limit = RE_hig; break ;//最上
		}
		
		REMOTE_BASIC_V  =  (sbus_ch[2] - sbus_ch_BAGIN[2]) / Remote_V_Limit;
		REMOTE_BASIC_V  = __fabs(sbus_ch[2] - sbus_ch_BAGIN[2]) < 15 ? 0 : REMOTE_BASIC_V ;
		
		RE_turn_V = (sbus_ch[3] - sbus_ch_BAGIN[3] )* 0.666;
		RE_turn_V  = __fabs(sbus_ch[3] - sbus_ch_BAGIN[3]) < 15 ? 0 : RE_turn_V ;
		
		REMOTE_V_Set(REMOTE_BASIC_V*R_GAIN,RE_turn_V *R_GAIN);
}


//速度赋值
void  REMOTE_V_Set( int  REMOTE_BASIC_V, int  RE_turn_V )
{
	float RE_left = 0; 
	float RE_right = 0;
	if(REMOTE_BASIC_V>0 )
	{
	 RE_left = REMOTE_BASIC_V + RE_turn_V;
	 RE_right = REMOTE_BASIC_V - RE_turn_V;
	}else 
	{
	 RE_left = REMOTE_BASIC_V - RE_turn_V;
	 RE_right = REMOTE_BASIC_V + RE_turn_V;	
	}

	
	
	switch ( Remote_V_Limit)
		{
			case RE_LOW:RE_left =  My_constrain(RE_left,-0.2 *RPM_MAX,0.2*RPM_MAX)  ;
			case RE_mid:RE_left =  My_constrain(RE_left,-0.4*RPM_MAX,0.4*RPM_MAX)  ;
			case RE_hig:RE_left =  My_constrain(RE_left,-0.7*RPM_MAX,0.7*RPM_MAX)  ;
		}
		switch ( Remote_V_Limit)
	{
		case RE_LOW:RE_right =  My_constrain(RE_right,-0.2 *RPM_MAX,0.2*RPM_MAX)  ;
		case RE_mid:RE_right =  My_constrain(RE_right,-0.4*RPM_MAX,0.4*RPM_MAX)  ;
		case RE_hig:RE_right =  My_constrain(RE_right,-0.7*RPM_MAX,0.7*RPM_MAX)  ;
	}
		
		target_velocity_1[0] = RE_left ;
		target_velocity_2[0] = RE_right;
		target_velocity_3[0] = RE_right;
		target_velocity_4[0] = RE_left ;
	
		osDelay(150);
//		printf("move(%d,%d)\n",(int)RE_left,(int)RE_right);
}



