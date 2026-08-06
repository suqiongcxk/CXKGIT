#ifndef  REMOTE_CONTOL_H
#define  REMOTE_CONTOL_H
#include "main.h"
#include "usart.h"
#define SBUS_FRAME_SIZE 25
extern  int Calibrate_flag ;
extern uint8_t sbus_buf[SBUS_FRAME_SIZE];
extern volatile uint8_t sbus_new_data ;
extern int16_t sbus_ch[16] ;  
extern char x ;
void  REMOTE_MOVE( void  );
void  REMOTE_V_Set( int  REMOTE_BASIC_V, int  RE_turn_V );
void  get_SBUS_data(void);
#endif

