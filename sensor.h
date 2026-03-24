#ifndef __SENSOR_H
#define __SENSOR_H

#include <stdint.h>

// 传感器映射(按物理位置从左到右:最左→左→中→右→最右)
// 对应sensor.c中digtal(1)到digtal(8)
#define L3 (digtal(1))  // 最左侧传感器
#define L2 (digtal(2))  // 左2传感器
#define L1 (digtal(3))  // 左1传感器(靠近中间)
#define M0 (digtal(4))  // 中间左传感器
#define M1 (digtal(5))  // 中间右传感器
#define R1 (digtal(6))  // 右1传感器(靠近中间)
#define R2 (digtal(7))  // 右2传感器
#define R3 (digtal(8))  // 最右侧传感器

// 函数声明
void SENSOR_GPIO_Config(void);
unsigned char digtal(unsigned char channel);

#endif