#ifndef __electricity_H
#define __electricity_H

#include "main.h"
#include "math.h"
#include "adc.h"
extern float adc1_init,adc2_init,adc3_init;

void electricity_init(void);
float calculate_current(float angle);

#endif
