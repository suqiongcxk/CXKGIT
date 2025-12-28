#include "remote_contol.h"



uint8_t sbus_buf[SBUS_FRAME_SIZE];
volatile uint8_t sbus_new_data = 0;
int16_t sbus_ch[16] = {0};  // 16��ͨ������(0-2047)
char x = '\n';



void get_SBUS_data(void)
{
	 if (sbus_new_data) {
      sbus_new_data = 0;
      
		  // 右边摇杆左右晃动500-1300  右边摇杆上下晃动600-1400   左边油门8-1608   左边左右摆动544-1344   
		  //左边挡位低到高1800，1000，200            右边挡位低到高1800，1000，200
      sbus_ch[0]  = (sbus_buf[1]  | (sbus_buf[2]  << 8)) & 0x07FF;
      sbus_ch[1]  = ((sbus_buf[2] >> 3) | (sbus_buf[3]  << 5)) & 0x07FF;
      sbus_ch[2]  = ((sbus_buf[3] >> 6) | (sbus_buf[4]  << 2) | (sbus_buf[5] << 10)) & 0x07FF;
      sbus_ch[3]  = ((sbus_buf[5] >> 1) | (sbus_buf[6]  << 7)) & 0x07FF;
      sbus_ch[4]  = ((sbus_buf[6] >> 4) | (sbus_buf[7]  << 4)) & 0x07FF;
      sbus_ch[5]  = ((sbus_buf[7] >> 7) | (sbus_buf[8]  << 1) | (sbus_buf[9] << 9)) & 0x07FF;
      }
}



