#pragma once

#include "stm32xx_hal.h"
#include "common.h"
#include "pinDefs.h"

void lvEnable_gpio_init(void);

pin_state_t get_lvEnable_supp(void);

pin_state_t get_lvEnable_powerSupply(void);
