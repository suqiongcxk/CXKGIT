#ifndef _ADC_M_H
#define _ADC_M_H

#include "main.h"
#include "adc.h"

extern int  GPIO_VALUE[2] ;

#define ADC1_CHANNEL_CNT 16    // ADC1采集的通道数量
#define ADC1_CHANNEL_FRE 1    // ADC1每个通道的采样次数（采样频率）



//引脚绑定
//左右灰度,左右边缘检测，前方连个测距，自己调整一下差值，记得freetos.c的352行的差值
#define  PA0    value[0]
#define  PA1    value[1] 
#define  PA2    value[2]
#define  PA3    value[3]                         
#define  PA4    value[4]
#define  PA5    value[5] 
#define  PA6    (value[6]/3)
#define  PA7    value[7]
#define  PB0    value[8]
#define  PB1    (value[9]/3 -30)
#define  PC0    value[10]
#define  PC1    value[11]
#define  PC2    value[12]
#define  PC3    value[13]
#define  PC4    value[14]
#define  PC5    value[15]

 

#define  B_JG                (B_JG_LEFT > B_JG_RIGHT ? B_JG_RIGHT: B_JG_LEFT)
//传感器绑定
//左右灰度,左右边缘检测，自己调整一下差值，记得freetos.c的352行的差值
#define  B_R_HUI      			PA1
#define  EDGE_JG_L      		PC2 
 #define B_JG_LEFT      		PC5
#define  L_huidu      			PB0   //小于600是台下 
#define  R_JG      				  PA4
#define  QIAN_L_JG      		PB1
#define  QIAN_R_JG      		PA6
#define  R_huidu      		    PA7
#define  L_JG     			    PC1 
#define  L_JG_30      			PC0
#define  R_JG_30      			PA5
#define  EDGE_JG_R      		PA3
#define  L_JG_60     			  PC3  
#define  B_JG_RIGHT     		PA0
#define  R_JG_60     			  PA2
#define  B_L_Hui            PC4

extern uint16_t dma_i;
// DMA采集的ADC1所有通道原始采样值缓冲区(通道数*单通道采样次数)
extern uint16_t adc1_val_buf[ADC1_CHANNEL_CNT*ADC1_CHANNEL_FRE];
// ADC1各通道采样值的平均值计算数组(用于累加求和防溢出)
extern uint32_t adc1_aver_val[ADC1_CHANNEL_CNT];
// ADC1各通道最终滤波输出的有效数值
extern uint16_t value[ADC1_CHANNEL_CNT];

void ADC_Sample_Start(void);  // ADC采样启动函数
void ADC_Process(void);       // ADC数据处理函数(求平均+滤波等)




#endif
