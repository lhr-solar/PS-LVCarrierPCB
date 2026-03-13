#pragma once
#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"
#include "bq25756e_preReqBits.h"
#include "commandLine.h"
#include "event_groups.h"
#include "statusLeds.h"

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

/**** CHARGE CURRENT MASKS ****/
typedef enum {
    BQ25756E_5A    =   0x0190,
    BQ25756E_2_6A  =   0x00D0,
    BQ25756E_1A    =   0x0014,
} bq25756e_chg_current_t;

typedef enum {
    BQ25756E_LOGIC_LOW=0,
    BQ25756E_LOGIC_HIGH
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
    BQ25756E_NOT_CHRG,
    BQ25756E_TRICKLE,
    BQ25756E_PRE,
    BQ25756E_FAST,
    BQ25756E_TAPER,
    _reserved,
    BQ25756E_TOP_OFF,
    BQ25756E_DONE_CHRG
} bq25756e_charge_status_t;

typedef struct {
    uint32_t device_id;
    I2C_HandleTypeDef* hi2c;
    SemaphoreHandle_t bq_i2c_smphr;
} BQ_HandleTypeDef;

/* Pulls CE pin in software to low or high (LOW enables charging) */
void bq25756e_write_ce(bq25756e_logic_t val);

/* Initializes I2C, GPIO, and hardware resources */
bq25756e_status_t bq25756e_init(void);

/* Initializes I2C peripheral */
bq25756e_status_t bq25756e_i2c4_init(void);

bq25756e_status_t bq25756e_charge(TickType_t delay, bq25756e_chg_current_t limit);

/**
 * On POR, device starts in watchdog timer expired state (WD_STAT = 1).
 * 
 * [] Function writes to WD_RST bit to reset timer. 
 * [] If watchdog timer expires, chip stops charging.
 * [] Watchdog must be pet every 160 seconds to ensure it doesn't expire.
 * 
 */
bq25756e_status_t bq25756e_pet_wdg(TickType_t delay);

bq25756e_status_t bq25756e_dump_status(TickType_t delay);

bq25756e_status_t bq25756e_dump_faults(TickType_t delay);

bq25756e_status_t bq25756e_charge_disable(TickType_t delay);