#ifndef __PID_H
#define __PID_H

typedef struct PID_HANDLE
{
    float kp;
    float ki;
    float kd;

	float I;
	
    float lim_output;    // 输出限幅使能
    float output_ramp;   //输出值变化速度限幅
	float lim_ki; // 误差积分限幅
	
	float last_time;
	float last_error;
	float last_output;
	     
}PID_Struct;

float PID(PID_Struct *x,float error);

#endif
