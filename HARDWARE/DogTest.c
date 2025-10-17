#include "stm32f10x.h"
#include "DogActions.h"
#include "OLED.h"
#include "Delay.h"
#include "LED.h"
#include "Buzzer.h"
#include "Key.h"

void Dog_Complete_Test(void)
{
    // 初始化
    LED_Init();
    Key_Init();
    OLED_Init();
    Buzzer_Init();
    Dog_Init();
    
    OLED_Clear();
    OLED_ShowString(1, 1, "===Dog Test===");
    OLED_ShowString(2, 1, "K1:Stand K2:Sit");
    OLED_ShowString(3, 1, "K3:Walk K4:Turn");
    OLED_ShowString(4, 1, "Mode: Waiting...");
    
    Buzzer_BeepPattern(BEEP_DOUBLE_BEEP);
    
    while(1)
    {
        uint8_t key = Key_GetNum();
        
        switch(key)
        {
            case 1: // 站立测试
                OLED_ShowString(4, 1, "Mode: Standing   ");
                LED1_ON();
                Dog_Stand();
                Buzzer_Beep(100);
                LED1_OFF();
                break;
                
            case 2: // 坐下测试
                OLED_ShowString(4, 1, "Mode: Sitting    ");
                LED2_ON();
                Dog_Sit();
                Buzzer_Beep(100);
                LED2_OFF();
                break;
                
            case 3: // 行走测试 - 重点观察！
                OLED_ShowString(4, 1, "Mode: Walking    ");
                LED3_ON();
                
                // 先测试改进版步态
                OLED_ShowString(2, 1, "Improved Gait:   ");
                for(int i=0; i<2; i++) {
                    Dog_WalkForward(3);
                }
                
                // 再测试原始步态
                OLED_ShowString(2, 1, "Original Gait:   ");
                for(int i=0; i<2; i++) {
                    Dog_WalkForward(2);
                }
                
                Buzzer_BeepPattern(BEEP_TRIPLE_BEEP);
                LED3_OFF();
                break;
                
            case 4: // 舵机4专项测试
                OLED_ShowString(4, 1, "Mode: Servo4 Test");
                LED4_ON();
                Dog_TestServos();
                LED4_OFF();
                break;
        }
        
        Delay_ms(10);
    }
}
