#ifndef __BLUETOOTH_CONTROL_H
#define __BLUETOOTH_CONTROL_H

#include "stm32f10x.h"

void BluetoothControl_Init(void);
void BluetoothControl_Update(void);
void BluetoothControl_ProcessCommand(uint8_t cmd);
uint8_t BluetoothControl_IsActive(void);

#endif
