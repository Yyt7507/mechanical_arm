#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f4xx.h"
#include "usart.h"

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* 扇区0,  16 字节 */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* 扇区1,  16 字节 */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* 扇区2,  16 字节 */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* 扇区3,  16 字节 */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* 扇区4,  64 字节 */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* 扇区5, 128 字节 */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* 扇区6, 128 字节 */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* 扇区7, 128 字节 */


void rtc_domain_access(void);
FLASH_Status flash_sector6_init(void);
FLASH_Status Study_Mode_Write(uint32_t sector_addr,uint8_t *data);
void Read_Study_Mode(uint32_t addr,uint8_t *data);
void Study_Show(void);
void Study_Mode(uint8_t *data);

#endif
