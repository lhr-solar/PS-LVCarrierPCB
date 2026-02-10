#pragma once

#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"

typedef enum{
    LSOM_HEARTBEAT_LED,
    HEARTBEAT_LED,
    SUPPBATT_FAULT_LED,
    NUM_STATUS_LEDs
}status_leds_t;

void statusLeds_init(void);

void statusLeds_toggle(status_leds_t led);

void statusLeds_write(status_leds_t led, pin_state_t newState);