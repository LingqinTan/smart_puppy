#include "stm32f10x.h"
#include "Servo.h"
#include "OLED.h"
#include "Delay.h"
#include "LED.h"

void Test_Each_Leg(void)
{
    OLED_Init();
    Servo_Init();
    LED_Init();
    
    OLED_Clear();
    OLED_ShowString(1, 1, "LEG TEST MODE");
    OLED_ShowString(2, 1, "Test each leg");
    OLED_ShowString(3, 1, "individually");
    
    Delay_ms(2000);
    
    while(1)
    {
        // 测试前右腿 (舵机1)
        OLED_Clear();
        OLED_ShowString(1, 1, "Testing: Front Right");
        OLED_ShowString(2, 1, "Servo1 -> Front Right");
        LED1_ON();
        Servo_SetAngle(1, 45);  // 抬起
        Delay_ms(1000);
        Servo_SetAngle(1, 90);  // 放下
        Delay_ms(1000);
        LED1_OFF();
        Delay_ms(500);
        
        // 测试前左腿 (舵机2)
        OLED_Clear();
        OLED_ShowString(1, 1, "Testing: Front Left");
        OLED_ShowString(2, 1, "Servo2 -> Front Left");
        LED2_ON();
        Servo_SetAngle(2, 135); // 抬起
        Delay_ms(1000);
        Servo_SetAngle(2, 90);  // 放下
        Delay_ms(1000);
        LED2_OFF();
        Delay_ms(500);
        
        // 测试后左腿 (舵机3)
        OLED_Clear();
        OLED_ShowString(1, 1, "Testing: Rear Left");
        OLED_ShowString(2, 1, "Servo3 -> Rear Left");
        LED3_ON();
        Servo_SetAngle(3, 135); // 抬起
        Delay_ms(1000);
        Servo_SetAngle(3, 90);  // 放下
        Delay_ms(1000);
        LED3_OFF();
        Delay_ms(500);
        
        // 测试后右腿 (舵机4)
        OLED_Clear();
        OLED_ShowString(1, 1, "Testing: Rear Right");
        OLED_ShowString(2, 1, "Servo4 -> Rear Right");
        LED4_ON();
        Servo_SetAngle(4, 45);  // 抬起
        Delay_ms(1000);
        Servo_SetAngle(4, 90);  // 放下
        Delay_ms(1000);
        LED4_OFF();
        Delay_ms(500);
    }
}