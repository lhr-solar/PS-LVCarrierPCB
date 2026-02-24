#pragma once

#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "faultBits.h"
#include "tasksConfig.h"
#include "task.h"

__weak void faultState_hook(EventBits_t pending);