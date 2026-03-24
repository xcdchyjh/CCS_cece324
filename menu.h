#ifndef __MENU_H__
#define __MENU_H__

#include <stdint.h>

// 声明全局变量
extern int Loop_Target;   // 圈数
extern int Map_Selection; // 地图选择 (1=地图1, 2=地图2)

void Menu_Init(void);
void Menu_Run(void);

#endif