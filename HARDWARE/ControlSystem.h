#ifndef __CONTROL_SYSTEM_H
#define __CONTROL_SYSTEM_H

#include "stm32f10x.h"

// 系统状态
typedef enum {
    SYS_IDLE = 0,
    SYS_STANDING,
    SYS_WALKING,
    SYS_TURNING,
    SYS_SITTING
} SystemState;

// 函数声明
void ControlSystem_Init(void);
void ControlSystem_Update(void);
SystemState ControlSystem_GetState(void);

// 用户控制接口
void Control_Stand(void);
void Control_Sit(void);
void Control_WalkForward(uint8_t steps);
void Control_WalkBackward(uint8_t steps);
void Control_TurnLeft(uint8_t steps);
void Control_TurnRight(uint8_t steps);
void Control_Stop(void);

#endif
