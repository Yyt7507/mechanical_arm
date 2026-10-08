#include "flash.h"

//解除RTC数据备份域写保护
void rtc_domain_access(void)
{
	// 使能PWR时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	// 让PWR解除RTC备份域的写保护
	PWR_BackupAccessCmd(ENABLE);
	/*
	// 开启LSE时钟
	RCC_LSEConfig(RCC_LSE_ON);
	// 等待LSE时钟启动并稳定下来
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);*/
	RCC_LSICmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);
	// 选择LSE时钟作为RTC的输入时钟源
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	// 使能RTC时钟
	RCC_RTCCLKCmd(ENABLE);
}


// 扇区6按字节擦除
FLASH_Status flash_sector6_init(void)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
				 FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR| FLASH_FLAG_PGSERR);
	FLASH_Status ret = FLASH_EraseSector(FLASH_Sector_6,VoltageRange_1);
//	if(ret == FLASH_COMPLETE)
//		printf("flash sector 4 erase complete\r\n");
//	else
//		printf("flash sector 4 erase error\r\n");
	//把记录长度置0
	RTC_WriteBackupRegister(RTC_BKP_DR4,0);
	return ret;
}

/*
学习模式：
uint8_t servo[6];存一个动作点往flash中存一次
flash若要修改存入数据才需要擦除，可以写入后接着往后写入
SG90机械臂，20ms一个点，10s最多500个点。
500*6 = 3000字节

Study_Mode_Write():往扇区4中传入uint8_t servo[6]和数组长度，写入数组数据，写入一次为一个动作点
Read_Study_Mode():读扇区中的动作序列，用于自动模式时读取并执行
Study_Show():验证扇区中的数据（测试用）
Study_Mode():学习模式下不断写入数据
*/
//函数：单次向扇区写入数据函数
FLASH_Status Study_Mode_Write(uint32_t sector_addr,uint8_t *data)
{
	FLASH_Unlock();
	int i = 0;
	FLASH_Status ret;
	for(i = 0;i < 6;i++)
	{
		ret = FLASH_ProgramByte(sector_addr+i,data[i]);
		if(ret != FLASH_COMPLETE)
		{
//			printf("sector 6 write string error\r\n");
			FLASH_Lock();
			return ret;
		}
	}
	FLASH_Lock();
	return FLASH_COMPLETE;
}
//函数；单次读取扇区数据
void Read_Study_Mode(uint32_t addr,uint8_t *data)
{
	//FLASH_Unlock();读不需要锁
	int i = 0;
	uint8_t *buf = (uint8_t *)addr;	
	for(i = 0; i < 6; i++)
	{
		data[i] = buf[i];
	}	
}
//函数：读取扇区数据并打印（调试）
void Study_Show(void)
{
	uint32_t len = (uint16_t)RTC_ReadBackupRegister(RTC_BKP_DR4);
	uint32_t i;
	uint8_t *s = (uint8_t *)ADDR_FLASH_SECTOR_6;
	for(i = 0;i < len;i++)
	{
		printf("s[%d] = %02X\t",i,s[i]);
		if((i+1)%6 == 0) 
			printf("\r\n"); //每6字节一个动作点换行		
	}
}
/*
函数：单次向扇区写入舵机数据函数
20ms调一次Study_Mode();内部不进行清零操作
每20ms采集一次servo[6]舵机角度
*/
extern volatile uint32_t study_offset;
void Study_Mode(uint8_t *data)
{
	//printf("扇区原始数据长度为：%d\r\n",len); //测试用
	Study_Mode_Write(ADDR_FLASH_SECTOR_6 + study_offset,data);		
	study_offset += 6;
	RTC_WriteBackupRegister(RTC_BKP_DR4,study_offset);
}
