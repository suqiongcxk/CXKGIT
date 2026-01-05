#include "electricity.h"
#include "FOC.h"

float adc1_sum = 0, adc2_sum = 0, adc3_sum = 0;	   // 我没有使能第三个注入通道，
float adc1_init = 0, adc2_init = 0, adc3_init = 0; // 我用的是规则通道母线采样
float current_a = 0.0f, current_b = 0.0f, current_c = 0.0f;
#define _1_SQRT3 0.57735026919f
#define _2_SQRT3 1.15470053838f
#define _ADC_CONV 0.0008058608f // 这个数据是根据3.3/4095算出来的，也就是将ADC值转换成电压值

void electricity_init(void)
{
	HAL_Delay(1000);
	for (int i = 0; i < 1000; i++)
	{
		adc_value[0] = hadc2.Instance->JDR1; // ADC模拟量是不用担心这一次取的值与上一次的相同的
		adc_value[1] = hadc2.Instance->JDR2;
		//		adc_value[2] = hadc2.Instance->JDR3;
		adc1_sum += (float)adc_value[0] * _ADC_CONV;
		adc2_sum += (float)adc_value[1] * _ADC_CONV;
		//		adc3_sum+=(float)adc_value[2]*_ADC_CONV;
		HAL_Delay(1);
	}
	adc1_init = adc1_sum / 1000.0f;
	adc2_init = adc2_sum / 1000.0f;
	//	adc3_init=adc3_sum/1000.0f;
}

float calculate_current(float angle)
{
	adc_value[0] = hadc2.Instance->JDR1;
	adc_value[1] = hadc2.Instance->JDR2;

	current_a = ((float)adc_value[0] * _ADC_CONV - adc1_init) * current_opamp_gain * current_Rshunt_gain; // 经过ADC转换为电流值公式得到该小数
	current_b = ((float)adc_value[1] * _ADC_CONV - adc2_init) * current_opamp_gain * current_Rshunt_gain; // 增益系数和分流电阻INA241A2

	float I_alpha = current_a;
	float I_beta = _1_SQRT3 * current_a + _2_SQRT3 * current_b; // 两相电流检测公式

	float ct = cosf(angle);
	float st = sinf(angle);

	// float I_d = I_alpha * ct + I_beta * st;
	float I_q = -I_alpha * st + I_beta * ct;

	return I_q;
}
