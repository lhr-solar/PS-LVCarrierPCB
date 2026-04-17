#include "supplementalMonitor.h"

StackType_t Task_SuppMon_Stack_Array[ TASK_SUPP_MON_STACK_SIZE ];
StaticTask_t Task_SuppMon_Buffer;

// ADC watchdog timers
TimerHandle_t adcTimers[NUM_ADC_SENSE_CHANNELS];
StaticTimer_t adcTimersBuffers[ NUM_ADC_SENSE_CHANNELS ];


#define ADC_WATCHDOG_PERIOD_TICKS  pdMS_TO_TICKS(5000)
#define ADC_TIMEOUT_TICKS          pdMS_TO_TICKS(200)

#define SUPP_MEASUREMENTS_PRINTOUT_PERIOD_TICKS pdMS_TO_TICKS(5000)

int16_t adc_To_Hall(uint32_t adcCounts){
  adcCounts = adcCounts > 4095 ? 4095 : adcCounts;
  return adc_counts_to_ma_tmcs1126[adcCounts];
}

uint32_t adc_to_SuppVoltage(uint32_t adcCounts){
  adcCounts = adcCounts > 4095 ? 4095 : adcCounts;

  // todo: have this read the vref of the ADC instead hard coding it.
  return ((uint32_t)adcCounts * 3000U * 11U) / 4095U; 
}

 void adcWatchdogTimerCallback( TimerHandle_t xTimer ){

    for(uint8_t i = 0; i < NUM_ADC_SENSE_CHANNELS; i++){
        // see which watchdog timer finished.
        if(adcTimers[i] == xTimer){
            set_faultBit(FAULT_ADC_TIMEOUT);
        }
    }
 } 


 uint8_t packSuppBatteryStatusMessage(supp_battery_status_t suppBattStatus, uint8_t msgArr[8]){
    if(msgArr == NULL){
        return 0;
    }

    // byte 0 is the supplemental battery fault
    msgArr[0] = suppBattStatus.Supplemental_Battery_Fault;

    // 2nd and 3rd(msb) bytes are supp voltage
    memcpy(&msgArr[1], &(suppBattStatus.Supplemental_Battery_Voltage), sizeof(uint16_t));

    // 3rd and 4th(msb) bytes are supp voltage
    memcpy(&msgArr[3], &(suppBattStatus.Supplemental_Battery_Current), sizeof(uint16_t));

    // byte 5 is the frame ID
    msgArr[4] = suppBattStatus.FrameID_Supp;
    return 1;
 }

 uint8_t packSuppBatteryRawMeasurementsMessage(supp_measurements_rawv_t rawMeasurements, uint8_t msgArr[8]){
    if(msgArr == NULL){
        return 0;
    }

    // 0th and 1st(msb) bytes are supp voltage
    memcpy(&msgArr[0], &(rawMeasurements.Supp_Battery_Voltage_RawV), sizeof(uint16_t));

    // 0th and 1st(msb) bytes are supp current
    memcpy(&msgArr[2], &(rawMeasurements.Supp_Battery_Current_RawV), sizeof(int16_t));

    msgArr[4] = rawMeasurements.FrameID_Supp;

    return 1;
 }

 static BaseType_t readSupplementalVoltage(uint32_t *suppVoltage, uint32_t *counts, TickType_t delay_ticks){

    if(suppVoltage == NULL || counts == NULL){
        return pdFAIL;
    }

    BaseType_t readStat;
    adc_status_t startStat;

    // start the ADC for reading the supplemental battery voltage
    startStat = adc_start_read(SUPPLEMENTAL_BATTERY_VOLTAGE, delay_ticks);

    if(startStat == ADC_OK){
        readStat = adc_read_value(SUPPLEMENTAL_BATTERY_VOLTAGE, counts, delay_ticks);

        // if a new ADC reading was recieved
        if(readStat == pdTRUE){
            *suppVoltage = adc_to_SuppVoltage(*counts);
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
 
 static BaseType_t readSupplementalCurrent(uint32_t *suppCurrent, uint32_t *counts, TickType_t delay_ms){

    if(suppCurrent == NULL || counts == NULL){
        return pdFAIL;
    }

    BaseType_t readStat;
    adc_status_t startStat;

    // start the ADC for reading the supplemental battery current
    startStat = adc_start_read(SUPPLEMENTAL_BATTERY_CURRENT, delay_ms);

    if(startStat == ADC_OK){
        readStat = adc_read_value(SUPPLEMENTAL_BATTERY_CURRENT, counts, delay_ms);

        // if a new ADC reading was recieved
        if(readStat == pdTRUE){
            *suppCurrent = adc_To_Hall(*counts);
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
    supp_measurements_rawv_t suppRawMeasurements;

    // initialize adc watchdog timers
    for(uint8_t i = 0; i < NUM_ADC_SENSE_CHANNELS; i++){
        // adcTimers
        adcTimers[i] = xTimerCreateStatic
        (
            "ADC Watchdog Timer",
            ADC_WATCHDOG_PERIOD_TICKS,
            pdFALSE, // one shot timer
            ( void * ) 0, // stores a count of times the timer has expired
            adcWatchdogTimerCallback, /* Each timer calls the same callback when it expires. */
            &adcTimersBuffers[i]
        );

        // start the watchdog timer
        xTimerStart(adcTimers[i], portMAX_DELAY);
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();

    TickType_t xLastPrintTime = xTaskGetTickCount();

    BaseType_t readStat;


    // TODO: be consistent about data sizes
    uint32_t supplementalBatteryVoltage;
    uint32_t supplementalBatteryVoltageCounts;

    uint32_t supplementalBatteryCurrent;
    uint32_t supplementalBatteryCurrentCounts;


    uint8_t suppStatusMsgData[8] = {0};
    uint8_t suppRawMeasurementsData[8] = {0};

    uint8_t suppMeasurementsFrameID = 0;


    while(1){

         // sync frame IDs between messages
        suppBattStatus.FrameID_Supp = suppMeasurementsFrameID;
        suppRawMeasurements.FrameID_Supp = suppMeasurementsFrameID;

        // TODO: make sure i'm consistent about ticks vs ms
        readStat = readSupplementalVoltage(&supplementalBatteryVoltage, &supplementalBatteryVoltageCounts, ADC_TIMEOUT_TICKS);

        // supp voltage was read succesfully
        if(readStat == pdPASS){

            suppBattStatus.Supplemental_Battery_Voltage = supplementalBatteryVoltage;

            // TODO: convert this from counts to mV
            suppRawMeasurements.Supp_Battery_Voltage_RawV = supplementalBatteryVoltageCounts;

            // TODO: set faults
        }  

        readStat = readSupplementalCurrent(&supplementalBatteryCurrent, &supplementalBatteryCurrentCounts, ADC_TIMEOUT_TICKS);

        if(readStat == pdPASS){
            suppBattStatus.Supplemental_Battery_Current = supplementalBatteryCurrent;

            // TODO: convert this from counts to mV
            suppRawMeasurements.Supp_Battery_Current_RawV = supplementalBatteryCurrentCounts;

            // TODO: set faults
        }

        // pack the supplemental battery voltage and current into a CAN message
        packSuppBatteryStatusMessage(suppBattStatus, suppStatusMsgData);
        canbus_send(CAN_ID_SUPP_BATTERY_STATUS, CAN_DLC_SUPP_BATTERY_STATUS, suppStatusMsgData, ADC_TIMEOUT_TICKS);

        // pack the raw ADC data for supp battery into a CAN message
        packSuppBatteryRawMeasurementsMessage(suppRawMeasurements, suppRawMeasurementsData);
        canbus_send(CAN_ID_SUPP_MEASUREMENTS_RAWV, CAN_DLC_SUPP_MEASUREMENTS_RAWV, suppRawMeasurementsData, ADC_TIMEOUT_TICKS);

        if(xLastPrintTime + SUPP_MEASUREMENTS_PRINTOUT_PERIOD_TICKS <= xTaskGetTickCount()){
            xLastPrintTime = xTaskGetTickCount();
        }

        // increment (and wrap) the frame ID
        suppMeasurementsFrameID = ((suppMeasurementsFrameID + 1) % 255);
        
        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        vTaskDelayUntil(&xLastWakeTime, SUPPLEMENTAL_MONITOR_THREAD_DELAY_TICKS);
    }

}