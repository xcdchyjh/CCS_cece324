#include "Encoder.h"
#include "ti_msp_dl_config.h" 

volatile int16_t encoder_l_count = 0;
volatile int16_t encoder_r_count = 0;

void Encoder_Init_Tim2(void) { }
void Encoder_Init_Tim4(void) { }

int Read_Encoder(uint8_t TIMX) {
    int Encoder_TIM;
    if (TIMX == 4) { // 右轮
        Encoder_TIM = encoder_r_count * 4;
        encoder_r_count = 0;
    }
    else if (TIMX == 2) { // 左轮
        Encoder_TIM = encoder_l_count * 4;
        encoder_l_count = 0;
    }
    else { Encoder_TIM = 0; }
    return Encoder_TIM;
}

void GROUP1_IRQHandler(void) {
    uint32_t gpio_interrupt = DL_GPIO_getEnabledInterruptStatus(GPIOA, DL_GPIO_PIN_15 | DL_GPIO_PIN_17);

    // --- 判断左轮 (A相: PA15 触发) ---
    if ((gpio_interrupt & DL_GPIO_PIN_15) == DL_GPIO_PIN_15) {
        if (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_16)) {
            encoder_l_count++;  // 左轮正转为正
        } else {
            encoder_l_count--;
        }
    }

    // --- 判断右轮 (A相: PA17 触发) ---
    if ((gpio_interrupt & DL_GPIO_PIN_17) == DL_GPIO_PIN_17) {
        if (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_22)) {
            // 【关键修改】：这里原来是 ++，现在改成了 --
            encoder_r_count--;  
        } else {
            // 【关键修改】：这里原来是 --，现在改成了 ++
            encoder_r_count++;  
        }
    }

    DL_GPIO_clearInterruptStatus(GPIOA, DL_GPIO_PIN_15 | DL_GPIO_PIN_17);
}