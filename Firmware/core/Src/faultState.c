#include "faultState.h"

StackType_t Task_FaultState_Stack_Array[ TASK_FAULT_STATE_STACK_SIZE ];
StaticTask_t Task_FaultState_Buffer;

void runFaultState(EventBits_t pending);

void faultState(){

    /*
    fault bits should be initialized before running this thread
    this thread should be the highest priority thread in the RTOS to preempt other threads.
    */

    while (1)
    {
        // wait forever for a fault to be set
        EventBits_t pending = faultBit_wait(NUM_FAULTS, portMAX_DELAY);

        faultState_hook(pending);

        if(pending != 0){
            // never return
            runFaultState(pending);
        }
    }
}

void runFaultState(EventBits_t pending){    
    while(1){
        // do some shit
    }
}

__weak void faultState_hook(EventBits_t pending){
    UNUSED(pending);
}