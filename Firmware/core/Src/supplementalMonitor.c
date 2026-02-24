#include "supplementalMonitor.h"

void supplementalMonitor(){

    adc_status_t senseInit = adc_sense_init();
    if(senseInit != ADC_OK){
        // do some error flag or some shit
    }

    // TODO: start a timer that needs to get reset after a certain amount of time or else we explode

    while(1){
        // statusLeds_toggle(LSOM_HEARTBEAT_LED);

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