#include "main.h"
#include "core_cm4.h" //取决于你的 Cortex-M 核

uint32_t start_cycle = 0;
uint32_t end_cycle = 0;
uint32_t delta_cycles = 0;
float time_us;

void Enable_DWT_CycleCounter(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // 启用调试跟踪
    DWT->CYCCNT = 0;                                // 清零计数器
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;          // 启用周期计数
}



//		start_cycle = DWT->CYCCNT; 开始计时
//		
//		
//		程序位置
//		
//		end_cycle = DWT->CYCCNT;结束计时
//		
//		printf("%d\n", end_cycle - start_cycle);


