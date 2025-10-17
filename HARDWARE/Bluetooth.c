#include "Bluetooth.h"
#include "stm32f10x_usart.h"
#include "Delay.h"
#include "stdio.h"
#include "DogActions.h"

// 全局变量
static volatile uint8_t rx_buffer[256];
static volatile uint16_t rx_index = 0;
static volatile uint8_t command_received = 0;
static volatile uint8_t current_command = 0;
static WorkMode current_mode = MODE_MANUAL;

void Bluetooth_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 1. 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    // 2. 配置USART2引脚 PA2-TX, PA3-RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;  // TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;  // RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 3. 配置USART2
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    
    // 4. 配置USART2中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 5. 启动USART2
    USART_Cmd(USART2, ENABLE);
    
    // 发送初始化完成信息
    Bluetooth_SendString("BT Ready\r\n");
    Bluetooth_SendString("Smart Dog Connected\r\n");
}

void Bluetooth_SendString(char *str)
{
    while(*str) {
        USART_SendData(USART2, *str++);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    }
}

void Bluetooth_SendData(uint8_t *data, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++) {
        USART_SendData(USART2, data[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    }
}

uint8_t Bluetooth_GetCommand(void)
{
    if(command_received) {
        command_received = 0;
        return current_command;
    }
    return 0;
}

WorkMode Bluetooth_GetMode(void)
{
    return current_mode;
}

uint8_t Bluetooth_Available(void)
{
    return command_received;
}

void Bluetooth_ProcessCommand(uint8_t cmd)
{
    char status_msg[32];
    
    switch(cmd) {
        case CMD_STAND:
            Bluetooth_SendString("CMD: Stand\r\n");
            break;
        case CMD_SIT:
            Bluetooth_SendString("CMD: Sit\r\n");
            break;
        case CMD_WALK_FORWARD:
            Bluetooth_SendString("CMD: Forward\r\n");
            break;
        case CMD_WALK_BACKWARD:
            Bluetooth_SendString("CMD: Backward\r\n");
            break;
        case CMD_TURN_LEFT:
            Bluetooth_SendString("CMD: Turn Left\r\n");
            break;
        case CMD_TURN_RIGHT:
            Bluetooth_SendString("CMD: Turn Right\r\n");
            break;
        case CMD_STOP:
            Bluetooth_SendString("CMD: Stop\r\n");
            break;
        case CMD_SPEED_UP:
            Bluetooth_SendString("CMD: Speed Up\r\n");
            break;
        case CMD_SPEED_DOWN:
            Bluetooth_SendString("CMD: Speed Down\r\n");
            break;
        case CMD_TEST:
            Bluetooth_SendString("CMD: Test Mode\r\n");
            break;
        case CMD_RESET:
            Bluetooth_SendString("CMD: Reset\r\n");
            break;
        default:
            sprintf(status_msg, "Unknown CMD: %c\r\n", cmd);
            Bluetooth_SendString(status_msg);
            break;
    }
}

void Bluetooth_SendStatus(void)
{
    char status_msg[64];
    sprintf(status_msg, "Status: Mode=%d, Speed=%d\r\n", 
            current_mode, Dog_GetWalkSpeed());
    Bluetooth_SendString(status_msg);
}

// USART2中断服务函数
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART2);
        
        // 简单的命令处理：单个字符命令
        if(data >= 'A' && data <= 'Z') {
            current_command = data;
            command_received = 1;
            
            // 回显接收到的命令
            USART_SendData(USART2, data);
            while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        }
        
        // 清除中断标志
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}
