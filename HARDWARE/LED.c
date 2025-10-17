#include "stm32f10x.h"                  // Device header
#include "LED.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 启用时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	
	// 禁用JTAG，释放PA15、PB3、PB4
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	// 配置GPIOA引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置GPIOB引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 初始化为高电平（LED熄灭）
    GPIO_SetBits(GPIOA, GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15);
    GPIO_SetBits(GPIOB, GPIO_Pin_3);
}

void LED1_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_11);
}

void LED1_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
}

void LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_11) == 0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_11);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	}
}

void LED2_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}

void LED2_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_12);
}

void LED2_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_12) == 0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_12);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	}
}

// 添加LED3函数 (PA15)
void LED3_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}

void LED3_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_15);
}

void LED3_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_15) == 0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_15);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_15);
	}
}

// 添加LED4函数 (PB3)
void LED4_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
}

void LED4_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_3);
}

void LED4_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_3) == 0)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_3);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	}
}
