#ifndef CONTOL_H
#define CONTOL_H

#include "RS485.h"

typedef struct {
    float normalized;  // 归一化角度 [0, 360)
    int16_t rotations; // 总旋转圈数（可正可负）
} AngleWithRotation;


typedef struct {
    float Velocity;  // 归一化角度 [0, 360)
    float CM; // 总旋转圈数（可正可负）
} Distance_Frame_Data;




#define    ON_STAGE          9    //在台上
#define    OFF_STAGE         1		 //在台下
#define    Reduction_ratio   14   //减速比
#define    Wheel_d      		 6.8f  //车轮直径单位cm
#define    M_PI      		     3.14159f  //圆周率
#define    RPM_MAX           1000      //转子最大的转速RPM
#define    BETWEEN_STAGE     5


extern float R_GAIN ;
extern float turn_angle;
extern float  Convert_radians_to_centimeters; 
extern volatile int16_t Rotation ;
extern  float   roaming_veocity_L ;
extern  float roaming_veocity_R ;  //台上漫游速度
extern volatile int CAR_STATUS  ;
extern volatile int Continuous_Occlusion_Count ;
extern Distance_Frame_Data RS485_Distance_Frame_Data;
extern volatile int stage_check_flag ;
extern int16_t Enemy_search_scilence_CNT ;
extern uint16_t Enemy_search_flag ;
extern uint8_t X[3] ;
extern uint16_t X_LOCATION ;
extern uint8_t SSQ_TYPE ;
extern float roaming_R ;
extern float roaming_veocity_STR ;
extern uint8_t ZHUIJI_Flag  ;
extern uint16_t ZHUIJI_TIME  ;  //追击时间1000ms
extern volatile int xuanzhuan_count;
extern uint8_t  zizhuan_mode    ;
extern uint8_t  pianzhuan_mode  ;
extern volatile uint8_t  Enemy_search_mode; 
extern int START_ZONE_flag;
extern volatile int TURN_BACK_ENABLE ;
extern int TURN_BACK_COUNT ;
extern uint8_t STAGE_UP_CONFIRMED;
extern uint8_t backing_attempt;
//自定义限幅函数
#define My_constrain(amt, low, high)   ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt))) 
#define PI   3.1415926f
#define  ALL_DECT        1      //两个都检测到了
#define  ONLY_L_DECT     2      //左边检测到了
#define  ONLY_R_DECT     3      //右边检测到了

void move(float LEFT , float RIGHT );
float UpdateAngleWithRotation(float now_degree, float add_degree , int16_t rotation);
void  taishangroaming   (void);
void TURN_PIANZHUAN ( int direct  ,int angle);
void TURN_ZIZHUAN ( float  degree);
void  edge_check ( void );
void STOP ( void );
void move_for_LCD(float LEFT , float RIGHT );
void walk_CM(float CM );
void walk_backward_TIME(int16_t BACK_Velocity,uint32_t us );
void roximal_Alignment (uint32_t TIME);
void  taixia (void );
void clear_RECEive_frame(void);
void  Anti_pursuit_Algorithm  ( void );
void  edge_MOVE(void );
void  Enemy_search(void);
void DOUBLE_MOVE( void );
void GET_VISUAL_DATA (void );
void VISUAL_MOVE(uint16_t X_LOCATION );
void FAR_Alignment (uint32_t TIME ,int DIR) ;
void TURN__taixia( float  degree);
void OPEN_LOOP_90_LOW_V( int angle );
#endif  

