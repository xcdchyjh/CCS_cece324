#ifndef __LINE_H__
#define __LINE_H__

#include "ti_msp_dl_config.h"
#include "Encoder.h"

// 引用 main.c 中的全局计时变量
extern volatile uint32_t time;

// 传感器数量
#define ADC_N 8

// PWM最大值
#define PWM_MAX 7200

// =====================================
// 【重新平衡的 PID 参数】
// Kp=1.2 保证有足够力气拉回，Kd=25.0 提供适中减震
// =====================================
#define Kp 1.2f   
#define Ki 0.0f
#define Kd 25.0f  

// PID结构体
typedef struct {
    int now;   
    int last;  
    int sum;   
} pid;

extern pid left;
extern pid right;

// 速度环参数声明
extern float Velcity_Kp1;
extern float Velcity_Ki1;
extern float Velcity_Kd1;
extern float Velcity_Kp2;
extern float Velcity_Ki2;
extern float Velcity_Kd2;
extern int PWM_BASIC;

// 函数声明
int Velocity_A(int target, int encoder);
int Velocity_B(int target, int encoder);
int whiteall(void);
void error_get(void);
void setspeed_pid(int direction);
void go_along_last(char RorL, int direction);
void PID_Reset(void);

// 跑图逻辑封装函数
void Run_Normal_Map(int target_loops);
void Run_Square_Map(int target_loops);

#endif