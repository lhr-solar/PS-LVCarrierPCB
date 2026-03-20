#pragma once

#include "stm32xx_hal.h"
#include "CAN_FD.h"

#define LVCARRIER_CAN_TX_PROFILE_PORT GPIOA
#define LVCARRIER_CAN_TX_PROFILE_PIN GPIO_PIN_10

#define LVCARRIER_CAN_RX_PROFILE_PORT GPIOB
#define LVCARRIER_CAN_RX_PROFILE_PIN GPIO_PIN_4

#define LVCARRIER_FDCAN_NVIC_PRIO configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3

/* LV Carrier CAN message IDs */
typedef enum {
    /**
     * 0x600
     * LV_Carrier_Status
     * 2 bytes DLC
    */
    LVCARRIER_CAN_MSG_STAT=0x600,

    /**
     * 0x300
     * Supp_Battery_Status
     * 8 bytes DLC
     * Supplemental_Battery_Voltage
    */
    LVCARRIER_CAN_MSG_SUPP=0x300,

     /**
     * 0x301
     * Supp_Battery_Charger_Status
     * 8 bytes DLC
    */
   LVCARRIER_CAN_MSG_SUPP_CHRG=0x300,
} lvcarrier_can_msg_t;

/************* LV Carrier Status *************/
#define LVCARRIER_CAN_STAT_SIZE 2 // 2 bytes to pack
/* LTC4421_HVDCDC_Selected */
#define LVCARRIER_CAN_STAT_DCDC_SELECT(m)        ((m) << 0)
/* LTC4421_HVDCDC_Fault */
#define LVCARRIER_CAN_STAT_DCDC_FAULT(m)         ((m) << 1)
/* LTC4421_HVDCDC_Valid */
#define LVCARRIER_CAN_STAT_DCDC_VALID(m)         ((m) << 2)
/* LTC4421_SuppBatt_Selected */
#define LVCARRIER_CAN_STAT_SUPP_SELECT(m)        ((m) << 3)
/* LTC4421_SuppBatt_Fault */
#define LVCARRIER_CAN_STAT_SUPP_FAULT(m)         ((m) << 4)
/* LTC4421_SuppBatt_Valid */
#define LVCARRIER_CAN_STAT_SUPP_VALID(m)         ((m) << 5)
/* LV_EN_SupplementalBattery */
#define LVCARRIER_CAN_STAT_SUPP_EN(m)            ((m) << 6)
/* LV_EN_PowerSupply */
#define LVCARRIER_CAN_STAT_POWER_SUPPLY_EN(m)    ((m) << 7)

/************* Supp Batt Status *************/
#define LVCARRIER_CAN_SUPP_SIZE 8 // 8 bytes to pack

/************* Supp Charger Status *************/
#define LVCARRIER_CAN_SUPP_CHRG_SIZE 8 // 8 bytes to pack

#define LVCARRIER_CAN_SUPP_CHRG_SIZE 8
/* SuppCharger_Status */
#define LVCARRIER_CAN_SUPP_CHRG_STAT(m)                  ((m) << 0)
/* Supplemental_Battery_Current */
#define LVCARRIER_CAN_SUPP_CHRG_SUPP_BATT_CURRENT(m)     ((m) << 0x10)
/* Supplemental_DCDC_Voltage */
#define LVCARRIER_CAN_SUPP_CHRG_STAT_DCDC_VOLTAGE(m)     ((m) << 0x20)
/* Supplemental_DCDC_Current */
#define LVCARRIER_CAN_SUPP_CHRG_STAT_DCDC_CURRENT(m)     ((m) << 0x30)






