#include "BluetoothControl.h"
#include "Bluetooth.h"
#include "ControlSystem.h"
#include "DogActions.h"
#include "OLED.h"
#include "Buzzer.h"
#include "LED.h"

static uint8_t bluetooth_active = 0;

void BluetoothControl_Init(void)
{
    Bluetooth_Init();
    bluetooth_active = 1;
    
    OLED_Clear();
    OLED_ShowString(1, 1, "Bluetooth Mode");
    OLED_ShowString(2, 1, "Waiting CMD...");
    OLED_ShowString(3, 1, "F:Forward B:Back");
    OLED_ShowString(4, 1, "L:Left   R:Right");
    
    Buzzer_BeepPattern(BEEP_TRIPLE_BEEP);
    
    // LED指示蓝牙模式
    LED1_ON(); LED2_ON(); LED3_ON(); LED4_ON();
    Delay_ms(500);
    LED1_OFF(); LED2_OFF(); LED3_OFF(); LED4_OFF();
}

void BluetoothControl_Update(void)
{
    if(bluetooth_active) {
        uint8_t cmd = Bluetooth_GetCommand();
        
        if(cmd != 0) {
            Bluetooth_ProcessCommand(cmd);
            BluetoothControl_ProcessCommand(cmd);
        }
    }
}

void BluetoothControl_ProcessCommand(uint8_t cmd)
{
    char display_msg[17];
    
    switch(cmd) {
        case CMD_STAND:
            Control_Stand();
            OLED_ShowString(2, 1, "Action: Stand    ");
            break;
            
        case CMD_SIT:
            Control_Sit();
            OLED_ShowString(2, 1, "Action: Sit      ");
            break;
            
        case CMD_WALK_FORWARD:
            Control_WalkForward(1);
            OLED_ShowString(2, 1, "Action: Forward  ");
            break;
            
        case CMD_WALK_BACKWARD:
            Control_WalkBackward(1);
            OLED_ShowString(2, 1, "Action: Backward ");
            break;
            
        case CMD_TURN_LEFT:
            Control_TurnLeft(1);
            OLED_ShowString(2, 1, "Action: Turn Left");
            break;
            
        case CMD_TURN_RIGHT:
            Control_TurnRight(1);
            OLED_ShowString(2, 1, "Action:Turn Right");
            break;
            
        case CMD_STOP:
            Control_Stop();
            OLED_ShowString(2, 1, "Action: Stop     ");
            break;
            
        case CMD_SPEED_UP:
            {
                uint8_t speed = Dog_GetWalkSpeed();
                if(speed < 10) {
                    Dog_SetWalkSpeed(speed + 1);
                    sprintf(display_msg, "Speed: %d/10    ", speed + 1);
                    OLED_ShowString(3, 1, display_msg);
                    Buzzer_Beep(50);
                }
            }
            break;
            
        case CMD_SPEED_DOWN:
            {
                uint8_t speed = Dog_GetWalkSpeed();
                if(speed > 1) {
                    Dog_SetWalkSpeed(speed - 1);
                    sprintf(display_msg, "Speed: %d/10    ", speed - 1);
                    OLED_ShowString(3, 1, display_msg);
                    Buzzer_Beep(50);
                }
            }
            break;
            
        case CMD_TEST:
            OLED_ShowString(2, 1, "Action: Test     ");
            Dog_TestServos();
            break;
            
        case CMD_RESET:
            OLED_ShowString(2, 1, "Action: Reset    ");
            Dog_ResetPose();
            break;
            
        default:
            sprintf(display_msg, "Unknown: %c       ", cmd);
            OLED_ShowString(2, 1, display_msg);
            break;
    }
    
    // 更新状态显示
    Bluetooth_SendStatus();
}

uint8_t BluetoothControl_IsActive(void)
{
    return bluetooth_active;
}
