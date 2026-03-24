#ifndef __KEY_H
#define __KEY_H
#include "ti_msp_dl_config.h"

// 使用绝对物理引脚宏，无视 SysConfig 的命名规则！
#define KEY0  (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_26)) // K0在 PA26
#define KEY1  (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_26)) // K1在 PB26
#define KEY2  (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_25)) // K2在 PA25
#define KEY3  (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_18)) // K3在 PA18

void key_init(void);
uint8_t Key_GetStatus(void);

#endif