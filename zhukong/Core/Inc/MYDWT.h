#ifndef    MYDWT_H
#define		MYDWT_H

#include "stm32f4xx_hal.h"
void Enable_DWT_CycleCounter(void);



extern  uint32_t start_cycle;
extern  uint32_t end_cycle ;
extern  uint32_t delta_cycles;




#endif
