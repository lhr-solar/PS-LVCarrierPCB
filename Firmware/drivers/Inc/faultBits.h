#pragma once

#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "event_groups.h"

#define ALL_FAULT_BITS    ((1UL << NUM_FAULTS) - 1UL)

// The max number of fault bits is dependent on the configUSE_16_BIT_TICKS defined in FreeRTOS.h
#if ( configUSE_16_BIT_TICKS == 0 )
    #define MAX_FAULT_BITS   24U
#else
    #define MAX_FAULT_BITS   8U
#endif

typedef enum
{
    FAULT_SUPPBATT_OVERVOLTAGE = 0,
    FAULT_SUPPBATT_UNDERVOLTAGE,
    FAULT_ADC_TIMEOUT,
    FAULT_LTC4421_FAULT,
    NUM_FAULTS
} fault_bit_t;

/* Convert enum to bitmask */
#define FAULT_BIT(fault)   (1UL << (fault))

_Static_assert(NUM_FAULTS <= MAX_FAULT_BITS, "Too many fault bits for EventGroup");

uint8_t faultBits_init(void);

void set_faultBit(fault_bit_t bit);

EventBits_t faultBit_wait(fault_bit_t bit, TickType_t xTicksToWait);