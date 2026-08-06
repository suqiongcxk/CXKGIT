#include "adc_m.h"
#include "string.h"
#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "adc.h"
#include  "usart.h"
uint16_t dma_i;
uint16_t adc1_val_buf[ADC1_CHANNEL_CNT*ADC1_CHANNEL_FRE];
uint32_t adc1_aver_val[ADC1_CHANNEL_CNT]={0};
uint16_t value[ADC1_CHANNEL_CNT]={0};
char strArray[ADC1_CHANNEL_CNT][4]={0};
int  GPIO_VALUE[2] = {0,0};
 int point[3]={0};
int lenth=0;
void ADC_Sample_Start(void)
{

	if(HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&adc1_val_buf,(ADC1_CHANNEL_CNT*ADC1_CHANNEL_FRE))!=HAL_OK)
	{
		Error_Handler();
	}
}
	

void ADC_Process(void)
{
	for(dma_i=0;dma_i<ADC1_CHANNEL_CNT;dma_i++)
	{
		adc1_aver_val[dma_i]=0;
	}
	
	
	
	for(dma_i=0;dma_i<ADC1_CHANNEL_FRE;dma_i++)
	{
		
			for(int dma_j=0;dma_j<ADC1_CHANNEL_CNT;dma_j++)
	{
		adc1_aver_val[dma_j]+=adc1_val_buf[dma_i*ADC1_CHANNEL_CNT+dma_j];
	}

	}
	for(dma_i=0;dma_i<ADC1_CHANNEL_CNT;dma_i++)
	{
		value[dma_i]=adc1_aver_val[dma_i]/ADC1_CHANNEL_FRE;
	}
//		printf("%d\n",value[7] );
}



