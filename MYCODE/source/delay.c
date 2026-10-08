#include "delay.h"

#ifndef INC_FREERTOS_H
void delay_us(uint32_t nus)
{
	SysTick->CTRL = 0;     // 关闭系统定时器
	SysTick->LOAD = 21-1;  //数21个数为1us
	SysTick->VAL  = 0;     // 清除当前计数值和计数标志
	SysTick->CTRL = 1;     // 使能系统定时器并选择系统时钟的8分频进行计数
	while(nus--)		   // 循环等待计数标志产生nus次
		while ((SysTick->CTRL & 0x00010000)==0);// 计数标志每隔1us产生一次
	SysTick->CTRL = 0;     // 关闭系统定时器
}

void delay_ms(uint32_t nms)
{
	SysTick->CTRL = 0;     // 关闭系统定时器
	SysTick->LOAD = 21000-1;//数21000个数为1ms
	SysTick->VAL  = 0;     // 清除当前计数值和计数标志
	SysTick->CTRL = 1;     // 使能系统定时器并选择系统时钟的8分频进行计数
	while(nms--)		   // 循环等待计数标志产生nms次
		while ((SysTick->CTRL & 0x00010000)==0);// 计数标志每隔1ms产生一次
	SysTick->CTRL = 0;     // 关闭系统定时器
}

#else
/* 如果有Free RTOS，就利用SysTick里面的计数器来做精准延时
思路：
	先统计计时需要多少个计数值
	然后统计计数器经过了多少个计数值
	当计数值数过的计数值 >= 需要的计数值时，就说明时间到了
问题1：SysTick的计数器是递减计数的，如何统计经过了多少计数值？
	在进入循环前获取一次计数值，保存到told中
	在循环内获取当前计数值，保存到tnow中
	用开始时的计数值 - 当前计数值 = 经过的计数值
	然后更新told，重复刚才的操作，不断记录经过的计数值
问题2：如果计数器当前的值已近快到底了，不够数完需要的计数值怎么办？
例如：需要数16800个数，但计数器当前已近是100了，快数完了怎么办？
	此时told = 100，如果是这种情况，那么当计数器数完后，会重载回最大计数值168000
	假设经过了1000个计数值，此时tnow = 100-1000 = 168000-900 = 167100
	因为当计数器数完100个计数值后，就已经重载回168000了，然后还有900个数
	此时tnow > told，而told+(重载值-tnow) = 经过的计数值
*/
/*
void delay_us(uint32_t nus)
{	 
	uint32_t tnow 	= 0;    	 			// 用来获取当前计数值的变量
	uint32_t tcnt 	= 0;					// 用来统计已经过计数值的变量
	uint32_t ticks 	= nus*168; 				// 计算延时所需的计数值
	uint32_t reload	= SysTick->LOAD;		// 获取SysTick的重载值	
	uint32_t told	= SysTick->VAL;     	// 先获取开始时的计数器值
	while(1)	
	{	
		tnow=SysTick->VAL;				// 每隔一段时间获取一下当前的计数值
		if(tnow != told)				// 如果不等于上一次计数值
		{	    	
			if(tnow < told)				// 如果小于上一次的计数值，意味着这一轮计数还没有更新
				tcnt+=told-tnow;		// 用刚进入时的值 - 当前值 = 已经过的计数值
			else 						// 如果大于上一次的计数值，意味着进入了下一轮计数
				tcnt+=reload-tnow+told;	// 那么就加上上一轮中剩下的计数值和本轮已经过的计数值	    
			told = tnow;				// 更新“上一轮”计数值
			if(tcnt >= ticks)			// 当已经过计数值大于等于需要的计数值
				break;					// 时间超过/等于要延迟的时间,则退出
		}  
	}
}
*/
void delay_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	// 使能TIM2时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);	
	// 设置预分频值：84MHz / 8400 = 10000Hz
	TIM_TimeBaseInitStruct.TIM_Prescaler = 8400-1;
	TIM_TimeBaseInitStruct.TIM_Period = 10-1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_Prescaler = 42-1;
	TIM_TimeBaseInitStruct.TIM_Period = 2-1;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);
	// 使能定时器
	TIM_Cmd(TIM6,ENABLE);
	TIM_Cmd(TIM7,ENABLE);
}

void delay_us(uint32_t nus)
{
	TIM7->CNT = 0;
	TIM_ClearFlag(TIM7,TIM_FLAG_Update);
	while(nus--)
	{
		while(TIM_GetFlagStatus(TIM7,TIM_FLAG_Update) != SET);
		TIM_ClearFlag(TIM7,TIM_FLAG_Update);
	}	
}

void delay_ms(uint32_t ms)
{
	//	vTaskDelay(nms);
	TIM6->CNT = 0;
	TIM_ClearFlag(TIM6,TIM_FLAG_Update);
	while(ms--)
	{
		//等待计数标志产生
		while(TIM_GetFlagStatus(TIM6,TIM_FLAG_Update) != SET);
		TIM_ClearFlag(TIM6,TIM_FLAG_Update);
	}
}
#endif

void delay(float ns)
{
	delay_ms((uint32_t)(ns*1000));
}
