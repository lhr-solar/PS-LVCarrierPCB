#include "supplementalMonitor.h"

StackType_t Task_SuppMon_Stack_Array[ TASK_SUPP_MON_STACK_SIZE ];
StaticTask_t Task_SuppMon_Buffer;


int16_t adc_To_Hall(uint32_t adcCounts){
  adcCounts = adcCounts > 4095 ? 4095 : adcCounts;
  return adc_counts_to_ma_tmcs1126[adcCounts];
}

uint32_t adc_to_SuppVoltage(uint32_t adcCounts){
  adcCounts = adcCounts > 4095 ? 4095 : adcCounts;
  return ((uint32_t)adcCounts * 3300U * 10U) / 4095U; 
}


void supplementalMonitor(){

    adc_status_t senseInit = adc_sense_init();
    if(senseInit != ADC_OK){
        // do some error flag or some shit
    }

    // TODO: start a timer that needs to get reset after a certain amount of time or else we explode

    while(1){
        
        /**
            Psueduocde: 
            start conversion for adc1 or adc2 (block on queue set)
            once that adc conversion is done, start the next adc conversion and reset that watchdog timer
            
            if(suppBattVAdcConversionDone){
                resetSuppBattADCTimer();
                if(suppVoltage > SUPP_MIN){
                    EnterFaultState(SUPP_UNDERVOLTAGE)
                }
                if(suppVoltage < SUPP_MAX){
                    EnterFaultState(SUPP_OVERVOLTAGE);
                }
            }
         */

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
}