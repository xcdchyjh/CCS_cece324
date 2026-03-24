#ifndef __ENCODER_H
#define __ENCODER_H
#include "ti_msp_dl_config.h"

// 声明给外部使用
extern volatile int16_t encoder_l_count;
extern volatile int16_t encoder_r_count;

void Encoder_Init_Tim2(void);
void Encoder_Init_Tim4(void);
int Read_Encoder(uint8_t TIMX);

#endif