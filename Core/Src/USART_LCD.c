#include "usart_lcd.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "adc_m.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "contol.h"
#include "jy901s.h"
#include "remote_contol.h"
#include "GPIO.h"
char LCD_Frame [ 30] = {0};
int16_t   LCD_HEX  = 0;
int16_t   LCD_MODE  = ADC_mode; //上电默认ADC调试模式
extern JY901S_AngleData IMU_DATA;



uint8_t huishou_FLAG = 1 ;
  
void Analyze_LCD ( void )
{
		if( USART2_flag) 
		{
						if(USART2_dma_buffer[0] == 0x00  ||  USART2_dma_buffer[0] == 0xFF )   //是数字帧
				{
					
					 LCD_HEX = (USART2_dma_buffer[2]<<8) | USART2_dma_buffer[3];
					
				}else if( USART2_dma_buffer[0] == 'R' && USART2_dma_buffer[1] == 'e'&& USART2_dma_buffer[2] == 's')
				{
					LCD_MODE = Normal_mode;
					LCD_HEX= 0;
				}
				else if(USART2_dma_buffer[0] == 'A' && USART2_dma_buffer[1] == 'D')
				{
					LCD_MODE = ADC_mode;
					LCD_HEX=0;
				}
				else if(USART2_dma_buffer[0] == 'R' && USART2_dma_buffer[1] == 'e'&& USART2_dma_buffer[2] == 'm')
				{
					LCD_MODE = Remote_mode;
					LCD_HEX=0;
				}
					else if(USART2_dma_buffer[0] == 'j' && USART2_dma_buffer[1] == 'i'&& USART2_dma_buffer[2] == 'a')
				{
					LCD_MODE  =  Jiasudu_mode  ; 
					LCD_HEX=0;
				}
				else if(USART2_dma_buffer[0] == 'C' && USART2_dma_buffer[1] == 'o')
				{
					if( USART2_dma_buffer[7] ==  '1')
					{
						LCD_MODE = Control1_mode;
						LCD_HEX=0;
					}
					else
					{
						LCD_MODE = turn_mode;
						LCD_HEX=0;
					}
					
				}
		
		
		}


		
		USART2_flag = 0;
}


void LCD_interact ( void )
{
	Analyze_LCD();
	switch( LCD_MODE ) 
	{
		case Control1_mode :  move_for_LCD( LCD_HEX,LCD_HEX ); break;
		case turn_mode     : if( LCD_HEX != 0)
												{
													TURN_ZIZHUAN( LCD_HEX );
													LCD_HEX = 0;
												}
												else 
												{
													printf( "wset page3.pointer13.angle %d\r\n" ,(int ) IMU_DATA.yaw); 
													STOP();
													osDelay(5);
												}break;
		
		 
		
		case Normal_mode   : 
												if(huishou_FLAG == 1)
												{
													move(0,0);  
													if(L_JG <= JG_RECOVERY_TRIGGER && R_JG <= JG_RECOVERY_TRIGGER){huishou_FLAG = 2;}
										 		}else if(huishou_FLAG == 2)
												{
													move(-400,-400);
													osDelay(200);
													move(-350,-450);  
													osDelay(1150);   
													TURN_ZIZHUAN(130);  
//													STOP();
//													osDelay(2000);
													Enemy_search_mode = zizhuan_mode;  
													xuanzhuan_count = XUANZHUAN_VALUE;
													huishou_FLAG = 3;
												}else if (huishou_FLAG ==3)
												{
												if(CAR_STATUS == ON_STAGE)
												{ 
												  taishangroaming();													
												}else if (CAR_STATUS == OFF_STAGE)
												{
													taixia(); 
												}else if(CAR_STATUS == BETWEEN_STAGE) 
												{
													if( IMU_DATA.roll > 0)
													{
														move(-300,-300);
														osDelay(400);
														move(0,0);
														osDelay(200);
														
													}else 
													{
														move(300,300);
														osDelay(400);
														move(0,0);
														osDelay(200);
													}
													CAR_STATUS = OFF_STAGE; 
												}
												} 
							 				





												break;
											 
												
												
												
												
		case ADC_mode      :  STOP();PRINT_ADC();   break;
		case Remote_mode   :  if( Calibrate_flag == 100) {REMOTE_MOVE(); }break;										
		case Jiasudu_mode  : 	move_for_LCD(LCD_HEX*R_GAIN,LCD_HEX*R_GAIN); osDelay(1000);LCD_HEX = 0;
	}
}





//打印ADC数据
void PRINT_ADC  ( void )
{
	
	
	/*
	FL		FR    X
	L3		R3		T
	L6		R6
	L9		R9
	L12		R12
	BL		EL  
	BR		ER
	FH		BH
	*/
	
	printf("wset page1.num%d.val %d\r\n",0,QIAN_L_JG);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",2,L_JG_30);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",3,L_JG_60);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",4,L_JG);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",5,B_L_Hui);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",6,B_JG_LEFT);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",7,B_JG_RIGHT);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",8,(L_huidu+ R_huidu )/ 2 ); //
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",9,QIAN_R_JG);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",10,R_JG_30);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",11,R_JG_60);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",12,R_JG);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",13,B_R_HUI);
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",14,EDGE_JG_L); //EDGE_JG_L
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",15,EDGE_JG_R);//EDGE_JG_R
	osDelay(10); 
	printf("wset page1.num%d.val %d\r\n",16,(B_L_Hui + B_R_HUI) /2 );
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",17,X_LOCATION); 
	osDelay(10);
	printf("wset page1.num%d.val %d\r\n",18,SSQ_TYPE ); //(int)(IMU_DATA.roll *100)
	osDelay(10);

	
	
}




//把LCD变成红温
void LCD_red (void)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);
	osDelay(1000);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);	
//		printf("wset page1.num1.val 1\r\n");
}



