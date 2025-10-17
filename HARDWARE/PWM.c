// PWM.c (修正版)
#include "stm32f10x.h"                 
#include "stm32f10x_tim.h" 
#include "PWM.h"

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* 0. 关键步骤：开启AFIO时钟，并禁用JTAG，释放PB4 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // 必须先开启AFIO时钟
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 这会禁用JTAG，但保留SWD调试功能，从而释放PB3, PB4, PA15

    /* 1. 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    /* 2. 配置GPIO引脚 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    // PB1 - TIM3通道4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB4 - TIM3通道1 (现在它被释放了，可以正常配置)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB8 - TIM4通道3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB9 - TIM4通道4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 3. 配置定时器时基单元 (TIM3 和 TIM4) */
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure); // 使用默认值初始化
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1; // 20ms 周期
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1; // 1MHz 计数频率
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
    
    /* 4. 配置定时器输出比较单元 */
    TIM_OCStructInit(&TIM_OCInitStructure); // 使用默认值初始化
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1500;  // 初始1.5ms
    
    // 初始化 TIM3 的两个通道
    TIM_OC1Init(TIM3, &TIM_OCInitStructure); // PB4 -> TIM3_CH1
    TIM_OC4Init(TIM3, &TIM_OCInitStructure); // PB1 -> TIM3_CH4
    
    // 初始化 TIM4 的两个通道
    TIM_OC3Init(TIM4, &TIM_OCInitStructure); // PB8 -> TIM4_CH3
    TIM_OC4Init(TIM4, &TIM_OCInitStructure); // PB9 -> TIM4_CH4
    
    /* 5. 启用所有通道的预装载 */
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

    /* 6. 启用自动重装载预装载 */
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    
    /* 7. 启动定时器 */
    TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

// 舵机1 -> PB1 (TIM3_CH4)
void PWM_SetCompare1(uint16_t Compare)
{
    TIM_SetCompare4(TIM3, Compare);
}

// 舵机2 -> PB4 (TIM3_CH1)
void PWM_SetCompare2(uint16_t Compare)
{
    TIM_SetCompare1(TIM3, Compare);
}

// 舵机3 -> PB8 (TIM4_CH3)
void PWM_SetCompare3(uint16_t Compare)
{
    TIM_SetCompare3(TIM4, Compare);
}

// 舵机4 -> PB9 (TIM4_CH4)
void PWM_SetCompare4(uint16_t Compare)
{
    TIM_SetCompare4(TIM4, Compare);
}
