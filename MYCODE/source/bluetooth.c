#include "main.h"

/*
环形缓冲区：存入蓝牙发过来的指令
每一个指令拆分为字节存入
指令会以\r或者\n格式结尾，这也会被存入环形缓冲区中
*/
static volatile uint8_t  bt_buf[BT_BUF_SIZE];
static volatile uint16_t bt_head = 0;   // 写指针（头）
static volatile uint16_t bt_tail = 0;   // 读指针（尾）

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        uint8_t ch = USART_ReceiveData(USART2);
        uint16_t next = (bt_head + 1) % BT_BUF_SIZE;
        if(next != bt_tail)   		// 缓冲区未满则写入，如果缓冲区满，丢弃新数据
        {
            bt_buf[bt_head] = ch;	//存入串口刚获取的字节
            bt_head = next;			//写指针向下偏移
        }
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

/*
函数：获取环形缓冲区里的可读字节数
以arr[10]为例，这里的下标范围是0~9
情况1：bt_head下标 > bt_tail下标
例如bt_head下标=9， bt_tail下标=0，这时环形缓冲区是满的
那么(bt_head - bt_tail + BT_BUF_SIZE) % BT_BUF_SIZE = (9-0+10)%10 = 9
情况2：bt_head下标 < bt_tail下标
例如bt_head下标=3， bt_tail下标=4，这时环形缓冲区是满的
那么(bt_head - bt_tail + BT_BUF_SIZE) % BT_BUF_SIZE = (3-4+10)%10 = 9
情况3：bt_head下标 = bt_tail下标
例如bt_head下标=3， bt_tail下标=3，这时环形缓冲区是空的
那么(bt_head - bt_tail + BT_BUF_SIZE) % BT_BUF_SIZE = (3-3+10)%10 = 0

由此可以得出
当你环形缓冲区是满的，bluetooth_available返回值 = 环形缓冲区最大下标值
当你环形缓冲区是空的，bluetooth_available返回值 = 0

明明数组有10个成员，为什么环形缓冲区里的可读字节数最大为9？
这是因为当bt_head下标 != bt_tail下标代表环形缓冲区有数据
bt_head指向的数组成员是有数据的，而bt_tail指向的数组成员是永远没数据的
就是说哪怕你环形缓冲区“存满”了，但是永远会有一个成员是没有数据的，它就是bt_tail指向的数组成员
*/
uint16_t bluetooth_available(void)
{
    return (bt_head - bt_tail + BT_BUF_SIZE) % BT_BUF_SIZE;
}

/*
函数：读取环形缓冲区一字节数据
有读到数据，返回值=1
没读到数据，返回值=0
*/
uint8_t bluetooth_read_byte(uint8_t *ch)
{
    if(bt_head == bt_tail)//环形缓冲区是空的
		return 0;   
    *ch = bt_buf[bt_tail];	
//	printf("bt = %c\r\n", *((char *)ch));
    bt_tail = (bt_tail + 1) % BT_BUF_SIZE;//读指针向下偏移
    return 1;
}

// 蓝牙任务（在 FreeRTOS 里创建）


// 发送字符串到蓝牙（向USART2发送数据，阻塞式，仅用于调试或状态回传）
void bluetooth_send_string(const char *str)
{
	while(*str)
    {
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        USART_SendData(USART2, (uint8_t)*str++);
    }
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}

/*
函数：蓝牙协议解析
手机端指令格式（每条以 \n 或 \r 结尾）：
"S1:90"    -> 舵机1转到90度
"S2:45"    -> 舵机2转到45度
"S6:120"   -> 舵机6转到120度
......
"RESET"    -> 复位
"CLAW"     -> 抓夹切换（抓/松）
"REC"      -> 开始录制
"STOP_REC" -> 停止录制
"PLAY"     -> 开始复刻
"STOP"     -> 停止复刻
"STATUS"   -> 请求回传状态
 */
void ParseBluetoothCommand(char *cmd)
{
    if(cmd[0] == 'S' && cmd[2] == ':')// 格式 "Sx:角度"，x取值范围：1~6
    {
        uint8_t servo_id = cmd[1] - '0'; 
        uint8_t angle = (uint8_t)atoi(cmd + 3);
        if(angle > 180) 
			angle = 180;			

        switch(servo_id)
        {
            case 1: servo_curr_rad[0] = angle; SG90_SetAngle(0x01, angle); break;
            case 2: servo_curr_rad[1] = angle; SG90_SetAngle(0x02, angle); break;
            case 3: servo_curr_rad[2] = angle; SG90_SetAngle(0x04, angle); break;
            case 4: servo_curr_rad[3] = angle; SG90_SetAngle(0x08, angle); break;
            case 5: servo_curr_rad[4] = angle; SG90_SetAngle(0x10, angle); break;
            case 6: servo_curr_rad[5] = angle; SG90_SetAngle(0x20, angle); break;
            default: break;
        }
    }
    else if(strcmp(cmd, "RESET") == 0)//等效按下复位按键
    {        
        hu_m40_data[1] |= 0x01;
    }
    else if(strcmp(cmd, "CLAW") == 0)//等效按下抓夹按键
    {        
        hu_m40_data[1] |= 0x02;
    }
    else if (strcmp(cmd, "REC") == 0)//等效按下录制按键（第一次按下）
    {        
        hu_m40_data[1] |= 0x04;
    }
    else if (strcmp(cmd, "STOP_REC") == 0)//等效再次按下录制按键（停止录制）
    {        
        hu_m40_data[1] |= 0x04;
    }
    else if (strcmp(cmd, "PLAY") == 0)// 等效按下复刻按键（第一次按下）
    {        
        hu_m40_data[1] |= 0x08;
    }
    else if (strcmp(cmd, "STOP") == 0)// 等效再次按下复刻按键（停止复刻）
    {        
        hu_m40_data[1] |= 0x08;
    }
    else if (strcmp(cmd, "STATUS") == 0)// 回传当前状态
    {        
        char buf[64];
        sprintf(buf, "S1:%d,S2:%d,S3:%d,S4:%d,S5:%d,S6:%d\r\n",
                servo_curr_rad[0], servo_curr_rad[1], servo_curr_rad[2],
                servo_curr_rad[3], servo_curr_rad[4], servo_curr_rad[5]);
        bluetooth_send_string(buf);
    }
}

