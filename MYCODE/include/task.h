#ifndef __TASK_H__
#define __TASK_H__

#include "main.h"

void detection_state(void);
void read_hu_m40_data(void);

void task1_recv_hu_m40_data(void *arg);
void task2_analyse_hu_m40_data(void *arg);
void task_bluetooth(void *arg);
void task_event_1(void *arg);
void task_event_2(void *arg);
void task_event_3(void *arg);
void task_event_4(void *arg);

void usart_test_m40_data(void);

#endif
