#ifndef _TASKMANAGER_H_
#define _TASKMANAGER_H_
#include "stm32f303xe.h"

void TaskManagerInit(void);
int32_t period_width_to_speed(int8_t direction, int32_t current_period_ticks);


#endif
