#include "main.h"

volatile uint16_t record_cnt = 0;	//保存当前记录的次数
volatile uint8_t key3_flag = 0;	   	//记录按键三按下的次数，初始为0，按下奇数次就执行学习模式，按下偶数次就退出学习模式
volatile uint8_t servo[6] = {0};	//保存当前时刻舵机角度
volatile uint32_t study_offset = 0;	//学习模式的地址偏移量
void Study_Mode_While(void)
{
	for(uint8_t i = 0;i < 6;i++)
	{
		servo[i] = servo_curr_rad[i];
	}
	Study_Mode((uint8_t *)servo);
	record_cnt++;
}

/*
自动模式：
读取扇区，重复执行单动作序列
*/
void Auto_Act(uint8_t *servo)
{
	SG90_SetAngle(0x01,servo[0]);  //舵机1
	SG90_SetAngle(0x02,servo[1]);  //舵机2
	SG90_SetAngle(0x04,servo[2]);  //舵机3
	SG90_SetAngle(0x08,servo[3]);  //舵机4
	SG90_SetAngle(0x10,servo[4]);  //舵机5
	SG90_SetAngle(0x20,servo[5]);  //舵机6
}

volatile uint8_t key4_flag = 0;		//记录按键4按下的次数，按下奇数次就是开启自动模式，按下偶数次就是关闭自动模式
volatile uint32_t total_len;		//RTC数据备份寄存器存储的学习到的字节数
volatile uint8_t total_point = 0;	//学习记录的动作点
volatile uint32_t auto_offset = 0;	//自动模式的地址偏移量
void Auto_Mode(void)  //重复调用，再按下按键4（标志位判断）才会回到空闲，且要保证一个动作序列执行完
{	   
	Read_Study_Mode(ADDR_FLASH_SECTOR_6 + auto_offset,(uint8_t *)servo);
	Auto_Act((uint8_t *)servo);
	auto_offset += 6;
}

uint8_t claw_flag = 0;//记录按键2（抓夹）按下的次数，按下奇数次为抓紧，按下偶数次为松开
void claw(void)
{
	claw_flag = (claw_flag + 1) % 2;
	if(claw_flag == 1)
	{
		servo_curr_rad[5] = CLAW_GRAB_ANG;
		SG90_SetAngle(0x20, servo_curr_rad[5]);	
	}
	else
	{
		servo_curr_rad[5] = CLAW_RELEASE_ANG ;
		SG90_SetAngle(0x20, servo_curr_rad[5]);
	}
}

volatile uint8_t sector_flag = 0;
//机械臂复位至初始位置
void Arm_ResetPos(void)
{
    SG90_SetAngle(0x01,90);
    SG90_SetAngle(0x02,90);
    SG90_SetAngle(0x04,90);
    SG90_SetAngle(0x08,90);
    SG90_SetAngle(0x10,90);
    SG90_SetAngle(0x20,90);
    memset((void *)servo_curr_rad, 90, sizeof(servo_curr_rad));
	
	//标志位清除
	record_cnt = 0;	
	key3_flag = 0;	 
	memset((void *)servo, 0, sizeof(servo));
	study_offset = 0;
	
	key4_flag = 0;
	total_len;
	total_point = 0;
	auto_offset = 0;
	
	claw_flag = 0;
}
/*
函数：复位，并判断是否要擦除扇区
1、sector_flag：全局变量
判断扇区是否存有动作序列，初始没有uint8_t  sector_flag = 0;
2、选择扇区四的原因：64KB足够存动作序列，擦除耗时短
3、解除写保护-->清除所有标志位-->擦除扇区-->添加写保护

*/
void Sector_Ctl(void)
{
    Arm_ResetPos();
	printf("reset start\r\n");
    if(sector_flag == 1)
    {
        flash_sector6_init();																																																																														
        sector_flag = 0;
    }
    else
    {
        PGout(11) = 1;
    }
}
