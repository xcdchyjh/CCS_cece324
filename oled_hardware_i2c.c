#include "oled_hardware_i2c.h"
#include "oledfont.h"
#include "Delay.h"

// =======================================
// 1. 基础延时与防卡死替换函数
// =======================================
void delay_ms(uint32_t ms)
{
    Delay_ms(ms);
}

void oled_i2c_sda_unlock(void)
{
    // 不做任何操作，保护单片机硬件状态
}

// =======================================
// 2. 核心底层通信：发送一个字节 (终极防卡死版)
// =======================================
void OLED_WR_Byte(uint8_t dat, uint8_t mode)
{
    uint8_t ptr[2];
    uint32_t timeout = 50000;
    
    ptr[0] = mode ? 0x40 : 0x00;
    ptr[1] = dat;
    
    while (!(DL_I2C_getControllerStatus(I2C_OLED_INST) & DL_I2C_CONTROLLER_STATUS_IDLE)) {
        timeout--;
        if(timeout == 0) return; 
    }
    
    DL_I2C_clearInterruptStatus(I2C_OLED_INST, DL_I2C_INTERRUPT_CONTROLLER_TX_DONE);
    DL_I2C_fillControllerTXFIFO(I2C_OLED_INST, ptr, 2);
    DL_I2C_startControllerTransfer(I2C_OLED_INST, 0x3C, DL_I2C_CONTROLLER_DIRECTION_TX, 2);
    
    timeout = 50000;
    while (!DL_I2C_getRawInterruptStatus(I2C_OLED_INST, DL_I2C_INTERRUPT_CONTROLLER_TX_DONE)) {
        timeout--;
        if(timeout == 0) return; 
    }

    for(volatile int wait=0; wait<100; wait++);
}

// =======================================
// 3. OLED 基础控制函数
// =======================================
void OLED_ColorTurn(uint8_t i)
{
    if(i==0) OLED_WR_Byte(0xA6,OLED_CMD); 
    if(i==1) OLED_WR_Byte(0xA7,OLED_CMD); 
}

void OLED_DisplayTurn(uint8_t i)
{
    if(i==0) {
        OLED_WR_Byte(0xC8,OLED_CMD); 
        OLED_WR_Byte(0xA1,OLED_CMD);
    }
    if(i==1) {
        OLED_WR_Byte(0xC0,OLED_CMD); 
        OLED_WR_Byte(0xA0,OLED_CMD);
    }
}

void OLED_Set_Pos(uint8_t x, uint8_t y) 
{ 
    OLED_WR_Byte(0xb0+y,OLED_CMD);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte((x&0x0f),OLED_CMD);
}

void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  
    OLED_WR_Byte(0X14,OLED_CMD);  
    OLED_WR_Byte(0XAF,OLED_CMD);  
}
   
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  
    OLED_WR_Byte(0X10,OLED_CMD);  
    OLED_WR_Byte(0XAE,OLED_CMD);  
}
      
void OLED_Clear(void)  
{  
    uint8_t i,n;            
    for(i=0;i<8;i++)  
    {  
        OLED_WR_Byte (0xb0+i,OLED_CMD);    
        OLED_WR_Byte (0x00,OLED_CMD);      
        OLED_WR_Byte (0x10,OLED_CMD);         
        for(n=0;n<128;n++) OLED_WR_Byte(0,OLED_DATA); 
    } 
}

// =======================================
// 4. OLED 字符与图形绘制函数
// =======================================
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t sizey)
{       
    uint8_t c=0,sizex=sizey/2;
    uint16_t i=0,size1;
    if(sizey==8)size1=6;
    else size1=(sizey/8+((sizey%8)?1:0))*(sizey/2);
    c=chr-' ';
    OLED_Set_Pos(x,y);
    for(i=0;i<size1;i++)
    {
        if(i%sizex==0&&sizey!=8) OLED_Set_Pos(x,y++);
        if(sizey==8) OLED_WR_Byte(asc2_0806[c][i],OLED_DATA);     
        else if(sizey==16) OLED_WR_Byte(asc2_1608[c][i],OLED_DATA);
        else return;
    }
}

uint32_t oled_pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;   
    while(n--)result*=m;    
    return result;
}

void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t sizey)
{           
    uint8_t t,temp,m=0;
    uint8_t enshow=0;
    if(sizey==8)m=2;
    for(t=0;t<len;t++)
    {
        temp=(num/oled_pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0) {
                OLED_ShowChar(x+(sizey/2+m)*t,y,' ',sizey);
                continue;
            } else enshow=1;
        }
        OLED_ShowChar(x+(sizey/2+m)*t,y,temp+'0',sizey);
    }
}

// 【新增】：显示带符号的数字 (支持负数)
void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t sizey)
{
    uint8_t t, temp, m = 0;
    uint8_t enshow = 0;
    if (sizey == 8) m = 2;
    
    if (num < 0) {
        OLED_ShowChar(x, y, '-', sizey);
        num = -num;  // 转换为正数进行提取
    } else {
        OLED_ShowChar(x, y, ' ', sizey); // 正数补空格保持对齐
    }
    
    for (t = 0; t < len; t++) {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                OLED_ShowChar(x + (sizey / 2 + m) * (t + 1), y, ' ', sizey);
                continue;
            } else enshow = 1;
        }
        OLED_ShowChar(x + (sizey / 2 + m) * (t + 1), y, temp + '0', sizey);
    }
}

void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t sizey)
{
    uint8_t j=0;
    while (chr[j]!='\0')
    {       
        OLED_ShowChar(x,y,chr[j++],sizey);
        if(sizey==8)x+=6;
        else x+=sizey/2;
    }
}

void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t no,uint8_t sizey)
{
    uint16_t i,size1=(sizey/8+((sizey%8)?1:0))*sizey;
    for(i=0;i<size1;i++)
    {
        if(i%sizey==0) OLED_Set_Pos(x,y++);
        if(sizey==16) OLED_WR_Byte(Hzk[no][i],OLED_DATA);
        else return;
    }               
}

void OLED_DrawBMP(uint8_t x,uint8_t y,uint8_t sizex, uint8_t sizey,uint8_t BMP[])
{   
    uint16_t j=0;
    uint8_t i,m;
    sizey=sizey/8+((sizey%8)?1:0);
    for(i=0;i<sizey;i++)
    {
        OLED_Set_Pos(x,i+y);
        for(m=0;m<sizex;m++) OLED_WR_Byte(BMP[j++],OLED_DATA);           
    }
}

// =======================================
// 5. OLED 核心初始化函数
// =======================================
void OLED_Init(void)
{
    delay_ms(200);

    OLED_WR_Byte(0xAE,OLED_CMD);
    OLED_WR_Byte(0x00,OLED_CMD);
    OLED_WR_Byte(0x10,OLED_CMD);
    OLED_WR_Byte(0x40,OLED_CMD);
    OLED_WR_Byte(0x81,OLED_CMD);
    OLED_WR_Byte(0xCF,OLED_CMD);
    OLED_WR_Byte(0xA1,OLED_CMD);
    OLED_WR_Byte(0xC8,OLED_CMD);
    OLED_WR_Byte(0xA6,OLED_CMD);
    OLED_WR_Byte(0xA8,OLED_CMD);
    OLED_WR_Byte(0x3F,OLED_CMD);
    OLED_WR_Byte(0xD3,OLED_CMD);
    OLED_WR_Byte(0x00,OLED_CMD);
    OLED_WR_Byte(0xD5,OLED_CMD);
    OLED_WR_Byte(0x80,OLED_CMD);
    OLED_WR_Byte(0xD9,OLED_CMD);
    OLED_WR_Byte(0xF1,OLED_CMD);
    OLED_WR_Byte(0xDA,OLED_CMD);
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);
    OLED_WR_Byte(0x40,OLED_CMD);
    OLED_WR_Byte(0x20,OLED_CMD);
    OLED_WR_Byte(0x02,OLED_CMD);
    OLED_WR_Byte(0x8D,OLED_CMD);
    OLED_WR_Byte(0x14,OLED_CMD);
    OLED_WR_Byte(0xA4,OLED_CMD);
    OLED_WR_Byte(0xA6,OLED_CMD);
    
    OLED_Clear();
    OLED_WR_Byte(0xAF,OLED_CMD); 
}