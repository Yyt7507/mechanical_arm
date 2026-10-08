#include "main.h"

uint8_t RX_BUF[NRF24L01_PLOAD_WIDTH];
uint8_t TX_BUF[NRF24L01_PLOAD_WIDTH];

static void NRF24L01_SPI_Init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOE, ENABLE);
  
  NRF24L01_GPIO_WRITE(NRF24L01_GPIO_CE,0);
  NRF24L01_GPIO_WRITE(NRF24L01_GPIO_CSN,1);
  
  NRF24L01_GPIO_WRITE( NRF24L01_GPIO_SCLK, 0 );
  NRF24L01_GPIO_WRITE( NRF24L01_GPIO_MOSI, 1 );
  NRF24L01_GPIO_WRITE( NRF24L01_GPIO_MISO, 1 );
  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed	= GPIO_High_Speed;
  GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed	= GPIO_High_Speed;
  GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOE,&GPIO_InitStructure);
  
}

void NRF24L01_Init(void)
{
//  uint8_t ret;
  NRF24L01_SPI_Init();
  NRF24L01_Check();
//  if(ret == 0)
//  {
//    printf("NRF24L01模块检测成功!\r\n");
//  }
//  else
//  {
//    printf("NRF24L01模块检测失败！\r\n");
//  }
  
}

/**
* Basic SPI operation: Write to SPIx and read
*/
static uint8_t SPI_Write_Then_Read(uint8_t data)
{
  uint8_t i;
	uint8_t res;
	
	res = 0;
	
	for( i=0; i<8; i++ )
	{
		res <<= 1;
		
		if( data&0x80 )
		{
			NRF24L01_GPIO_WRITE(NRF24L01_GPIO_MOSI,1);
		}
		else
		{
			NRF24L01_GPIO_WRITE(NRF24L01_GPIO_MOSI,0);
		}
		
		NRF24L01_GPIO_WRITE(NRF24L01_GPIO_SCLK,1);
		
		if( GPIO_ReadInputDataBit(NRF24L01_GPIO_MISO) != 0 )
		{
			res |= 0x01;
		}
		
		NRF24L01_GPIO_WRITE(NRF24L01_GPIO_SCLK,0);
    
		data <<= 1;
	}
	
	return res;
}

/**
* Read a 1-bit register
*/
uint8_t NRF24L01_Read_Reg(uint8_t reg)
{
  uint8_t value;
  CSN(0);
  SPI_Write_Then_Read(reg);
  value = SPI_Write_Then_Read(NRF24L01_CMD_NOP);
  CSN(1);
  return value;
}

/**
* Write a 1-byte register
*/
uint8_t NRF24L01_Write_Reg(uint8_t reg, uint8_t value)
{
  uint8_t status;
  CSN(0);
  if (reg < NRF24L01_CMD_REGISTER_W) {
    // This is a register access
    status = SPI_Write_Then_Read(NRF24L01_CMD_REGISTER_W | (reg & NRF24L01_MASK_REG_MAP));
    SPI_Write_Then_Read(value);

  } else {
    // This is a single byte command or future command/register
    status = SPI_Write_Then_Read(reg);
    if ((reg != NRF24L01_CMD_FLUSH_TX) 
        && (reg != NRF24L01_CMD_FLUSH_RX) 
        && (reg != NRF24L01_CMD_REUSE_TX_PL) 
        && (reg != NRF24L01_CMD_NOP)) {
      // Send register value
      SPI_Write_Then_Read(value);
    }
  }
  CSN(1);
  return status; 
}

/**
* Read a multi-byte register
*  reg  - register to read
*  buf  - pointer to the buffer to write
*  len  - number of bytes to read
*/
uint8_t NRF24L01_Read_To_Buf(uint8_t reg, uint8_t *buf, uint8_t len)
{
  CSN(0);
  uint8_t status = SPI_Write_Then_Read(reg);
  while (len--) {
    *buf++ = SPI_Write_Then_Read(NRF24L01_CMD_NOP);
  }
  CSN(1);
  return status;
}

/**
* Write a multi-byte register
*  reg - register to write
*  buf - pointer to the buffer with data
*  len - number of bytes to write
*/
uint8_t NRF24L01_Write_From_Buf(uint8_t reg, uint8_t *buf, uint8_t len)
{
  CSN(0);
  uint8_t status = SPI_Write_Then_Read(reg);
  while (len--) {
    SPI_Write_Then_Read(*buf++);
  }
  CSN(1);
  return status;
}


uint8_t NRF24L01_Check(void)
{
  uint8_t rxbuf[5];
  uint8_t i;
  uint8_t *ptr = (uint8_t *)NRF24L01_TEST_ADDR;

  // Write test TX address and read TX_ADDR register
  NRF24L01_Write_From_Buf(NRF24L01_CMD_REGISTER_W | NRF24L01_REG_TX_ADDR, ptr, 5);
  NRF24L01_Read_To_Buf(NRF24L01_CMD_REGISTER_R | NRF24L01_REG_TX_ADDR, rxbuf, 5);

  // Compare buffers, return error on first mismatch
  for (i = 0; i < 5; i++) {
    if (rxbuf[i] != *ptr++) return 1;
  }
  return 0;
}

/**
* Flush the RX FIFO
*/
void NRF24L01_FlushRX(void)
{
  NRF24L01_Write_Reg(NRF24L01_CMD_FLUSH_RX, NRF24L01_CMD_NOP);
}

/**
* Flush the TX FIFO
*/
void NRF24L01_FlushTX(void)
{
  NRF24L01_Write_Reg(NRF24L01_CMD_FLUSH_TX, NRF24L01_CMD_NOP);
}

/**
* Clear IRQ bit of the STATUS register
*   reg - NRF24L01_FLAG_RX_DREADY
*         NRF24L01_FLAG_TX_DSENT
*         NRF24L01_FLAG_MAX_RT
*/
void NRF24L01_ClearIRQFlag(uint8_t reg) {
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_STATUS, reg);
}

/**
* Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
*/
void NRF24L01_ClearIRQFlags(void) {
  uint8_t reg;
  reg  = NRF24L01_Read_Reg(NRF24L01_REG_STATUS);
  reg |= NRF24L01_MASK_STATUS_IRQ;
  NRF24L01_Write_Reg(NRF24L01_REG_STATUS, reg);
}

/**
* Common configurations of RX and TX, internal function
*/
void _NRF24L01_Config(uint8_t *tx_addr)
{
  // TX Address
  NRF24L01_Write_From_Buf(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_TX_ADDR, tx_addr, NRF24L01_ADDR_WIDTH);
  // RX P0 Payload Width
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RX_PW_P0, NRF24L01_PLOAD_WIDTH);
  // Enable Auto ACK
//  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_EN_AA, 0x3f);
  // Enable RX channels
//  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_EN_RXADDR, 0x3f);
  
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_EN_AA, 0x01);//P0自动应答开启 
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_EN_RXADDR, 0x01);//P0管道开启 
  
  
  // RF channel: 2.400G  + 0.001 * x
//  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RF_CH, 40);
  // 000+0+[0:1Mbps,1:2Mbps]+[00:-18dbm,01:-12dbm,10:-6dbm,11:0dbm]+[0:LNA_OFF,1:LNA_ON]
  // 01:1Mbps,-18dbm; 03:1Mbps,-12dbm; 05:1Mbps,-6dbm; 07:1Mbps,0dBm
  // 09:2Mbps,-18dbm; 0b:2Mbps,-12dbm; 0d:2Mbps,-6dbm; 0f:2Mbps,0dBm, 
//  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RF_SETUP, 0x03);
  
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RF_CH, 25);//通道25，和遥控器匹配 ?
//  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RF_SETUP, 0x0d);
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RF_SETUP, 0x25); // 250kbps 0db//250kbps，0dBm ?
  
  // 0A:delay=250us,count=10, 1A:delay=500us,count=10
//  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_SETUP_RETR, 0x0a);
// NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_SETUP_RETR, 0x62); // 
//NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_SETUP_RETR, 0x88); // 修改点4：重发间隔:2000us,8次！！！！！！
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_SETUP_RETR, 0x4F);//500us,15次
}

/**
* Switch NRF24L01 to RX mode
*/
void NRF24L01_RX_Mode(uint8_t *rx_addr, uint8_t *tx_addr)
{
  CE(0);
  _NRF24L01_Config(tx_addr);
  // RX Address of P0
  NRF24L01_Write_From_Buf(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RX_ADDR_P0, rx_addr, NRF24L01_ADDR_WIDTH);
  /**
  REG 0x00: 
  0)PRIM_RX     0:TX             1:RX
  1)PWR_UP      0:OFF            1:ON
  2)CRCO        0:8bit CRC       1:16bit CRC
  3)EN_CRC      Enabled if any of EN_AA is high
  4)MASK_MAX_RT 0:IRQ low        1:NO IRQ
  5)MASK_TX_DS  0:IRQ low        1:NO IRQ
  6)MASK_RX_DR  0:IRQ low        1:NO IRQ
  7)Reserved    0
  */
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_CONFIG, 0x0f); //RX,PWR_UP,CRC16,EN_CRC
  CE(1);
}

/**
* Switch NRF24L01 to TX mode
*/
void NRF24L01_TX_Mode(uint8_t *rx_addr, uint8_t *tx_addr)
{
  CE(0);
  _NRF24L01_Config(tx_addr);
  // On the PTX the **TX_ADDR** must be the same as the **RX_ADDR_P0** and as the pipe address for the designated pipe
  // RX_ADDR_P0 will be used for receiving ACK
  NRF24L01_Write_From_Buf(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_RX_ADDR_P0, tx_addr, NRF24L01_ADDR_WIDTH);
  NRF24L01_Write_Reg(NRF24L01_CMD_REGISTER_W + NRF24L01_REG_CONFIG, 0x0e); //TX,PWR_UP,CRC16,EN_CRC
  CE(1);
}

/**
* Hold till data received and written to rx_buf
*/
uint8_t NRF24L01_RxPacket(uint8_t *rx_buf)
{
  uint8_t status, result = 0;
//  while(IRQ);
  
  
  //CE(0);  修改点7！！！！
  status = NRF24L01_Read_Reg(NRF24L01_REG_STATUS);
  //printf("Interrupted, status: %02X\r\n", status);

  if(status & NRF24L01_FLAG_RX_DREADY) {
    NRF24L01_Read_To_Buf(NRF24L01_CMD_RX_PLOAD_R, rx_buf, NRF24L01_PLOAD_WIDTH);
//    for (int i = 0; i < 32; i++) {
//      printf("%02X ", RX_BUF[i]);
//    }
    result = 1;
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_RX_DREADY);
  }
  //CE(1);
  return result;
}

/**
* Send data in tx_buf and wait till data is sent or max re-tr reached
*/
uint8_t NRF24L01_TxPacket(uint8_t *tx_buf, uint8_t len)
{
  uint8_t status = 0x00;
  uint32_t timeout;
  CE(0);
  NRF24L01_FlushTX();//修改点6！！！！新增
  len = len > NRF24L01_PLOAD_WIDTH? NRF24L01_PLOAD_WIDTH : len;
  NRF24L01_Write_From_Buf(NRF24L01_CMD_TX_PLOAD_W, tx_buf, len);
  CE(1);
  
#if defined( NRF24L01_GPIO_IRQ )
  while(IRQ != 0); // Waiting send finish
  
#else
  //for(timeout=0;timeout<10000;timeout++); // ??????130uS
  for(timeout=0;timeout<20000;timeout++); // 修改点5！！！！！
#endif
  
  
  CE(0);
  status = NRF24L01_Read_Reg(NRF24L01_REG_STATUS);
  //printf("Interrupted, status: %02X\r\n", status);
  if(status & NRF24L01_FLAG_TX_DSENT) {
//    printf("Data sent: ");
//    for (uint8_t i = 0; i < len; i++) {
//      printf("%02X ", tx_buf[i]);
//    }
//    printf("\r\n");
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_TX_DSENT);

  } else if(status & NRF24L01_FLAG_MAX_RT) {
//    printf("Sending exceeds max retries\r\n");
    NRF24L01_FlushTX();
    NRF24L01_ClearIRQFlag(NRF24L01_FLAG_MAX_RT);
  }
  CE(1);
  return status;
}



