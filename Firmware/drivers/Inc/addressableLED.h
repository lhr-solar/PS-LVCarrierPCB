#pragma once
#include "WS2812B.h"
#include "stm32xx_hal.h"

static ws2812b_handle_t wsHandle;

#define MAX_RGB_LEDS 8 // number of addressable leds on the LSOM


void addressableLED_init(void);

