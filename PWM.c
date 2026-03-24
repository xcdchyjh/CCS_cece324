#include "PWM.h"
#include "ti_msp_dl_config.h"

void PWM_Init(void) {
    // 必须手动开启定时器计数！否则没有PWM输出
    DL_TimerA_startCounter(PWM_MOTOR_INST);
}

void PWM_SetMotorA(uint16_t duty) {
    if (duty > 7200) duty = 7200;
    
    // 【核心修复】：TI默认的边沿PWM是向下计数，高电平时间 = 7200 - CC值
    // 所以必须用 7200 减去你的目标占空比，这样算法给的数值越大，车才越快！
    uint16_t cc_value = 7200 - duty; 
    
    DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, cc_value, DL_TIMER_CC_0_INDEX);
}

void PWM_SetMotorB(uint16_t duty) {
    if (duty > 7200) duty = 7200;
    
    // 【核心修复】：同理，进行反向映射
    uint16_t cc_value = 7200 - duty; 
    
    DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, cc_value, DL_TIMER_CC_1_INDEX);
}