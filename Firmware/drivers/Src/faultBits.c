#include "faultBits.h"

// Event group handle to store fault state bits
EventGroupHandle_t faultStateBits;

// Static buffer to store the event handle
StaticEventGroup_t faultStateBitsBuffer;

uint8_t faultBits_init(void){
    faultStateBits = xEventGroupCreateStatic( &faultStateBitsBuffer );
    if(faultStateBits == NULL){
        return 0;
    }
    return 1;
}

void set_faultBit(fault_bit_t bit){
    // not a valid fault
    if(bit >= NUM_FAULTS){ 
        return;
    }

    // chat we're cooked
    xEventGroupSetBits(faultStateBits, FAULT_BIT(bit));
    // should never return from here
    taskYIELD();
}

void set_faultBitFromISR(fault_bit_t bit){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(bit >= NUM_FAULTS){
        return;
    }

    xEventGroupSetBitsFromISR(
        faultStateBits,
        FAULT_BIT(bit),
        &xHigherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

EventBits_t faultBit_wait(fault_bit_t bit, TickType_t xTicksToWait){

    if(bit > NUM_FAULTS){
        return 0;
    }
    
    EventBits_t uxBitsToWaitFor;

    if(bit == NUM_FAULTS){
        uxBitsToWaitFor = ALL_FAULT_BITS;
    }
    else{
        uxBitsToWaitFor = (FAULT_BIT(bit));
    }
    EventBits_t pending = xEventGroupWaitBits(
        faultStateBits,
        uxBitsToWaitFor,  // wait for any defined fault
        pdFALSE,          // fault bits are not reset
        pdFALSE,          // wait for ANY bit to be set
        xTicksToWait 
    );
    return pending;
}