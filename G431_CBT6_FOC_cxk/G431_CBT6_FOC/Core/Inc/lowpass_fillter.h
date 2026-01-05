#ifndef __lowpass_fillter_H
#define __lowpass_fillter_H

typedef struct LowPassFillter{
	float Ts;
	float timestamp_prev;
	float y_prev;
}lowpass_fillter;

float LowPass_Fillter(lowpass_fillter *name,float x);

#endif
