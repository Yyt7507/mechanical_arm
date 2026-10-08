#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include "stm32f4xx.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "servo_driver.h"

// 环形缓冲区大小（必须是2的幂，便于取模优化，这里用64）
#define BT_BUF_SIZE     64

uint16_t bluetooth_available(void);
uint8_t bluetooth_read_byte(uint8_t *ch);
void bluetooth_send_string(const char *str);
void ParseBluetoothCommand(char *cmd);

#endif
