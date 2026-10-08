#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f4xx.h"
#include "string.h"
#include "sys.h"
#include "servo_driver.h"
#include "flash.h"
#include "usart.h"
#include "delay.h"
#include "tim.h"
#include "remote_control_buttons.h"
#include "hu_m40.h"
#include "nRF24L01.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#define 		MAX			98
#define 		MIN			158
#define 	RECORD_MAX_POINT 		500	
//定义事件标志位
#define		EVENT_1		0x01
#define		EVENT_2		0x02
#define		EVENT_3		0x04
#define		EVENT_4		0x08

extern volatile uint8_t  sector_flag;
extern volatile uint8_t servo_curr_rad[6];	  
extern volatile uint8_t key3_flag;
extern volatile uint8_t key4_flag;
extern volatile uint16_t record_cnt;
extern volatile uint32_t total_len;
extern volatile uint8_t servo[6];
extern volatile uint8_t total_point;
extern volatile uint32_t auto_offset;
extern volatile uint32_t study_offset;

extern volatile uint8_t hu_m40_data[2];	
extern volatile uint8_t record_time[4];	
extern volatile uint8_t hu_m40_key;	  	
extern volatile uint8_t stop_request;		
extern volatile uint8_t state;	

enum{
	STATE_IDLE = 0,		//空闲状态，等待遥控器指令，舵机保持当前位置
	STATE_MANUAL = 1,	//手动控制状态，事件1--按键2按下（抓夹）
	STATE_RECORDING = 2,//录制轨迹状态，事件2--按键3按下
	STATE_PLAYBACK = 3,	//复刻轨迹状态，事件3--按键4按下
	STATE_RESET = 4		//复位状态，事件4--按键1按下
};

void detection_state(void);
void read_hu_m40_data(void);
void usart_test_m40_data(void);

#endif
