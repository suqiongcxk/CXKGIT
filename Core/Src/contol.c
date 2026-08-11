#include "CONTOL.h"
#include "JY901S.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "PID.h"
#include "RS485.h"
#include "math.h"
#include  "adc_m.h"
#include "stdio.h"
#include "remote_contol.h"
#include  "JY901S.h"
#include "arena_config.h"
  /*git 脚本
  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
  .\git-push.ps1    //每天都要上传
  */
extern JY901S_AngleData IMU_DATA;
AngleWithRotation   TURN_ANGLE;
Distance_Frame_Data RS485_Distance_Frame_Data;
float turn_velocity   = 0;
float roaming_veocity_L = 0;  //台上转圈速度
float roaming_veocity_R = 0;  //台上转圈速度
float roaming_veocity_STR = 0;  //台上漫游速度
float roaming_R = 0;
volatile int CAR_STATUS    =  ON_STAGE; //ON_STAGE 
volatile int Continuous_Occlusion_Count = 0; 
float  Convert_radians_to_centimeters   = 0;   //转速转成cm/s
float  R_GAIN    =  0;           //减速比增益    
int edge = 0;
int8_t sensor_slience_falg = 0;
uint32_t sensor_slience_star = 0;
uint32_t sensor_slience_end = 0;
float turn_angle = 0;  
int16_t Enemy_search_scilence_CNT = 0;
uint8_t  zizhuan_mode    = 0x05;
uint8_t  pianzhuan_mode  = 0xA0;
volatile uint8_t  Enemy_search_mode  = 0x05;
volatile int stage_check_flag = 1;
//自转权重 
#define  weight_30     30 
#define  weight_60     65  
#define  weight_90     90 
#define  weight_120    90 
#define roximal_Alignment_JIGUANG_GAIN   4
#define Bomb_CUBE   2 //炸弹块
uint8_t ZHUIJI_Flag  = 0;
uint16_t ZHUIJI_TIME  =  1000;  //追击时间1000ms
int START_ZONE_flag = 0;
volatile int TURN_BACK_ENABLE = 0;
int TURN_BACK_COUNT  = 0;
uint8_t STAGE_UP_CONFIRMED = 1;  //初始假设在台上，掉下后清除
uint8_t backing_attempt = 0;
//检测
volatile int xuanzhuan_count = 2000; //每次触发追击就把它重置
/* 灰度/边缘阈值已迁移至 arena_config.h */
void  edge_check ( void )
{
	edge = 0;
	if (QIAN_L_JG >= JG_EDGE_DETECT_ENTRY || QIAN_R_JG >= JG_EDGE_DETECT_ENTRY)
	{ 
			if( L_huidu > ARENA_GRAY_L_HUIDU_VALUE && R_huidu * ARENA_GRAY_HUIDU_GAIN > ARENA_GRAY_R_HUIDU_VALUE &&	ZHUIJI_Flag== 0 )//不在角落并且没有触发追击的话的时候用激光
			{
			
				if( (EDGE_JG_L >= EDGE_JG_L_VALUE  )|| (EDGE_JG_R >= EDGE_JG_R_VALUE ) )
				{

					edge = 1;  //个位赋值为1		
				}
			}else //在角落的或者触发追击时候用灰度，灰度小于200的话算边缘
			{
				if( L_huidu <= ARENA_GRAY_EDGE_DARK ||  R_huidu <= ARENA_GRAY_EDGE_DARK)
				{
					
					edge = 2;  //个位赋值为2  
				}	
			}
	}else  //前方全被物块遮挡了
	{
		if( L_huidu <= ARENA_GRAY_EDGE_DARK ||  R_huidu <= ARENA_GRAY_EDGE_DARK)
		{
			
					edge = 3;  //个位赋值为3
		
		}
	}
	if(edge != 0) //有边缘检测到了
	{
		if(SSQ_TYPE == 5 || SSQ_TYPE == 0)
		{
					if( QIAN_L_JG <= JG_BOTH_BLOCKED  && QIAN_R_JG <= JG_BOTH_BLOCKED  )
					{
							edge += 40;  //前面两个都被遮挡了，十位赋值位4
							
					}else if (QIAN_L_JG <= JG_BOTH_BLOCKED  && QIAN_R_JG >= 650 )
					{
						edge += 50;//左边被遮挡了，十位赋值位5
					
					}else if(  QIAN_L_JG >= JG_BOTH_BLOCKED  && QIAN_R_JG <= JG_BOTH_BLOCKED ) 
					{
						edge  += 60;//右边被遮挡了，十位赋值位6
					}
		}else if(SSQ_TYPE == 4 && X_LOCATION != 0)
		{
					//敌方战车在台下：用摄像头X坐标判断堵台位置
					if(X_LOCATION < 290)
					{
						edge += 50;  //敌人在左侧
					}else if(X_LOCATION > 350)
					{
						edge += 60;  //敌人在右侧
					}else
					{
						edge += 40;  //敌人在中间
					}
		}
		//SSQ_TYPE == 2(炸弹) 或 == 3(方块)：不触发遮挡，不堵

	}

}



void DOUBLE_MOVE( void )
{
		edge_check();
//		switch( edge )
//	{
//		 case 1 : move(-600,-600);osDelay(150);break;
//		 case 2 : move(-600,-600);osDelay(150);break;	
//		 case 3 : move(-600,-600);osDelay(150);break;	
//	}
	
}


//运动
void  edge_MOVE(void )
{
	uint16_t F_AVE = 0;
	uint16_t B_AVE = 0;
	F_AVE = L_huidu + R_huidu; 
	B_AVE = B_L_Hui + B_R_HUI;
	if(L_huidu < ARENA_GRAY_EDGE_DARK ) {F_AVE = 2* R_huidu ;}
	if(R_huidu < ARENA_GRAY_EDGE_DARK ) {F_AVE = 2* L_huidu ;}
	
	if(ZHUIJI_Flag != 0)//进入追击模式了，只有灰度探测到才会边缘检测
	{
			if(edge%10 != 2 && edge%10 != 3 )
			{
			return;
			}
	}
	
	if( (F_AVE  > B_AVE   ) &&  (F_AVE/2 < ARENA_GRAY_FRONT_BACK_MID  ))//前面颜色浅，后面颜色深
	{
	switch( edge /10 )
	{

		
		case 4 : move(-100,-100);osDelay(200);DOUBLE_MOVE();STOP() ;osDelay(100);ZHUIJI_Flag =  ALL_DECT; 
		Enemy_search_mode = zizhuan_mode;   
		xuanzhuan_count = EDGE_VAlue;      
		break;
		case 5 : move(-100,-100);osDelay(200);DOUBLE_MOVE();STOP() ;osDelay(100);ZHUIJI_Flag =  ONLY_L_DECT; 
		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = EDGE_VAlue;
		break;
		case 6 : move(-100,-100);osDelay(200);DOUBLE_MOVE();STOP() ;osDelay(100);ZHUIJI_Flag =  ONLY_R_DECT; 
		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = EDGE_VAlue;
		break;
		case 0:switch(edge%10)
		{
				case 1 : move(-150,-150);osDelay(250);DOUBLE_MOVE();STOP() ;osDelay(100);TURN_ZIZHUAN(60);//250
				Enemy_search_mode = zizhuan_mode;  
				xuanzhuan_count = EDGE_VAlue;
				break;
		
				case 2 : move(-100,-100);osDelay(250);DOUBLE_MOVE();STOP() ;osDelay(100);TURN_ZIZHUAN(120);STOP();osDelay(150);
				Enemy_search_mode = zizhuan_mode;  
				xuanzhuan_count = EDGE_VAlue;
				break;	
		
				case 3 : move(-100,-100);osDelay(250);DOUBLE_MOVE();STOP() ;osDelay(100);TURN_ZIZHUAN(60);
				Enemy_search_mode = zizhuan_mode;  
				xuanzhuan_count = EDGE_VAlue;
				break;	
		}
		
		break;	
		
	}	
		
	
	}else
{	
	
	switch( edge /10 )
	{
		
		case 4 : move(-300,-300);osDelay(200);DOUBLE_MOVE();STOP() ;osDelay(100);ZHUIJI_Flag =  ALL_DECT; 
		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = EDGE_VAlue;
		break;
		
		case 5 : move(-200,-100);osDelay(300);DOUBLE_MOVE();STOP() ;osDelay(100);ZHUIJI_Flag =  ONLY_L_DECT; 
		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = EDGE_VAlue;
		break; 
		
		case 6 : move(-100,-200);osDelay(300);DOUBLE_MOVE();STOP() ;osDelay(100);ZHUIJI_Flag =  ONLY_R_DECT; 
		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = EDGE_VAlue;
		break;		 
		
		case 0:switch(edge%10){
				case 1 : move(-250,-250);osDelay(350);DOUBLE_MOVE();STOP() ;osDelay(100);TURN_ZIZHUAN(60);
				Enemy_search_mode = zizhuan_mode;  
				xuanzhuan_count = EDGE_VAlue;
				break;
		
				case 2 : move(-300,-300);osDelay(350);DOUBLE_MOVE();STOP() ;osDelay(100);TURN_ZIZHUAN(170);STOP();osDelay(150);
				Enemy_search_mode = zizhuan_mode;  
				xuanzhuan_count = EDGE_VAlue;
				break;	
				
				case 3 : move(-250,-250);osDelay(350);DOUBLE_MOVE();STOP() ;osDelay(100);TURN_ZIZHUAN(60);
				Enemy_search_mode = zizhuan_mode;   
				xuanzhuan_count = EDGE_VAlue;
				break;	
		}		
				
		break;	
		
		
	}
}
	edge =0;
}


//索敌函数
void  Enemy_search(void)
{
	
	  
	int8_t  zizhaun_flag[10] = {0,0,0,0,0,0,0,0,0,0};
	//传感器触发计数
	uint8_t R_SENSOR_CNT = 0;
	uint8_t L_SENSOR_CNT = 0;
	if(L_huidu>ARENA_GRAY_LIGHT_AREA && R_huidu>ARENA_GRAY_LIGHT_AREA)
	{
			if(QIAN_L_JG <= JG_FRONT_LIGHT){zizhaun_flag[0] = 1;}
			if(QIAN_R_JG <= JG_FRONT_LIGHT){zizhaun_flag[1] = 1;}
	}else 
	{
		if(QIAN_L_JG <= JG_FRONT_DARK){zizhaun_flag[0] = 1;}
	if(QIAN_R_JG <= JG_FRONT_DARK){zizhaun_flag[1] = 1;}
	}

	
	if( (B_JG_LEFT < JG_BACK_DARK || B_JG_RIGHT < JG_BACK_DARK  ) && ( QIAN_L_JG > JG_SEARCH_ENTRY_FRONT && QIAN_R_JG > JG_SEARCH_ENTRY_FRONT ) && TURN_BACK_ENABLE )
	{  
			if(Enemy_search_scilence_CNT <= 0) //每次转向静默200ms
		{
		move(0,0);  
			
		osDelay(100);
		TURN_ZIZHUAN(170);
		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = XUANZHUAN_VALUE;	
		move(0,0);  
		osDelay(100);
		Enemy_search_scilence_CNT = SEARCH_SILENCE_CNT; //80
		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = XUANZHUAN_VALUE;
		}
	}
	
	if(zizhaun_flag[0] == 1 && zizhaun_flag[1] == 0)
	{
			if(SSQ_TYPE == Bomb_CUBE) //前面是炸弹块
		{
			TURN_ZIZHUAN(170);
			STOP();
			osDelay(200);
			TURN_BACK_COUNT = ZHUIJI_TURN_BACK_COUNT;
			TURN_BACK_ENABLE = 0;
		}
		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = XUANZHUAN_VALUE;	 
		

				if(L_huidu + R_huidu >=ARENA_GRAY_BLUE*2)
		{
		move( 400,500);
		}else 
		{
			move(100 , 250);
		}
	}else if(zizhaun_flag[0] == 0 && zizhaun_flag[1] == 1)
	{
		if(SSQ_TYPE == Bomb_CUBE) //前面是炸弹块
		{
			TURN_ZIZHUAN(170);
			STOP();
			osDelay(200);
			TURN_BACK_COUNT = ZHUIJI_TURN_BACK_COUNT;
			TURN_BACK_ENABLE = 0;
		}
		if(L_huidu + R_huidu >=ARENA_GRAY_BLUE*2)
		{
			move(500 , 400);
		}else 
		{
			move(250 , 100);
		}

		Enemy_search_mode = zizhuan_mode;  
		xuanzhuan_count = XUANZHUAN_VALUE;
	}
	 if (zizhaun_flag[0] == 0 && zizhaun_flag[1] == 0)
	{
			//在代码的23行开关

			if(L_huidu>ARENA_GRAY_LIGHT_AREA && R_huidu>ARENA_GRAY_LIGHT_AREA)
			{
				if(L_JG <= JG_SIDE_LIGHT){zizhaun_flag[2] = -1;}
				if(R_JG <= JG_SIDE_LIGHT)   {zizhaun_flag[6] = 1;}
				if(L_JG_30 <= JG_SIDE_30_LIGHT){zizhaun_flag[3] = -1; }
			  if(L_JG_60 <= JG_SIDE_60_LIGHT){zizhaun_flag[4] = -1;}

				if(R_JG_30 <= JG_SIDE_30_LIGHT){ zizhaun_flag[7] = 1;} 
				if(R_JG_60 <= JG_SIDE_60_LIGHT){zizhaun_flag[8] = 1;}
				
			}
			else 
			{
				if(L_JG <= JG_SIDE_DARK){zizhaun_flag[2] = -1;}
				if(R_JG <= JG_SIDE_DARK)   {zizhaun_flag[6] = 1;}
				if(L_JG_30 <= JG_SIDE_30_DARK){zizhaun_flag[3] = -1; } 
			  if(L_JG_60 <= JG_SIDE_60_DARK){zizhaun_flag[4] = -1;}
				if(R_JG_30 <= JG_SIDE_30_DARK){ zizhaun_flag[7] = 1;}
				if(R_JG_60 <= JG_SIDE_60_DARK){zizhaun_flag[8] = 1;}
			}

//			if(R_JG_120 <= 600){zizhaun_flag[9] = 1;}		
  
			for(int i = 0; i<10;i++)
		{
			if(zizhaun_flag[i] == 1)R_SENSOR_CNT ++;
			if(zizhaun_flag[i] == -1)L_SENSOR_CNT ++;
		}  
		//防止除以零出现正负无穷
		if(R_SENSOR_CNT == 0) R_SENSOR_CNT = 1;
		if(L_SENSOR_CNT == 0) L_SENSOR_CNT = 1;			
			 turn_angle =( zizhaun_flag[2] * weight_90 + zizhaun_flag[3] * weight_30+
								zizhaun_flag[6] * weight_90  +  zizhaun_flag[7] * weight_30   + 
								zizhaun_flag[4] * weight_60   + zizhaun_flag[8] * weight_60);
			turn_angle = turn_angle>0 ? turn_angle/R_SENSOR_CNT : turn_angle/L_SENSOR_CNT ;
		/*视觉追踪测试
		turn_angle = 0;
		*/
		 
		if(target_velocity_1[0] >= TURN_ANGLE_ATTEN_SPEED_THRESH && target_velocity_2[0] >= TURN_ANGLE_ATTEN_SPEED_THRESH)
		{
			turn_angle= turn_angle*ARENA_GRAY_TURN_ANGLE_ATTEN;
		}
		if(  turn_angle != 0)
		{
		
		if(Enemy_search_mode == zizhuan_mode)
		{  
			if(Enemy_search_scilence_CNT <= 0) //每次转向静默200ms
			{
				if(turn_angle != 0)
				{
					TURN_ZIZHUAN(turn_angle);
					turn_angle= 0;
					Enemy_search_scilence_CNT = SEARCH_SILENCE_CNT; //80
					Enemy_search_mode = zizhuan_mode;  
					xuanzhuan_count = XUANZHUAN_VALUE;
				}
			} 
		}else if (Enemy_search_mode == pianzhuan_mode)
		{
					TURN_ZIZHUAN(turn_angle);
					turn_angle= 0;
					Enemy_search_scilence_CNT = SEARCH_SILENCE_CNT; //80
					Enemy_search_mode = zizhuan_mode;
					xuanzhuan_count = XUANZHUAN_VALUE;

		}
		
	}
		
		
		VISUAL_MOVE(X_LOCATION);//roaming_veocity
		//前面为空的时候能转就转，不转就300速
	}else if(zizhaun_flag[0] == 1 && zizhaun_flag[1] == 1)
	{
		if(SSQ_TYPE == Bomb_CUBE) //前面是炸弹块
		{
			TURN_ZIZHUAN(170);
			STOP();
			osDelay(200);
			TURN_BACK_COUNT = ZHUIJI_TURN_BACK_COUNT;
			TURN_BACK_ENABLE = 0;
		}
				if(roaming_veocity_STR  > 300 )
				{
						move(roaming_veocity_STR *1.7f, roaming_veocity_STR *1.7f);
				}else 
				{
						move(roaming_veocity_STR , roaming_veocity_STR );
				}
			Enemy_search_mode = zizhuan_mode;  
			xuanzhuan_count = XUANZHUAN_VALUE;
	}
}
       

//视觉追踪方块
void VISUAL_MOVE(uint16_t X_LOCATION )
{
	int detal_X = 0;
	if(X_LOCATION != 0 || (SSQ_TYPE !=5  && SSQ_TYPE !=0)) //有方块的时候
	{
		if(SSQ_TYPE == Bomb_CUBE) //在该文件第1行修改类型
		{  
			detal_X =  X_LOCATION - 320;
		}else
		{
		  detal_X =  320 -  X_LOCATION ;
		}  
		
	int vs_detal_veocity_L = roaming_veocity_STR  - 0.5f * detal_X +100; 
	int vs_detal_veocity_R = roaming_veocity_STR + 0.5f * detal_X +100;
	if(vs_detal_veocity_L <150){vs_detal_veocity_L = 150;}
	if(vs_detal_veocity_R <150){vs_detal_veocity_R = 150;}
	
	
	move(vs_detal_veocity_L,vs_detal_veocity_R);
	  
	}else   
	{
	
	/*
		1.调速机制改一下，改成台上绕圈，但是在转向了之后的1s内不会绕圈
		2.绕圈基础半径为80cm,灰度值越高半径越大
	*/
		
		switch (ZHUIJI_Flag)
		{
			case 0: 
			
			if(Enemy_search_mode == pianzhuan_mode)
			{
				move(roaming_veocity_L, roaming_veocity_R );
			}else if (Enemy_search_mode == zizhuan_mode)
			{

						move(roaming_veocity_STR , roaming_veocity_STR );
			}
			
			break;
			case ALL_DECT : move(50, 50 );break;
			case ONLY_L_DECT :move(75, 150 );break;
			case ONLY_R_DECT :move(150, 75);break;
		}
		
	}

}




void  taishangroaming   (void)
{		
		edge_check(); 
		edge_MOVE();
		Enemy_search();
}

 






//把速度值乘以0.71就是cm每秒的速度
void move(float LEFT , float RIGHT )
{
	LEFT =   My_constrain(LEFT,-0.7*RPM_MAX,0.7*RPM_MAX)  ;
	RIGHT =  My_constrain(RIGHT,-0.7*RPM_MAX,0.7*RPM_MAX) ;
//	LEFT*= 0.5;
//	RIGHT*= 0.5;
//	
	target_velocity_1[0] = LEFT ;
	target_velocity_2[0] = RIGHT;
	target_velocity_3[0] = RIGHT;
	target_velocity_4[0] = LEFT ;
	
}


//LCD控制下的移动函数
void move_for_LCD(float LEFT , float RIGHT )
{
	LEFT =  My_constrain(LEFT,-0.7*RPM_MAX,0.7*RPM_MAX)  ;
	RIGHT = My_constrain(RIGHT,-0.7*RPM_MAX,0.7*RPM_MAX) ;
	
	target_velocity_1[0] = LEFT ; 
	target_velocity_2[0] = RIGHT;  
	target_velocity_3[0] = RIGHT;
	target_velocity_4[0] = LEFT ; 
}



//停止函数
void STOP ( void )
{
	target_velocity_1[0] = 0;
	target_velocity_2[0] = 0;
	target_velocity_3[0] = 0;
	target_velocity_4[0] = 0;
}

//偏向转向函数，转向半径是距离的一半，小于60厘米再再转向
//速度乘以3.73等于转速 ，现在轮距是220mm
//(V1/V2)  ==  (D/D +440)
//V1慢速轮子的轮速，V2快速轮子的转速 ,D测到的对象距离，毫米单位
void TURN_PIANZHUAN ( int direct  ,int angle)
{
	
		if(direct > 0)
		{
			move(PIANZHUAN_SPEED_FAST, PIANZHUAN_SPEED_SLOW);
		}else
		{
			move(PIANZHUAN_SPEED_SLOW, PIANZHUAN_SPEED_FAST);
		}
		uint32_t TIM_START = TIM2->CNT;
		uint32_t time_count = 0;
		if(turn_angle >= PIANZHUAN_LARGE_ANGLE)
		{
			time_count= PIANZHUAN_LARGE_TIME;
		}else
		{
			time_count= PIANZHUAN_SMALL_TIME;
		}
		while( TIM2->CNT <  TIM_START + time_count )
		{
					edge_check();

			if(edge != 0)
			{
				edge_MOVE();
				break;
			}
				if( QIAN_L_JG <= PIANZHUAN_BLOCK_THRESH || QIAN_R_JG <= PIANZHUAN_BLOCK_THRESH)
				{
					break;
				}
		}  

		/*调试区
				STOP();
		osDelay(7000);
		*/
		//偏转完成之后变成自传
		Enemy_search_mode = zizhuan_mode;
		xuanzhuan_count = XUANZHUAN_VALUE;
}




//自转转向函数
/* 

顺时针正    逆时针负

理论上我知道轮距W(m)，知道每个轮子的线速度V(m/s)，
知道转动的角度(弧度制)，根据刚体转动，可以得到
估算时间 t = degree * W / (2*V)
同样的我们也可以设定速度，反推大概的时间

1月24
W = 0.25 m   暂定94ms转90度   v= 2.1m/s
给到300转
1月26日，我猜到会有偏差没想到差这么多
3月12日，我又测试了一下，发现300转90度需要400ms,原来给的是300rad
*/

void TURN_ZIZHUAN (float degree)
{	
    current_Send_frame_type = DATA_Frame;
    
    uint32_t TURN_PID_START = TIM2->CNT;  // 统一初始化
    float NOW_DEGERR = 0;
    float TARGET_degree = UpdateAngleWithRotation(IMU_DATA.yaw, degree, Rotation);
    
    // 开环加速阶段（根据degree符号决定方向）
    float open_loop_speed = (degree > 0) ? ZIZHUAN_OPEN_LOOP_SPEED : -ZIZHUAN_OPEN_LOOP_SPEED;
    move(open_loop_speed, -open_loop_speed);

    float FIRST_TIME = ZIZHUAN_FIRST_TIME_A * fabs(degree)* fabs(degree) + ZIZHUAN_FIRST_TIME_B * fabs(degree) + ZIZHUAN_FIRST_TIME_C;
    osDelay(FIRST_TIME);
		uint32_t  wait_time = 0;
		if(CAR_STATUS == OFF_STAGE)
		{
			wait_time = ZIZHUAN_PID_TIMEOUT;
    // PID闭环阶段（统一逻辑，无需区分正负） 
    while(1)
    {
        NOW_DEGERR = IMU_DATA.yaw + 360 * Rotation;
        
        if(TIM2->CNT > TURN_PID_START + wait_time || 
           fabs(NOW_DEGERR - TARGET_degree) <= ZIZHUAN_PID_TOLERANCE)
        break;
        
        turn_velocity = LV1_PID_BACK(&IMU_PID, NOW_DEGERR, TARGET_degree);
        turn_velocity = fabs(turn_velocity) >= ZIZHUAN_SPEED_LIMIT * RPM_MAX ? 
                        (turn_velocity >= 0 ? ZIZHUAN_SPEED_LIMIT * RPM_MAX : -ZIZHUAN_SPEED_LIMIT * RPM_MAX) : turn_velocity;
        move(turn_velocity, -turn_velocity);
    }
		}
		//	在台上的话就不停了		
	if(CAR_STATUS == ON_STAGE) return ;  

    STOP();
		osDelay(200);
}
 
//低速90度开环转向
void OPEN_LOOP_90_LOW_V( int angle )
{
		if(angle == 90)
		{
			move(LOWV_90_SPEED, -LOWV_90_SPEED);
			osDelay(LOWV_90_TIME);
		}else if(angle == -90)
		{
			move(-LOWV_90_SPEED, LOWV_90_SPEED);
			osDelay(LOWV_90_TIME);
		}
		else if(angle == -60)
		{
			move(-LOWV_60_SPEED, LOWV_60_SPEED);
			osDelay(LOWV_60_TIME);
		}else if(angle == 60)
		{
			move(LOWV_60_SPEED, -LOWV_60_SPEED);
			osDelay(LOWV_60_TIME);
		}
		STOP();
		osDelay(300);
}



//专门给台下写的慢速180度转向

void TURN__taixia( float  degree)
{

    current_Send_frame_type = DATA_Frame;
    
    uint32_t TURN_PID_START = TIM2->CNT;  // 统一初始化
    float NOW_DEGERR = 0;
    float TARGET_degree = UpdateAngleWithRotation(IMU_DATA.yaw, degree, Rotation);
    
    // 开环加速阶段（根据degree符号决定方向）

    // PID闭环阶段（统一逻辑，无需区分正负） 
    while(1)
    {
        NOW_DEGERR = IMU_DATA.yaw + 360 * Rotation;
        
        if(TIM2->CNT > TURN_PID_START + 1000000 || 
           fabs(NOW_DEGERR - TARGET_degree) <= ZIZHUAN_PID_TOLERANCE)
        break;
        
        turn_velocity = LV1_PID_BACK(&IMU_PID, NOW_DEGERR, TARGET_degree);
        turn_velocity = fabs(turn_velocity) >= ZIZHUAN_TAIXIA_SPEED_LIMIT * RPM_MAX ? 
                        (turn_velocity >= 0 ? ZIZHUAN_TAIXIA_SPEED_LIMIT * RPM_MAX : -ZIZHUAN_TAIXIA_SPEED_LIMIT * RPM_MAX) : turn_velocity;
				
        turn_velocity = fabs(turn_velocity) <= ZIZHUAN_TAIXIA_MIN_SPEED * RPM_MAX ? 
                        (turn_velocity >= 0 ? ZIZHUAN_TAIXIA_MIN_SPEED * RPM_MAX : -ZIZHUAN_TAIXIA_MIN_SPEED * RPM_MAX) : turn_velocity;
        move(turn_velocity, -turn_velocity);
    }

    STOP();
		osDelay(200);
		
}


  

//记录圈数的角度函数
float UpdateAngleWithRotation(float now_degree, float add_degree , int16_t rotation)
{
	float OUTPUT  = 0;
	if(now_degree + add_degree  <=  0)
	{
		OUTPUT  =  (rotation -1 )*360  + 360 + (now_degree + add_degree);
	}else if(now_degree + add_degree  >=  360)
	{
		OUTPUT  =  (rotation +1 )*360  + (now_degree + add_degree) -  360;
	}
	else
	{
	  OUTPUT  =  (rotation  )*360  + (now_degree + add_degree);
	}
	return OUTPUT;
}





//后退一定时间的函数 
void walk_backward_TIME(int16_t BACK_Velocity,uint32_t us )
{
		move( BACK_Velocity,BACK_Velocity);
		uint32_t  taixia_start = 0; 
		taixia_start = TIM2->CNT;
	while(TIM2->CNT - taixia_start < us && CAR_STATUS != ON_STAGE )   //时间结束或者不在台下截至
		{
				if(L_huidu + R_huidu > ARENA_GRAY_BACK_ON_STAGE)
				{
					osDelay(200);
					break;
				}
				move( BACK_Velocity,BACK_Velocity);
		}
		STOP();
}  

//清除接收帧标志位
void clear_RECEive_frame(void)
{
	current_Receive_frame_type = DISTANCE_Uncomplete_Frame;
	Receive_frame_4ID[0] = 0;
	Receive_frame_4ID[1] = 0;
	Receive_frame_4ID[2] = 0;
	Receive_frame_4ID[3] = 0;
	Receive_frame_2ID[0] = 0;
	Receive_frame_2ID[1] = 0;
}


//后退一定距离的函数
void walk_CM(float CM )
{
	
//////使用位置模式///////////
//	clear_RECEive_frame();
//	if (__fabs(CM) <= 1.0f)return;
//	//发送一个位移帧
//	RS485_Distance_Frame_Data.CM = CM;
//	current_Send_frame_type = Distance_Frame;
//	osDelay(10);//延迟5毫秒一直到485发送完成
//	
//	//等待移动指定距离
//	while (1)
//	{
//		/*
//		 一直发送命令帧
//		 放分析报文，分析出来是未完成的
//		 那就继续卡在while中
//		 收到了完成帧之后 ，跳出while循环
//		 加上一小段延时
//		 并且改变485发送模式为正常模式
//		*/
//	current_Send_frame_type = Command_Frame; //发送指令帧获取当前状态	
//	Analyze_RS485_data();  
//	#ifdef MY_ID_diantiao
//	if (Receive_frame_2ID[0] == 1 ){ current_Receive_frame_type = DISTANCE_OK_Frame;}//&& Receive_frame_2ID[1] == 1
//	#endif
//	#ifdef MY_ID
//	if (Receive_frame_4ID[0] + Receive_frame_4ID[1] + Receive_frame_4ID[2] + Receive_frame_4ID[3] >=2){ current_Receive_frame_type = DISTANCE_OK_Frame;}
//	#endif
//	if ( current_Receive_frame_type == DISTANCE_OK_Frame)
//	{
//		//收到了发来的距离帧结束标识
//		current_Send_frame_type = DATA_Frame; //恢复正常模式
//		break;
//	}
//	}


/////////////不使用位置模式///////////////    
//600转216.34cm
	float F_CM = fabsf(CM);
	float Xtime =  ( F_CM / 216.34f )*1000;
	if(CM>0){
	move(600,600);
	}else 
	{
	move(-600,-600);	
	}
	osDelay(Xtime);
	
	STOP();
	osDelay(150);
}




//反追击算法
void  Anti_pursuit_Algorithm  ( void )
{
	
	float Anti_pur_Al_cm = 0;  //反追击距离
	
		if( B_JG <= JG_BACK_ENEMY && CAR_STATUS == OFF_STAGE)  //辟谷后面有敌人或者障碍
	{
		TURN__taixia(180);
		move(ANTI_PURSUIT_ESCAPE_SPEED, ANTI_PURSUIT_ESCAPE_SPEED);
		osDelay(ANTI_PURSUIT_ESCAPE_TIME);
	}
	if( B_JG <= JG_BACK_ENEMY && CAR_STATUS == OFF_STAGE)  //辟谷后面有敌人或者障碍
	{
		
		Continuous_Occlusion_Count++; //第二次遮挡
		move(ANTI_PURSUIT_ESCAPE_SPEED, ANTI_PURSUIT_ESCAPE_SPEED);
		osDelay(ANTI_PURSUIT_ESCAPE_TIME);
		move(0,0);
		osDelay(200);
		int Anti_pur_Al_DIR = ( L_JG <= R_JG ) ? 1 : -1;
		if( L_JG <= R_JG )		Anti_pur_Al_cm = (JG_ARENA_EDGE_DIST - L_JG)/10;   //到擂台边缘上台			
		if( L_JG > R_JG )	  	Anti_pur_Al_cm = (JG_ARENA_EDGE_DIST - R_JG)/10;   //到擂台边缘上台		
		move(-300,-300); 
		osDelay(250);	   
		
		move(0,0); 
		osDelay(200);		
		
		OPEN_LOOP_90_LOW_V(Anti_pur_Al_DIR*90);
		osDelay(200);
		if(QIAN_L_JG >= JG_ANTI_PURSUIT_FRONT && QIAN_R_JG >= JG_ANTI_PURSUIT_FRONT  && CAR_STATUS == OFF_STAGE)
		{
				walk_CM(Anti_pur_Al_cm);
				TURN__taixia(-Anti_pur_Al_DIR*90);
				roximal_Alignment(100000);
		}
		
		Continuous_Occlusion_Count = 0;
	}else if( B_JG > JG_BACK_ENEMY &&CAR_STATUS == OFF_STAGE )  //空了，那就后退吧
	{
			backing_attempt = 1;  //标记：正在后退上台
			walk_backward_TIME(-ANTI_PURSUIT_BACK_SPEED * RPM_MAX, ANTI_PURSUIT_BACK_TIME);

		if( CAR_STATUS == ON_STAGE )
		{		TURN_ZIZHUAN(160);  }      
		STOP();
		osDelay(ON_STAGE_WAIT_TIME);		
			backing_attempt = 0;	//后退结束，清除标志
	}
		 
}
//近端对正
void roximal_Alignment (uint32_t TIME) 
{
	
	int direction = 0;
	if(QIAN_L_JG <= JG_SIDE_LIGHT && QIAN_R_JG <= JG_SIDE_LIGHT)
	{
		direction =	(QIAN_L_JG > QIAN_R_JG ) ? 1 : -1;
	}else 
	{
		direction =	(L_JG > R_JG ) ? 1 : -1;
	}
	
	uint32_t  ROX_ALI_SATRT = 0 ;
	ROX_ALI_SATRT = TIM2->CNT;
		while (!(QIAN_L_JG < JG_ROX_FRONT_CLOSE && QIAN_R_JG < JG_ROX_FRONT_CLOSE && __fabs (QIAN_L_JG-QIAN_R_JG) < JG_ROX_ALIGN_TOLERANCE) ) {
			if (CAR_STATUS == ON_STAGE ) {
					break;
			}
			if( TIM2->CNT  - ROX_ALI_SATRT >TIME) 
			{
				if(QIAN_L_JG > JG_ROX_FRONT_NEAR && QIAN_R_JG > JG_ROX_FRONT_NEAR)
				{
				walk_backward_TIME(JG_ROX_BACK_SPEED_FACTOR * RPM_MAX, JG_ROX_BACK_TIME);  	
				}else 
				{
				walk_backward_TIME(-JG_ROX_FWD_SPEED_FACTOR * RPM_MAX, JG_ROX_BACK_TIME);  	
				}

				break;
			}
//				  if(L_JG + R_JG >1300)
//					{
//					turn_speed=__fabs(QIAN_L_JG-QIAN_R_JG )*roximal_Alignment_JIGUANG_GAIN;						
//					}else
//					{
//					turn_speed = 200;
//					}

//					turn_speed = My_constrain(turn_speed,-0.2*RPM_MAX,0.2*RPM_MAX );

					move(JG_ROX_TURN_SPEED * direction, -JG_ROX_TURN_SPEED * direction);
			
	}
		
		STOP();
		osDelay(150);	
}



//远端对正
void FAR_Alignment (uint32_t TIME ,int DIR) 
{
	uint32_t  ROX_ALI_SATRT = 0 ;
	ROX_ALI_SATRT = TIM2->CNT;
		while (!(QIAN_L_JG + QIAN_R_JG > JG_FAR_TARGET) ) {
			if (CAR_STATUS == ON_STAGE ) 
			{
					break;
			}
			if( TIM2->CNT  - ROX_ALI_SATRT >TIME) 
			{
				break;
			}
					move(JG_FAR_TURN_SPEED * DIR, -JG_FAR_TURN_SPEED * DIR); 
	}
		
		STOP();
		osDelay(150);	
}


void  taixia (void )
{
//	          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET); //高电平为空
	
	if(START_ZONE_flag == 1 )
	{
				move(200,200);
				osDelay(500);
				backing_attempt = 1;
				move(-400,-400);
				osDelay(200);
				move(-400,-500);
				osDelay(700);
			  TURN_ZIZHUAN(160);
				backing_attempt = 0;
				Enemy_search_mode = zizhuan_mode;  
				xuanzhuan_count = 2000;
			 START_ZONE_flag = 0;
	}else if(START_ZONE_flag == 2)
	{
			move(200,200);
			osDelay(500);
			move(-200,-200);
			osDelay(220);
			STOP();
		  osDelay(100);
			TURN__taixia(90);
			backing_attempt = 1;
			move(-300,-300);
			osDelay(800);
			STOP();
			osDelay(150);
			backing_attempt = 0;
			TURN__taixia(-90);
			START_ZONE_flag = 0;
	}
	
	if(L_JG > JG_TAIXIA_CORRIDOR_SIDE &&  R_JG > JG_TAIXIA_CORRIDOR_SIDE && B_JG > JG_BACK_CLEAR   && ( QIAN_L_JG  + QIAN_R_JG < JG_TAIXIA_FRONT_SUM) )  //在过道中间，正背对擂台的话
	{ 
			backing_attempt = 1;  //标记：正在后退上台
			walk_backward_TIME(-JG_TAIXIA_BACK_SPEED * RPM_MAX, JG_TAIXIA_BACK_TIME);
		
		if( CAR_STATUS == ON_STAGE ){TURN_ZIZHUAN(160) ;}
		STOP();						
		osDelay(ON_STAGE_WAIT_TIME);
			backing_attempt = 0;	//后退结束，清除标志
		
	}else 
	{
				while(__fabs (QIAN_L_JG-QIAN_R_JG)>25   ||  QIAN_L_JG>= 300 || QIAN_R_JG>= 300 )  
		{
		
		if(CAR_STATUS == ON_STAGE){ break;};
		roximal_Alignment(1400000);  //1400ms之后退出
		
		}

		if (B_JG <= JG_BACK_WALL && CAR_STATUS == OFF_STAGE) //背后有墙
		{ 
				if(L_JG + R_JG > JG_TAIXIA_SIDE_SUM_LARGE)
				{
						TURN__taixia (180);	
				}else 
				{
						move(-300,-300);
						osDelay(250);
					TURN__taixia(90);
				}
		}
		 		if(CAR_STATUS == OFF_STAGE)
		{  
			if(  L_JG  <1000 || R_JG  <1000 )
			{
			move(200,200);
			osDelay(500);
			STOP();
			osDelay(300);
			}
		}
				if(QIAN_L_JG <= JG_TAIXIA_ALIGN_CLOSE && QIAN_R_JG <= JG_TAIXIA_ALIGN_CLOSE && (__fabs(QIAN_L_JG - QIAN_R_JG) < JG_TAIXIA_ALIGN_DIFF)  )
		{
			if(   L_JG  <250 || R_JG  <250  )
			{
				float  detal_back_v = 0;
				int dirction = 0;
				if( L_JG > R_JG  )
				{
					detal_back_v = (250 - R_JG )*0.35f;
					dirction = 1;
				}else 
				{
					detal_back_v = (250 - L_JG )*0.35;
					dirction = -1;				
				}
				if(dirction == 1)
				{
					move(-400 +detal_back_v ,-400);
				}else  
				{
					move(-400 ,-400 +detal_back_v );
				} 
				
				osDelay(900);
				STOP();
				osDelay(300);
				OPEN_LOOP_90_LOW_V(dirction *60);
				roximal_Alignment(800000);
			}
			else if (   L_JG  <900 || R_JG  <900  )
			{
				int dirction = 0;
				float MOVE_cm = 0;
				if( L_JG > R_JG  )
				{
					MOVE_cm  = (1200 - R_JG)/10.0; 
					dirction = -1;
				}else 
				{
					MOVE_cm  = (1200 - L_JG )/10.0 ;
					dirction = 1;				
				}
				move(-220,-220);
				osDelay(200);
				STOP();
				osDelay(200);
				OPEN_LOOP_90_LOW_V(dirction *90);
				osDelay(100);
				walk_CM(MOVE_cm);
				STOP();
				osDelay(200);
				TURN__taixia(-dirction *90);
				roximal_Alignment(800000);
			}
		
		}
		//到这里就是正对了擂台或者挡板了
		Anti_pursuit_Algorithm();		
		
	}
	
}

uint8_t X[3] = {0};
uint16_t X_LOCATION = 0;
uint8_t SSQ_TYPE = 0;
//获取上位机信息
void GET_VISUAL_DATA (void )
{
	uint8_t TYPE_OK = 0;
	uint8_t X_OK = 0;
	for (int i = 0; i<14 ;i++)
	{
		
		//开始取出X的坐标
		if(USART3_dma_buffer[i] == 'X')
		{
				for(int j = 1;j<4;j++)
			{
				if(USART3_dma_buffer[i + j] >= '0' && USART3_dma_buffer[i + j] <= '9')
				{
					X[j-1] = USART3_dma_buffer[i + j]  - '0';
				}else 
				{
					break;
				}
			}
			X_OK = 1;
		}
		//X提取完毕,TYPE开始提取
		if(USART3_dma_buffer[i] == 'T')
		{
				SSQ_TYPE = USART3_dma_buffer[i+1] - '0'; 
			  TYPE_OK = 1;
		}
		if(TYPE_OK && X_OK){  
		X_LOCATION= X[0] *100 +X[1] *10 +X[2];
		break;  
		}
	}
}





