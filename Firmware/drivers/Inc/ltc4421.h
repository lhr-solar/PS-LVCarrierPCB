#pragma once

#include "stm32xx_hal.h"
#include "common.h"
#include "pinDefs.h"

#define LTC4421_SHUTDOWN OFF
#define LTC4421_ENABLED ON

#define LTC4421_HV_DCDC_CHANNEL CHANNEL1
#define LTC4421_SUPP_DCDC_CHANNEL CHANNEL2

typedef enum {
    NO_CHANNEL,
    CHANNEL1,
    CHANNEL2
}ltc4421_channel_t;

void ltc4421_gpio_init(void);

void ltc4421_shdn_enable(pin_state_t newShutDownState);

pin_state_t ltc4421_read_valid(ltc4421_channel_t channel);

ltc4421_channel_t ltc4421_channel_selected(void);

pin_state_t ltc4421_read_fault(ltc4421_channel_t channel);