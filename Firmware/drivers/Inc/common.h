#pragma once

#include "stm32xx_hal.h"
#include "stdint.h"

typedef enum pinState {OFF = GPIO_PIN_RESET, ON = GPIO_PIN_SET} pin_state_t;

void gpio_clock_init(GPIO_TypeDef *port);

void gpio_pin_init(GPIO_TypeDef *port, uint32_t pin);