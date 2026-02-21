#pragma once
#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"

// Chip Device Address
#define DEVICE_ADDR 0x6a

// Reg Addresses
#define REG_CHARGE_CONTROL 0x17
#define REG_PIN_CONTROL  0x18
#define REG_CHARGE_STATUS_1 0x21
#define REG_CHARGE_STATUS_2 0x22
#define REG_CHARGE_STATUS_3 0x23
#define REG_FAULT_STATUS 0x24
#define REG_TEMP 0x1c
#define REG_REVERSE_MODE 0x19

// 0x02 --- charge current limit
// input current limit

// Relevant Bitstrings
static const uint8_t BIT_WDG_RESET =   0x30;
static const uint8_t BIT_CHARGE_ENABLE =   0x30;
static const uint8_t BIT_CHARGE_LIMIT_ENABLE =   0x80;
static const uint8_t BIT_HIZ_ENABLE =   0x40;
static const uint8_t BIT_EN_CHG_ENABLE =   0x3B;
static const uint8_t BIT_TEMP_SENSE_ENABLE =    0x01;
static const uint8_t BIT_REVERSE_MODE_ENABLE =    0x21;

// Host Message Types
typedef enum {
    STOP=0,
    START
} message_t;

typedef enum {
    LOW=0,
    HIGH
} logic_t;

/* Initializes I2C, GPIO, and hardware resources */
void bq25756e_init(void);

/* Write to a register on chip (transmit w/ 2 bytes) */
uint8_t bq25756e_write_reg(uint8_t reg, uint8_t data);

/* Read a register on chip (transmit + receive) */
uint8_t bq25756e_read_reg(uint8_t reg, uint8_t* buffer);

/* Initializes I2C peripheral */
void bq25756e_i2c4_init(void);

/* Writes value to CE register to enable or disable charging */
void bq25756e_write_ce(uint8_t value);

uint8_t bq25756e_charge(message_t MSG);

uint8_t bq25756e_pet_wdg(void);
void bq25756e_dump(uint8_t reg);
void bq25756e_gpio_init(void);