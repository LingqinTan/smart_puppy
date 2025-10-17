#include "stm32f10x.h"
#include "LED.h"
#include "Key.h"
// #include "Buzzer.h"
#include "OLED.h"
#include "DogActions.h"
#include "Delay.h"
#include "ControlSystem.h"
// #include "BluetoothControl.h"
#include "Ultrasonic.h"

// -----------------------------------------------------------------
// 步骤 2.1: 定义系统模式 (不变)
// -----------------------------------------------------------------
typedef enum {
    MODE_IDLE = 0, MODE_AVOIDANCE, MODE_ACTION_HELLO, MODE_ACTION_SIT, MODE_ACTION_SHAKE
} SystemMode;

static SystemMode current_mode = MODE_IDLE;
static uint8_t key_pressed = 0;

// -----------------------------------------------------------------
// 步骤 2.2: 移植你原有的避障逻辑 (不变)
// -----------------------------------------------------------------
typedef enum {
    AVOID_CLEAR = 0, AVOID_WARNING, AVOID_DANGER, AVOID_TURNING
} AvoidState;

static uint32_t action_counter = 0;

void Safe_Servo4_Move(float angle)
{
    if(angle < 45.0f) angle = 45.0f;
    if(angle > 135.0f) angle = 135.0f;
    Servo_SetAngle(4, angle);
    Delay_ms(5); // 5ms延时很短，无需修改
}


// -----------------------------------------------------------------
// 步骤 2.3: 💥 关键修正 - 可中断的延时 💥
// (这是我们解决K4失灵问题的核心函数)
// -----------------------------------------------------------------

/**
 * @brief  可被 K4 中断的延时函数 (非阻塞式延时)
 * @param  delay_ms: 总延时时间 (毫秒)
 * @return 1: 如果延时被 K4 中断; 0: 如果延时正常完成
 * @note   K4 (Key_GetNum() == 4) 被定义为全局“紧急停止”键
 */
uint8_t Delay_ms_Interruptible(uint32_t delay_ms)
{
    // 我们将总延时拆分成 N 个 20ms 的小块
    // 20ms 的颗粒度足以快速响应按键
    uint32_t chunk_delay = 20; 
    uint32_t num_chunks = delay_ms / chunk_delay;
    uint32_t remainder_delay = delay_ms % chunk_delay;
    
    for (uint32_t i = 0; i < num_chunks; i++)
    {
        Delay_ms(chunk_delay); // 执行一小块延时
        
        // 检查“紧急电话” K4
        if (Key_GetNum() == 4) 
        {
            OLED_Clear();
            OLED_ShowString(1, 1, "Mode -> IDLE");
            Dog_Stand(); // 强制站立
            current_mode = MODE_IDLE; // 强制切换到 IDLE 模式
            Delay_ms(500); // 短暂显示提示 (这里用普通延时，因为它很短)
            OLED_Clear();
            return 1; // 返回 1，代表“被中断了”
        }
    }
    
    // 处理剩余的不足 20ms 的延时
    if (remainder_delay > 0)
    {
        Delay_ms(remainder_delay);
    }
    
    return 0; // 返回 0，代表“正常结束”
}


// -----------------------------------------------------------------
// 步骤 2.4: 💥 关键修正 - 修正避障函数 💥
// (我们将用 Delay_ms_Interruptible 替换所有长延时)
// -----------------------------------------------------------------

// 改进的超声波读取函数
float Safe_Ultrasonic_GetDistance(void)
{
    float distance = 0;
    uint8_t retry_count = 0;
    for(retry_count = 0; retry_count < 3; retry_count++) {
        distance = Ultrasonic_GetDistance();
        if(distance > 0 && distance < 500) {
            return distance;
        }
        // Delay_ms(50); // <-- 原来的阻塞代码
        if (Delay_ms_Interruptible(50)) return -1; // <-- 修正：使用可中断延时
    }
    return -1; 
}

// 避障决策函数 (不变)
AvoidState Avoidance_Decision(float distance)
{
    if(distance < 0)    return AVOID_CLEAR;
    else if(distance < 15.0) return AVOID_DANGER; 
    else if(distance < 30.0) return AVOID_WARNING;
    else return AVOID_CLEAR;
}

// 绘制避障雷达界面 (不变)
void Draw_Avoidance_Radar(float distance, AvoidState state)
{
    OLED_ShowString(3, 1, "Distance:       ");
    if(distance >= 0) {
        OLED_ShowNum(3, 10, (uint16_t)distance, 3);
        OLED_ShowString(3, 13, "cm");
    } else {
        OLED_ShowString(3, 10, "Error");
    }
}

// 修正：执行避障动作
void Execute_Avoidance_Action(AvoidState state)
{
    action_counter++;
    
    switch(state) {
        case AVOID_CLEAR:
            OLED_ShowString(1, 1, "STATE: WALKING   ");
            LED3_ON(); LED1_OFF(); LED2_OFF(); LED4_OFF();
            Dog_Stand(); 
            // Delay_ms(200); // <-- 原来的阻塞代码
            if (Delay_ms_Interruptible(200)) return; // <-- 修正
            
            Safe_Servo4_Move(100); Servo_SetAngle(1, 80); 
            Servo_SetAngle(2, 100); Servo_SetAngle(3, 80);
            // Delay_ms(300); // <-- 原来的阻塞代码
            if (Delay_ms_Interruptible(300)) return; // <-- 修正
            Dog_Stand();
            break;
            
        case AVOID_WARNING:
            OLED_ShowString(1, 1, "STATE: SLOW DOWN ");
            LED2_ON(); LED1_OFF(); LED3_OFF(); LED4_OFF();
            Dog_Stand(); 
            // Delay_ms(800); // <-- 原来的阻塞代码
            if (Delay_ms_Interruptible(800)) return; // <-- 修正
            break;
            
        case AVOID_DANGER:
            OLED_ShowString(1, 1, "STATE: TURNING   ");
            LED1_ON(); LED2_OFF(); LED3_OFF(); LED4_OFF();
            
            if((action_counter % 2) == 0) {
                OLED_ShowString(2, 1, "ACTION: TURN LEFT ");
                Safe_Servo4_Move(70); Servo_SetAngle(1, 110); 
                Servo_SetAngle(2, 70); Servo_SetAngle(3, 110); 
                // Delay_ms(600); // <-- 原来的阻塞代码
                if (Delay_ms_Interruptible(600)) return; // <-- 修正
                Dog_Stand();
            } else {
                OLED_ShowString(2, 1, "ACTION: TURN RIGHT");
                Safe_Servo4_Move(110); Servo_SetAngle(1, 70);
                Servo_SetAngle(2, 110); Servo_SetAngle(3, 70); 
                // Delay_ms(600); // <-- 原来的阻塞代码
                if (Delay_ms_Interruptible(600)) return; // <-- 修正
                Dog_Stand();
            }
            break;
            
        case AVOID_TURNING:
            OLED_ShowString(1, 1, "STATE: TURNING...");
            LED4_ON(); LED1_OFF(); LED2_OFF(); LED3_OFF();
            break;
    }
}

// -----------------------------------------------------------------
// 步骤 2.5: 修正模式调度器
// -----------------------------------------------------------------

// 模式: 空闲待机 (不变)
void Mode_Idle_Loop(void)
{
    OLED_ShowString(1, 1, "== SMART PUPPY ==");
    OLED_ShowString(2, 1, "MODE: IDLE      ");
    OLED_ShowString(3, 1, "K1: AVOIDANCE   ");
    OLED_ShowString(4, 1, "K2: HELLO K3:SIT");
    Dog_Stand();
    Delay_ms(100); // IDLE模式下的延时很短，不需要修改
}

// 修正：自主避障
void Mode_Avoidance_Loop(void)
{
    float distance = Safe_Ultrasonic_GetDistance();
    // 检查在测距时是否被中断
    if (current_mode == MODE_IDLE) return; 

    AvoidState new_state = Avoidance_Decision(distance);
    Draw_Avoidance_Radar(distance, new_state);
    
    // 执行动作
    Execute_Avoidance_Action(new_state);
    // 检查在执行动作时是否被中断
    if (current_mode == MODE_IDLE) return; 

    // 控制行动节奏
    // Delay_ms(1200); // <-- 原来的阻塞代码
    if (Delay_ms_Interruptible(1200)) return; // <-- 修正
}

// 模式: 执行一次 "你好" (不变)
void Mode_Action_Hello_Once(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "== ACTION ==");
    OLED_ShowString(2, 1, "   Hello !   ");
    LED1_ON(); LED2_ON();
    
    Dog_Action_Hello(); // (这个动作里面的延时很短，暂时不改)
    
    LED1_OFF(); LED2_OFF();
    OLED_Clear();
    current_mode = MODE_IDLE; 
}

// 模式: 执行一次 "坐下" (不变)
void Mode_Action_Sit_Once(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "== ACTION ==");
    OLED_ShowString(2, 1, "   Sit Down   ");
    LED3_ON(); LED4_ON();
    
    Dog_Action_SitDown(); 
    // Delay_ms(1000); // <-- 原来的阻塞代码
    if (Delay_ms_Interruptible(1000)) return; // <-- 修正
    
    LED3_OFF(); LED4_OFF();
    OLED_Clear();
    current_mode = MODE_IDLE;
}

// -----------------------------------------------------------------
// 步骤 2.6: 💥 关键修正 - 检查按键输入 💥
// -----------------------------------------------------------------
void Check_Key_Input(void)
{
    key_pressed = Key_GetNum(); 
    
    if (key_pressed)
    {
        OLED_Clear(); 
        
        switch (key_pressed)
        {
            case 1: 
                OLED_ShowString(1, 1, "Mode -> AVOIDANCE");
                current_mode = MODE_AVOIDANCE;
                action_counter = 0;
                break;
                
            case 2: 
                OLED_ShowString(1, 1, "Mode -> HELLO");
                current_mode = MODE_ACTION_HELLO;
                break;
                
            case 3: 
                OLED_ShowString(1, 1, "Mode -> SIT");
                current_mode = MODE_ACTION_SIT;
                break;
                
            case 4: // K4 仍然在这里作为标准切换方式
                OLED_ShowString(1, 1, "Mode -> IDLE");
                Dog_Stand(); 
                current_mode = MODE_IDLE;
                break;
        }
        // Delay_ms(1000); // <-- 错误！这个延时本身也会阻塞IDLE模式的响应
        Delay_ms(300); // <-- 修正：缩短延时，仅用于按键消抖和短暂提示
        OLED_Clear();
    }
}


// -----------------------------------------------------------------
// 步骤 2.7: 最终的主函数 (main)
// (修正了启动过程中的延时)
// -----------------------------------------------------------------
int main(void)
{
    float distance = 0;
    
    // 初始化所有外设
    OLED_Init();
    LED_Init();
    Key_Init();
    Ultrasonic_Init();
    Dog_Init(); 
    
    OLED_Clear();
    OLED_ShowString(1, 1, "AUTO AVOID DOG");
    OLED_ShowString(2, 1, "Initializing...");
    
    // 舵机测试
    OLED_ShowString(3, 1, "Testing Servo4...");
    LED4_ON();
    Safe_Servo4_Move(60);
    if (Delay_ms_Interruptible(400)) goto START_MAIN_LOOP; // 如果中断，直接跳到主循环
    Safe_Servo4_Move(120);
    if (Delay_ms_Interruptible(400)) goto START_MAIN_LOOP;
    Safe_Servo4_Move(90);
    LED4_OFF();
    OLED_ShowString(3, 1, "Servo4 Test Done ");
    
    Dog_Stand(); 
    if (Delay_ms_Interruptible(800)) goto START_MAIN_LOOP;
    
    // 超声波测试
    OLED_ShowString(4, 1, "Testing US Sensor");
    distance = Safe_Ultrasonic_GetDistance();
    if(distance > 0) {
        OLED_ShowString(4, 1, "US Sensor: OK   ");
    } else {
        OLED_ShowString(4, 1, "US Sensor: FAIL ");
    }
    if (Delay_ms_Interruptible(1000)) goto START_MAIN_LOOP;
    
    OLED_Clear();
    OLED_ShowString(1, 1, "READY!");
    OLED_ShowString(3, 1, "Mode: IDLE");
    if (Delay_ms_Interruptible(1500)) goto START_MAIN_LOOP;
    
    OLED_Clear();
    
START_MAIN_LOOP: // 标签，用于跳过启动
    current_mode = MODE_IDLE; // 无论如何，都以 IDLE 模式启动
    
    while(1)
    {
        // 1. 每次循环都先检查按键
        // (因为K4的中断逻辑已包含在 Delay_ms_Interruptible 中,
        //  所以这里的 Check_Key_Input 主要是为了在 IDLE 模式下切换)
        Check_Key_Input();
        
        // 2. 根据当前模式，执行对应的任务
        switch(current_mode)
        {
            case MODE_IDLE:
                Mode_Idle_Loop();
                break;
                
            case MODE_AVOIDANCE:
                Mode_Avoidance_Loop();
                break;
                
            case MODE_ACTION_HELLO:
                Mode_Action_Hello_Once();
                break;
                
            case MODE_ACTION_SIT:
                Mode_Action_Sit_Once();
                break;
                
            default:
                current_mode = MODE_IDLE;
                break;
        }
    }
}
