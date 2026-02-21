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

StaticTask_t Task_PowerMux_Buffer;
StackType_t Task_PowerMux_Stack_Array[ TASK_POWER_MUX_MON_STACK_SIZE ];