#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"

// 函数声明
void Buzzer_Init(void);                    // 蜂鸣器初始化
void Buzzer_On(void);                      // 打开蜂鸣器
void Buzzer_Off(void);                     // 关闭蜂鸣器
void Buzzer_Beep(uint16_t duration_ms);    // 蜂鸣器鸣叫指定时间
void Buzzer_BeepPattern(uint8_t pattern);  // 播放预设的鸣叫模式

// 引脚定义 - 现在在头文件中定义，方便修改
#define BEEP_GPIO_PORT    GPIOA
#define BEEP_GPIO_PIN     GPIO_Pin_8
#define BEEP_RCC_CLOCK    RCC_APB2Periph_GPIOA

// 鸣叫模式定义
#define BEEP_SINGLE_SHORT   0  // 单次短鸣
#define BEEP_SINGLE_LONG    1  // 单次长鸣  
#define BEEP_DOUBLE_BEEP    2  // 双鸣
#define BEEP_TRIPLE_BEEP    3  // 三连鸣
#define BEEP_SOS            4  // SOS信号

#endif
