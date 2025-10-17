#ifndef __DOG_ACTIONS_H
#define __DOG_ACTIONS_H

#include "stm32f10x.h"

// 动作模式定义
typedef enum {
    MODE_STAND = 0,
    MODE_SIT,          
    MODE_WALK_FORWARD, 
    MODE_WALK_BACKWARD,
    MODE_TURN_LEFT,    
    MODE_TURN_RIGHT,   
    MODE_INIT          
} DogMode;

// 舵机ID定义 
typedef enum {
    SERVO_FRONT_RIGHT = 1,  // 舵机1 -> 前右腿
    SERVO_FRONT_LEFT = 2,   // 舵机2 -> 前左腿  
    SERVO_REAR_LEFT = 3,    // 舵机3 -> 后左腿
    SERVO_REAR_RIGHT = 4    // 舵机4 -> 后右腿
} ServoID;

// 舵机角度配置结构体
typedef struct {
    float stand;      // 站立
    float sit;        // 坐下
    float lift_high;  // 抬腿高位
    float lift_low;   // 抬腿低位
    float push_high;  // 推地位
    float push_low;   // 推地低位
} ServoAngles;

// 函数声明
void Dog_Init(void);
void Dog_Stand(void);
void Dog_Sit(void);
void Dog_WalkForward(uint8_t steps);
void Dog_WalkBackward(uint8_t steps);
void Dog_TurnLeft(uint8_t steps);
void Dog_TurnRight(uint8_t steps);
void Dog_ResetPose(void);
void Dog_Stop(void);
void Dog_TestServos(void);

// 高级控制函数
void Dog_SetWalkSpeed(uint8_t speed);
void Dog_SetActionCompleteCallback(void (*callback)(void));
uint8_t Dog_GetWalkSpeed(void);
void Dog_AdjustServoConfig(uint8_t servo_id, float stand, float sit, 
                          float lift_high, float lift_low, 
                          float push_high, float push_low);
ServoAngles Dog_GetServoConfig(uint8_t servo_id);

// 工具函数
void Dog_SetAllServos(float fl_angle, float fr_angle, float rl_angle, float rr_angle);
void Dog_SmoothMove(uint8_t servo_id, float start_angle, float end_angle, uint16_t duration_ms);

void Dog_Action_Hello(void);
void Dog_Action_SitDown(void);
void Dog_Action_ShakeBody(void);

#endif
