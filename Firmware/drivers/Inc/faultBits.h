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
    FAULT_INIT_FAILED = 0,
    FAULT_SUPPBATT_OVERVOLTAGE,
    FAULT_SUPPBATT_UNDERVOLTAGE,
    FAULT_SUPPREG_OVERVOLTAGE,
    FAULT_SUPPREG_UNDERVOLTAGE,

    // Supp Charger Faults
    /* DC Input Undervoltage 20V */ 
    FAULT_SUPPCHARGER_IN_UNDERVOLTAGE,
    /* DC Input Overvoltages 30V */ 
    FAULT_SUPPCHARGER_IN_OVERVOLTAGE,
    /* Charge current max is 5.25A */ 
    FAULT_SUPPCHARGER_OVERCURRENT,
    /* Thrown if battery voltage exceeds 28.7V */
    FAULT_SUPPCHARGER_BATT_OVERVOLTAGE,
    /* Chip Thermal Shutdown (~150 C) */
    FAULT_SUPPCHARGER_BATT_TSHUT,
    /* If charging for more than 12 hours */
    FAULT_SUPPCHARGER_TIMER,
    /* DRV SUP pin out of the range (gate supply) */
    FAULT_SUPPCHARGER_DRV_SUP,      
    
    FAULT_ADC_TIMEOUT,
    FAULT_LTC4421_FAULT,
    NUM_FAULTS
} fault_bit_t;

/* Convert enum to bitmask */
#define FAULT_BIT(fault)   (1UL << (fault))

_Static_assert(NUM_FAULTS <= MAX_FAULT_BITS, "Too many fault bits for EventGroup");



/**
 * @brief Initializes fault bitmap
 * 
 * @param none
 * @return 0 on failure, 1 on success
 */
uint8_t faultBits_init(void);

/**
 * @brief Set a fault in the fault bitmap
 * 
 * @param bit which fault is being set
 * @return none
 */
void set_faultBit(fault_bit_t bit);

/**
 * @brief Wait for a fault to be set 
 * 
 * @param bit which fault to wait for, pass NUM_FAULTS if waiting for any fault
 * @param xTicksToWait delay when waiting
 * @return the event bit that was set
 */
EventBits_t faultBit_wait(fault_bit_t bit, TickType_t xTicksToWait);


/**
 * @brief Set a fault in the fault bitmap from an ISR
 * 
 * @param bit which fault is being set
 * @return none
 */
void set_faultBitFromISR(fault_bit_t bit);