#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "ADC.h"
#include "stm32xx_hal.h"
#include "tmcs1126_lut.h"
#include "adc_lut.h"

#define ADC_QUEUE_LENGTH                   10

typedef enum{
    SUPPLEMENTAL_BATTERY_VOLTAGE,
    SUPPLEMENTAL_BATTERY_CURRENT,
    REGULATED_BATTERY_VOLTAGE,
    REGULATED_BATTERY_CURRENT,
    NUM_ADC_SENSE_CHANNELS,
}adc_sense_channel_t;


adc_status_t adc_sense_init(void);

adc_status_t adc_start_read(adc_sense_channel_t channel);

BaseType_t adc_read_value(adc_sense_channel_t channel, uint32_t *result, TickType_t delay_ticks);

QueueSetMemberHandle_t adc_wait(adc_sense_channel_t channels[], uint8_t numChannels, adc_status_t *status);
