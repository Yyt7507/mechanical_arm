#include "main.h"
/*
hu_m40_data[0]表示遥感数据，hu_m40_data[1]表示按键数据

hu_m40_data[0]第0bit表示左摇杆往左推
hu_m40_data[0]第1bit表示左摇杆往右推
hu_m40_data[0]第2bit表示左摇杆往前推
hu_m40_data[0]第3bit表示左摇杆往后推
hu_m40_data[0]第4bit表示右摇杆往左推
hu_m40_data[0]第5bit表示右摇杆往右推
hu_m40_data[0]第6bit表示右摇杆往前推
hu_m40_data[0]第7bit表示右摇杆往后推

hu_m40_data[1]第0bit表示KEY1--复位
hu_m40_data[1]第1bit表示KEY2--抓夹
hu_m40_data[1]第2bit表示KEY3--录制轨迹
hu_m40_data[1]第3bit表示KEY4--复刻轨迹
*/
volatile uint8_t hu_m40_data[2] = {0};	//舵机转动方向的数据
volatile uint8_t record_time[4] = {0};	//记录五个按键持续按下时间
volatile uint8_t hu_m40_key = 0;	  	//按键触发标志位（触发了不代表按键按下了，需要经过防抖），1代表出发了
volatile uint8_t stop_request = 0;		//请求停止自动模式标志位
volatile uint8_t state = STATE_IDLE;	//状态机初始状态为空闲模式

#define			STACK_SIZE				128 // 堆栈大小!=实际字节数
StackType_t 	xStack[ STACK_SIZE ];		// 用做任务堆栈的数组
StaticTask_t 	xTaskBuffer;				// 保存任务数据的TCB

//事件标志组句柄
EventGroupHandle_t xEventGroup;

// 创建初始任务例程
void routinue(void *arg)
{
    TaskHandle_t xHandle1 = NULL;
    TaskHandle_t xHandle2 = NULL;
    TaskHandle_t xHandle_event1 = NULL;
    TaskHandle_t xHandle_event2 = NULL;
    TaskHandle_t xHandle_event3 = NULL;
    TaskHandle_t xHandle_event4 = NULL;
	xTaskCreate(task1_recv_hu_m40_data,    "task1_recv_hu_m40_data",    STACK_SIZE,NULL, tskIDLE_PRIORITY+4, &xHandle1);
	xTaskCreate(task2_analyse_hu_m40_data, "task2_analyse_hu_m40_data", STACK_SIZE,NULL, tskIDLE_PRIORITY+1, &xHandle2);
	xTaskCreate(task_event_1,    		   "task_event_1",    			STACK_SIZE,NULL, tskIDLE_PRIORITY+2, &xHandle_event1);
	xTaskCreate(task_event_2,  			   "task_event_2",    			STACK_SIZE,NULL, tskIDLE_PRIORITY+2, &xHandle_event2);
	xTaskCreate(task_event_3,    		   "task_event_3",    			STACK_SIZE,NULL, tskIDLE_PRIORITY+2, &xHandle_event3);
	xTaskCreate(task_event_4,  			   "task_event_4",    			STACK_SIZE,NULL, tskIDLE_PRIORITY+2, &xHandle_event4);
	while(1);
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	usart1_init(115200);
	delay_init();
	hu_m40_init();
	steering_engine_init();
	tim2_init();
	tim5_init();
	rtc_domain_access();
	
	Arm_ResetPos();	
	
	xEventGroup = xEventGroupCreate();
	if(xEventGroup == NULL)
		printf("create event group error\r\n");	
	
	TaskHandle_t xHandle = NULL;
	// 创建初始任务
	xTaskCreate(	routinue,         	/* 任务函数名/地址 */
                    "routinue",       	/* 描述任务用的名字 */
                    STACK_SIZE,      	/* 栈的大小 */
                    NULL,    			/* 传递给任务函数的参数 */
                    tskIDLE_PRIORITY,	/* 优先级 */
                    &xHandle);			/* 任务句柄 */  	

	vTaskStartScheduler();
}

#if 1
/* 静态内存：空闲任务 TCB 和栈 */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    /* 将静态TCB地址传给内核 */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    /* 将静态栈数组首地址传给内核 */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    /* 栈深度，数组元素个数，不是字节！ */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* 静态内存：软件定时器任务 TCB 和栈 */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];  
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif
