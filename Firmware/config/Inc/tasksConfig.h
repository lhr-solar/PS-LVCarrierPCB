#pragma once

#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"

// Task Priority 
#define TASK_INIT_PRIO                  tskIDLE_PRIORITY + 1
#define TASK_SUPP_MON_PRIO              tskIDLE_PRIORITY + 2
#define TASK_POWER_MUX_MON_PRIO         tskIDLE_PRIORITY + 4

// Task Stack Size 
#define TASK_INIT_STACK_SIZE                    configMINIMAL_STACK_SIZE
#define TASK_SUPP_MON_STACK_SIZE                configMINIMAL_STACK_SIZE
#define TASK_POWER_MUX_MON_STACK_SIZE           configMINIMAL_STACK_SIZE


// (exposed so that tests can init tasks)
// Task Stack Arrays 
extern StackType_t Task_SuppMon_Stack_Array[ TASK_SUPP_MON_STACK_SIZE ];
extern StackType_t Task_PowerMux_Stack_Array[ TASK_POWER_MUX_MON_STACK_SIZE ];

// Task Buffers
extern StaticTask_t Task_SuppMon_Buffer;
extern StaticTask_t Task_PowerMux_Buffer;


// Task Functions
void initThread();
void powerMuxMonitor();
void supplementalMonitor();