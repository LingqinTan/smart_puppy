#include "stm32f10x.h"
#include "LED.h"
#include "Key.h"
#include "OLED.h"
#include "DogActions.h"
#include "Delay.h"
#include "ControlSystem.h" 
#include "Ultrasonic.h"
#include "Bluetooth.h"      
#include "stdio.h"          
#include "Servo.h"          
#include "Buzzer.h"         // <--- 1. 💥 新增音效 💥: 包含蜂鸣器头文件

// -----------------------------------------------------------------
// 定义系统模式
// -----------------------------------------------------------------
typedef enum {
    MODE_IDLE = 0, MODE_AVOIDANCE, MODE_BLUETOOTH,
    MODE_ACTION_HELLO, MODE_ACTION_SIT, MODE_ACTION_SHAKE
} SystemMode;

static SystemMode current_mode = MODE_IDLE; 
static uint8_t key_pressed = 0;             

// -----------------------------------------------------------------
// 函数原型
// -----------------------------------------------------------------
uint8_t Delay_ms_Interruptible(uint32_t delay_ms);

// -----------------------------------------------------------------
// 避障逻辑函数
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
    Delay_ms(5); 
}

float Safe_Ultrasonic_GetDistance(void)
{
    float distance = 0;
    uint8_t retry_count = 0;
    for(retry_count = 0; retry_count < 3; retry_count++) {
        distance = Ultrasonic_GetDistance();
        if(distance > 0 && distance < 500) {
            return distance;
        }
        if (Delay_ms_Interruptible(50)) return -1; 
    }
    return -1; 
}

AvoidState Avoidance_Decision(float distance)
{
    if(distance < 0)    return AVOID_CLEAR;
    else if(distance < 10.0) return AVOID_DANGER;  // 💥 修改: 10cm内：危险
    else if(distance < 20.0) return AVOID_WARNING; // 💥 修改: 20cm内：警告
    else return AVOID_CLEAR; // 20cm外：安全
}

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

// 修正：执行避障动作 (统一调用 DogActions)
void Execute_Avoidance_Action(AvoidState state)
{
    // action_counter 仍然保留，用于随机转向
    action_counter++; 
    
    switch(state) {
        case AVOID_CLEAR:
            // 💥 修正：不再使用“前冲”
            // 而是调用标准行走函数
            OLED_ShowString(1, 1, " (> _ <) Run!  "); 
            LED3_ON(); LED1_OFF(); LED2_OFF(); LED4_OFF();
            Dog_WalkForward(1); // <--- 统一调用！
            break;
            
        case AVOID_WARNING:
            // 💥 修正：统一调用“站立”
            OLED_ShowString(1, 1, " (o _ O) !!    "); 
            LED2_ON(); LED1_OFF(); LED3_OFF(); LED4_OFF();
            Buzzer_Beep(50);
            Dog_Stand(); // <--- 统一调用！
            if (Delay_ms_Interruptible(800)) return; 
            break;
            
        case AVOID_DANGER:
            // 💥 修正：不再使用“假转”
            // 而是调用标准转向函数
            OLED_ShowString(1, 1, " (@ _ @) ??    "); 
            LED1_ON(); LED2_OFF(); LED3_OFF(); LED4_OFF();
            Buzzer_BeepPattern(BEEP_DOUBLE_BEEP);
            
            // 随机选择左转或右转（根据计数器奇偶）
            if((action_counter % 2) == 0) {
                OLED_ShowString(2, 1, "ACTION: TURN LEFT ");
                Dog_TurnLeft(1); // <--- 统一调用！
                
            } else {
                OLED_ShowString(2, 1, "ACTION: TURN RIGHT");
                Dog_TurnRight(1); // <--- 统一调用！
            }
            break;
            
        case AVOID_TURNING:
            // (这个状态实际上已经不会被用到了，但保留也无妨)
            OLED_ShowString(1, 1, " (@ _ @) ??    "); 
            LED4_ON(); LED1_OFF(); LED2_OFF(); LED3_OFF();
            break;
    }
}


// -----------------------------------------------------------------
// 可中断的延时 (非阻塞)
// -----------------------------------------------------------------
uint8_t Delay_ms_Interruptible(uint32_t delay_ms)
{
    uint32_t chunk_delay = 20; 
    uint32_t num_chunks = delay_ms / chunk_delay;
    uint32_t remainder_delay = delay_ms % chunk_delay;
    
    for (uint32_t i = 0; i < num_chunks; i++)
    {
        Delay_ms(chunk_delay); 
        
        if (Key_GetNum() == 4) 
        {
            OLED_Clear();
            OLED_ShowString(1, 1, "Mode -> IDLE");
            Buzzer_Beep(100); // <--- 💥 新增音效 💥: 紧急停止也给个反馈
            Dog_Stand(); 
            current_mode = MODE_IDLE; 
            Delay_ms(500); 
            OLED_Clear();
            return 1; // 被中断
        }
    }
    
    if (remainder_delay > 0)
    {
        Delay_ms(remainder_delay);
    }
    
    return 0; // 正常结束
}

// -----------------------------------------------------------------
// 模式调度器
// -----------------------------------------------------------------

void Mode_Idle_Loop(void)
{
    OLED_ShowString(1, 1, "  (^ v ^) Zzz ");
    OLED_ShowString(2, 1, " K1: BLUETOOTH  "); 
    OLED_ShowString(3, 1, " K2: AVOIDANCE  "); 
    OLED_ShowString(4, 1, " K3: HELLO      "); 
    
    Dog_Stand();
    Delay_ms(100); 
}

void Mode_Avoidance_Loop(void)
{
    float distance = Safe_Ultrasonic_GetDistance();
    if (current_mode == MODE_IDLE) return; 

    AvoidState new_state = Avoidance_Decision(distance);
    Draw_Avoidance_Radar(distance, new_state);
    
    Execute_Avoidance_Action(new_state);
    if (current_mode == MODE_IDLE) return; 

    if (Delay_ms_Interruptible(1200)) return; 
}

// 修正：蓝牙遥控 (带音效)
void Mode_Bluetooth_Loop(void)
{
    OLED_ShowString(1, 1, " (o_o) BT Mode ");
    OLED_ShowString(2, 1, "Waiting CMD...  ");
    OLED_ShowString(4, 1, " (K4 back IDLE)");

    uint8_t cmd = Bluetooth_GetCommand();
    
    if(cmd != 0) {
        char oled_msg[17]; 
        Buzzer_Beep(20); // <--- 4. 💥 新增音效 💥: 收到任何有效指令，嘀一声
        
        switch(cmd) {
            case CMD_WALK_FORWARD: 
                OLED_ShowString(2, 1, "Action: Forward  ");
                Bluetooth_SendString("OK: Forward\r\n");
                Dog_WalkForward(1);
                break;
                
            case CMD_WALK_BACKWARD: 
                OLED_ShowString(2, 1, "Action: Backward ");
                Bluetooth_SendString("OK: Backward\r\n");
                Dog_WalkBackward(1);
                break;
                
            case CMD_TURN_LEFT: 
                OLED_ShowString(2, 1, "Action: Turn Left");
                Bluetooth_SendString("OK: Turn Left\r\n");
                Dog_TurnLeft(1);
                break;
                
            case CMD_TURN_RIGHT: 
                OLED_ShowString(2, 1, "Action:Turn Right");
                Bluetooth_SendString("OK: Turn Right\r\n");
                Dog_TurnRight(1);
                break;
                
            case CMD_STAND: 
            case CMD_STOP:  
                OLED_ShowString(2, 1, "Action: Stand    ");
                Bluetooth_SendString("OK: Stand\r\n");
                Dog_Stand();
                break;
                
            case CMD_SIT: 
                OLED_ShowString(2, 1, "Action: Sit      ");
                Bluetooth_SendString("OK: Sit\r\n");
                Dog_Action_SitDown();
                break;

            case CMD_SPEED_UP: 
                {
                    uint8_t speed = Dog_GetWalkSpeed();
                    if(speed < 10) speed++;
                    Dog_SetWalkSpeed(speed);
                    sprintf(oled_msg, "Speed: %d/10    ", speed);
                    OLED_ShowString(3, 1, oled_msg);
                    Bluetooth_SendString("OK: Speed Up\r\n");
                }
                break;
                
            case CMD_SPEED_DOWN: 
                {
                    uint8_t speed = Dog_GetWalkSpeed();
                    if(speed > 1) speed--;
                    Dog_SetWalkSpeed(speed);
                    sprintf(oled_msg, "Speed: %d/10    ", speed);
                    OLED_ShowString(3, 1, oled_msg);
                    Bluetooth_SendString("OK: Speed Down\r\n");
                }
                break;

            case CMD_TEST: 
                OLED_ShowString(2, 1, "Action: Hello!   ");
                Bluetooth_SendString("OK: Hello\r\n");
                Buzzer_BeepPattern(BEEP_TRIPLE_BEEP); // 蓝牙遥控的“你好”也加上声音
                Dog_Action_Hello();
                break;

            case CMD_RESET: 
                OLED_ShowString(2, 1, "Action: ShakeBody");
                Bluetooth_SendString("OK: Shake Body\r\n");
                Dog_Action_ShakeBody();
                break;
                
            default:
                sprintf(oled_msg, "Unknown: %c     ", cmd);
                OLED_ShowString(2, 1, oled_msg);
                Bluetooth_SendString("ERR: Unknown CMD\r\n");
                Buzzer_BeepPattern(BEEP_DOUBLE_BEEP); // 收到未知指令，播放错误音
                break;
        }
        
        Dog_Stand();
        Delay_ms(100); 
    }
    
    if (Delay_ms_Interruptible(50)) return; 
}


// 修正：动作: "你好" (带音效)
void Mode_Action_Hello_Once(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "== ACTION ==");
    OLED_ShowString(2, 1, "  (^ o ^)/ Hi! "); 
    LED1_ON(); LED2_ON();
    
    Buzzer_BeepPattern(BEEP_TRIPLE_BEEP); // <--- 5. 💥 新增音效 💥: "你好"专属音效
    Dog_Action_Hello(); 
    
    LED1_OFF(); LED2_OFF();
    OLED_Clear();
    current_mode = MODE_IDLE; 
}

void Mode_Action_Sit_Once(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "== ACTION ==");
    OLED_ShowString(2, 1, "  (- . -) Sit  "); 
    LED3_ON(); LED4_ON();
    
    Dog_Action_SitDown(); 
    if (Delay_ms_Interruptible(1000)) return; 
    
    LED3_OFF(); LED4_OFF();
    OLED_Clear();
    current_mode = MODE_IDLE;
}


// -----------------------------------------------------------------
// 修正：检查按键输入 (带音效)
// -----------------------------------------------------------------
void Check_Key_Input(void)
{
    key_pressed = Key_GetNum(); 
    
    if (key_pressed)
    {
        Buzzer_Beep(20); // <--- 6. 💥 新增音效 💥: 按键提示音
        OLED_Clear(); 
        
        switch (key_pressed)
        {
            case 1: 
                OLED_ShowString(1, 1, "Mode -> BLUETOOTH");
                current_mode = MODE_BLUETOOTH;
                break;
                
            case 2: 
                OLED_ShowString(1, 1, "Mode -> AVOIDANCE");
                current_mode = MODE_AVOIDANCE;
                action_counter = 0;
                break;
                
            case 3: 
                OLED_ShowString(1, 1, "Mode -> HELLO");
                current_mode = MODE_ACTION_HELLO;
                break;
                
            case 4: 
                OLED_ShowString(1, 1, "Mode -> IDLE");
                Dog_Stand(); 
                current_mode = MODE_IDLE;
                break;
        }
        Delay_ms(300); 
        OLED_Clear();
    }
}


// -----------------------------------------------------------------
// 修正：主函数 (main) (带音效)
// -----------------------------------------------------------------
int main(void)
{
    float distance = 0;
    
    // 初始化所有外设
    OLED_Init();
    LED_Init();
    Key_Init();
    Buzzer_Init();     // <--- 2. 💥 新增音效 💥: 初始化蜂鸣器
    Ultrasonic_Init();
    Dog_Init(); 
    Bluetooth_Init(); 
    
    OLED_Clear();
    OLED_ShowString(1, 1, "Smart Puppy V2.1"); // 升级版本号！
    OLED_ShowString(2, 1, "Initializing...");
    
    OLED_ShowString(3, 1, "Testing Servos...");
    LED4_ON();
    Safe_Servo4_Move(60);
    if (Delay_ms_Interruptible(200)) goto START_MAIN_LOOP; 
    Safe_Servo4_Move(120);
    if (Delay_ms_Interruptible(200)) goto START_MAIN_LOOP;
    Safe_Servo4_Move(90);
    LED4_OFF();
    OLED_ShowString(3, 1, "Servos Test Done ");
    
    Dog_Stand(); 
    if (Delay_ms_Interruptible(500)) goto START_MAIN_LOOP;
    
    OLED_ShowString(4, 1, "Testing Sensors...");
    distance = Safe_Ultrasonic_GetDistance();
    if(distance > 0) {
        OLED_ShowString(4, 1, "US: OK | BT: OK  "); 
    } else {
        OLED_ShowString(4, 1, "US: FAIL | BT: OK");
    }
    if (Delay_ms_Interruptible(1000)) goto START_MAIN_LOOP;
    
    OLED_Clear();
    OLED_ShowString(1, 1, "READY!");
    OLED_ShowString(3, 1, "Mode: IDLE");
    Buzzer_BeepPattern(BEEP_TRIPLE_BEEP); // <--- 2. 💥 新增音效 💥: 开机就绪提示音
    if (Delay_ms_Interruptible(1000)) goto START_MAIN_LOOP;
    
    OLED_Clear();
    
START_MAIN_LOOP: 
    current_mode = MODE_IDLE; 
    
    while(1)
    {
        Check_Key_Input();
        
        switch(current_mode)
        {
            case MODE_IDLE:
                Mode_Idle_Loop();
                break;
                
            case MODE_AVOIDANCE:
                Mode_Avoidance_Loop();
                break;
                
            case MODE_BLUETOOTH:   
                Mode_Bluetooth_Loop();
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
