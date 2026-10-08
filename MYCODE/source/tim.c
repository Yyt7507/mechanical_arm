#include "main.h"

void tim2_init(void)// tim2设置的计数周期为1毫秒
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_Prescaler = 8400-1;
	TIM_TimeBaseInitStruct.TIM_Period = 10-1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM2, ENABLE);
}

/*
函数：按键防抖--按键单次触发
当该按键的record_time达到了我们要求的时间DEBOUNCE_TIME时
对hu_m40_data[1]对应bit位置1，并保留hu_m40_key按键触发标志位的置1
这样的话，一直按下按键不松开，也只会hu_m40_data[1]对应bit位置1一次
并且只当hu_m40_key & 0x01的结果不成立时清除record_time
而hu_m40_data[1]对应bit位何时清零呢？在状态机中清零
*/
void key_debounce()
{
	//按键1按下
	if(hu_m40_key & 0x01)
	{
		if(record_time[0] < DEBOUNCE_TIME)
		{
			record_time[0]++;
			if (record_time[0] == DEBOUNCE_TIME)//最好只在record_time==DEBOUNCE_TIME时将flag置1，避免重复置1发生错误		
				hu_m40_data[1] |= 0x01;
		}
	}
	else
		record_time[0] = 0;
	//按键2按下
	if(hu_m40_key & 0x02)
	{
		if(record_time[1] < DEBOUNCE_TIME)
		{
			record_time[1]++;
			if (record_time[1] == DEBOUNCE_TIME)
				hu_m40_data[1] |= 0x01<<1;
		}
	}
	else
		record_time[1] = 0;
	//按键3按下
	if(hu_m40_key & 0x04)
	{
		if(record_time[2] < DEBOUNCE_TIME)
		{
			record_time[2]++;
			if (record_time[2] == DEBOUNCE_TIME)
				hu_m40_data[1] |= 0x01<<2;
		}
	}
	else
		record_time[2] = 0;
	//按键4按下
	if(hu_m40_key & 0x08)
	{
		if(record_time[3] < DEBOUNCE_TIME)
		{
			record_time[3]++;
			if (record_time[3] == DEBOUNCE_TIME)
				hu_m40_data[1] |= 0x01<<3;
			
			printf("record_time[3] = %d\r\n", record_time[3]);
		}
	}
	else
		record_time[3] = 0;
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		key_debounce();			
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void tim5_init(void)// tim2设置的计数周期为20毫秒
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_Prescaler = 8400-1;
	TIM_TimeBaseInitStruct.TIM_Period = 200-1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM5, ENABLE);
}

//20ms调用一次中断执行Study_Mode()
void TIM5_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM5,TIM_FLAG_Update) == SET)
    {
        if((key3_flag == 1) && (record_cnt < RECORD_MAX_POINT))
        {
			Study_Mode_While();
			if(record_cnt == RECORD_MAX_POINT)
			{
				key3_flag = 0;
			}
			if(record_cnt % 50 == 0)
				printf("rec cnt=%u, offset=%u\r\n", record_cnt, study_offset);
        }
		else if(key4_flag == 1)
		{
			Auto_Mode();
			if(auto_offset >= total_len)
			{
				if(stop_request == 1)
				{
					auto_offset = 0;
					key4_flag = 0;
					stop_request = 0;				
					state = STATE_IDLE;
				}
				else
				{
					auto_offset = 0;
				}
			}
			if(auto_offset % 300 == 0)
				printf("play offset=%u\r\n", auto_offset);
		}
		TIM_ClearITPendingBit(TIM5,TIM_FLAG_Update);
    }
}
