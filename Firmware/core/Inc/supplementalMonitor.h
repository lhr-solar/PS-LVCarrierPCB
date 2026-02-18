#pragma once

#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "statusLeds.h"
#include "pinDefs.h"
#include "commandLine.h"
#include "lvEnable.h"
#include "ltc4421.h"
#include "tasksConfig.h"

StackType_t Task_SuppMon_Stack_Array[ TASK_SUPP_MON_STACK_SIZE ];
StaticTask_t Task_SuppMon_Buffer;