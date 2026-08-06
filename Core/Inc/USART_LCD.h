#ifndef USART_LCD_H
#define USART_LCD_H
#include  "main.h"
#define Normal_mode      1
#define ADC_mode         2
#define Control1_mode    3
#define turn_mode        4
#define Remote_mode      5
#define Jiasudu_mode     6
extern int16_t  LCD_MODE;
void PRINT_ADC  ( void );
void Analyze_LCD ( void );
void LCD_interact ( void );
void LCD_red (void);
#endif  
