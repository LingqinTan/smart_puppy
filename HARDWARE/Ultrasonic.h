#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "stm32f10x.h"

// 引脚定义 - 这里对应你的硬件连接！
#define TRIG_GPIO_PORT    GPIOA
#define TRIG_GPIO_PIN     GPIO_Pin_4
#define TRIG_RCC_CLOCK    RCC_APB2Periph_GPIOA

#define ECHO_GPIO_PORT    GPIOA  
#define ECHO_GPIO_PIN     GPIO_Pin_5
#define ECHO_RCC_CLOCK    RCC_APB2Periph_GPIOA

// 函数声明
void Ultrasonic_Init(void);        // 初始化函数
float Ultrasonic_GetDistance(void); // 获取距离函数，返回单位是厘米
void Ultrasonic_Debug_Info(void);

#endif
