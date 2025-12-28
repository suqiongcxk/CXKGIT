#ifndef    MYDWT_H
#define		MYDWT_H

#include "stm32g4xx_hal.h"
void Enable_DWT_CycleCounter(void);



extern  uint64_t start_cycle;
extern  uint64_t end_cycle ;
extern  uint64_t delta_cycles;




#endif
