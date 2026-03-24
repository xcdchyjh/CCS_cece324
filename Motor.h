#ifndef __MOTOR_H
#define __MOTOR_H

#include <stdint.h>

// 定义电机方向枚举
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_BRAKE
} Motor_Dir;

void Motor_Init(void);
void Motor_SetDirA(Motor_Dir dir);
void Motor_SetDirB(Motor_Dir dir);
void motor(int Motor_1, int Motor_2);

#endif