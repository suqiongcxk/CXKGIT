#ifndef  REMOTE_CONTOL_H
#define  REMOTE_CONTOL_H
#include "main.h"

#define SBUS_FRAME_SIZE 25
extern uint8_t sbus_buf[SBUS_FRAME_SIZE];
extern volatile uint8_t sbus_new_data ;
extern int16_t sbus_ch[16] ;  
extern char x ;

void get_SBUS_data(void);
#endif

