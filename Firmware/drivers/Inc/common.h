#pragma once

#include "stm32xx_hal.h"
#include "stdint.h"

void gpio_clock_init(uint32_t port);

void gpio_pin_init(uint32_t port, uint32_t pin);