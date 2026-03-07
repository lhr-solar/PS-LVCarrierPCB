#pragma once
#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"
#include "commandLine.h"
#include "event_groups.h"
#include "statusLeds.h"

/**** DEVICE ADDRESSES  ****/
#define DEVICE_ADDR 0x6a

/**** REGISTER ADDRESSES  ****/
#define REG_CHARGE_CURRENT_LIMIT_A 0x02
#define REG_CHARGE_CURRENT_LIMIT_B 0x03
#define REG_CHARGE_CONTROL 0x17
#define REG_PIN_CONTROL  0x18
#define REG_CHARGE_STATUS_1 0x21
#define REG_CHARGE_STATUS_2 0x22
#define REG_CHARGE_STATUS_3 0x23
#define REG_FAULT_STATUS 0x24
#define REG_TEMP 0x1c
#define REG_REVERSE_MODE 0x19

/**** RELEVANT REGISTER MASKS ****/
static const uint8_t BIT_WDG_RESET =   0x30;
static const uint8_t BIT_CHARGE_ENABLE =   0x01;
static const uint8_t BIT_CHARGE_LIMIT_ENABLE =   0x80;
static const uint8_t BIT_HIZ_ENABLE =   0x40;
static const uint8_t BIT_EN_CHG_ENABLE =   0x3B;
static const uint8_t BIT_TEMP_SENSE_ENABLE =    0x01;
static const uint8_t BIT_REVERSE_MODE_ENABLE =    0x21;
static const uint8_t BIT_CHARGE_CURRENT_FIELD_B = 0x07; // 0000 0111
static const uint8_t BIT_CHARGE_CURRENT_FIELD_A = 0xFC; // 1111 1100
static const uint8_t BIT_CHARGE_STAT =    0x07;

/**** FAULT BITMAP ****/
static const uint8_t FAULT_BMAP_MASK = 0x90;

/**** CHARGE CURRENT MASKS ****/
typedef enum {
    BQ25756E_5A_MASK    =   0x0190,
    BQ25756E_2_6A_MASK  =   0x00D0,
    BQ25756E_1A_MASK    =   0x0014,
} bq25756e_chg_current_t;

// Host Message Types
typedef enum {
    BQ25756E_CHRG_START=0,
    BQ25756E_CHRG_STOP,
    BQ25756E_CHRG_DUMP,
    BQ25756E_CHRG_DUMP_FAULT,
    BQ25756E_PET_WDG
} bq25756e_message_t;

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

typedef enum {
    BQ25756E_PREREQ_LTC_VALID,
    BQ25756E_PREREQ_SUPP_VALID,
    BQ25756E_NUM_PREREQS
} bq25756e_prereqs_t;

extern EventGroupHandle_t BQ25756E_preReqBits;
extern StaticEventGroup_t BQ25756E_preReqBitsBuffer;

uint8_t bq25756e_preReqBits_init(void);

void bq25756e_set_preReqBit(bq25756e_prereqs_t bit);

EventBits_t bq25756e_preReqBit_wait(bq25756e_prereqs_t bit, TickType_t xTicksToWait);

/* Initializes I2C, GPIO, and hardware resources */
bq25756e_status_t bq25756e_init(void);

/* Write to a register on chip (transmit w/ 2 bytes) */
bq25756e_status_t bq25756e_write_reg(uint8_t reg, uint8_t data);

/* Read a register on chip (transmit + receive) */
bq25756e_status_t bq25756e_read_reg(uint8_t reg, uint8_t* buffer);

/* Initializes I2C peripheral */
bq25756e_status_t bq25756e_i2c4_init(void);

/* Pulls CE pin in software to low or high (LOW enables charging) */
void bq25756e_write_ce(bq25756e_logic_t val);

/* Enables/disables settings on I2C to start charging */
bq25756e_status_t bq25756e_charge(bq25756e_message_t MSG);

bq25756e_status_t bq25756e_pet_wdg(uint8_t* buffer);

void bq25756e_gpio_init(void);