#include "main.h"

void Enable_DWT_CycleCounter(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // ??????
    DWT->CYCCNT = 0;                                // ?????
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;          // ??????
}
