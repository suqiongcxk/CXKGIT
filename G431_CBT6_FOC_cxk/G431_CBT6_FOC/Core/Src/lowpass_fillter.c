#include "lowpass_fillter.h"
#include "main.h"

float LowPass_Fillter(lowpass_fillter *name,float x)
{
	unsigned long timestamp = micros();
    float dt = (timestamp - name->timestamp_prev)*1e-6f;

    if (dt < 0.0f ) dt = 1e-3f;
    else if(dt > 0.3f) {
        name->y_prev = x;
        name->timestamp_prev = timestamp;
        return x;
    }

    float alpha = name->Ts/(name->Ts + dt);
    float y = alpha*name->y_prev + (1.0f - alpha)*x;
    name->y_prev = y;
    name->timestamp_prev = timestamp;
    return y;
}
