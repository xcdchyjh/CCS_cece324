#include "Motor.h"
#include "ti_msp_dl_config.h"

void Motor_Init(void) {
    // 基础硬件初始化已在 SYSCFG_DL_init() 完成
    // 初始状态让电机停止：清除 PA12, PA13 (A电机) 和 PB0, PB16 (B电机)
    DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_12 | DL_GPIO_PIN_13);
    DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_0 | DL_GPIO_PIN_16);
    
    // 强制启动 PWM 底层定时器
    DL_Timer_startCounter(PWM_MOTOR_INST);
}

/**
 * @brief 控制双电机转速和方向
 */
void motor(int left_pwm, int right_pwm) {
    static int current_left_pwm = 0;
    static int current_right_pwm = 0;
    
    // ---------------- 左电机控制 (A电机: PA13, PA12) ----------------
    // 【修改点】：把 PA13 和 PA12 的 set/clear 互换了，现在给正数就会往前转！
    if (left_pwm > 0) {
        DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_13);
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_12);
        current_left_pwm = left_pwm;
    } else if (left_pwm < 0) {
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_13);
        DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_12);
        current_left_pwm = -left_pwm;
    } else {
        DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_12 | DL_GPIO_PIN_13);
        current_left_pwm = 0;
    }

    // ---------------- 右电机控制 (B电机: PB0, PB16) ----------------
    // (右轮正常，保持不变)
    if (right_pwm > 0) {
        DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_0);
        DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_16);
        current_right_pwm = right_pwm;
    } else if (right_pwm < 0) {
        DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_0);
        DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_16);
        current_right_pwm = -right_pwm;
    } else {
        DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_0 | DL_GPIO_PIN_16);
        current_right_pwm = 0;
    }

    // --- 将处理好的绝对值直接写入底层寄存器 ---
    DL_Timer_setCaptureCompareValue(PWM_MOTOR_INST, current_left_pwm, GPIO_PWM_MOTOR_C0_IDX);
    DL_Timer_setCaptureCompareValue(PWM_MOTOR_INST, current_right_pwm, GPIO_PWM_MOTOR_C1_IDX);
}