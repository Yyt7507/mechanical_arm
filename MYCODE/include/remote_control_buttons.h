#ifndef __REMOTE_CONTROL_BUTTONS_H__
#define __REMOTE_CONTROL_BUTTONS_H__

#include "stm32f4xx.h"

#define 		CLAW_GRAB_ANG     		120			//в╔
#define 		CLAW_RELEASE_ANG  		90 			//ки

void Study_Mode_While(void);
void Auto_Act(uint8_t *servo);
void Auto_Mode(void);
void claw(void);
void Arm_ResetPos(void);
void Sector_Ctl(void);

#endif
