#include "DogActions.h"
#include "Servo.h"
#include "Delay.h"
#include "stddef.h"

// 全局变量
static uint8_t WalkSpeed = 5;
static void (*ActionCompleteCallback)(void) = NULL;

// 舵机角度配置
static ServoAngles ServoConfig[5] = {
    {0, 0, 0, 0, 0, 0}, // 索引0不用
    // 舵机1 - 前右腿
    {90.0f,  45.0f,  110.0f, 70.0f,  120.0f, 60.0f},
    // 舵机2 - 前左腿  
    {90.0f,  135.0f, 70.0f,  110.0f, 60.0f,  120.0f},
    // 舵机3 - 后左腿
    {90.0f,  135.0f, 70.0f,  110.0f, 60.0f,  120.0f},
    // 舵机4 - 后右腿
    {90.0f,  45.0f,  110.0f, 70.0f,  120.0f, 60.0f}
};

void Dog_Init(void)
{
    Servo_Init();
    Dog_ResetPose();
}

void Dog_SetAllServos(float fl_angle, float fr_angle, float rl_angle, float rr_angle)
{
    Servo_SetAngle(SERVO_FRONT_LEFT, fl_angle);
    Servo_SetAngle(SERVO_FRONT_RIGHT, fr_angle);
    Servo_SetAngle(SERVO_REAR_LEFT, rl_angle);
    Servo_SetAngle(SERVO_REAR_RIGHT, rr_angle);
}

void Dog_SmoothMove(uint8_t servo_id, float start_angle, float end_angle, uint16_t duration_ms)
{
    const uint8_t steps = 10;
    float current_angle = start_angle;
    float increment = (end_angle - start_angle) / steps;
    uint16_t step_delay = duration_ms / steps;
    
    for(uint8_t i = 0; i <= steps; i++) {
        Servo_SetAngle(servo_id, current_angle);
        current_angle += increment;
        Delay_ms(step_delay);
    }
}

void Dog_SetWalkSpeed(uint8_t speed)
{
    if(speed >= 1 && speed <= 10) {
        WalkSpeed = speed;
    }
}

void Dog_SetActionCompleteCallback(void (*callback)(void))
{
    ActionCompleteCallback = callback;
}

static void Dog_NotifyActionComplete(void)
{
    if(ActionCompleteCallback != NULL) {
        ActionCompleteCallback();
    }
}

void Dog_Stand(void)
{
    Servo_SetAngle(SERVO_FRONT_LEFT, ServoConfig[SERVO_FRONT_LEFT].stand);
    Servo_SetAngle(SERVO_FRONT_RIGHT, ServoConfig[SERVO_FRONT_RIGHT].stand);
    Servo_SetAngle(SERVO_REAR_LEFT, ServoConfig[SERVO_REAR_LEFT].stand);
    Servo_SetAngle(SERVO_REAR_RIGHT, ServoConfig[SERVO_REAR_RIGHT].stand); 
}

void Dog_Sit(void)
{
    Dog_SetAllServos(
        ServoConfig[SERVO_FRONT_LEFT].sit,
        ServoConfig[SERVO_FRONT_RIGHT].sit,
        ServoConfig[SERVO_REAR_LEFT].sit,
        ServoConfig[SERVO_REAR_RIGHT].sit
    );
    Delay_ms(500);
}

void Dog_WalkForward(uint8_t steps)
{
    uint16_t step_delay = 200 - (WalkSpeed * 15);
    
    for(uint8_t step = 0; step < steps; step++) {
        
        // 💥 修正：分时启动，平滑电流尖峰
        
        // 相位1：抬左前腿和右后腿，推右前腿和左后腿
        Servo_SetAngle(SERVO_FRONT_LEFT, ServoConfig[SERVO_FRONT_LEFT].lift_high);   // 前左抬 (ID 2)
        Delay_ms(20); // <-- 增加 20ms 延时
        Servo_SetAngle(SERVO_FRONT_RIGHT, ServoConfig[SERVO_FRONT_RIGHT].push_low);  // 前右推 (ID 1)
        Delay_ms(20); // <-- 增加 20ms 延时
        Servo_SetAngle(SERVO_REAR_LEFT, ServoConfig[SERVO_REAR_LEFT].push_low);      // 后左推 (ID 3)
        Delay_ms(20); // <-- 增加 20ms 延时
        Servo_SetAngle(SERVO_REAR_RIGHT, ServoConfig[SERVO_REAR_RIGHT].lift_high);   // 后右抬 (ID 4)
        Delay_ms(step_delay / 2); // 保持原来的主延时
        
        // 相位2：向前摆动
        Servo_SetAngle(SERVO_FRONT_LEFT, ServoConfig[SERVO_FRONT_LEFT].lift_low);    // 前左摆 (ID 2)
        Delay_ms(20);
        Servo_SetAngle(SERVO_FRONT_RIGHT, ServoConfig[SERVO_FRONT_RIGHT].push_high); // 前右摆 (ID 1)
        Delay_ms(20);
        Servo_SetAngle(SERVO_REAR_LEFT, ServoConfig[SERVO_REAR_LEFT].push_high);     // 后左摆 (ID 3)
        Delay_ms(20);
        Servo_SetAngle(SERVO_REAR_RIGHT, ServoConfig[SERVO_REAR_RIGHT].lift_low);    // 后右摆 (ID 4)
        Delay_ms(step_delay / 2);
        
        // 相位3：抬右前腿和左后腿，推左前腿和右后腿
        Servo_SetAngle(SERVO_FRONT_LEFT, ServoConfig[SERVO_FRONT_LEFT].push_low);    // 前左推 (ID 2)
        Delay_ms(20);
        Servo_SetAngle(SERVO_FRONT_RIGHT, ServoConfig[SERVO_FRONT_RIGHT].lift_high); // 前右抬 (ID 1)
        Delay_ms(20);
        Servo_SetAngle(SERVO_REAR_LEFT, ServoConfig[SERVO_REAR_LEFT].lift_high);     // 后左抬 (ID 3)
        Delay_ms(20);
        Servo_SetAngle(SERVO_REAR_RIGHT, ServoConfig[SERVO_REAR_RIGHT].push_low);    // 后右推 (ID 4)
        Delay_ms(step_delay / 2);
        
        // 相位4：向前摆动
        Servo_SetAngle(SERVO_FRONT_LEFT, ServoConfig[SERVO_FRONT_LEFT].push_high);   // 前左摆 (ID 2)
        Delay_ms(20);
        Servo_SetAngle(SERVO_FRONT_RIGHT, ServoConfig[SERVO_FRONT_RIGHT].lift_low);  // 前右摆 (ID 1)
        Delay_ms(20);
        Servo_SetAngle(SERVO_REAR_LEFT, ServoConfig[SERVO_REAR_LEFT].lift_low);      // 后左摆 (ID 3)
        Delay_ms(20);
        Servo_SetAngle(SERVO_REAR_RIGHT, ServoConfig[SERVO_REAR_RIGHT].push_high);   // 后右摆 (ID 4)
        Delay_ms(step_delay / 2);
    }
    
    Dog_Stand();
}

void Dog_WalkBackward(uint8_t steps)
{
    uint16_t step_delay = 200 - (WalkSpeed * 15);
    
    for(uint8_t step = 0; step < steps; step++) {
        Dog_SetAllServos(
            ServoConfig[SERVO_FRONT_LEFT].lift_high,
            ServoConfig[SERVO_FRONT_RIGHT].push_low,
            ServoConfig[SERVO_REAR_LEFT].push_low,
            ServoConfig[SERVO_REAR_RIGHT].lift_high
        );
        Delay_ms(step_delay / 2);
        
        Dog_SetAllServos(
            ServoConfig[SERVO_FRONT_LEFT].lift_low,
            ServoConfig[SERVO_FRONT_RIGHT].push_high,
            ServoConfig[SERVO_REAR_LEFT].push_high,
            ServoConfig[SERVO_REAR_RIGHT].lift_low
        );
        Delay_ms(step_delay / 2);
        
        Dog_SetAllServos(
            ServoConfig[SERVO_FRONT_LEFT].push_low,
            ServoConfig[SERVO_FRONT_RIGHT].lift_high,
            ServoConfig[SERVO_REAR_LEFT].lift_high,
            ServoConfig[SERVO_REAR_RIGHT].push_low
        );
        Delay_ms(step_delay / 2);
        
        Dog_SetAllServos(
            ServoConfig[SERVO_FRONT_LEFT].push_high,
            ServoConfig[SERVO_FRONT_RIGHT].lift_low,
            ServoConfig[SERVO_REAR_LEFT].lift_low,
            ServoConfig[SERVO_REAR_RIGHT].push_high
        );
        Delay_ms(step_delay / 2);
    }
    
    Dog_Stand();
    // Dog_NotifyActionComplete();
}


void Dog_TurnLeft(uint8_t steps)
{
    uint16_t step_delay = 300 - (WalkSpeed * 20);
    
    for(uint8_t step = 0; step < steps; step++) {
        // 左转：右腿向前，左腿向后
        Servo_SetAngle(SERVO_FRONT_LEFT, 70);   // 前左向后
        Servo_SetAngle(SERVO_FRONT_RIGHT, 110); // 前右向前
        Servo_SetAngle(SERVO_REAR_LEFT, 70);    // 后左向后  
        Servo_SetAngle(SERVO_REAR_RIGHT, 110);  // 后右向前
        Delay_ms(step_delay);
        
        Servo_SetAngle(SERVO_FRONT_LEFT, 110);  // 前左向前
        Servo_SetAngle(SERVO_FRONT_RIGHT, 70);  // 前右向后
        Servo_SetAngle(SERVO_REAR_LEFT, 110);   // 后左向前
        Servo_SetAngle(SERVO_REAR_RIGHT, 70);   // 后右向后
        Delay_ms(step_delay);
    }
    
    Dog_Stand();
}

void Dog_TurnRight(uint8_t steps)
{
    uint16_t step_delay = 300 - (WalkSpeed * 20);
    
    for(uint8_t step = 0; step < steps; step++) {
        // 💥 修正：右转：左腿向前，右腿向后
        Servo_SetAngle(SERVO_FRONT_LEFT, 110);  // 前左向前
        Servo_SetAngle(SERVO_FRONT_RIGHT, 70);   // 前右向后
        Servo_SetAngle(SERVO_REAR_LEFT, 110);    // 后左向前  
        Servo_SetAngle(SERVO_REAR_RIGHT, 70);   // 后右向后
        Delay_ms(step_delay);
        
        // 💥 修正：反向动作
        Servo_SetAngle(SERVO_FRONT_LEFT, 70);   // 前左向后
        Servo_SetAngle(SERVO_FRONT_RIGHT, 110);  // 前右向前
        Servo_SetAngle(SERVO_REAR_LEFT, 70);    // 后左向后
        Servo_SetAngle(SERVO_REAR_RIGHT, 110);  // 后右向前
        Delay_ms(step_delay);
    }
    
    Dog_Stand();
    //Dog_NotifyActionComplete();
}

void Dog_ResetPose(void)
{
    Dog_SetAllServos(90, 90, 90, 90);
    Delay_ms(500);
}

void Dog_Stop(void)
{
    Dog_Stand();
}

void Dog_TestServos(void)
{
    Servo_SetAngle(SERVO_FRONT_LEFT, 0);
    Delay_ms(500);
    Servo_SetAngle(SERVO_FRONT_LEFT, 180);
    Delay_ms(500);
    Servo_SetAngle(SERVO_FRONT_LEFT, 90);
    Delay_ms(500);
    
    Servo_SetAngle(SERVO_FRONT_RIGHT, 0);
    Delay_ms(500);
    Servo_SetAngle(SERVO_FRONT_RIGHT, 180);
    Delay_ms(500);
    Servo_SetAngle(SERVO_FRONT_RIGHT, 90);
    Delay_ms(500);
    
    Servo_SetAngle(SERVO_REAR_LEFT, 0);
    Delay_ms(500);
    Servo_SetAngle(SERVO_REAR_LEFT, 180);
    Delay_ms(500);
    Servo_SetAngle(SERVO_REAR_LEFT, 90);
    Delay_ms(500);
    
    Servo_SetAngle(SERVO_REAR_RIGHT, 0);
    Delay_ms(500);
    Servo_SetAngle(SERVO_REAR_RIGHT, 180);
    Delay_ms(500);
    Servo_SetAngle(SERVO_REAR_RIGHT, 90);
    Delay_ms(500);
}

uint8_t Dog_GetWalkSpeed(void)
{
    return WalkSpeed;
}

void Dog_AdjustServoConfig(uint8_t servo_id, float stand, float sit, 
                          float lift_high, float lift_low, 
                          float push_high, float push_low)
{
    if(servo_id >= 1 && servo_id <= 4) {
        ServoConfig[servo_id].stand = stand;
        ServoConfig[servo_id].sit = sit;
        ServoConfig[servo_id].lift_high = lift_high;
        ServoConfig[servo_id].lift_low = lift_low;
        ServoConfig[servo_id].push_high = push_high;
        ServoConfig[servo_id].push_low = push_low;
    }
}

ServoAngles Dog_GetServoConfig(uint8_t servo_id)
{
    if(servo_id >= 1 && servo_id <= 4) {
        return ServoConfig[servo_id];
    }
    return ServoConfig[0];
}

void Dog_WalkForward_Smooth(uint8_t steps)
{
    uint16_t step_delay = 250 - (WalkSpeed * 20);
    
    for(uint8_t step = 0; step < steps; step++) {
        // 更平滑的四相位步态
        // 相位1：准备
        Dog_SetAllServos(95, 85, 95, 85);
        Delay_ms(step_delay/4);
        
        // 相位2：抬腿
        Dog_SetAllServos(110, 70, 70, 110);
        Delay_ms(step_delay/4);
        
        // 相位3：摆动
        Dog_SetAllServos(100, 80, 80, 100);
        Delay_ms(step_delay/4);
        
        // 相位4：落地
        Dog_SetAllServos(90, 90, 90, 90);
        Delay_ms(step_delay/4);
    }
    
    Dog_Stand();
}

void Dog_Action_Hello(void)
{
    // 抬起右前腿挥手
    for(int i=0; i<2; i++) {
        Servo_SetAngle(SERVO_FRONT_RIGHT, 45);  // 抬起
        Delay_ms(300);
        Servo_SetAngle(SERVO_FRONT_RIGHT, 90);  // 放下
        Delay_ms(300);
    }
    Dog_Stand();
}

void Dog_Action_SitDown(void)
{
    // 蹲下动作
    Dog_SetAllServos(
        60,   // 前左
        120,  // 前右  
        120,  // 后左
        60    // 后右
    );
    Delay_ms(1000);
}

void Dog_Action_ShakeBody(void)
{
    // 抖动身体
    for(int i=0; i<3; i++) {
        Dog_SetAllServos(95, 85, 95, 85);
        Delay_ms(150);
        Dog_SetAllServos(85, 95, 85, 95);
        Delay_ms(150);
    }
    Dog_Stand();
}