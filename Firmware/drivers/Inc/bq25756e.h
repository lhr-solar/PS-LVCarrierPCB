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
#define BQ25756E_REG_CHARGE_CURRENT_LIMIT_A 0x02
#define BQ25756E_REG_CHARGE_CURRENT_LIMIT_B 0x03
#define BQ25756E_REG_CHARGE_CONTROL 0x17
#define BQ25756E_REG_PIN_CONTROL  0x18
#define BQ25756E_REG_CHARGE_STATUS_1 0x21
#define BQ25756E_REG_CHARGE_STATUS_2 0x22
#define BQ25756E_REG_CHARGE_STATUS_3 0x23
#define BQ25756E_REG_FAULT_STATUS 0x24
#define BQ25756E_REG_TEMP 0x1c
#define BQ25756E_REG_REVERSE_MODE 0x19

/**** RELEVANT REGISTER MASKS ****/
static const uint8_t BQ25756E_BIT_WDG_RESET =               0x30;
static const uint8_t BQ25756E_BIT_CHARGE_ENABLE =           0x01;
static const uint8_t BQ25756E_BIT_CHARGE_LIMIT_ENABLE =     0x80;
static const uint8_t BQ25756E_BIT_HIZ_ENABLE =              0x40;
static const uint8_t BQ25756E_BIT_EN_CHG_ENABLE =           0x3B;
static const uint8_t BQ25756E_BIT_TEMP_SENSE_ENABLE =       0x01;
static const uint8_t BQ25756E_BIT_REVERSE_MODE_ENABLE =     0x21;
static const uint8_t BQ25756E_BIT_CHARGE_CURRENT_FIELD_B =  0x07; // 0000 0111
static const uint8_t BQ25756E_BIT_CHARGE_CURRENT_FIELD_A =  0xFC; // 1111 1100
static const uint8_t BQ25756E_BIT_CHARGE_STAT =             0x07;

static const uint8_t BQ25756E_BIT_INPUT_UV_FAULT =          (1 << 7);
static const uint8_t BQ25756E_BIT_INPUT_OV_FAULT =          (1 << 6);
static const uint8_t BQ25756E_BIT_BATT_OVERCURRENT_FAULT =  (1 << 5);
static const uint8_t BQ25756E_BIT_BATT_OV_FAULT =           (1 << 4);
static const uint8_t BQ25756E_BIT_THERMAL_SHDN_FAULT =      (1 << 3); 
static const uint8_t BQ25756E_BIT_CHRG_TIMER_FAULT =        (1 << 2);
static const uint8_t BQ25756E_BIT_DRV_SUP_FAULT =           (1 << 1);

/**** FAULT BITMAP ****/
static const uint8_t FAULT_BMAP_MASK = 0x90;

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
    BQ25756E_OK,
    BQ25756E_INIT_FAIL,
    BQ25756E_READ_FAIL,
    BQ25756E_WRITE_FAIL,
    BQ25756E_PIN_FAIL,
    BQ25756E_TIMEOUT
} bq25756e_status_t;

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

bq25756e_status_t bq25756e_pet_wdg(TickType_t delay);

bq25756e_status_t bq25756e_dump_status(TickType_t delay);

bq25756e_status_t bq25756e_dump_faults(TickType_t delay);

bq25756e_status_t bq25756e_charge_disable(TickType_t delay);