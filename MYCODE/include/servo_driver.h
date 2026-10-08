#ifndef __SERVO_DRIVER_H__
#define __SERVO_DRIVER_H__

#include "stm32f4xx.h"

#define		steering_engine_1		1		//PC6	TIM3
#define		steering_engine_2		1		//PC7	TIM3
#define		steering_engine_3		1		//PC8	TIM3
#define		steering_engine_4		1		//PC9	TIM3
#define		steering_engine_5		1		//PB6	TIM4
#define		steering_engine_6		1		//PB7	TIM4

#define		SERVO_ADD_RAD			1

extern volatile uint8_t steering_engine_num;

void steering_engine_init(void);
void SG90_SetAngle(uint8_t steering_engine_num, uint8_t angle);
void rad_rev(void);

#endif
