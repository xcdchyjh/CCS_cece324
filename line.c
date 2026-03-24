#include "line.h"
#include "Motor.h"
#include "Encoder.h"
#include "sensor.h"
#include "Delay.h"
#include "oled_hardware_i2c.h" 


// 在文件开头增加上上一次的误差变量
int Vel_A_Prev_bias = 0;
int Vel_B_Prev_bias = 0;

int Turn_Lock_State = 0;

float Velcity_Kp1 = 0.0001f, Velcity_Ki1 = 0.000000f, Velcity_Kd1 = 0.000000f;
float Velcity_Kp2 = 0.0001f, Velcity_Ki2 = 0.000000f, Velcity_Kd2 = 0.000000f;

// 基础速度设置 (满量程3200)
int PWM_BASIC = 500;
int OFFSET_RIGHT = 0; 
int OFFSET_LEFT  = 0;   

pid right = {0, 0, 0};
pid left = {0, 0, 0};
char RorL = 'S'; 

int Vel_A_ControlVelocity = 0, Vel_A_Last_bias = 0;
int Vel_B_ControlVelocity = 0, Vel_B_Last_bias = 0;

void PID_Reset(void){
    left.now = 0; left.last = 0; left.sum = 0;
    right.now = 0; right.last = 0; right.sum = 0;
    Vel_A_ControlVelocity = 0; Vel_A_Last_bias = 0;
    Vel_B_ControlVelocity = 0; Vel_B_Last_bias = 0;
    RorL = 'S'; Turn_Lock_State = 0;
    Read_Encoder(4); Read_Encoder(2);
    motor(0, 0); Delay_ms(200);
}

// A 是左轮的 PID
// 修正后的 Velocity_A
int Velocity_A(int target, int encoder) {
    int Bias = target - encoder;
    // 标准增量式公式: Kp*(e(k)-e(k-1)) + Ki*e(k) + Kd*(e(k) - 2e(k-1) + e(k-2))
    Vel_A_ControlVelocity += Velcity_Kp1 * (Bias - Vel_A_Last_bias) 
                           + Velcity_Ki1 * Bias 
                           + Velcity_Kd1 * (Bias - 2 * Vel_A_Last_bias + Vel_A_Prev_bias);
    
    Vel_A_Prev_bias = Vel_A_Last_bias;
    Vel_A_Last_bias = Bias;
    
    // 增加一个输出限幅，防止 PWM 超过底层定时器允许的最大值 (例如 3200)
    if(Vel_A_ControlVelocity > 3000) Vel_A_ControlVelocity = 3000;
    if(Vel_A_ControlVelocity < -3000) Vel_A_ControlVelocity = -3000;
    
    return Vel_A_ControlVelocity;
}

// B 是右轮的 PID
int Velocity_B(int target, int encoder) {
    // 1. 计算当前误差
    int Bias = target - encoder;
    
    // 2. 标准增量式 PID 公式计算速度增量并累加
    // 公式: 控制量 += Kp*(本次误差-上次误差) + Ki*本次误差 + Kd*(本次误差 - 2*上次误差 + 上上一次误差)
    Vel_B_ControlVelocity += Velcity_Kp2 * (Bias - Vel_B_Last_bias) 
                           + Velcity_Ki2 * Bias 
                           + Velcity_Kd2 * (Bias - 2 * Vel_B_Last_bias + Vel_B_Prev_bias);
    
    // 3. 更新历史误差，为下一次计算做准备（注意顺序不能反）
    Vel_B_Prev_bias = Vel_B_Last_bias;
    Vel_B_Last_bias = Bias;
    
    // 4. PWM 输出限幅保护 (假设你底层的 PWM 最大满量程是 3200)
    // 这一步非常重要，能防止积分饱和导致电机突然疯转
    if(Vel_B_ControlVelocity > 3000) Vel_B_ControlVelocity = 3000;
    if(Vel_B_ControlVelocity < -3000) Vel_B_ControlVelocity = -3000;
    
    return Vel_B_ControlVelocity;
}

int whiteall(void) {
    if (L3 + L2 + L1 + M0 + M1 + R1 + R2 + R3 == 0) return 1;
    else return 0;
}

void error_get(void) {
    // 建议平滑过渡，缩小倍率差距
    int error1 = 80, error2 = 180, error3 = 400;
    
    // 回到内侧4个传感器时解除状态锁
    if (L1 == 1 || M0 == 1 || M1 == 1 || R1 == 1) { 
        Turn_Lock_State = 0; 
    }

    if (Turn_Lock_State != 2) { 
        if (L1 == 1) { left.now -= error1; right.now += error1; } 
        if (L2 == 1) { left.now -= error2; right.now += error2; } 
        if (L3 == 1) { left.now -= error3; right.now += error3; RorL = 'L'; Turn_Lock_State = 1; } 
    }

    if (Turn_Lock_State != 1) {
        if (R1 == 1) { left.now += error1; right.now -= error1; } 
        if (R2 == 1) { left.now += error2; right.now -= error2; } 
        if (R3 == 1) { left.now += error3; right.now -= error3; RorL = 'R'; Turn_Lock_State = 2; } 
    }
}

// 完全脱线时的找线逻辑
void go_along_last(char RorL, int direction) {
    int turn_speed = -400;    
    int forward_speed = 1000; 
    
    // 注意：底层函数是 motor(左轮速度, 右轮速度)
    if (RorL == 'L') {
        // 刚才是左转时脱线，继续左转：左轮倒转，右轮前进
        motor(turn_speed, forward_speed); 
    } else if (RorL == 'R') {
        // 刚才是右转时脱线，继续右转：左轮前进，右轮倒转
        motor(forward_speed, turn_speed); 
    } else {
        motor(PWM_BASIC + OFFSET_LEFT, PWM_BASIC + OFFSET_RIGHT);      
    }
}

void setspeed_pid(int direction) {
    int PWM_Right, PWM_Left; 
    
    // 每次计算前先清零当前误差
    left.now = 0; right.now = 0;
    error_get();
    
    if (whiteall()) {
        go_along_last(RorL, direction);
        // 脱线时保持 last 不变，不要清零，否则找回线瞬间 D 项会暴走
        left.last = left.now; 
        right.last = right.now;
        return; 
    }
    
    // 【关键修改1】：彻底删除 left.sum 和 right.sum 相关的积分累加代码！
    // 循迹不需要 I，否则 100% 会画龙震荡！
    
    // 【关键修改2】：计算公式中只保留 P 和 D
    PWM_Right = (PWM_BASIC + OFFSET_RIGHT) + (Kp * right.now) + (Kd * (right.now - right.last)); 
    PWM_Left  = (PWM_BASIC + OFFSET_LEFT)  + (Kp * left.now)  + (Kd * (left.now - left.last));   
    
    // 【关键修改3】：限幅保护优化
    // 允许 PWM 出现一定的负数，这样当 D 项（刹车）起作用时，内侧轮可以短暂反转，刹车更灵敏
    if(PWM_Left > 3000) PWM_Left = 3000;
    if(PWM_Left < -1000) PWM_Left = -1000; 
    if(PWM_Right > 3000) PWM_Right = 3000;
    if(PWM_Right < -1000) PWM_Right = -1000;

    // 直接输出给电机
    motor(PWM_Left, PWM_Right);
    
    // 必须在最后更新历史误差，供下一次 D 项计算使用
    left.last = left.now; 
    right.last = right.now;
}

// ==========================================
// 三段式状态机跑圈：完全按照“白进黑 -> 锁定0.15s -> 黑进白 -> 计数”逻辑
// ==========================================
void Run_Square_Map(int target_loops) {
    int Total_Corners = target_loops * 4; 
    int Corner_Count = 0;
    int Current_Loop = 0;
    
    uint32_t T_Start_Run = time;      
    uint32_t T_Compensation = 0;      
    uint32_t Turn_Start_Time = 0; 
    int Is_First_Corner = 1; 
    
    int Corner_State = 0; 
    
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Map: Square", 16); 
    OLED_ShowString(0, 2, (uint8_t*)"Loop: 0 / ", 16);   
    OLED_ShowNum(80, 2, target_loops, 1, 16);           
    OLED_ShowString(0, 4, (uint8_t*)"Corner: 0", 16);    
    
    while (time - T_Start_Run < 200) {
        setspeed_pid(1);
        Delay_ms(5);
    }

    while (1) {
        setspeed_pid(1);
        Delay_ms(5);
        
        if (Corner_State == 0) {
            if (L3 == 1 || R3 == 1) { 
                Corner_State = 1;         
                Turn_Start_Time = time;   
                
                if (Is_First_Corner) { 
                    T_Compensation = time - T_Start_Run; 
                    Is_First_Corner = 0; 
                }
            }
        }
        else if (Corner_State == 1) {
            if (time - Turn_Start_Time > 150) {
                Corner_State = 2;         
            }
        }
        else if (Corner_State == 2) {
            if (L3 == 0 && R3 == 0) {
                Corner_State = 0;         
                Corner_Count++;           
                
                Current_Loop = Corner_Count / 4; 
                OLED_ShowNum(48, 2, Current_Loop, 1, 16); 
                OLED_ShowNum(64, 4, Corner_Count, 2, 16); 
            }
        }

        if (Corner_Count >= Total_Corners) {
            uint32_t T_End_Start = time;
            OLED_ShowString(0, 6, (uint8_t*)"Auto Homing...", 16); 
            
            uint32_t Final_Run_Time = (uint32_t)(T_Compensation * 0.80f);
            
            while (time - T_End_Start < Final_Run_Time) {
                setspeed_pid(1);
                Delay_ms(5);
            }
            
            motor(0, 0); 
            return; 
        }
    }
}

void Run_Normal_Map(int target_loops) {
    Run_Square_Map(target_loops);
}