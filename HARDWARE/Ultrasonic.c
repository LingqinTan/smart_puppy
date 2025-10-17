#include "Ultrasonic.h"
#include "Delay.h" // 我们需要用到微秒和毫秒的延时函数

static uint32_t debug_timeout_count = 0;
static uint32_t debug_echo_high_time = 0;

/**
  * @brief  超声波模块初始化
  * @param  无
  * @retval 无
  * @detail 配置Trig引脚为输出，Echo引脚为输入
  */
void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. 开启GPIOA的时钟 (因为Trig和Echo都接在PA4和PA5)
    RCC_APB2PeriphClockCmd(TRIG_RCC_CLOCK, ENABLE);
    
    // 2. 初始化Trig引脚 (输出模式，用于发送触发信号)
    GPIO_InitStructure.GPIO_Pin = TRIG_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TRIG_GPIO_PORT, &GPIO_InitStructure);
    
    // 3. 初始化Echo引脚 (输入模式，用于读取高电平持续时间)
    GPIO_InitStructure.GPIO_Pin = ECHO_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // 上拉输入 (模块Echo脚平时是弱低电平，收到回声变高)
    // 注意：有些模块Echo脚需要上拉，有些是推挽输出。如果不行，可以试试 GPIO_Mode_IN_FLOATING(浮空输入)
    GPIO_Init(ECHO_GPIO_PORT, &GPIO_InitStructure);
    
    // 4. 初始状态：Trig引脚输出低电平
    GPIO_ResetBits(TRIG_GPIO_PORT, TRIG_GPIO_PIN);
}

/**
  * @brief  获取超声波测距结果
  * @param  无
  * @retval 距离值，单位：厘米 (cm)
  */
float Ultrasonic_GetDistance(void)
{
    uint32_t timeout = 0;
    uint32_t time_high = 0;
    float distance_cm = 0;
    
    // 调试：记录超时次数
    debug_timeout_count++;
    
    // 步骤1: 发送Trig信号
    GPIO_SetBits(TRIG_GPIO_PORT, TRIG_GPIO_PIN);
    Delay_us(20);
    GPIO_ResetBits(TRIG_GPIO_PORT, TRIG_GPIO_PIN);
    
    // 步骤2: 等待Echo变高 - 添加详细调试
    timeout = 100000;
    while(GPIO_ReadInputDataBit(ECHO_GPIO_PORT, ECHO_GPIO_PIN) == 0) {
        if(timeout-- == 0) {
            return -1; // Error 1: 等待Echo高电平超时
        }
    }
    
    // 步骤3: 测量高电平时间
    time_high = 0;
    timeout = 50000;
    
    while(GPIO_ReadInputDataBit(ECHO_GPIO_PORT, ECHO_GPIO_PIN) == 1) {
        time_high++;
        Delay_us(1);
        if(timeout-- == 0) {
            return -2; // Error 2: Echo高电平持续时间超时
        }
    }
    
    // 记录调试信息
    debug_echo_high_time = time_high;
    
    // 步骤4: 计算距离
    distance_cm = time_high * 0.017f;
    
    return distance_cm;
}

// 添加调试函数
void Ultrasonic_Debug_Info(void)
{
    char debug_msg[32];
    
    // 在OLED上显示调试信息
    OLED_ShowString(2, 1, "Debug:          ");
    sprintf(debug_msg, "Tout:%lu", debug_timeout_count);
    OLED_ShowString(3, 1, debug_msg);
    
    sprintf(debug_msg, "Time:%lu", debug_echo_high_time);
    OLED_ShowString(4, 1, debug_msg);
}