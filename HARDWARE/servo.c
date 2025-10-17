// servo.c
#include "stm32f10x.h"
#include "PWM.h" // 引用我们底层的PWM驱动

/**
  * @brief  舵机初始化
  * @param  无
  * @retval 无
  */
void Servo_Init(void)
{
	PWM_Init(); // 底层PWM初始化，一次即可
}

/**
  * @brief  设定指定舵机的角度
  * @param  id: 你想控制的舵机编号，范围 [1, 4]
  * @param  Angle: 你期望的角度，范围 [0, 180]
  * @retval 无
  */
void Servo_SetAngle(uint8_t id, float Angle)
{
    uint16_t pulse;
    
    // 放宽角度限制，特别是舵机4
    if(Angle < 30) Angle = 30;
    if(Angle > 150) Angle = 150;
    
    // 特别保护舵机4，但不过度限制
    if(id == 4) {
        if(Angle < 40) Angle = 40;
        if(Angle > 140) Angle = 140;
    }
    
    pulse = (uint16_t)(Angle / 180.0f * 2000.0f + 500.0f);
    
    if(pulse < 500) pulse = 500;
    if(pulse > 2500) pulse = 2500;
    
    switch(id) {
        case 1: PWM_SetCompare1(pulse); break;
        case 2: PWM_SetCompare2(pulse); break;
        case 3: PWM_SetCompare3(pulse); break;
        case 4: PWM_SetCompare4(pulse); break;
    }
    
    // 添加小延时，减少电流冲击
    Delay_ms(2);
}
