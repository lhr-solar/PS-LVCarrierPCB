#pragma once

#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "faultBits.h"
#include "tasksConfig.h"
#include "task.h"
#include "statusLeds.h"

/**
 * @brief Weakly defined hook that's called when a fault state is set
 * 
 * @param pending    Which event bit is set
 * @return none
 */
__weak void faultState_hook(EventBits_t pending);