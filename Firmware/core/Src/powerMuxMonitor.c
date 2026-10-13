#include "powerMuxMonitor.h"

StaticTask_t Task_PowerMux_Buffer;
StackType_t Task_PowerMux_Stack_Array[ TASK_POWER_MUX_MON_STACK_SIZE ];

#define LTC_CAN_MSG_MAX_DELAY_TICKS pdMS_TO_TICKS(100)

void updateLtcStatus(lv_carrier_status_t *status){

    if(status == NULL){
        return;
    }

    status->LTC4421_HVDCDC_Fault = (ltc4421_read_fault(LTC4421_HV_DCDC_CHANNEL) == ON) ? LV_CARRIER_STATUS_LTC4421_HVDCDC_FAULT_OK : LV_CARRIER_STATUS_LTC4421_HVDCDC_FAULT_FAULT;
    status->LTC4421_SuppBatt_Fault = (ltc4421_read_fault(LTC4421_SUPP_DCDC_CHANNEL) == ON) ? LV_CARRIER_STATUS_LTC4421_SUPPBATT_FAULT_OK : LV_CARRIER_STATUS_LTC4421_SUPPBATT_FAULT_FAULT;

    status->LTC4421_HVDCDC_Valid = (ltc4421_read_valid(LTC4421_HV_DCDC_CHANNEL) == ON) ? LV_CARRIER_STATUS_LTC4421_HVDCDC_VALID_OK : LV_CARRIER_STATUS_LTC4421_HVDCDC_VALID_NOT_OK;
    status->LTC4421_SuppBatt_Valid = (ltc4421_read_valid(LTC4421_SUPP_DCDC_CHANNEL) == ON) ? LV_CARRIER_STATUS_LTC4421_SUPPBATT_VALID_OK : LV_CARRIER_STATUS_LTC4421_SUPPBATT_VALID_NOT_OK;

    // update supp batt lv enable status
    status->LV_EN_SupplementalBattery = (get_lvEnable_supp() == ON) ? LV_CARRIER_STATUS_LV_EN_SUPPLEMENTALBATTERY_ENABLED : LV_CARRIER_STATUS_LV_EN_SUPPLEMENTALBATTERY_DISABLED; 

    ltc4421_channel_t channel = ltc4421_channel_selected();
    status->LTC4421_HVDCDC_Selected = (channel == LTC4421_HV_DCDC_CHANNEL) ? 1 : 0;
    status->LTC4421_SuppBatt_Selected = (channel == LTC4421_SUPP_DCDC_CHANNEL) ? 1 : 0;

    // update power supply enable status
    status->LV_EN_PowerSupply = (get_lvEnable_powerSupply() == ON) ? LV_CARRIER_STATUS_LV_EN_POWERSUPPLY_ENABLED : LV_CARRIER_STATUS_LV_EN_POWERSUPPLY_DISABLED;
}

void packLvCarrierStatusMsg(lv_carrier_status_t status, uint8_t msgArr[8]){
    if(msgArr == NULL){
        return;
    }

    msgArr[0] = ((uint8_t)(status.LV_EN_PowerSupply                    & 0x01) << 7);
    msgArr[0] |= ((uint8_t)(status.LV_EN_SupplementalBattery           & 0x01) << 6);
    msgArr[0] |= ((uint8_t)(status.LTC4421_SuppBatt_Valid              & 0x01) << 5);
    msgArr[0] |= ((uint8_t)(status.LTC4421_SuppBatt_Fault              & 0x01) << 4);
    msgArr[0] |= ((uint8_t)(status.LTC4421_SuppBatt_Selected           & 0x01) << 3);
    msgArr[0] |= ((uint8_t)(status.LTC4421_HVDCDC_Valid                & 0x01) << 2);
    msgArr[0] |= ((uint8_t)(status.LTC4421_HVDCDC_Fault                & 0x01) << 1);
    msgArr[0] |= ((uint8_t)(status.LTC4421_HVDCDC_Selected             & 0x01) << 0);

}



void powerMuxMonitor(){
        
    // initialize pins for the power mux
    ltc4421_gpio_init();

    // initialize pins to read the state of the LV enable signals
    lvEnable_gpio_init();

    TickType_t xLastWakeTime = xTaskGetTickCount();

    ltc4421_shdn_enable(OFF);
    statusLeds_write(HEARTBEAT_LED, ON);


    // turn on shutdown after 1 second
    vTaskDelay(pdMS_TO_TICKS(500));
    ltc4421_shdn_enable(ON);
    printf("LTC SHDN Disabled\n\r");

    lv_carrier_status_t status;
    uint8_t lvCarrierStatusMsgData[8] = {0};

    while(1){

        ltc4421_shdn_enable(ON);
        printf("LTC SHDN Disabled\n\r");

        statusLeds_toggle(HEARTBEAT_LED);

        // bq25756e_set_preReqBit(BQ25756E_PREREQ_LTC_VALID);

        updateLtcStatus(&status);

        packLvCarrierStatusMsg(status, lvCarrierStatusMsgData);

        canbus_send(CAN_ID_LV_CARRIER_STATUS, CAN_DLC_LV_CARRIER_STATUS, lvCarrierStatusMsgData, LTC_CAN_MSG_MAX_DELAY_TICKS);

        vTaskDelayUntil(&xLastWakeTime, POWER_MUX_MONITOR_THREAD_DELAY_TICKS);
    }
}