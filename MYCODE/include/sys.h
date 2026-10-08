#ifndef	__SYS_H__
#define	__SYS_H__


//求出“位带区”地址对应的“别名区地址”
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
//将地址转换成指针并解引用
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
//把上面两个宏定义结合，求出地址理解转换成指针并解引用
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 


//GPIO的ODR寄存器地址
#define GPIOA_ODR_Addr    (GPIOA_BASE+0x14) 
#define GPIOB_ODR_Addr    (GPIOB_BASE+0x14) 
#define GPIOC_ODR_Addr    (GPIOC_BASE+0x14) 
#define GPIOD_ODR_Addr    (GPIOD_BASE+0x14) 
#define GPIOE_ODR_Addr    (GPIOE_BASE+0x14) 
#define GPIOF_ODR_Addr    (GPIOF_BASE+0x14)    
#define GPIOG_ODR_Addr    (GPIOG_BASE+0x14)  
//GPIO的IDR寄存器地址
#define GPIOA_IDR_Addr    (GPIOA_BASE+0x10) 
#define GPIOB_IDR_Addr    (GPIOB_BASE+0x10) 
#define GPIOC_IDR_Addr    (GPIOC_BASE+0x10) 
#define GPIOD_IDR_Addr    (GPIOD_BASE+0x10)
#define GPIOE_IDR_Addr    (GPIOE_BASE+0x10) 
#define GPIOF_IDR_Addr    (GPIOF_BASE+0x10)  
#define GPIOG_IDR_Addr    (GPIOG_BASE+0x10) 
 
//n：引脚号，0-15
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入


#endif



