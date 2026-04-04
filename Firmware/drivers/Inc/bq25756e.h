#pragma once
#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"
#include "bq25756e_preReqBits.h"
#include "commandLine.h"
#include "event_groups.h"
#include "statusLeds.h"

/* I2C Driver */
#define BQ25756E_I2C_PERIPH     I2C4
// Lesser priority than default RTOS interrupt
#define I2C4_NVIC_PRIO  configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1

#define BQ25756E_I2C_PORT       GPIOC

#define BQ25756E_I2C_SDA_PIN    GPIO_PIN_7
#define BQ25756E_I2C_SCL_PIN    GPIO_PIN_6
#define BQ25756E_I2C_AF         GPIO_AF8_I2C4

/**** DEVICE ADDRESSES  ****/
#define DEVICE_ADDR 0x6a // not shifted

/**** REGISTER ADDRESSES  ****/
// Charge configuration
#define BQ25756E_REG_CHARGE_VOLTAGE_LIMIT 0x00
#define BQ25756E_REG_CHARGE_CURRENT_LIMIT_A 0x02
#define BQ25756E_REG_CHARGE_CURRENT_LIMIT_B 0x03

// Input limits
#define BQ25756E_REG_INPUT_CURRENT_DPM_LIMIT_A 0x06
#define BQ25756E_REG_INPUT_CURRENT_DPM_LIMIT_B 0x07
#define BQ25756E_REG_INPUT_VOLTAGE_DPM_LIMIT_A 0x08
#define BQ25756E_REG_INPUT_VOLTAGE_DPM_LIMIT_B 0x09

// Reverse mode configuration
#define BQ25756E_REG_REVERSE_MODE_INPUT_CURRENT_LIMIT_A 0x0A
#define BQ25756E_REG_REVERSE_MODE_INPUT_CURRENT_LIMIT_B 0x0B
#define BQ25756E_REG_REVERSE_MODE_INPUT_VOLTAGE_LIMIT_A 0x0C
#define BQ25756E_REG_REVERSE_MODE_INPUT_VOLTAGE_LIMIT_B 0x0D

// Precharge / termination
#define BQ25756E_REG_PRECHARGE_CURRENT_LIMIT_A 0x10
#define BQ25756E_REG_PRECHARGE_CURRENT_LIMIT_B 0x11
#define BQ25756E_REG_TERMINATION_CURRENT_LIMIT_A 0x12
#define BQ25756E_REG_TERMINATION_CURRENT_LIMIT_B 0x13

// Control registers
#define BQ25756E_REG_PRECHARGE_TERMINATION_CONTROL 0x14
#define BQ25756E_REG_TIMER_CONTROL 0x15
#define BQ25756E_REG_CHARGE_CONTROL 0x17
#define BQ25756E_REG_PIN_CONTROL  0x18
#define BQ25756E_REG_REVERSE_MODE 0x19

// Temperature / monitoring
#define BQ25756E_REG_TEMP 0x1C

// Status registers
#define BQ25756E_REG_CHARGE_STATUS_1 0x21
#define BQ25756E_REG_CHARGE_STATUS_2 0x22
#define BQ25756E_REG_CHARGE_STATUS_3 0x23
#define BQ25756E_REG_FAULT_STATUS 0x24

// ADC / measurement
#define BQ25756E_REG_ADC_CONTROL 0x2B
#define BQ25756E_REG_VBAT_ADC 0x33
#define BQ25756E_REG_TS_ADC 0x37
#define BQ25756E_REG_VFB_ADC 0x39

// Gate driver configuration
#define BQ25756E_REG_GATE_DRIVER_STRENGTH 0x3B
#define BQ25756E_REG_GATE_DRIVER_DEAD_TIME 0x3C

// Device info
#define BQ25756E_REG_PART_INFORMATION 0x3D

/**** RELEVANT REGISTER MASKS ****/
#define BQ25756E_BIT_WDG_RESET              (0x30)
#define BQ25756E_BIT_CHARGE_ENABLE          (0x01)
#define BQ25756E_BIT_CHARGE_LIMIT_ENABLE    (0x80)
#define BQ25756E_BIT_HIZ_ENABLE             (0x40)
#define BQ25756E_BIT_EN_CHG_ENABLE          (0x3B)
#define BQ25756E_BIT_TEMP_SENSE_ENABLE      (0x01)
#define BQ25756E_BIT_REVERSE_MODE_ENABLE    (0x21)
#define BQ25756E_BIT_CHARGE_CURRENT_FIELD_B (0x07) // 0000 0111
#define BQ25756E_BIT_CHARGE_CURRENT_FIELD_A (0xFC) // 1111 1100
#define BQ25756E_BIT_CHARGE_STAT            (0x07)

#define BQ25756E_BIT_INPUT_UV_FAULT         (1 << 7)
#define BQ25756E_BIT_INPUT_OV_FAULT         (1 << 6)
#define BQ25756E_BIT_BATT_OVERCURRENT_FAULT (1 << 5)
#define BQ25756E_BIT_BATT_OV_FAULT          (1 << 4)
#define BQ25756E_BIT_THERMAL_SHDN_FAULT     (1 << 3)
#define BQ25756E_BIT_CHRG_TIMER_FAULT       (1 << 2)
#define BQ25756E_BIT_DRV_SUP_FAULT          (1 << 1)

// Pin enables are active low [negative logic]
typedef enum {
    BQ25756E_LOGIC_HIGH=0,
    BQ25756E_LOGIC_LOW
} bq25756e_logic_t;

typedef enum {
    /* generic ok */
    BQ25756E_OK,
    /* generic error */
    BQ25756E_ERR,
    /* bq25756e_init did not run successfully */
    BQ25756E_INIT_FAIL,
    /* read_reg did not run successfully */
    BQ25756E_READ_FAIL,
    /* write_reg did not run successfully */
    BQ25756E_WRITE_FAIL,
    /* rtos calls that block exited without returning */
    BQ25756E_TIMEOUT
} bq25756e_status_t;

typedef enum {
    BQ25756E_NOT_STARTED,
    BQ25756E_TRICKLE,
    BQ25756E_PRE,
    BQ25756E_FAST,
    BQ25756E_TAPER,
    BQ25756E_RESERVED,
    BQ25756E_TOP_OFF,
    BQ25756E_DONE_CHRG
} bq25756e_charge_status_t;

/* Enable serial output when dumping status or faults */
typedef enum {
    BQ25756E_SERIAL_DISABLE=0,
    BQ25756E_SERIAL_ENABLE 
} bq25756e_serial_config_t;

typedef struct {
    uint8_t device_id; // address is shifted by 1
    I2C_HandleTypeDef* hi2c;
    SemaphoreHandle_t bq_i2c_smphr;
} BQ_HandleTypeDef;

/* Pulls CE pin in software to low or high (LOW enables charging) */
void bq25756e_write_ce(bq25756e_logic_t val);

/**
 * @brief Initializes the BQ25756E driver and GPIO/I2C resources.
 *
 * Configures the internal driver handle, initializes the CE and STAT pins,
 * and sets up a binary semaphore for asynchronous I2C communication.  
 * Prepares the driver for all subsequent operations.
 *
 * @param _bq_handle Pointer to the BQ25756E driver handle structure.
 * @param bq_i2c_handle Pointer to the HAL I2C handle used for communication.
 *
 * @return bq25756e_status_t Returns BQ25756E_OK on success,
 *                           or BQ25756E_ERR if handle creation fails.
 */
bq25756e_status_t bq25756e_init(BQ_HandleTypeDef *bq_handle, I2C_HandleTypeDef *bq_i2c_handle);

/**
 * @brief Enables battery charging with a software charge current limit.
 *
 * This function:
 * - Disables the hardware charge limit.
 * - Sets the requested software charge limit across registers 0x02/0x03.
 * - Disables HiZ, TempSense, and Reverse Mode.
 * - Asserts the CE pin to start charging.
 *
 * @param delay Maximum wait time for I2C transactions (in FreeRTOS ticks).
 * @param limit Desired charge current limit of type bq25756e_chg_current_t.
 *
 * @return bq25756e_status_t Returns BQ25756E_OK if charging was successfully enabled,
 *                           or an error code on I2C failure.
 */
bq25756e_status_t bq25756e_charge(TickType_t delay, uint32_t limit);

/**
 * @brief Resets the BQ25756E watchdog timer to prevent charge interruption.
 *
 * On POR, the device starts in a watchdog-expired state.  
 * Writing to the WD_RST bit clears the timer. Must be called
 * periodically (<= 160 seconds) to maintain charging.
 *
 * @param delay Maximum wait time for I2C transactions (in FreeRTOS ticks).
 *
 * @return bq25756e_status_t Returns BQ25756E_OK if the watchdog was reset,
 *                           BQ25756E_READ_FAIL if the control register could not be read,
 *                           BQ25756E_WRITE_FAIL if writing fails.
 */
bq25756e_status_t bq25756e_pet_wdg(TickType_t delay);

/**
 * @brief Reads and optionally prints the current charger status.
 *
 * Retrieves CHARGE_STATUS_1 register and maps the status bits
 * to the bq25756e_charge_status_t enum.
 * Can print the status to serial if requested.
 *
 * @param charge_state Pointer to store the parsed charge state.
 * @param serial Enable or disable serial printing of the status.
 * @param delay Maximum wait time for I2C transactions (in FreeRTOS ticks).
 *
 * @return bq25756e_status_t Returns BQ25756E_OK if status was read,
 *                           BQ25756E_READ_FAIL if I2C read fails,
 *                           or BQ25756E_ERR if an invalid status is detected.
 */
bq25756e_status_t bq25756e_dump_status(bq25756e_charge_status_t *charge_state, bq25756e_serial_config_t serial, TickType_t delay);

/**
 * @brief Reads and optionally prints charger fault conditions.
 *
 * Retrieves FAULT_STATUS register and maps all fault bits.
 * Optionally prints human-readable fault messages via serial.
 * Returns BQ25756E_ERR if any faults are active.
 *
 * @param fault_state Pointer to store raw fault register value.
 * @param serial Enable or disable serial printing of fault messages.
 * @param delay Maximum wait time for I2C transactions (in FreeRTOS ticks).
 *
 * @return bq25756e_status_t Returns BQ25756E_OK if no faults detected,
 *                           BQ25756E_ERR if one or more faults exist,
 *                           or BQ25756E_READ_FAIL if register read fails.
 */
bq25756e_status_t bq25756e_dump_faults(uint8_t *fault_state, bq25756e_serial_config_t serial, TickType_t delay);

/**
 * @brief Disables battery charging and clears the CE pin.
 *
 * Clears the CHARGE_ENABLE bit in CHARGE_CONTROL register and
 * drives the CE pin low to stop charging.
 *
 * @param delay Maximum wait time for I2C transactions (in FreeRTOS ticks).
 *
 * @return bq25756e_status_t Returns BQ25756E_OK if charging was successfully disabled,
 *                           BQ25756E_READ_FAIL if reading CHARGE_CONTROL fails,
 *                           BQ25756E_WRITE_FAIL if writing fails.
 */
bq25756e_status_t bq25756e_charge_disable(TickType_t delay);