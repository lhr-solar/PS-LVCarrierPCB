#include "bq25756e_preReqBits.h"

EventGroupHandle_t BQ25756E_preReqBits;
StaticEventGroup_t BQ25756E_preReqBitsBuffer;

uint8_t bq25756e_preReqBits_init(void){
    BQ25756E_preReqBits = xEventGroupCreateStatic( &BQ25756E_preReqBitsBuffer );
    if(BQ25756E_preReqBits == NULL){
        return pdFAIL;
    }
    return pdPASS;
}

void bq25756e_set_preReqBit(bq25756e_prereqs_t bit){
    // not a valid fault
    if(bit >= BQ25756E_NUM_PREREQS){ 
        return;
    }

    // lakshays cooked
    xEventGroupSetBits(BQ25756E_preReqBits, BQ25756E_PREREQ(bit) );

    taskYIELD(); 
}

EventBits_t bq25756e_preReqBit_wait(bq25756e_prereqs_t bit, TickType_t xTicksToWait){

    // NUM_PREREQS indicates you want to wait for all bits
    if(bit > BQ25756E_NUM_PREREQS){
        return 0;
    }

    // if NUM
    EventBits_t uxBitsToWaitFor = bit == BQ25756E_NUM_PREREQS ? BQ25756E_ALL_PREREQ_BITS : BQ25756E_PREREQ(bit);

    EventBits_t pending = xEventGroupWaitBits(
        BQ25756E_preReqBits,
        uxBitsToWaitFor,  // wait for any defined fault
        pdFALSE,          // fault bits are not reset
        pdFALSE,          // wait for ANY bit to be set
        xTicksToWait 
    );
    return pending;
}