#include "menu.h"
#include <stdint.h>
#include "Key.h"
#include "oled_hardware_i2c.h"
#include "Delay.h"
#include "line.h" 

int Loop_Target = 1;    
int Map_Selection = 1;  

#define SETTING_ITEMS_NUM  5  
#define VISIBLE_LINES      3  

enum { ITEM_SPEED = 0, ITEM_LOOPS, ITEM_MAP, ITEM_KP, ITEM_KD };
typedef enum { MENU_MAIN = 0, MENU_SETTINGS, MENU_EDIT_INT, MENU_EDIT_FLOAT } MenuState_t;

static uint8_t cursor_main = 0;      
static uint8_t cursor_settings = 0;  
static uint8_t scroll_offset = 0;    
static MenuState_t current_state = MENU_MAIN;

static int* pIntVal = 0;
static float* pFloatVal = 0;

uint8_t Menu_GetKey(void) {
    uint8_t key_val = 0;
    static uint8_t key_state = 0; 
    if (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 0) {
        if (key_state == 0) {
            Delay_ms(20); 
            if      (KEY0 == 0) key_val = 2;
            else if (KEY1 == 0) key_val = 1;
            else if (KEY2 == 0) key_val = 3;
            else if (KEY3 == 0) key_val = 4;
            
            if (key_val != 0) {
                key_state = 1; 
                return key_val; 
            }
        }
    } else { key_state = 0; }
    return 0;
}

void Show_Main_Menu(void) {
    OLED_ShowString(0, 0, (uint8_t*)"== Main Menu ==", 16); // 页0
    OLED_ShowString(8, 2, (uint8_t*)"Run Mode", 16);        // 页2
    OLED_ShowString(8, 4, (uint8_t*)"Settings", 16);        // 页4
    OLED_ShowString(0, 6, (uint8_t*)"                ", 16); // 页6
    
    OLED_ShowString(0, 2, (uint8_t*)" ", 16);
    OLED_ShowString(0, 4, (uint8_t*)" ", 16);
    // 动态光标位置，步进为 2 页
    OLED_ShowString(0, 2 + cursor_main * 2, (uint8_t*)">", 16);
}

void Show_Settings_Menu(void) {
    OLED_ShowString(0, 0, (uint8_t*)"== Settings ==", 16);
    if (cursor_settings < scroll_offset) scroll_offset = cursor_settings;
    if (cursor_settings >= scroll_offset + VISIBLE_LINES) scroll_offset = cursor_settings - VISIBLE_LINES + 1;

    for (int i = 0; i < VISIBLE_LINES; i++) {
        int item_index = scroll_offset + i; 
        
        // 【关键修复】：将 16 + i * 16 改为 2 + i * 2 (转换为页坐标)
        int line_y = 2 + i * 2;                 
        
        OLED_ShowString(0, line_y, (uint8_t*)"                ", 16);
        if (item_index >= SETTING_ITEMS_NUM) continue;
        
        if (item_index == cursor_settings) OLED_ShowString(0, line_y, (uint8_t*)">", 16);
        else OLED_ShowString(0, line_y, (uint8_t*)" ", 16);

        switch (item_index) {
            case ITEM_SPEED:
                OLED_ShowString(8, line_y, (uint8_t*)"Speed:", 16); OLED_ShowNum(56, line_y, PWM_BASIC, 4, 16); break;
            case ITEM_LOOPS:
                OLED_ShowString(8, line_y, (uint8_t*)"Loops:", 16); OLED_ShowNum(56, line_y, Loop_Target, 2, 16); break;
            case ITEM_MAP:
                OLED_ShowString(8, line_y, (uint8_t*)"Map:  ", 16); OLED_ShowNum(56, line_y, Map_Selection, 1, 16); break;
            case ITEM_KP:
                OLED_ShowString(8, line_y, (uint8_t*)"Kp:   ", 16); OLED_ShowNum(56, line_y, (int)Velcity_Kp1, 2, 16); 
                OLED_ShowString(72, line_y, (uint8_t*)".", 16); OLED_ShowNum(80, line_y, (int)(Velcity_Kp1 * 100) % 100, 2, 16); break;
            case ITEM_KD:
                OLED_ShowString(8, line_y, (uint8_t*)"Kd:   ", 16); OLED_ShowNum(56, line_y, (int)Velcity_Kd1, 2, 16);
                OLED_ShowString(72, line_y, (uint8_t*)".", 16); OLED_ShowNum(80, line_y, (int)(Velcity_Kd1 * 100) % 100, 2, 16); break;
        }
        
        if ((current_state == MENU_EDIT_INT || current_state == MENU_EDIT_FLOAT) && item_index == cursor_settings) {
            OLED_ShowString(112, line_y, (uint8_t*)"*", 16);
        }
    }
}

void Menu_Init(void) {
    cursor_main = 0; cursor_settings = 0; scroll_offset = 0; current_state = MENU_MAIN;
}

void Menu_Run(void) {
    uint8_t key = 0; uint8_t run_flag = 0;
    OLED_Clear(); 
    Delay_ms(20); 
    Show_Main_Menu();

    while (!run_flag) {
        key = Menu_GetKey();
        switch (current_state) {
            case MENU_MAIN:
                if (key == 1) { cursor_main++; if (cursor_main > 1) cursor_main = 0; Show_Main_Menu(); }
                else if (key == 2) { if (cursor_main == 0) cursor_main = 1; else cursor_main--; Show_Main_Menu(); }
                else if (key == 3) { 
                    if (cursor_main == 0) { run_flag = 1; OLED_Clear(); Delay_ms(20); } 
                    else if (cursor_main == 1) { current_state = MENU_SETTINGS; cursor_settings = 0; scroll_offset = 0; OLED_Clear(); Delay_ms(20); Show_Settings_Menu(); }
                }
                break;
            case MENU_SETTINGS:
                if (key == 1) { cursor_settings++; if (cursor_settings >= SETTING_ITEMS_NUM) cursor_settings = 0; Show_Settings_Menu(); }
                else if (key == 2) { if (cursor_settings == 0) cursor_settings = SETTING_ITEMS_NUM - 1; else cursor_settings--; Show_Settings_Menu(); }
                else if (key == 3) { 
                    switch(cursor_settings) {
                        case ITEM_SPEED: pIntVal = &PWM_BASIC; current_state = MENU_EDIT_INT; break;
                        case ITEM_LOOPS: pIntVal = &Loop_Target; current_state = MENU_EDIT_INT; break;
                        case ITEM_MAP:   pIntVal = &Map_Selection; current_state = MENU_EDIT_INT; break;
                        case ITEM_KP:    pFloatVal = &Velcity_Kp1; current_state = MENU_EDIT_FLOAT; break; 
                        case ITEM_KD:    pFloatVal = &Velcity_Kd1; current_state = MENU_EDIT_FLOAT; break;
                    }
                    Show_Settings_Menu(); 
                }
                else if (key == 4) { current_state = MENU_MAIN; OLED_Clear(); Delay_ms(20); Show_Main_Menu(); }
                break;
            case MENU_EDIT_INT:
                if (key == 1) { 
                    if (pIntVal == &PWM_BASIC) *pIntVal -= 100;
                    else if (pIntVal == &Map_Selection) { if(*pIntVal == 1) *pIntVal = 2; else *pIntVal = 1; } 
                    else *pIntVal -= 1;
                }
                else if (key == 2) { 
                    if (pIntVal == &PWM_BASIC) *pIntVal += 100;
                    else if (pIntVal == &Map_Selection) { if(*pIntVal == 1) *pIntVal = 2; else *pIntVal = 1; }
                    else *pIntVal += 1;
                }
                else if (key == 3 || key == 4) { current_state = MENU_SETTINGS; }
                Show_Settings_Menu();
                break;
            case MENU_EDIT_FLOAT:
                if (key == 1) *pFloatVal -= 0.1f; 
                else if (key == 2) *pFloatVal += 0.1f;
                else if (key == 3 || key == 4) current_state = MENU_SETTINGS;
                Show_Settings_Menu();
                break;
        }
    }
}