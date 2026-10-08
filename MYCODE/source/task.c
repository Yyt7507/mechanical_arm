#include "task.h"

void detection_state(void)
{		
	switch(state)
	{
		case STATE_IDLE:
		{
			if((hu_m40_data[1] & 0x01) != 0)//按键一按下--复位
			{
				//将按键1对应bit位清零
				hu_m40_data[1] &= ~(0x01);				
				state = STATE_RESET;
				xEventGroupSetBits(xEventGroup, EVENT_4);//事件4产生
			}
			else if((hu_m40_data[1] & (0x01<<1)) != 0)//按键二按下--抓夹
			{
				//将按键2对应bit位清零
				hu_m40_data[1] &= ~(0x01<<1);
				state = STATE_MANUAL;
				xEventGroupSetBits(xEventGroup, EVENT_1);//事件1产生
			}
			else if((hu_m40_data[1] & (0x01<<2)) != 0)//按键三按下--录制轨迹
			{
				key3_flag = (key3_flag + 1) % 2;
				//将按键3对应bit位清零
				hu_m40_data[1] &= ~(0x01<<2);
				state = STATE_RECORDING;
				xEventGroupSetBits(xEventGroup, EVENT_2);//事件2产生
			}
			else if((hu_m40_data[1] & (0x01<<3)) != 0)//按键四按下--复刻轨迹
			{
				//将复刻按键对应bit位清零
				hu_m40_data[1] &= ~(0x01<<3);
				taskENTER_CRITICAL();
				if(key4_flag == 0)
				{
					key4_flag = 1;
					stop_request = 0;
					state = STATE_PLAYBACK;
					printf("BBB\r\n");
				}
				else
				{
					stop_request = 1;
				}
				taskEXIT_CRITICAL();
				xEventGroupSetBits(xEventGroup, EVENT_3);//事件3产生
			}
			break;
		}
		case STATE_MANUAL:
		{
			
			break;
		}
		case STATE_RECORDING:
		{	
			if((hu_m40_data[1] & 0x01) != 0)//按键一按下--复位
			{				
				state = STATE_RESET;
				xEventGroupSetBits(xEventGroup, EVENT_4);//事件4产生
			}
			else if((hu_m40_data[1] & (0x01<<1)) != 0)//按键二按下--抓夹
			{
				//将按键2对应bit位清零
				hu_m40_data[1] &= ~(0x01<<1);
				state = STATE_MANUAL;
				xEventGroupSetBits(xEventGroup, EVENT_1);//事件1产生
			}			
			else if((hu_m40_data[1] & (0x01<<2)) != 0)//按键三按下--录制轨迹
			{
				key3_flag = (key3_flag + 1) % 2;
				//将按键3对应bit位清零
				hu_m40_data[1] &= ~(0x01<<2);
				state = STATE_RECORDING;				
				xEventGroupSetBits(xEventGroup, EVENT_2);//事件2产生				
			}
			break;
		}
		case STATE_PLAYBACK:
		{
			if((hu_m40_data[1] & (0x01<<3)) != 0)//按键四按下--复刻轨迹
			{
				//将复刻按键对应bit位清零
				hu_m40_data[1] &= ~(0x01<<3);
				taskENTER_CRITICAL();//进入临界保护区
				if(key4_flag == 0)
				{
					key4_flag = 1;
					stop_request = 0;
					state = STATE_PLAYBACK;
				}
				else
				{
					stop_request = 1;		
				}
				taskEXIT_CRITICAL();//退出临界保护区
				xEventGroupSetBits(xEventGroup, EVENT_3);//事件3产生
			}
			break;
		}
		case STATE_RESET:
		{	
			break;
		}
	}
}

/*
函数：读取hu_m40发来的数据，将数据转换为舵机转动的方向
hu_m40_data[0]表示遥感数据
hu_m40_key表示按键被触发了
真正确认按键有数据是在定时器中做了防抖以后，才能往hu_m40_data[1]写入数据
bit位1表示触发

该函数是在任务中循环出触发的，故hu_m40_data[0]和hu_m40_key每次都会覆盖上一次的数据
为了双重保险，hu_m40_data[0]和hu_m40_data[1]会在消息队列发送完数据后，再一次进行清除
*/
void read_hu_m40_data(void)
{	
	hu_m40_data[0] = 0;
    hu_m40_key = 0; 
	if (hu_m40_read() == 1) // ==1有收到数据
	{
		//摇杆
		uint8_t lx = hu_m40_analog(HU_LX);
		uint8_t ly = hu_m40_analog(HU_LY);
		uint8_t rx = hu_m40_analog(HU_RX);
		uint8_t ry = hu_m40_analog(HU_RY);

		//左摇杆(左右)
		if(lx<=MAX) // 左摇杆往左推存入0bit 
			hu_m40_data[0] |= 0x01;
		else
			hu_m40_data[0] &= ~(0x01);
		if(lx>=MIN && lx<=255)//左摇杆往右推存入1bit
			hu_m40_data[0] |= 0x01<<1;
		else
			hu_m40_data[0] &= ~(0x01<<1);
		//左摇杆(前后)
		if(ly<=MAX) // 左摇杆往前推存入2bit
			hu_m40_data[0] |= 0x01<<2;
		else
			hu_m40_data[0] &= ~(0x01<<2);
		if(ly>=MIN && ly<=255) // 左摇杆往后推，存入3bit
			hu_m40_data[0] |= 0x01<<3;
		else
			hu_m40_data[0] &= ~(0x01<<3);
		//右摇杆(左右)
		if(rx<=MAX) // 右摇杆往左推存入4bit
			hu_m40_data[0] |= 0x01<<4;
		else
			hu_m40_data[0] &= ~(0x01<<4);
		if(rx>=MIN && rx<=255)// 右摇杆往右推存入5bit
			hu_m40_data[0] |= 0x01<<5;
		else
			hu_m40_data[0] &= ~(0x01<<5);
		//右摇杆(前后)
		if(ry<=MAX) // 右摇杆往前推存入6bit
			hu_m40_data[0] |= 0x01<<6;
		else
			hu_m40_data[0] &= ~(0x01<<6);
		if(ry>=MIN && ry<=255) // 右摇杆往后推，存入7bit
			hu_m40_data[0] |= 0x01<<7;		
		else
			hu_m40_data[0] &= ~(0x01<<7);
		
		//按键
		// KEY1--复位，存入1bit
		if (hu_m40_button(HU_K1))
			hu_m40_key |= 0x01;
		else
			hu_m40_key &= ~(0x01);
		// KEY2--抓夹，存入2bit
		if (hu_m40_button(HU_K2))
			hu_m40_key |= 0x01<<1;
		else
			hu_m40_key &= ~(0x01<<1);
		// KEY3--录制轨迹，存入3bit
		if (hu_m40_button(HU_K3))
			hu_m40_key |= 0x01<<2;
		else
			hu_m40_key &= ~(0x01<<2);
		// KEY4--复刻轨迹，存入4bit
		if (hu_m40_button(HU_K4))
		{
			hu_m40_key |= 0x01<<3;			
				printf("AAA\r\n");
		}
		else
			hu_m40_key &= ~(0x01<<3);
	}
}

/*
任务一：不断接收NRF发送的数据吗，将接收的NRF数据转换为舵机齿轮运转方向
*/
void task1_recv_hu_m40_data(void *arg)
{
	while(1)
	{		
		read_hu_m40_data();
		if (hu_m40_data[0] != 0)
        {
            rad_rev();
            hu_m40_data[0] = 0;
        }
		vTaskDelay(5);
	}
}   

/*
任务二：状态机切换
*/
void task2_analyse_hu_m40_data(void *arg)
{
	while(1)
	{
		detection_state();
		vTaskDelay(1);
	}
} 

/*
事件标志位event_1执行的任务：手动控制--按键2按下
*/
void task_event_1(void *arg)
{
	while(1)
	{
		//等待事件标志位event_1为1，任务执行完毕后自动将事件标志组对应bit位清零
		xEventGroupWaitBits(xEventGroup,EVENT_1, pdTRUE, pdTRUE, portMAX_DELAY);
		state = STATE_IDLE;	
		//抓夹函数
		claw();
	}
}

/*
事件标志位event_2执行的任务：录制轨迹--按键3按下
*/
void task_event_2(void *arg)
{
	while(1)
	{
		//等待事件标志位event_2为1，任务执行完毕后自动将事件标志组对应bit位清零
		xEventGroupWaitBits(xEventGroup,EVENT_2, pdTRUE, pdTRUE, portMAX_DELAY);
		printf("rec start, key3_flag=%d\r\n", key3_flag);
		//判断录制轨迹是否结束
		if(key3_flag == 0)
		{
			state = STATE_IDLE;//切换回空闲状态
			RTC_WriteBackupRegister(RTC_BKP_DR4, study_offset);//双重保险，再次将学习到的字节数写入RTC数据备份寄存器
            printf("rec stop, final RTC=%u\r\n", RTC_ReadBackupRegister(RTC_BKP_DR4));
		}
		else
		{
			//判断是否需要擦除扇区和清零RTC数据备份寄存器
			uint32_t len = (uint16_t)RTC_ReadBackupRegister(RTC_BKP_DR4);
			if(len != 0)
				flash_sector6_init();
			study_offset = 0;
			record_cnt = 0;
		}
	}
}

/*
事件标志位event_3执行的任务：复刻轨迹--按键4按下
*/
void task_event_3(void *arg)
{
	while(1)
	{
		//等待事件标志位event_3为1，任务执行完毕后自动将事件标志组对应bit位清零
		xEventGroupWaitBits(xEventGroup,EVENT_3, pdTRUE, pdTRUE, portMAX_DELAY);
		if(key4_flag == 1)//复刻轨迹函数
		{			
			total_len = (uint16_t)RTC_ReadBackupRegister(RTC_BKP_DR4);
			printf("play start, total_len=%u, total_point=%u\r\n", total_len, total_point);
			if(total_len == 0)
			{
				printf("no record data\r\n");
				key4_flag = 0;
				state = STATE_IDLE;
				continue;
			}
			memset((void *)servo, 0, sizeof(servo));
			total_point = total_len / 6;
			auto_offset = 0;
			printf("total_len = %d\r\n", total_len);
		}		
	}
}

/*
事件标志位event_4执行的任务：复位
*/
void task_event_4(void *arg)
{
	while(1)
	{
		//等待事件标志位event_4为1，任务执行完毕后自动将事件标志组对应bit位清零
		xEventGroupWaitBits(xEventGroup, EVENT_4, pdTRUE, pdTRUE, portMAX_DELAY);
		state = STATE_IDLE;	
		//判断是否需要擦除扇区和清零RTC数据备份寄存器
		uint32_t len = (uint16_t)RTC_ReadBackupRegister(RTC_BKP_DR4);
		if(len != 0)
			sector_flag = 1;
		//复位函数
		Sector_Ctl();
	}
}


void usart_test_m40_data(void)
{
//    usart1_init(9600);
//    hu_m40_init();
    //uint8_t hu_m40_rx_data[16] = {0};局部会覆盖全局
    while(1)
    {
        //delay_ms(1);
        if(hu_m40_read() == 1)
        {
            uint8_t rx  = hu_m40_analog(HU_RX);
            uint8_t ry  = hu_m40_analog(HU_RY);
            uint8_t lx  = hu_m40_analog(HU_LX);
            uint8_t ly  = hu_m40_analog(HU_LY);
            uint8_t key_byte = g_hu_m40_rx_data[8];

            //打印原始16字节数据包
            printf("RAW:");
            for(uint8_t i=0;i<16;i++)
            {
                printf("%02X ",g_hu_m40_rx_data[i]);
            }
            printf("\r\n");
            
            //新增：把每个索引和数值全部打印出来
            printf("RAW的对应数值:\r\n");
            for(uint8_t i=0;i<16;i++)
            {
                printf("RAW[%d]=%02X \r\n",i,g_hu_m40_rx_data[i]);
            }
            //printf("RAW的数值\r\n");
            
            //摇杆文字描述
            printf("【摇杆】\r\n");
            printf("右摇杆X:%d(%s) \r\n", rx, rx<128?"左推":(rx>128?"右推":"中位"));
            printf("右摇杆Y:%d(%s) \r\n", ry, ry<128?"前推":(ry>128?"后推":"中位"));
            printf("左摇杆X:%d(%s) \r\n", lx, lx<128?"左推":(lx>128?"右推":"中位"));
            printf("左摇杆Y:%d(%s) \r\n", ly, ly<128?"前推":(ly>128?"后推":"中位"));
            /*
            printf("右摇杆X:%d(%s) ", rx, (rx<128)?"左推":"右推");
            printf("右摇杆Y:%d(%s) ", ry, (ry<128)?"前推":"后推");
            printf("左摇杆X:%d(%s) ", lx, (lx<128)?"左推":"右推");
            printf("左摇杆Y:%d(%s)\r\n", ly, (ly<128)?"前推":"后推");
            */

            //按键文字描述
            printf("【按键】");
            if(key_byte & HU_K1) printf("K1 ");
            if(key_byte & HU_K2) printf("K2 ");
            if(key_byte & HU_K3) printf("K3 ");
            if(key_byte & HU_K4) printf("K4 ");
            if(key_byte & HU_K5) printf("LB ");
            if(key_byte & HU_K6) printf("RB ");
            if(key_byte == 0) printf("无按键");
            printf("\r\n------------------------------------\r\n");
        }
    }
}
