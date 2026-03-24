#include "ti_msp_dl_config.h"
#include "Delay.h"
#include "Motor.h"
#include "PWM.h"
#include "sensor.h"
#include "Encoder.h"
#include "Key.h"           
#include "menu.h"            
#include "oled_hardware_i2c.h" 
#include "line.h"     

// 声明外部变量
extern volatile int16_t encoder_r_count; 
extern volatile int16_t encoder_l_count;
extern int Map_Selection;                
extern int Loop_Target;                   

volatile uint32_t time = 0; 
void SysTick_Handler(void) {
    time++;  
}

int main(void) {
    // 1. 初始化底层硬件
    SYSCFG_DL_init();
    
    // 2. 开启绝对物理中断
    NVIC_EnableIRQ(GPIOA_INT_IRQn); 
    NVIC_EnableIRQ(GPIOB_INT_IRQn); 

    // 3. 开启系统滴答定时器
    DL_SYSTICK_enable();

    // 4. 初始化核心外设
    Delay_Init();
    Motor_Init();
    key_init();    
    
    Delay_ms(500); 
    OLED_Init();   

    // ============================================
    // --- [电机专项自测环节 - 已注释保留] ---
    // ============================================
    /* OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Motor Test...", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Run Forward  ", 16);
    while(1) {
        motor(1500, 1500);  
    }
    */

    // ============================================
    // --- [主程序发车跑图逻辑 - 已启用] ---
    // ============================================
    while(1) {
        // 呼出 OLED 菜单，等待 K0 按下确认发车
        Menu_Init();
        Menu_Run(); 
        
        OLED_Clear();
        Delay_ms(50); 
        OLED_ShowString(0, 2, (uint8_t*)"  Ready...  ", 16); 
        Delay_ms(1000); 
        
        OLED_Clear();
        Delay_ms(50);
        OLED_ShowString(0, 2, (uint8_t*)"    GO!     ", 16); 
        
        // 发车前清空历史误差
        PID_Reset();

        // 根据菜单选择进入对应的跑图逻辑
        if (Map_Selection == 1) {
            Run_Normal_Map(Loop_Target);
        } else {
            Run_Square_Map(Loop_Target);
        }
        
        // 跑完停车
        motor(0, 0);
        
        OLED_Clear();
        Delay_ms(50);
        OLED_ShowString(0, 2, (uint8_t*)"  Finished! ", 16);
        Delay_ms(2000); 
    }
}