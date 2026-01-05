#ifndef __FOC_H
#define __FOC_H

#include "main.h"
#include "AS5600.h"
#include "tim.h"
#include <math.h>
#include "usart.h"

#define _SQRT3 1.73205080757f
#define _SQRT3_2 0.86602540378f
#define _1_SQRT3 0.57735026919f
#define _2_SQRT3 1.15470053838f
#define _PI 3.14159265359f
#define _PI_2 1.57079632679f
#define _PI_3 1.0471975512f
#define _3_PI 0.9549296586f
#define _3PI_2 4.71238898038f
#define TWO_PI = 2.0f * PI;
#define _constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

extern float pid_output;
extern float current_opamp_gain, current_Rshunt_gain;
extern float angle;
extern int16_t raw_angle;
extern float v_fillter, v;

float _electricalAngle(float angle);
void FOC_clark_park(float Uq, float angle_el);
void setMotor(int _PP, int _DIR);
void FOC_setVelocity(float target);
void FOC_setAngle(float target);
float getVelocity_Fillter(void);
float getCurrent_fillter(float angle);
void FOC_setCurrent(float target);
void FOC_setCurrent_velocity_angle(float target);
void velocityOpenloop(float target_velocity);
void calculate_variety(void);
void generate_sincos_table(void);
float find_sin_in_table(uint16_t rawangle);
float find_cos_in_table(uint16_t rawangle);

#endif
