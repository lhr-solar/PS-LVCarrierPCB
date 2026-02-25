#pragma once

#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "faultBits.h"
#include "tasksConfig.h"
#include "task.h"
#include "statusLeds.h"
#include "commandLine.h"


/**
 * @brief Initialize needed variables for fault state
 * 
 * @param  none
 * @return none
 */
void faultState_init(void);