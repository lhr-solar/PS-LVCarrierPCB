#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "ADC.h"
#include "stm32xx_hal.h"
#include "tmcs1126_lut.h"
#include "adc_lut.h"
#include "statusLeds.h"

#define ADC_QUEUE_LENGTH 10

typedef enum{
    SUPPLEMENTAL_BATTERY_VOLTAGE,
    SUPPLEMENTAL_BATTERY_CURRENT,
    REGULATED_BATTERY_VOLTAGE,
    REGULATED_BATTERY_CURRENT,
    NUM_ADC_SENSE_CHANNELS,
}adc_sense_channel_t;

/**
 * @brief Converts adc counts to current measurement for the tmcs1126 Hall effect
 * 
 * @param adcCounts     ADC value between 0 to 4095
 * @return status of initialization
 */
adc_status_t adc_sense_init(void);

/**
 * @brief Converts adc counts to current measurement for the tmcs1126 Hall effect
 * 
 * @param channel           ADC channel to read from
 * @return status of the read
 */
adc_status_t adc_start_read(adc_sense_channel_t channel, TickType_t delay_ticks);


/**
 * @brief Converts adc counts 
 * 
 * @param channel           ADC channel to read from
 * @param result            pointer to the adc counts result
 * @param delay_ticks       Ticks to wait for data (0 = non-blocking, portMAX_DELAY = block until available).
 * @return 
 */
BaseType_t adc_read_value(adc_sense_channel_t channel, uint32_t *result, TickType_t delay_ticks);