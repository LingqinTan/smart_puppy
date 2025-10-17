#include "ControlSystem.h"
#include "DogActions.h"
#include "LED.h"
#include "Buzzer.h"
#include "OLED.h"

static SystemState CurrentState = SYS_IDLE;

void ControlSystem_Init(void)
{
    Dog_Init();
    Dog_SetActionCompleteCallback(ControlSystem_Update);
    CurrentState = SYS_IDLE;
    
    OLED_Clear();
    OLED_ShowString(1, 1, "Control System OK");
    OLED_ShowString(2, 1, "State: IDLE");
    
    Buzzer_BeepPattern(BEEP_SINGLE_SHORT);
}

void ControlSystem_Update(void)
{
    switch(CurrentState) {
        case SYS_WALKING:
        case SYS_TURNING:
            CurrentState = SYS_STANDING;
            OLED_ShowString(2, 1, "State: STANDING ");
            LED2_ON();
            LED1_OFF(); LED3_OFF(); LED4_OFF();
            break;
            
        case SYS_SITTING:
            CurrentState = SYS_IDLE;
            OLED_ShowString(2, 1, "State: IDLE      ");
            LED1_ON();
            LED2_OFF(); LED3_OFF(); LED4_OFF();
            break;
            
        default:
            break;
    }
}

SystemState ControlSystem_GetState(void)
{
    return CurrentState;
}

void Control_Stand(void)
{
    CurrentState = SYS_STANDING;
    OLED_ShowString(2, 1, "State: STANDING ");
    Dog_Stand();
    LED2_ON();
    LED1_OFF(); LED3_OFF(); LED4_OFF();
    Buzzer_Beep(50);
}

void Control_Sit(void)
{
    CurrentState = SYS_SITTING;
    OLED_ShowString(2, 1, "State: SITTING   ");
    Dog_Sit();
    LED3_ON();
    LED1_OFF(); LED2_OFF(); LED4_OFF();
    Buzzer_Beep(50);
}

void Control_WalkForward(uint8_t steps)
{
    CurrentState = SYS_WALKING;
    OLED_ShowString(2, 1, "State: WALKING   ");
    LED4_ON();
    LED1_OFF(); LED2_OFF(); LED3_OFF();
    Buzzer_BeepPattern(BEEP_DOUBLE_BEEP);
    Dog_WalkForward(steps);
}

void Control_TurnLeft(uint8_t steps)
{
    CurrentState = SYS_TURNING;
    OLED_ShowString(2, 1, "State: TURNING   ");
    LED4_ON();
    LED1_OFF(); LED2_OFF(); LED3_OFF();
    Buzzer_Beep(100);
    Dog_TurnLeft(steps);
}

void Control_TurnRight(uint8_t steps)
{
    CurrentState = SYS_TURNING;
    OLED_ShowString(2, 1, "State: TURNING   ");
    LED4_ON();
    LED1_OFF(); LED2_OFF(); LED3_OFF();
    Buzzer_Beep(100);
    Dog_TurnRight(steps);
}

void Control_Stop(void)
{
    Control_Stand();
}
