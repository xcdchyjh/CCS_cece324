#include "sensor.h"
#include "ti_msp_dl_config.h"

void SENSOR_GPIO_Config(void) { 
    // 引脚初始化已在 SysConfig 中完成，这里留空
}

// 绝对物理引脚读取，完全无视 SysConfig 的宏命名
unsigned char digtal(unsigned char channel) {
    switch(channel) {
        case 1: return (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_2) ? 1 : 0); // S1: PA2
        case 2: return (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_3) ? 1 : 0); // S2: PA3
        case 3: return (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_4) ? 1 : 0); // S3: PA4
        case 4: return (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_5) ? 1 : 0); // S4: PA5
        case 5: return (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_6) ? 1 : 0); // S5: PA6
        case 6: return (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_2) ? 1 : 0); // S6: PB2 (新位置)
        case 7: return (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_3) ? 1 : 0); // S7: PB3 (新位置)
        case 8: return (DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_7) ? 1 : 0); // S8: PA7
        default: return 0;
    }
}