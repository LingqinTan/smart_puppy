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

// -----------------------------------------------------------------
// 步骤 1: 定义系统模式
// -----------------------------------------------------------------
typedef enum {
    MODE_IDLE = 0, MODE_AVOIDANCE, MODE_BLUETOOTH,
    MODE_ACTION_HELLO, MODE_ACTION_SIT, MODE_ACTION_SHAKE
} SystemMode;

static SystemMode current_mode = MODE_IDLE; 
static uint8_t key_pressed = 0;             

// -----------------------------------------------------------------
// 💥 步骤 1.5: 关键修正 - 函数原型 (Function Prototype) 💥
// -----------------------------------------------------------------
/**
 * @brief  可被 K4 中断的延时函数 (非阻塞式延时)
 * @note   这是“函数原型”或“前向声明”
 */
uint8_t Delay_ms_Interruptible(uint32_t delay_ms);
// -----------------------------------------------------------------


// -----------------------------------------------------------------
// 步骤 2: 避障逻辑函数
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
        // 现在编译器在编译这一行时，已经通过“原型”知道这个函数长什么样了
        if (Delay_ms_Interruptible(50)) return -1; 
    }
    return -1; 
}

// 避障决策函数
AvoidState Avoidance_Decision(float distance)
{
    if(distance < 0)    return AVOID_CLEAR;
    else if(distance < 15.0) return AVOID_DANGER; 
    else if(distance < 30.0) return AVOID_WARNING;
    else return AVOID_CLEAR;
}

// 绘制避障雷达界面
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

// 执行避障动作 (带表情)
void Execute_Avoidance_Action(AvoidState state)
{
    action_counter++;
    
    switch(state) {
        case AVOID_CLEAR:
            OLED_ShowString(1, 1, " (> _ <) Run!  "); 
            LED3_ON(); LED1_OFF(); LED2_OFF(); LED4_OFF();
            Dog_Stand(); 
            if (Delay_ms_Interruptible(200)) return; 
            
            Safe_Servo4_Move(100); Servo_SetAngle(1, 80); 
            Servo_SetAngle(2, 100); Servo_SetAngle(3, 80);
            if (Delay_ms_Interruptible(300)) return; 
            Dog_Stand();
            break;
            
        case AVOID_WARNING:
            OLED_ShowString(1, 1, " (o _ O) !!    "); 
            LED2_ON(); LED1_OFF(); LED3_OFF(); LED4_OFF();
            Dog_Stand(); 
            if (Delay_ms_Interruptible(800)) return; 
            break;
            
        case AVOID_DANGER:
            OLED_ShowString(1, 1, " (@ _ @) ??    "); 
            LED1_ON(); LED2_OFF(); LED3_OFF(); LED4_OFF();
            
            if((action_counter % 2) == 0) {
                OLED_ShowString(2, 1, "ACTION: TURN LEFT ");
                Safe_Servo4_Move(70); Servo_SetAngle(1, 110); 
                Servo_SetAngle(2, 70); Servo_SetAngle(3, 110); 
                if (Delay_ms_Interruptible(600)) return; 
                Dog_Stand();
            } else {
                OLED_ShowString(2, 1, "ACTION: TURN RIGHT");
                Safe_Servo4_Move(110); Servo_SetAngle(1, 70);
                Servo_SetAngle(2, 110); Servo_SetAngle(3, 70); 
                if (Delay_ms_Interruptible(600)) return; 
                Dog_Stand();
            }
            break;
            
        case AVOID_TURNING:
            OLED_ShowString(1, 1, " (@ _ @) ??    "); 
            LED4_ON(); LED1_OFF(); LED2_OFF(); LED3_OFF();
            break;
    }
}


// -----------------------------------------------------------------
// 步骤 3: 可中断的延时 (非阻塞)
// (这里是函数的“定义”)
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
// 步骤 4: 模式调度器
// -----------------------------------------------------------------

/**
 * @brief 模式: 空闲待机 (带表情)
 */
void Mode_Idle_Loop(void)
{
    OLED_ShowString(1, 1, "  (^ v ^) Zzz ");
    OLED_ShowString(2, 1, " K1: BLUETOOTH  "); 
    OLED_ShowString(3, 1, " K2: AVOIDANCE  "); 
    OLED_ShowString(4, 1, " K3: HELLO      "); 
    
    Dog_Stand();
    Delay_ms(100); 
}

/**
 * @brief 模式: 自主避障
 */
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

/**
 * @brief 模式: 蓝牙遥控
 */
void Mode_Bluetooth_Loop(void)
{
    OLED_ShowString(1, 1, " (o_o) BT Mode ");
    OLED_ShowString(2, 1, "Waiting CMD...  ");
    OLED_ShowString(4, 1, " (K4 back IDLE)");

    uint8_t cmd = Bluetooth_GetCommand();
    
    if(cmd != 0) {
        char oled_msg[17]; 
        
        switch(cmd) {
            case CMD_WALK_FORWARD: // 'F'
                OLED_ShowString(2, 1, "Action: Forward  ");
                Bluetooth_SendString("OK: Forward\r\n");
                Dog_WalkForward(1);
                break;
                
            case CMD_WALK_BACKWARD: // 'B'
                OLED_ShowString(2, 1, "Action: Backward ");
                Bluetooth_SendString("OK: Backward\r\n");
                Dog_WalkBackward(1);
                break;
                
            case CMD_TURN_LEFT: // 'L'
                OLED_ShowString(2, 1, "Action: Turn Left");
                Bluetooth_SendString("OK: Turn Left\r\n");
                Dog_TurnLeft(1);
                break;
                
            case CMD_TURN_RIGHT: // 'R'
                OLED_ShowString(2, 1, "Action:Turn Right");
                Bluetooth_SendString("OK: Turn Right\r\n");
                Dog_TurnRight(1);
                break;
                
            case CMD_STAND: // 'S'
            case CMD_STOP:  // 'P'
                OLED_ShowString(2, 1, "Action: Stand    ");
                Bluetooth_SendString("OK: Stand\r\n");
                Dog_Stand();
                break;
                
            case CMD_SIT: // 'T'
                OLED_ShowString(2, 1, "Action: Sit      ");
                Bluetooth_SendString("OK: Sit\r\n");
                Dog_Action_SitDown();
                break;

            case CMD_SPEED_UP: // 'U'
                {
                    uint8_t speed = Dog_GetWalkSpeed();
                    if(speed < 10) speed++;
                    Dog_SetWalkSpeed(speed);
                    sprintf(oled_msg, "Speed: %d/10    ", speed);
                    OLED_ShowString(3, 1, oled_msg);
                    Bluetooth_SendString("OK: Speed Up\r\n");
                }
                break;
                
            case CMD_SPEED_DOWN: // 'D'
                {
                    uint8_t speed = Dog_GetWalkSpeed();
                    if(speed > 1) speed--;
                    Dog_SetWalkSpeed(speed);
                    sprintf(oled_msg, "Speed: %d/10    ", speed);
                    OLED_ShowString(3, 1, oled_msg);
                    Bluetooth_SendString("OK: Speed Down\r\n");
                }
                break;

            case CMD_TEST: // 'M' (趣味动作: 你好)
                OLED_ShowString(2, 1, "Action: Hello!   ");
                Bluetooth_SendString("OK: Hello\r\n");
                Dog_Action_Hello();
                break;

            case CMD_RESET: // 'X' (趣味动作: 抖一抖)
                OLED_ShowString(2, 1, "Action: ShakeBody");
                Bluetooth_SendString("OK: Shake Body\r\n");
                Dog_Action_ShakeBody();
                break;
                
            default:
                sprintf(oled_msg, "Unknown: %c     ", cmd);
                OLED_ShowString(2, 1, oled_msg);
                Bluetooth_SendString("ERR: Unknown CMD\r\n");
                break;
        }
        
        Dog_Stand();
        Delay_ms(100); 
    }
    
    if (Delay_ms_Interruptible(50)) return; 
}


/**
 * @brief 模式: 执行一次 "你好" (带表情)
 */
void Mode_Action_Hello_Once(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "== ACTION ==");
    OLED_ShowString(2, 1, "  (^ o ^)/ Hi! "); 
    LED1_ON(); LED2_ON();
    
    Dog_Action_Hello(); 
    
    LED1_OFF(); LED2_OFF();
    OLED_Clear();
    current_mode = MODE_IDLE; 
}

/**
 * @brief 模式: 执行一次 "坐下" (带表情)
 */
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
// 步骤 5: 检查按键输入
// -----------------------------------------------------------------
void Check_Key_Input(void)
{
    key_pressed = Key_GetNum(); 
    
    if (key_pressed)
    {
        OLED_Clear(); 
        
        switch (key_pressed)
        {
            case 1: // K1: 切换到蓝牙模式
                OLED_ShowString(1, 1, "Mode -> BLUETOOTH");
                current_mode = MODE_BLUETOOTH;
                break;
                
            case 2: // K2: 切换到自主避障
                OLED_ShowString(1, 1, "Mode -> AVOIDANCE");
                current_mode = MODE_AVOIDANCE;
                action_counter = 0;
                break;
                
            case 3: // K3: 执行 "你好"
                OLED_ShowString(1, 1, "Mode -> HELLO");
                current_mode = MODE_ACTION_HELLO;
                break;
                
            case 4: // K4: 强制返回空闲模式
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
// 步骤 6: 主函数 (main)
// -----------------------------------------------------------------
int main(void)
{
    float distance = 0;
    
    OLED_Init();
    LED_Init();
    Key_Init();
    Ultrasonic_Init();
    Dog_Init(); 
    Bluetooth_Init(); 
    
    OLED_Clear();
    OLED_ShowString(1, 1, "Smart Puppy V2.0"); 
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
