#include "stm32f10x.h"
#include "Servo.h"
#include "OLED.h"
#include "Delay.h"
#include "LED.h"
#include "Buzzer.h"

void Servo_Test_All(void)
{
    // 初始化所有外设
    LED_Init();
    OLED_Init();
    Servo_Init();
    Buzzer_Init();
    
    OLED_Clear();
    OLED_ShowString(1, 1, "===Servo Test===");
    OLED_ShowString(2, 1, "S1:PB1 S2:PB4");
    OLED_ShowString(3, 1, "S3:PB8 S4:PB9");
    OLED_ShowString(4, 1, "Press KEY to test");
    
    Buzzer_Beep(200);
    
    while(1)
    {
        // 测试舵机1 (PB1)
        OLED_ShowString(2, 1, "Servo1 Testing...");
        LED1_ON();
        for(int angle = 0; angle <= 180; angle += 30) {
            Servo_SetAngle(1, angle);
            OLED_ShowNum(3, 1, angle, 3);
            OLED_ShowString(4, 1, "PB1 -> TIM3_CH4");
            Delay_ms(500);
        }
        LED1_OFF();
        Delay_ms(1000);
        
        // 测试舵机2 (PB4)  
        OLED_ShowString(2, 1, "Servo2 Testing...");
        LED2_ON();
        for(int angle = 0; angle <= 180; angle += 30) {
            Servo_SetAngle(2, angle);
            OLED_ShowNum(3, 1, angle, 3);
            OLED_ShowString(4, 1, "PB4 -> TIM3_CH1");
            Delay_ms(500);
        }
        LED2_OFF();
        Delay_ms(1000);
        
        // 测试舵机3 (PB8)
        OLED_ShowString(2, 1, "Servo3 Testing...");
        LED3_ON();
        for(int angle = 0; angle <= 180; angle += 30) {
            Servo_SetAngle(3, angle);
            OLED_ShowNum(3, 1, angle, 3);
            OLED_ShowString(4, 1, "PB8 -> TIM4_CH3");
            Delay_ms(500);
        }
        LED3_OFF();
        Delay_ms(1000);
        
        // 测试舵机4 (PB9) - 重点关注这个！
        OLED_ShowString(2, 1, "Servo4 Testing...");
        LED4_ON();
        for(int angle = 0; angle <= 180; angle += 30) {
            Servo_SetAngle(4, angle);
            OLED_ShowNum(3, 1, angle, 3);
            OLED_ShowString(4, 1, "PB9 -> TIM4_CH4");
            Delay_ms(500);
        }
        LED4_OFF();
        Delay_ms(1000);
    }
}
