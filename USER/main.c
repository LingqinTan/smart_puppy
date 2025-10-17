#include "stm32f10x.h"
#include "LED.h"
#include "Key.h"
#include "Buzzer.h"
#include "OLED.h"
#include "DogActions.h"
#include "Delay.h"
#include "ControlSystem.h"
#include "BluetoothControl.h"
#include "Ultrasonic.h"

// 避障状态定义
typedef enum {
    AVOID_CLEAR = 0,    // 前方无障碍
    AVOID_WARNING,      // 前方有障碍，警告
    AVOID_DANGER,       // 前方障碍很近，需要转向
    AVOID_TURNING       // 正在转向避障
} AvoidState;

// 全局变量
static AvoidState current_state = AVOID_CLEAR;
static uint32_t action_counter = 0;
static uint8_t servo4_error_count = 0;

AvoidState Improved_Avoidance_Decision(float distance)
{
    static uint8_t obstacle_count = 0;
    
    if(distance < 0) {
        return AVOID_CLEAR;
    }
    else if(distance < 12.0) { // 缩短危险距离
        obstacle_count++;
        if(obstacle_count > 2) {
            return AVOID_DANGER;
        }
        return AVOID_WARNING;
    }
    else if(distance < 25.0) {
        obstacle_count = 0;
        return AVOID_WARNING;
    }
    else {
        obstacle_count = 0;
        return AVOID_CLEAR;
    }
}

// 保护舵机4的特殊函数 - 放宽限制
void Safe_Servo4_Move(float angle)
{
    // 放宽舵机4的角度范围，避免过度保护
    if(angle < 45.0f) angle = 45.0f;
    if(angle > 135.0f) angle = 135.0f;
    
    Servo_SetAngle(4, angle);
    
    // 每次移动后短暂延时，减少电流冲击
    Delay_ms(5); // 减少延时时间
}

// 改进的超声波读取函数，增加重试机制
float Safe_Ultrasonic_GetDistance(void)
{
    float distance = 0;
    uint8_t retry_count = 0;
    
    for(retry_count = 0; retry_count < 3; retry_count++) {
        distance = Ultrasonic_GetDistance();
        if(distance > 0 && distance < 500) { // 有效距离范围
            return distance;
        }
        Delay_ms(50); // 短暂延时后重试
    }
    
    return -1; // 多次尝试后仍然失败
}

// 避障决策函数
AvoidState Avoidance_Decision(float distance)
{
    if(distance < 0) {
        return AVOID_CLEAR; // 传感器错误时默认安全
    }
    else if(distance < 15.0) {
        return AVOID_DANGER; // 15cm内：危险，立即转向
    }
    else if(distance < 30.0) {
        return AVOID_WARNING; // 30cm内：警告，准备转向
    }
    else {
        return AVOID_CLEAR; // 30cm外：安全，继续前进
    }
}

// 执行避障动作 - 简化版本，减少舵机压力
void Execute_Avoidance_Action(AvoidState state)
{
    action_counter++;
    
    switch(state) {
        case AVOID_CLEAR:
            // 安全区域，前进
            OLED_ShowString(1, 1, "STATE: WALKING   ");
            LED3_ON(); LED1_OFF(); LED2_OFF(); LED4_OFF();
            
            // 简化的前进动作，减少舵机压力
            Dog_Stand();
            Delay_ms(200);
            
            // 轻微前进动作
            Safe_Servo4_Move(100);  // 后右轻微向前
            Servo_SetAngle(1, 80);  // 前右轻微向后
            Servo_SetAngle(2, 100); // 前左轻微向前  
            Servo_SetAngle(3, 80);  // 后左轻微向后
            Delay_ms(300);
            Dog_Stand();
            break;
            
        case AVOID_WARNING:
            // 警告区域，原地等待
            OLED_ShowString(1, 1, "STATE: SLOW DOWN ");
            LED2_ON(); LED1_OFF(); LED3_OFF(); LED4_OFF();
            Buzzer_Beep(50); // 警告音
            Dog_Stand(); // 保持站立，等待
            Delay_ms(800); // 增加等待时间
            break;
            
        case AVOID_DANGER:
            // 危险区域，停止并转向
            OLED_ShowString(1, 1, "STATE: TURNING   ");
            LED1_ON(); LED2_OFF(); LED3_OFF(); LED4_OFF();
            Buzzer_BeepPattern(BEEP_DOUBLE_BEEP); // 紧急提示音
            
            // 随机选择左转或右转（根据计数器奇偶）
            if((action_counter % 2) == 0) {
                OLED_ShowString(2, 1, "ACTION: TURN LEFT ");
                
                // 简化的左转动作，特别保护舵机4
                Safe_Servo4_Move(70);   // 后右向后
                Servo_SetAngle(1, 110); // 前右向前
                Servo_SetAngle(2, 70);  // 前左向后
                Servo_SetAngle(3, 110); // 后左向前
                Delay_ms(600); // 增加转向时间
                Dog_Stand();
                
            } else {
                OLED_ShowString(2, 1, "ACTION: TURN RIGHT");
                
                // 简化的右转动作，特别保护舵机4
                Safe_Servo4_Move(110);  // 后右向前
                Servo_SetAngle(1, 70);  // 前右向后
                Servo_SetAngle(2, 110); // 前左向前
                Servo_SetAngle(3, 70);  // 后左向后
                Delay_ms(600); // 增加转向时间
                Dog_Stand();
            }
            break;
            
        case AVOID_TURNING:
            // 转向中，显示状态
            OLED_ShowString(1, 1, "STATE: TURNING...");
            LED4_ON(); LED1_OFF(); LED2_OFF(); LED3_OFF();
            break;
    }
}

// 绘制避障雷达界面
void Draw_Avoidance_Radar(float distance, AvoidState state)
{
    // 第3行：距离显示
    OLED_ShowString(3, 1, "Distance:       ");
    if(distance >= 0) {
        OLED_ShowNum(3, 10, (uint16_t)distance, 3);
        OLED_ShowString(3, 13, "cm");
        
        // 在第4行显示舵机4状态
        OLED_ShowString(4, 1, "Servo4: ACTIVE  ");
    } else {
        OLED_ShowString(3, 10, "Error");
        OLED_ShowString(4, 1, "Servo4: US ERROR");
    }
}

// 舵机4健康检查 - 简化版本
void Servo4_Health_Check(void)
{
    static uint32_t last_check_time = 0;
    
    // 每20次循环检查一次舵机4，减少频繁操作
    if(action_counter - last_check_time > 20) {
        last_check_time = action_counter;
        
        // 轻微移动舵机4，检查是否响应
        Safe_Servo4_Move(95.0f);
        Delay_ms(50);
        Safe_Servo4_Move(85.0f);
        Delay_ms(50);
        Safe_Servo4_Move(90.0f);
        
        OLED_ShowString(4, 1, "Servo4: ACTIVE  ");
    }
}

// 超声波传感器诊断
void Ultrasonic_Diagnostic(void)
{
    static uint32_t last_diagnostic = 0;
    
    if(action_counter - last_diagnostic > 15) {
        last_diagnostic = action_counter;
        
        // 显示超声波调试信息
        Ultrasonic_Debug_Info();
        
        // 短暂蜂鸣提示传感器工作
        Buzzer_Beep(20);
    }
}

int main(void)
{
    float distance = 0;
    AvoidState prev_state = AVOID_CLEAR;
    uint8_t ultrasonic_retry = 0;
    
    // 初始化所有外设
    OLED_Init();
    LED_Init();
    Buzzer_Init();
    Ultrasonic_Init();
    Dog_Init(); // 初始化机器狗
    
    // 显示启动界面
    OLED_Clear();
    OLED_ShowString(1, 1, "AUTO AVOID DOG");
    OLED_ShowString(2, 1, "Initializing...");
    
    // 特别测试舵机4 - 简化测试
    OLED_ShowString(3, 1, "Testing Servo4...");
    LED4_ON();
    Safe_Servo4_Move(60);
    Delay_ms(400);
    Safe_Servo4_Move(120);
    Delay_ms(400);
    Safe_Servo4_Move(90);
    LED4_OFF();
    OLED_ShowString(3, 1, "Servo4 Test Done ");
    
    // 启动动画：机器狗站起来
    Dog_Stand();
    Delay_ms(800);
    
    // 超声波传感器测试
    OLED_ShowString(4, 1, "Testing US Sensor");
    distance = Safe_Ultrasonic_GetDistance();
    if(distance > 0) {
        OLED_ShowString(4, 1, "US Sensor: OK   ");
    } else {
        OLED_ShowString(4, 1, "US Sensor: FAIL ");
    }
    Delay_ms(1000);
    
    // 准备提示
    Buzzer_BeepPattern(BEEP_TRIPLE_BEEP);
    OLED_Clear();
    OLED_ShowString(1, 1, "READY TO WALK!");
    OLED_ShowString(3, 1, "Obstacle Avoid");
    OLED_ShowString(4, 1, "Mode: ACTIVE");
    Delay_ms(1500);
    
    OLED_Clear();
    
    while(1)
    {
        // 获取距离数据（使用安全读取函数）
        distance = Safe_Ultrasonic_GetDistance();
        
        // 避障决策
        current_state = Avoidance_Decision(distance);
        
        // 显示雷达界面
        Draw_Avoidance_Radar(distance, current_state);
        
        // 舵机4健康检查（减少频率）
        if(action_counter % 25 == 0) {
            Servo4_Health_Check();
        }
        
        // 超声波诊断（减少频率）
        if(action_counter % 30 == 0) {
            Ultrasonic_Diagnostic();
        }
        
        // 状态变化时执行动作
        if(current_state != prev_state) {
            Execute_Avoidance_Action(current_state);
            prev_state = current_state;
        }
        else {
            // 状态未变化，根据当前状态执行相应动作
            Execute_Avoidance_Action(current_state);
        }
        
        // 控制行动节奏 - 给电源恢复时间
        Delay_ms(1200); // 增加延时，减少动作频率
    }
}

// // 避障状态定义
// typedef enum {
//     AVOID_CLEAR = 0,    // 前方无障碍
//     AVOID_WARNING,      // 前方有障碍，警告
//     AVOID_DANGER,       // 前方障碍很近，需要转向
//     AVOID_TURNING       // 正在转向避障
// } AvoidState;

// // 全局变量
// static AvoidState current_state = AVOID_CLEAR;
// static uint32_t action_counter = 0;

// // 避障决策函数
// AvoidState Avoidance_Decision(float distance)
// {
//     if(distance < 0) {
//         return AVOID_CLEAR; // 传感器错误时默认安全
//     }
//     else if(distance < 15.0) {
//         return AVOID_DANGER; // 15cm内：危险，立即转向
//     }
//     else if(distance < 30.0) {
//         return AVOID_WARNING; // 30cm内：警告，准备转向
//     }
//     else {
//         return AVOID_CLEAR; // 30cm外：安全，继续前进
//     }
// }

// // 执行避障动作
// void Execute_Avoidance_Action(AvoidState state)
// {
//     action_counter++;
    
//     switch(state) {
//         case AVOID_CLEAR:
//             // 安全区域，前进
//             OLED_ShowString(1, 1, "STATE: WALKING   ");
//             LED3_ON(); LED1_OFF(); LED2_OFF(); LED4_OFF();
//             Dog_WalkForward(1); // 前进1步
//             break;
            
//         case AVOID_WARNING:
//             // 警告区域，原地等待
//             OLED_ShowString(1, 1, "STATE: SLOW DOWN ");
//             LED2_ON(); LED1_OFF(); LED3_OFF(); LED4_OFF();
//             Buzzer_Beep(50); // 警告音
//             Dog_Stand(); // 保持站立，等待
//             Delay_ms(500);
//             break;
            
//         case AVOID_DANGER:
//             // 危险区域，停止并转向
//             OLED_ShowString(1, 1, "STATE: TURNING   ");
//             LED1_ON(); LED2_OFF(); LED3_OFF(); LED4_OFF();
//             Buzzer_BeepPattern(BEEP_DOUBLE_BEEP); // 紧急提示音
            
//             // 随机选择左转或右转（根据计数器奇偶）
//             if((action_counter % 2) == 0) {
//                 OLED_ShowString(2, 1, "ACTION: TURN LEFT ");
//                 Dog_TurnLeft(1); // 左转1步
//             } else {
//                 OLED_ShowString(2, 1, "ACTION: TURN RIGHT");
//                 Dog_TurnRight(1); // 右转1步
//             }
//             break;
            
//         case AVOID_TURNING:
//             // 转向中，显示状态
//             OLED_ShowString(1, 1, "STATE: TURNING...");
//             LED4_ON(); LED1_OFF(); LED2_OFF(); LED3_OFF();
//             break;
//     }
// }

// // 绘制避障雷达界面
// void Draw_Avoidance_Radar(float distance, AvoidState state)
// {
//     // 第3行：距离显示
//     OLED_ShowString(3, 1, "Distance:       ");
//     if(distance >= 0) {
//         OLED_ShowNum(3, 10, (uint16_t)distance, 3);
//         OLED_ShowString(3, 13, "cm");
//     } else {
//         OLED_ShowString(3, 10, "Error");
//     }
    
//     // 第4行：安全区域指示
//     OLED_ShowString(4, 1, "                ");
//     if(state == AVOID_CLEAR) {
//         OLED_ShowString(4, 1, "[SAFE ZONE]     ");
//     } else if(state == AVOID_WARNING) {
//         OLED_ShowString(4, 1, "[!WARNING!]     ");
//     } else {
//         OLED_ShowString(4, 1, "[!DANGER!]      ");
//     }
// }

// int main(void)
// {
//     float distance = 0;
//     AvoidState prev_state = AVOID_CLEAR;
    
//     // 初始化所有外设
//     OLED_Init();
//     Ultrasonic_Init();
//     LED_Init();
//     Buzzer_Init();
//     Dog_Init(); // 初始化机器狗
    
//     // 显示启动界面
//     OLED_Clear();
//     OLED_ShowString(1, 1, "AUTO AVOID DOG");
//     OLED_ShowString(2, 1, "Initializing...");
    
//     // 启动动画：机器狗站起来
//     Dog_Stand();
//     Delay_ms(1000);
    
//     // 准备提示
//     Buzzer_BeepPattern(BEEP_TRIPLE_BEEP);
//     OLED_Clear();
//     OLED_ShowString(1, 1, "READY TO WALK!");
//     OLED_ShowString(3, 1, "Obstacle Avoid");
//     OLED_ShowString(4, 1, "Mode: ACTIVE");
//     Delay_ms(2000);
    
//     OLED_Clear();
    
//     while(1)
//     {
//         // 获取距离数据
//         distance = Ultrasonic_GetDistance();
        
//         // 避障决策
//         current_state = Avoidance_Decision(distance);
        
//         // 显示雷达界面
//         Draw_Avoidance_Radar(distance, current_state);
        
//         // 状态变化时执行动作
//         if(current_state != prev_state) {
//             Execute_Avoidance_Action(current_state);
//             prev_state = current_state;
//         }
//         else {
//             // 状态未变化，根据当前状态执行相应动作
//             Execute_Avoidance_Action(current_state);
//         }
        
//         // 控制行动节奏
//         Delay_ms(800); // 增加延时，让动作更稳定
//     }
// }

// // 避障状态定义
// typedef enum {
//     AVOID_CLEAR = 0,    // 前方无障碍
//     AVOID_WARNING,      // 前方有障碍，警告
//     AVOID_DANGER,       // 前方障碍很近，需要转向
//     AVOID_TURNING       // 正在转向避障
// } AvoidState;

// // 全局变量
// static AvoidState current_state = AVOID_CLEAR;
// static uint32_t last_action_time = 0;

// // 避障决策函数
// AvoidState Avoidance_Decision(float distance)
// {
//     if(distance < 0) {
//         return AVOID_CLEAR; // 传感器错误时默认安全
//     }
//     else if(distance < 15.0) {
//         return AVOID_DANGER; // 15cm内：危险，立即转向
//     }
//     else if(distance < 30.0) {
//         return AVOID_WARNING; // 30cm内：警告，准备转向
//     }
//     else {
//         return AVOID_CLEAR; // 30cm外：安全，继续前进
//     }
// }

// // 执行避障动作
// void Execute_Avoidance_Action(AvoidState state)
// {
//     uint32_t current_time = 0; // 这里应该用系统时钟，简化用计数器
//     static uint32_t time_counter = 0;
//     time_counter++;
    
//     switch(state) {
//         case AVOID_CLEAR:
//             // 安全区域，前进
//             OLED_ShowString(1, 1, "STATE: WALKING   ");
//             LED3_ON(); LED1_OFF(); LED2_OFF(); LED4_OFF();
//             break;
            
//         case AVOID_WARNING:
//             // 警告区域，减速前进
//             OLED_ShowString(1, 1, "STATE: SLOW DOWN ");
//             LED2_ON(); LED1_OFF(); LED3_OFF(); LED4_OFF();
//             Buzzer_Beep(50); // 警告音
//             break;
            
//         case AVOID_DANGER:
//             // 危险区域，停止并转向
//             OLED_ShowString(1, 1, "STATE: TURNING   ");
//             LED1_ON(); LED2_OFF(); LED3_OFF(); LED4_OFF();
//             Buzzer_BeepPattern(BEEP_DOUBLE_BEEP); // 紧急提示音
            
//             // 随机选择左转或右转（简单实现：根据时间计数奇偶）
//             if((time_counter % 2) == 0) {
//                 OLED_ShowString(2, 1, "ACTION: TURN LEFT ");
//                 Dog_TurnLeft(2); // 左转2步
//             } else {
//                 OLED_ShowString(2, 1, "ACTION: TURN RIGHT");
//                 Dog_TurnRight(2); // 右转2步
//             }
//             break;
            
//         case AVOID_TURNING:
//             // 转向中，显示状态
//             OLED_ShowString(1, 1, "STATE: TURNING...");
//             LED4_ON(); LED1_OFF(); LED2_OFF(); LED3_OFF();
//             break;
//     }
// }

// // 绘制避障雷达界面
// void Draw_Avoidance_Radar(float distance, AvoidState state)
// {
//     // 第3行：距离显示
//     OLED_ShowString(3, 1, "Distance:       ");
//     if(distance >= 0) {
//         OLED_ShowNum(3, 10, (uint16_t)distance, 3);
//         OLED_ShowString(3, 13, "cm");
//     } else {
//         OLED_ShowString(3, 10, "Error");
//     }
    
//     // 第4行：安全区域指示
//     OLED_ShowString(4, 1, "                ");
//     if(state == AVOID_CLEAR) {
//         OLED_ShowString(4, 1, "[SAFE ZONE]     ");
//     } else if(state == AVOID_WARNING) {
//         OLED_ShowString(4, 1, "[!WARNING!]     ");
//     } else {
//         OLED_ShowString(4, 1, "[!DANGER!]      ");
//     }
// }

// int main(void)
// {
//     float distance = 0;
//     AvoidState prev_state = AVOID_CLEAR;
    
//     // 初始化所有外设
//     OLED_Init();
//     Ultrasonic_Init();
//     LED_Init();
//     Buzzer_Init();
//     Dog_Init(); // 初始化机器狗
    
//     // 显示启动界面
//     OLED_Clear();
//     OLED_ShowString(1, 1, "AUTO AVOID DOG");
//     OLED_ShowString(2, 1, "Initializing...");
    
//     // 启动动画：机器狗站起来
//     Dog_Stand();
//     Delay_ms(1000);
    
//     // 准备提示
//     Buzzer_BeepPattern(BEEP_TRIPLE_BEEP);
//     OLED_Clear();
//     OLED_ShowString(1, 1, "READY TO WALK!");
//     OLED_ShowString(3, 1, "Obstacle Avoid");
//     OLED_ShowString(4, 1, "Mode: ACTIVE");
//     Delay_ms(2000);
    
//     OLED_Clear();
    
//     while(1)
//     {
//         // 获取距离数据
//         distance = Ultrasonic_GetDistance();
        
//         // 避障决策
//         current_state = Avoidance_Decision(distance);
        
//         // 显示雷达界面
//         Draw_Avoidance_Radar(distance, current_state);
        
//         // 状态变化时执行动作
//         if(current_state != prev_state) {
//             Execute_Avoidance_Action(current_state);
//             prev_state = current_state;
//         }
        
//         // 根据状态控制机器狗行为
//         if(current_state == AVOID_CLEAR) {
//             // 安全区域，前进1步
//             OLED_ShowString(2, 1, "ACTION: WALK FORWARD");
//             Dog_WalkForward(1);
//         }
//         else if(current_state == AVOID_WARNING) {
//             // 警告区域，原地踏步（等待）
//             OLED_ShowString(2, 1, "ACTION: WAITING    ");
//             Dog_Stand(); // 保持站立
//             Delay_ms(500);
//         }
//         // AVOID_DANGER状态在Execute_Avoidance_Action中已经处理了转向
        
//         // 短暂延时，控制行动节奏
//         Delay_ms(300);
//     }
// }

// 避障显示
// // 绘制距离条形图
// void DrawDistanceBar(float distance)
// {
//     // 在OLED上绘制一个简单的距离条
//     // 清除条形图区域 (第3行)
//     OLED_ShowString(3, 1, "                ");
    
//     if(distance >= 0 && distance <= 100) {
//         // 计算条形长度 (1-16个字符)
//         uint8_t bar_length = (uint8_t)(16 - (distance / 100.0 * 16));
//         if(bar_length < 1) bar_length = 1;
//         if(bar_length > 16) bar_length = 16;
        
//         // 绘制条形
//         for(uint8_t i = 1; i <= bar_length; i++) {
//             OLED_ShowChar(3, i, '|');
//         }
        
//         // 在条形后面显示距离数值
//         OLED_ShowNum(3, 17, (uint16_t)distance, 3);
//         OLED_ShowString(3, 20, "cm");
//     }
// }

// // 绘制雷达式扫描效果
// void DrawRadarScan(float distance, uint8_t scan_pos)
// {
//     // 在第4行绘制简单的雷达扫描效果
//     OLED_ShowString(4, 1, "                ");
    
//     // 扫描指针位置 (1-16)
//     uint8_t pointer_pos = (scan_pos % 16) + 1;
    
//     // 如果有障碍物在扫描位置附近，显示特殊标记
//     if(distance >= 0 && distance < 50.0) {
//         // 根据距离显示不同标记
//         if(distance < 10.0) {
//             OLED_ShowChar(4, pointer_pos, 'X'); // 很近，显示X
//         } else if(distance < 25.0) {
//             OLED_ShowChar(4, pointer_pos, 'o'); // 中等距离，显示o
//         } else {
//             OLED_ShowChar(4, pointer_pos, '.'); // 较远，显示.
//         }
//     } else {
//         // 无障碍物或距离很远，显示扫描线
//         OLED_ShowChar(4, pointer_pos, '-');
//     }
// }

// // 声音提示函数
// void DistanceSoundAlert(float distance)
// {
//     if(distance >= 0 && distance < 10.0) {
//         // 很近：急促警告声
//         Buzzer_Beep(100);
//         Delay_ms(100);
//     } else if(distance >= 10.0 && distance < 25.0) {
//         // 中等距离：单次提示
//         Buzzer_Beep(50);
//     }
//     // 远距离不发声
// }

// int main(void)
// {
//     float distance = 0;
//     uint8_t scan_counter = 0;
//     uint8_t update_display = 1;
    
//     // 初始化所有外设
//     OLED_Init();
//     Ultrasonic_Init();
//     LED_Init();
//     Buzzer_Init();
    
//     // 显示酷炫的启动界面
//     OLED_Clear();
//     OLED_ShowString(1, 1, "== RADAR DOG ==");
//     OLED_ShowString(2, 1, "Initializing...");
    
//     // 启动动画
//     for(int i = 0; i < 3; i++) {
//         OLED_ShowString(4, 1, ">");
//         Delay_ms(150);
//         OLED_ShowString(4, 2, ">");
//         Delay_ms(150);
//         OLED_ShowString(4, 3, ">");
//         Delay_ms(150);
//         OLED_ShowString(4, 1, "   ");
//         Delay_ms(150);
//     }
    
//     OLED_Clear();
    
//     // 显示静态界面
//     OLED_ShowString(1, 1, "RADAR ACTIVE");
//     OLED_ShowString(2, 1, "Dist:");
    
//     // 开机提示音
//     Buzzer_BeepPattern(BEEP_DOUBLE_BEEP);
    
//     while(1)
//     {
//         // 获取距离数据
//         distance = Ultrasonic_GetDistance();
        
//         // 更新扫描计数器
//         scan_counter++;
//         if(scan_counter >= 32) scan_counter = 0;
        
//         // 在OLED第2行显示精确距离
//         OLED_ShowString(2, 7, "      "); // 清空旧数据
//         if(distance >= 0) {
//             // 显示距离数值
//             OLED_ShowNum(2, 7, (uint16_t)distance, 3);
//             OLED_ShowString(2, 10, ".");
//             // 显示小数部分
//             uint8_t decimal = (uint8_t)((distance - (uint16_t)distance) * 10);
//             OLED_ShowNum(2, 11, decimal, 1);
//             OLED_ShowString(2, 12, "cm");
//         } else {
//             OLED_ShowString(2, 7, "Error!");
//         }
        
//         // 绘制距离条形图
//         DrawDistanceBar(distance);
        
//         // 绘制雷达扫描效果
//         DrawRadarScan(distance, scan_counter);
        
//         // LED状态指示
//         LED1_OFF(); LED2_OFF(); LED3_OFF(); LED4_OFF();
//         if(distance >= 0) {
//             if(distance < 10.0) {
//                 LED1_ON(); // 红色警报：很近
//                 OLED_ShowString(1, 1, "!DANGER CLOSE!");
//             } else if(distance < 25.0) {
//                 LED2_ON(); // 黄色警告：中等
//                 OLED_ShowString(1, 1, "CAUTION NEAR ");
//             } else if(distance < 50.0) {
//                 LED3_ON(); // 绿色：安全
//                 OLED_ShowString(1, 1, "SAFE         ");
//             } else {
//                 LED4_ON(); // 蓝色：很远
//                 OLED_ShowString(1, 1, "CLEAR        ");
//             }
//         } else {
//             // 错误状态：所有LED闪烁
//             LED1_ON(); LED2_ON(); LED3_ON(); LED4_ON();
//             OLED_ShowString(1, 1, "SENSOR ERROR!");
//         }
        
//         // 声音提示
//         DistanceSoundAlert(distance);
        
//         // 测量间隔 (快速更新以获得流畅的雷达效果)
//         Delay_ms(100);
//     }
// }

// OLED测试
// int main(void)
// {

//     // 开机提示：LED闪烁一下
//     LED1_ON(); LED2_ON(); LED3_ON(); LED4_ON();
//     Delay_ms(300);
//     LED1_OFF(); LED2_OFF(); LED3_OFF(); LED4_OFF();

//     OLED_Init();
//     OLED_ShowString(1, 1, "PB6/PB7 Test OK!");
//     OLED_ShowString(2, 1, "SCL->PB6");
//     OLED_ShowString(3, 1, "SDA->PB7"); 
//     OLED_ShowString(4, 1, "Ultrasonic Ready");
    

//     while(1) {
//         // 闪烁证明程序运行
//         OLED_ShowString(4, 1, "Running...");
//         Delay_ms(500);
//         OLED_ShowString(4, 1, "           ");
//         Delay_ms(500);
//     }
// }

// 超声波模块测试
// int main(void)
// {
//     float distance = 0;
    
//     // 初始化
//     LED_Init();
//     Ultrasonic_Init();
    
//     // 开机提示：所有LED闪一下
//     LED1_ON(); LED2_ON(); LED3_ON(); LED4_ON();
//     Delay_ms(500);
//     LED1_OFF(); LED2_OFF(); LED3_OFF(); LED4_OFF();
//     Delay_ms(500);
    
//     while(1)
//     {
//         // 获取距离
//         distance = Ultrasonic_GetDistance();
        
//         // 先关闭所有LED
//         LED1_OFF();
//         LED2_OFF(); 
//         LED3_OFF();
//         LED4_OFF();
        
//         // 根据距离点亮不同的LED
//         if(distance < 0) {
//             // 测量错误，D4闪烁
//             LED4_ON();
//         }
//         else if(distance < 10.0) {
//             // 很近，D1亮
//             LED1_ON();
//         }
//         else if(distance < 30.0) {
//             // 中等距离，D2亮
//             LED2_ON();
//         }
//         else if(distance < 50.0) {
//             // 较远，D3亮
//             LED3_ON();
//         }
//         else {
//             // 很远，D4亮
//             LED4_ON();
//         }
        
//         // 每隔300ms测量一次
//         Delay_ms(300);
//     }
// }

// 行走
// void System_Init(void)
// {
//     LED_Init();
//     Key_Init();
//     Buzzer_Init();
//     OLED_Init();
//     ControlSystem_Init();
    
//     OLED_Clear();
//     OLED_ShowString(1, 1, "Smart Dog V1.2");
//     OLED_ShowString(2, 1, "State: IDLE");
//     OLED_ShowString(3, 1, "K1:Stand K2:Sit");
//     OLED_ShowString(4, 1, "K3:Walk K4:Turn");

//     Buzzer_BeepPattern(BEEP_DOUBLE_BEEP);
    
//     LED1_ON(); Delay_ms(200); LED1_OFF();
//     LED2_ON(); Delay_ms(200); LED2_OFF();
//     LED3_ON(); Delay_ms(200); LED3_OFF();
//     LED4_ON(); Delay_ms(200); LED4_OFF();                                                          
// }

// int main(void)
// {
//     System_Init();
    
//     while(1)
//     {
//         uint8_t key = Key_GetNum();
                               
//         switch(key)
//         {
//             case 1: // K1: 站立
//                 Control_Stand();
//                 break;
                
//             case 2: // K2: 坐下
//                 Control_Sit();
//                 break;
                
//             case 3: // K3: 前进2步
//                 Control_WalkForward(2);
//                 break;
                
//             case 4: // K4: 左转1步
//                 Control_TurnLeft(1);
//                 break;
//         }
        
//         Delay_ms(10);
//     }
// }


// // 灯亮
// int main(void)
// {
//     // 系统初始化
//     SysTick_Init(72);
    
//     // LED初始化
//     LED_Init();
    
//     while(1)
//     {
//         // LED1亮，其他灭
//         LED1_ON();
//         LED2_OFF();
//         LED3_OFF();
//         LED4_OFF();
//         Delay_ms(500);
        
//         // LED2亮，其他灭
//         LED1_OFF();
//         LED2_ON();
//         LED3_OFF();
//         LED4_OFF();
//         Delay_ms(500);
        
//         // LED3亮，其他灭
//         LED1_OFF();
//         LED2_OFF();
//         LED3_ON();
//         LED4_OFF();
//         Delay_ms(500);
        
//         // LED4亮，其他灭
//         LED1_OFF();
//         LED2_OFF();
//         LED3_OFF();
//         LED4_ON();
//         Delay_ms(500);
//     }
// }

// uint8_t i;
// uint8_t KeyNum;
// float Angle=0;

// 舵机旋转
// int main(void)
// {
// 	// 1. 初始化所有舵机 (这会调用PWM_Init)
// 	Servo_Init();
// 	SysTick_Init(72);
// 	// OLED_Init();
// 	// Key_Init();

// 	// 延时一小会，等待舵机上电稳定
// 	Delay_ms(500);

// 	while (1)
// 	{
// 		// --- 动作一: 所有舵机转到 0 度 ---
// 		Servo_SetAngle(1, 0);
// 		Servo_SetAngle(2, 0);
// 		Servo_SetAngle(3, 0);
// 		Servo_SetAngle(4, 0);
// 		Delay_ms(1000); // 等待1秒
		
// 		// --- 动作二: 所有舵机转到 90 度 (中间位置) ---
// 		Servo_SetAngle(1, 90);
// 		Servo_SetAngle(2, 90);
// 		Servo_SetAngle(3, 90);
// 		Servo_SetAngle(4, 90);
// 		Delay_ms(1000); // 等待1秒
		
// 		// --- 动作三: 所有舵机转到 180 度 ---
// 		Servo_SetAngle(1, 180);
// 		Servo_SetAngle(2, 180);
// 		Servo_SetAngle(3, 180);
// 		Servo_SetAngle(4, 180);
// 		Delay_ms(1000); // 等待1秒
// 	}
// }

// h9舵机转动
// int main(void)
// {
//     SysTick_Init(72);
// 	OLED_Init();
// 	Servo_Init();
// 	Key_Init();
// 	Servo_SetAngle(50);
// 	PWM_Init();
// 	while (1)
// 	{
		
// 		KeyNum = Key_GetNum();
// 		if(KeyNum==1)
// 		{
// 			Angle += 15;
// 			if(Angle > 180)
// 			{
// 				Angle = 0;
// 			}
// 		}
// 		if(KeyNum==2)
// 		{
// 			Angle -= 15;
// 			if(Angle <= 0)
// 			{
// 				Angle = 180;
// 			}
// 		}
// 		Servo_SetAngle(Angle);
		

// 		for (i = 0; i <= 100; i++)
// 		{
// 			PWM_SetCompare4(i);
// 			Delay_ms(10);
// 		}
// 		for (i = 0; i <= 100; i++)
// 		{
// 			PWM_SetCompare4(100 - i);
// 			Delay_ms(10);
// 		}
// 	}
// }


// int main(void)
// {
//     // 初始化SysTick，用于精确延时
//     SysTick_Init(75); 

//     // 初始化PWM，传入我们计算好的参数，得到50Hz频率
//     // arr = 19999, psc = 71
//     TIMx_PWM_Init(19999, 71); 
    
//     while(1)
//     {
//         // 命令舵机转到0度 (脉冲宽度值 500)
//         TIM_Set_Pulse(TIM2, 2, 500);
//         delay_ms(1000); // 延时1秒
        
//         // 命令舵机转到90度 (脉冲宽度值 1500)
//         TIM_Set_Pulse(TIM2, 2, 1500);
//         delay_ms(1000); // 延时1秒
        
//         // 命令舵机转到180度 (脉冲宽度值 2500)
//         TIM_Set_Pulse(TIM2, 2, 2500);
//         delay_ms(1000); // 延时1秒
//     }
// }

// int main(void)
// {	

	
// 	while(1);
// }
