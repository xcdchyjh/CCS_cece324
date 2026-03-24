#ifndef PWM_H
#define PWM_H

#include <stdint.h> 

// 初始化PWM
void PWM_Init(void);


void PWM_SetMotorA(uint16_t duty);


void PWM_SetMotorB(uint16_t duty);

#endif