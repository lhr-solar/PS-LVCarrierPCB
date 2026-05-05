#include "supplementalMonitor.h"

StackType_t Task_SuppMon_Stack_Array[ TASK_SUPP_MON_STACK_SIZE ];
StaticTask_t Task_SuppMon_Buffer;

// ADC watchdog timers
TimerHandle_t adcTimers[NUM_ADC_SENSE_CHANNELS];
StaticTimer_t adcTimersBuffers[ NUM_ADC_SENSE_CHANNELS ];


#define ADC_WATCHDOG_PERIOD_TICKS  pdMS_TO_TICKS(5000)
#define ADC_TIMEOUT_TICKS          pdMS_TO_TICKS(200)

#define SUPP_MEASUREMENTS_PRINTOUT_PERIOD_TICKS pdMS_TO_TICKS(2500)

int16_t adc_To_Hall(uint32_t adcCounts){
  adcCounts = adcCounts > 4095 ? 4095 : adcCounts;
  return adc_counts_to_ma_tmcs1126[adcCounts];
}

uint32_t adc_to_SuppVoltage(uint32_t adcCounts){
  adcCounts = adcCounts > 4095 ? 4095 : adcCounts;

  // todo: have this read the vref of the ADC instead hard coding it.
  return (uint32_t)((adcCounts * 3045U) / 4095U) * 11U;
}

 uint8_t packSuppBatteryStatusMessage(supp_battery_status_t suppBattStatus, uint8_t msgArr[8]){
    if(msgArr == NULL){
        return 0;
    }

    // byte 0 is the supplemental battery fault
    msgArr[0] = suppBattStatus.Supplemental_Battery_Fault;

    // 1st and 2nd(msb) bytes are supp voltage
    memcpy(&msgArr[1], &(suppBattStatus.Supplemental_Battery_Voltage), sizeof(uint16_t));

    // 3rd and 4th(msb) bytes are supp current
    memcpy(&msgArr[3], &(suppBattStatus.Supplemental_Battery_Current), sizeof(uint16_t));

    // byte 5 is the frame ID
    msgArr[5] = suppBattStatus.FrameID_Supp;

    return 1;
 }

 uint8_t packSuppBatteryRawMeasurementsMessage(supp_measurements_adc_t rawMeasurements, uint8_t msgArr[8]){
    if(msgArr == NULL){
        return 0;
    }

    // 0th and 1st(msb) bytes are supp voltage
    memcpy(&msgArr[0], &(rawMeasurements.Supp_Battery_Voltage_ADC), sizeof(uint16_t));

    // 0th and 1st(msb) bytes are supp current
    memcpy(&msgArr[2], &(rawMeasurements.Supp_Battery_Current_ADC), sizeof(int16_t));

    msgArr[4] = rawMeasurements.FrameID_Supp;

    return 1;
 }

 uint8_t packSuppVicorRawMeasurements(supp_vicor_measurements_adc_t suppVicorMeasurements, uint8_t msgArr[8]){
    if(msgArr == NULL){
        return 0;
    }

    memcpy(&msgArr[0], &(suppVicorMeasurements.Supp_Vicor_Voltage_ADC), sizeof(uint16_t));
    memcpy(&msgArr[2], &(suppVicorMeasurements.Supp_Vicor_Current_ADC), sizeof(uint16_t));

    msgArr[4] = suppVicorMeasurements.FrameID_Supp_Charger;

    return 1;
 }

 static BaseType_t readVoltage(adc_sense_channel_t channel, uint32_t *voltage, uint32_t *counts, TickType_t delay_ticks){
    if(voltage == NULL || counts == NULL){
        return pdFAIL;
    }

    BaseType_t readStat;
    adc_status_t startStat;

    // start the ADC for reading the supplemental battery voltage
    startStat = adc_start_read(channel, delay_ticks);

    if(startStat == ADC_OK){
        readStat = adc_read_value(channel, counts, delay_ticks);

        // if a new ADC reading was recieved
        if(readStat == pdTRUE){
            *voltage = adc_to_SuppVoltage(*counts);
        }
        else{
            return pdFAIL;
        }
    }
    else{
        return pdFAIL;
    }
    return pdPASS;
 }

  static BaseType_t readCurrent(adc_sense_channel_t channel, uint32_t *current, uint32_t *counts, TickType_t delay_ticks){
    
    if(current == NULL || counts == NULL){
        return pdFAIL;
    }

    BaseType_t readStat;
    adc_status_t startStat;

    // start the ADC read
    startStat = adc_start_read(channel, delay_ticks);

    if(startStat == ADC_OK){
        readStat = adc_read_value(channel, counts, delay_ticks);

        // if a new ADC reading was recieved
        if(readStat == pdTRUE){
            *current = adc_To_Hall(*counts);
        }
        else{
            return pdFAIL;
        }
    }
    else{
        return pdFAIL;
    }
    return pdPASS;
 }

void supplementalMonitor(){

    adc_status_t senseInit = adc_sense_init();
    if(senseInit != ADC_OK){
        // do some error flag or some shit
    }

    supp_battery_status_t suppBattStatus;
    supp_measurements_adc_t suppRawMeasurements;
    supp_vicor_measurements_adc_t suppVicorMeasurements;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    TickType_t xLastPrintTime = xTaskGetTickCount();

    BaseType_t readStat;


    // TODO: be consistent about data sizes
    uint32_t supplementalBatteryVoltage;
    uint32_t supplementalBatteryVoltageCounts;

    uint32_t supplementalBatteryCurrent;
    uint32_t supplementalBatteryCurrentCounts;

    uint32_t vicorVoltage;
    uint32_t vicorVoltageCounts;

    uint32_t vicorCurrent;
    uint32_t vicorCurrentCounts;


    uint8_t suppStatusMsgData[8] = {0};
    uint8_t suppRawMeasurementsData[8] = {0};
    uint8_t vicorRawMeasurementsData[8] = {0};

    uint8_t suppMeasurementsFrameID = 0;


    while(1){

         // sync frame IDs between messages
        suppBattStatus.FrameID_Supp = suppMeasurementsFrameID;
        suppRawMeasurements.FrameID_Supp = suppMeasurementsFrameID;

        bq25756e_set_preReqBit(BQ25756E_PREREQ_SUPP_VALID);

        readStat = readVoltage(SUPPLEMENTAL_BATTERY_VOLTAGE, &supplementalBatteryVoltage, &supplementalBatteryVoltageCounts, ADC_TIMEOUT_TICKS);

        // supp voltage was read succesfully
        if(readStat == pdPASS){

            suppBattStatus.Supplemental_Battery_Voltage = supplementalBatteryVoltage;

            suppRawMeasurements.Supp_Battery_Voltage_ADC = supplementalBatteryVoltageCounts;

            // TODO: set faults
        }  

        

        readStat = readCurrent(SUPPLEMENTAL_BATTERY_CURRENT, &supplementalBatteryCurrent, &supplementalBatteryCurrentCounts, ADC_TIMEOUT_TICKS);

        if(readStat == pdPASS){
            suppBattStatus.Supplemental_Battery_Current = supplementalBatteryCurrent;

            suppRawMeasurements.Supp_Battery_Current_ADC = supplementalBatteryCurrentCounts;

            // TODO: set faults
        }


        readStat = readVoltage(REGULATED_BATTERY_VOLTAGE, &vicorVoltage, &vicorVoltageCounts, ADC_TIMEOUT_TICKS);

        if(readStat == pdPASS){

            suppVicorMeasurements.Supp_Vicor_Voltage_ADC = vicorVoltageCounts;
        }

        readStat = readCurrent(REGULATED_BATTERY_CURRENT, &vicorCurrent, &vicorCurrentCounts, ADC_TIMEOUT_TICKS);

        if(readStat == pdPASS){

            suppVicorMeasurements.Supp_Vicor_Current_ADC = vicorCurrentCounts;
        }

        // pack the supplemental battery voltage and current into a CAN message
        packSuppBatteryStatusMessage(suppBattStatus, suppStatusMsgData);
        canbus_send(CAN_ID_SUPP_BATTERY_STATUS, CAN_DLC_SUPP_BATTERY_STATUS, suppStatusMsgData, ADC_TIMEOUT_TICKS);

        // pack the raw ADC data for supp battery into a CAN message
        packSuppBatteryRawMeasurementsMessage(suppRawMeasurements, suppRawMeasurementsData);
        canbus_send(CAN_ID_SUPP_MEASUREMENTS_ADC, CAN_DLC_SUPP_MEASUREMENTS_ADC, suppRawMeasurementsData, ADC_TIMEOUT_TICKS);

        packSuppVicorRawMeasurements(suppVicorMeasurements, vicorRawMeasurementsData);
        canbus_send(CAN_ID_SUPP_VICOR_MEASUREMENTS_ADC, CAN_DLC_SUPP_VICOR_MEASUREMENTS_ADC, vicorRawMeasurementsData, ADC_TIMEOUT_TICKS);

        if(xLastPrintTime + SUPP_MEASUREMENTS_PRINTOUT_PERIOD_TICKS <= xTaskGetTickCount()){

            xLastPrintTime = xTaskGetTickCount();

            printf("Supp battery voltage: %ld counts \n\r", supplementalBatteryVoltageCounts);
            printf("Supp battery voltage: %ld mV \n\r", suppBattStatus.Supplemental_Battery_Voltage);

            printf("Supp battery current: %ld counts \n\r", supplementalBatteryCurrentCounts);
            printf("Supp battery current: %d mA \n\r", suppBattStatus.Supplemental_Battery_Current);

            printf("Vicor output voltage: %ld counts \n\r", vicorVoltageCounts);
            printf("Vicor output voltage: %ld mv \n\r", adc_to_SuppVoltage(vicorVoltageCounts));

        }

        // increment (and wrap) the frame ID
        suppMeasurementsFrameID = ((suppMeasurementsFrameID + 1) % 255);
        
        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        vTaskDelayUntil(&xLastWakeTime, SUPPLEMENTAL_MONITOR_THREAD_DELAY_TICKS);
    }

}