#include "main.h"

volatile uint8_t steering_engine_num = 0x00;
//函数：舵机初始化
void steering_engine_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	//时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	//GPIO复用基础配置
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed	= GPIO_High_Speed;
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	
#if	steering_engine_1
	//初始化PC6为复用模式(TIM3_CH1)
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_6;
	GPIO_Init(GPIOC,&GPIO_InitStruct);	
	//连接引脚的复用对象
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);
#endif

#if	steering_engine_2
	//初始化PC7为复用模式(TIM3_CH2)
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_7;
	GPIO_Init(GPIOC,&GPIO_InitStruct);	
	//连接引脚的复用对象
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM3);	
#endif

#if	steering_engine_3
	//初始化PC8为复用模式(TIM3_CH3)
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_8;
	GPIO_Init(GPIOC,&GPIO_InitStruct);	
	//连接引脚的复用对象
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_TIM3);
#endif

#if	steering_engine_4
	//初始化PC9为复用模式(TIM3_CH4)
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_9;
	GPIO_Init(GPIOC,&GPIO_InitStruct);	
	//连接引脚的复用对象
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM3);
#endif

#if	steering_engine_5
	//初始化PB6为复用模式(TIM4_CH1)
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_6;
	GPIO_Init(GPIOB,&GPIO_InitStruct);	
	//连接引脚的复用对象
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
#endif

#if	steering_engine_6
	//初始化PB7为复用模式(TIM4_CH2)
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_7;
	GPIO_Init(GPIOB,&GPIO_InitStruct);	
	//连接引脚的复用对象
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
#endif

	//初始化定时器基本参数	
	TIM_TimeBaseInitStruct.TIM_Prescaler 	= 84-1;
	TIM_TimeBaseInitStruct.TIM_Period 		= 20000-1;
	/*
	预分频值	计数周期	时长	角度	比较值	占空比	高电平	低电平
	  84		 200000		20ms	0度	 	 500	 2.5%	0.5ms	19.5ms
	  84		 200000		20ms	45度	 1000	 5.0%	1.0ms	19.0ms
	  84		 200000		20ms	90度	 1500	 7.5%	1.5ms	18.5ms
	  84		 200000		20ms	135度	 2000	 10.0%	2.0ms	18.0ms
	  84		 200000		20ms	180度	 2500	 12.5%	2.5ms	17.5ms
	1000个计数值代表1ms
	*/
	TIM_TimeBaseInitStruct.TIM_CounterMode 	= TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision= TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
	//初始化定时器输出比较通道，先初始化所有舵机为90度，让舵机处于复位状态
	TIM_OCInitStruct.TIM_OCMode		= TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OutputState= TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse		= 1500;
	TIM_OCInitStruct.TIM_OCPolarity	= TIM_OCPolarity_High;
	//每个通道的初始化通道函数不一样
	TIM_OC1Init(TIM3, &TIM_OCInitStruct);
	TIM_OC2Init(TIM3, &TIM_OCInitStruct);
	TIM_OC3Init(TIM3, &TIM_OCInitStruct);
	TIM_OC4Init(TIM3, &TIM_OCInitStruct);
	TIM_OC1Init(TIM4, &TIM_OCInitStruct);
	TIM_OC2Init(TIM4, &TIM_OCInitStruct);
	//使能定时器
	TIM_Cmd(TIM3,ENABLE);
	TIM_Cmd(TIM4,ENABLE);
}

/* 
舵机的周期为20ms,可旋转0 ~ 180度
0.5ms -- 0度		比较值：500
1.0ms -- 46度		比较值：1000
1.5ms -- 90度		比较值：1500
2.0ms -- 135度		比较值：2000
2.5ms -- 180度		比较值：2500

可以得出舵机未转动角度时的比较值已经为500了
每转动1角度，比较值增加：（2500-500）/180 ≈ 11.11
每个角度应该设置的比较值：ccr = 500 + angle * 2000 / 180

steering_engine_num			steering_engine			定时器通道
		0x01				steering_engine_1		TIM3_CH1
		0x02				steering_engine_2		TIM3_CH2
		0x04				steering_engine_3		TIM3_CH3
		0x08				steering_engine_4		TIM3_CH4
		0x10				steering_engine_5		TIM4_CH1
		0x20				steering_engine_6		TIM4_CH2
*/
void SG90_SetAngle(uint8_t steering_engine_num, uint8_t angle)
{
    uint16_t ccr;
    if(angle > 180) 
		angle = 180;
    ccr = 500 + (uint16_t)((uint32_t)angle * 2000 / 180);
	switch(steering_engine_num)
	{
		case 0x01:TIM_SetCompare1(TIM3, ccr);break;
		case 0x02:TIM_SetCompare2(TIM3, ccr);break;
		case 0x04:TIM_SetCompare3(TIM3, ccr);break;
		case 0x08:TIM_SetCompare4(TIM3, ccr);break;
		case 0x10:TIM_SetCompare1(TIM4, ccr);break;
		case 0x20:TIM_SetCompare2(TIM4, ccr);break;
		default: break;
	}
}

/*
函数：根据遥感数据转动舵机

舵机从下往上数编号1~6
两路遥感
左摇杆：
LX控制舵机1左右转动
LY控制舵机2、3前后转动（需错峰）
右摇杆：
RX控制舵机5左右转动
RY控制舵机4前后转动
LB(摇杆按钮5)控制机械爪抓取
RB(摇杆按钮6)控制机械爪松开
舵机上电初始角度为90度，全局数组记录六个舵机当前角度
往左、往前都是>90+5°，往右、往后都是<90-5°
四个按键
KEY1复位:
Flash动作序列-->存储一帧一帧的6路舵机角度
{servo1,servo2,servo3,servo4,servo5,servo6}(数组传)
每一组就是一个动作点;同时记录动作总点数。

*/
volatile uint8_t servo_curr_rad[6] = {90,90,90,90,90,90};//
void rad_rev(void)
{
	uint8_t data = hu_m40_data[0];   
	
	if((data & (0x01<<0)) != 0)        //左摇杆左推
	{
		if((servo_curr_rad[0] + SERVO_ADD_RAD) < 180)
		{
			servo_curr_rad[0] += SERVO_ADD_RAD;  //全局变量更新
			SG90_SetAngle(0x01,servo_curr_rad[0]);  //舵机1
		}
		else
		{
			servo_curr_rad[0] = 180;
			SG90_SetAngle(0x01,180); 
		}
	}
	if((data & (0x01<<1)) != 0)        //左摇杆右推
	{
		if((servo_curr_rad[0] - SERVO_ADD_RAD) > 0)
		{
			servo_curr_rad[0] -= SERVO_ADD_RAD;
			SG90_SetAngle(0x01,servo_curr_rad[0]);  //舵机1
		}
		else
		{
			servo_curr_rad[0] = 0;
			SG90_SetAngle(0x01,0); 
		}
	}
	if((data & (0x01<<2)) != 0)        //左摇杆前推
	{
		if((servo_curr_rad[1] + SERVO_ADD_RAD) < 180 && (servo_curr_rad[2] + SERVO_ADD_RAD) <180)
		{
			servo_curr_rad[1] += SERVO_ADD_RAD;
			servo_curr_rad[2] += SERVO_ADD_RAD;
			SG90_SetAngle(0x02,servo_curr_rad[1]);  //舵机2
			SG90_SetAngle(0x04,servo_curr_rad[2]+3);  //舵机3错峰
		}
		else
		{
			servo_curr_rad[1] = 180;
			servo_curr_rad[2] = 180;
			SG90_SetAngle(0x02,180); 
			SG90_SetAngle(0x04,180);
		}
	}
	if((data & (0x01<<3)) != 0)        //左摇杆后推
	{
		if((servo_curr_rad[1] - SERVO_ADD_RAD) > 0 && (servo_curr_rad[2] - SERVO_ADD_RAD) > 0)
		{
			servo_curr_rad[1] -= SERVO_ADD_RAD;
			servo_curr_rad[2] -= SERVO_ADD_RAD;
			SG90_SetAngle(0x02,servo_curr_rad[1]);  //舵机2
			SG90_SetAngle(0x04,servo_curr_rad[2]-3);  //舵机3错峰
		}
		else
		{
			servo_curr_rad[1] = 0;
			servo_curr_rad[2] = 0;
			SG90_SetAngle(0x02,0); 
			SG90_SetAngle(0x04,0);
		}
	}
	if((data & (0x01<<4)) != 0)        //右摇杆左推
	{
		if((servo_curr_rad[4] + SERVO_ADD_RAD) < 180)
		{
			servo_curr_rad[4] += SERVO_ADD_RAD;
			SG90_SetAngle(0x10,servo_curr_rad[4]);  //舵机5
		}
		else
		{
			servo_curr_rad[4] = 180;
			SG90_SetAngle(0x10,180); 
		}
	}
	if((data & (0x01<<5)) != 0)        //右摇杆右推
	{
		if((servo_curr_rad[4] - SERVO_ADD_RAD) > 0)
		{
			servo_curr_rad[4] -= SERVO_ADD_RAD;
			SG90_SetAngle(0x10,servo_curr_rad[4]);  //舵机5
		}
		else
		{
			servo_curr_rad[4] = 0;
			SG90_SetAngle(0x10,0); 
		}
	}
	if((data & (0x01<<6)) != 0)        //右摇杆前推
	{
		if((servo_curr_rad[3] + SERVO_ADD_RAD) < 180)
		{
			servo_curr_rad[3]+= SERVO_ADD_RAD;
			SG90_SetAngle(0x08,servo_curr_rad[3]);  //舵机4
		}
		else
		{
			servo_curr_rad[3] = 180;
			SG90_SetAngle(0x08,180); 
		}
	}
	if((data & (0x01<<7)) != 0)        //右摇杆后推
	{
		if((servo_curr_rad[3] - SERVO_ADD_RAD) > 0)
		{
			servo_curr_rad[3] -= SERVO_ADD_RAD;
			SG90_SetAngle(0x08,servo_curr_rad[3]);  //舵机4
		}
		else
		{
			servo_curr_rad[3] = 0;
			SG90_SetAngle(0x08,0); 
		}
	}
	
	printf("servo0=%d servo1=%d servo2=%d servo3=%d servo4=%d servo5=%d\r\n",
       servo_curr_rad[0], servo_curr_rad[1], servo_curr_rad[2],
       servo_curr_rad[3], servo_curr_rad[4], servo_curr_rad[5]);
}
