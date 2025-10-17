#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "stm32f10x.h"

// 蓝牙命令定义
typedef enum {
    CMD_STAND = 'S',         // 站立
    CMD_SIT = 'T',           // 坐下  
    CMD_WALK_FORWARD = 'F',  // 前进
    CMD_WALK_BACKWARD = 'B', // 后退
    CMD_TURN_LEFT = 'L',     // 左转
    CMD_TURN_RIGHT = 'R',    // 右转
    CMD_STOP = 'P',          // 停止
    CMD_SPEED_UP = 'U',      // 加速
    CMD_SPEED_DOWN = 'D',    // 减速
    CMD_TEST = 'M',          // 测试模式
    CMD_RESET = 'X'          // 重置
} BluetoothCommand;

// 工作模式
typedef enum {
    MODE_MANUAL = 0,     // 手动遥控模式
    MODE_AUTO = 1,       // 自主模式
    MODE_FOLLOW = 2      // 跟随模式
} WorkMode;

// 函数声明
void Bluetooth_Init(void);
void Bluetooth_SendString(char *str);
void Bluetooth_SendData(uint8_t *data, uint16_t len);
uint8_t Bluetooth_GetCommand(void);
WorkMode Bluetooth_GetMode(void);
uint8_t Bluetooth_Available(void);
void Bluetooth_ProcessCommand(uint8_t cmd);
void Bluetooth_SendStatus(void);

#endif
