#include "Buzzer.h"
#include "Delay.h"

/**
  * @brief  蜂鸣器初始化
  * @param  无
  * @retval 无
  */
void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 1. 开启GPIO时钟 */
    RCC_APB2PeriphClockCmd(BEEP_RCC_CLOCK, ENABLE);
    
    /* 2. 配置GPIO引脚 */
    GPIO_InitStructure.GPIO_Pin = BEEP_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);
    
    
    /* 3. 初始状态关闭蜂鸣器 - 输出高电平 */
    GPIO_SetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN);  // 确保初始为高电平
    
    /* 4. 初始化成功提示 */
    Buzzer_Beep(50);
}

/**
  * @brief  打开蜂鸣器
  * @param  无
  * @retval 无
  */
void Buzzer_On(void)
{
    GPIO_SetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN);
}

/**
  * @brief  关闭蜂鸣器
  * @param  无
  * @retval 无
  */
void Buzzer_Off(void)
{
    GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN);
}

/**
  * @brief  蜂鸣器鸣叫指定时间
  * @param  duration_ms: 鸣叫持续时间(毫秒)
  * @retval 无
  */
void Buzzer_Beep(uint16_t duration_ms)
{
    Buzzer_On();
    Delay_ms(duration_ms);
    Buzzer_Off();
}

/**
  * @brief  播放预设的鸣叫模式
  * @param  pattern: 鸣叫模式，见Buzzer.h中的定义
  * @retval 无
  */
void Buzzer_BeepPattern(uint8_t pattern)
{
    switch(pattern)
    {
        case BEEP_SINGLE_SHORT:     // 单次短鸣
            Buzzer_Beep(100);
            break;
            
        case BEEP_SINGLE_LONG:      // 单次长鸣
            Buzzer_Beep(500);
            break;
            
        case BEEP_DOUBLE_BEEP:      // 双鸣
            Buzzer_Beep(100);
            Delay_ms(100);
            Buzzer_Beep(100);
            break;
            
        case BEEP_TRIPLE_BEEP:      // 三连鸣
            Buzzer_Beep(80);
            Delay_ms(80);
            Buzzer_Beep(80);
            Delay_ms(80);
            Buzzer_Beep(80);
            break;
            
        case BEEP_SOS:              // SOS信号: ...---...
            // S: ... (3短)
            Buzzer_Beep(100); Delay_ms(100);
            Buzzer_Beep(100); Delay_ms(100); 
            Buzzer_Beep(100); Delay_ms(300);
            // O: --- (3长)
            Buzzer_Beep(300); Delay_ms(100);
            Buzzer_Beep(300); Delay_ms(100);
            Buzzer_Beep(300); Delay_ms(300);
            // S: ... (3短)
            Buzzer_Beep(100); Delay_ms(100);
            Buzzer_Beep(100); Delay_ms(100);
            Buzzer_Beep(100);
            break;
            
        default:
            Buzzer_Beep(200);       // 默认鸣叫
            break;
    }
}

/**
  * @brief  调试函数：检查GPIO配置
  * @param  无  
  * @retval 无
  */
void Buzzer_Debug_Info(void)
{
    // 检查GPIOA时钟是否开启
    if(RCC->APB2ENR & RCC_APB2Periph_GPIOA)
    {
        // 时钟已开启
    }
    else
    {
        // 时钟未开启 - 这是问题所在！
    }
    
    // 检查PA8的当前状态
    if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8))
    {
        // PA8当前输出高电平
    }
    else
    {
        // PA8当前输出低电平
    }
}
