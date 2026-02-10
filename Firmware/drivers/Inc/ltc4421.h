#pragma once

#include "stm32xx_hal.h"
#include "common.h"
#include "pinDefs.h"

#define LTC4421_SHUTDOWN OFF
#define LTC4421_ENABLED ON


void ltc4421_gpio_init(void);

void ltc4421_shdn_enable(pin_state_t newShutDownState);