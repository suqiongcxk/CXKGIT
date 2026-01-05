#include "PID.h"
#include "main.h"

#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

float PID(PID_Struct *x,float error)
{
	uint32_t now_time = micros();
    float Ts = (now_time-x->last_time)*1e-6f;
	if(Ts <= 0 || Ts > 0.5f) Ts = 1e-3f;
	x->last_time =now_time;
	
	float P = x->kp*error;
	
	x->I+=x->ki*Ts*(error+x->last_error)*0.5f;
	x->I = _constrain(x->I,-x->lim_ki,x->lim_ki);
	
	float D = x->kd*(error-x->last_error)/Ts;
	
	x->last_error = error;
	
	float output = P + x->I + D;
	output = _constrain(output,-x->lim_output,x->lim_output);
	
	float output_rate = (output - x->last_output)/Ts;
	if (output_rate > x->output_ramp)
            output = x->last_output + x->output_ramp*Ts;
    else if (output_rate < -x->output_ramp)
            output = x->last_output - x->output_ramp*Ts;
	x->last_output = output;
	
	return output;
	
}
