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

/**
 * @brief   Initialize the GPIO pins to read the LTC4421 status
 * @return  none
 */
void ltc4421_gpio_init(void);

/**
 * @brief   Read the valid pin of the  LTC4421 channel
 * @param   newShutDownState LTC4421_ENABLED = enable LTC4421, LTC4421_SHUTDOWN = LTC4421 not enabled
 * @return  pin_state_t ON = valid, OFF = invalid
 */
void ltc4421_shdn_enable(pin_state_t newShutDownState);

/**
 * @brief   Read the valid pin of the  LTC4421 channel
 * @param   channel which LTC4421 channel
 * @return  pin_state_t ON = valid, OFF = invalid
 */
pin_state_t ltc4421_read_valid(ltc4421_channel_t channel);

/**
 * @brief   Read which channel is selected on the LTC4421
 * @return  ltc4421_channel_t the current selected channel
 */
ltc4421_channel_t ltc4421_channel_selected(void);

/**
 * @brief   Read the fault of the LTC4421
 * @param   channel which LTC4421 channel
 * @return  pin_state_t ON = fault, OFF = no fault
 */
pin_state_t ltc4421_read_fault(ltc4421_channel_t channel);