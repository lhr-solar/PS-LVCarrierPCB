#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"
#include "lvEnable.h"
#include "statusLeds.h"
#include "faultState.h"
#include "tasksConfig.h"
#include "commandLine.h"


StaticTask_t runningThreadTaskBuffer;
StackType_t runningThreadTaskStack[configMINIMAL_STACK_SIZE];

uint8_t faultStateActivated = 0;

void runningThread(){

    static uint8_t threadCounts = 0;
    uint8_t threadCountsTillFault = 10;
    
    while(1){

        threadCounts++;
        
        // enter fault state after around 10 seconds
        if(threadCounts > threadCountsTillFault){
            // enter fault state
            printf("time to fault :D\n\r");
            set_faultBit(FAULT_SUPPBATT_OVERVOLTAGE);
        }
        
        if(faultStateActivated == 0){
            printf("No faults, we're straight chilling\n\r");
        }
        else{
            // this indiciates that we did not stay in fault state
            printf("Left fault and came back \n\r");
        }

        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void faultState_hook(EventBits_t pending){
    
    printf("zooweeemama, we're in fault state\r\n");
    statusLeds_toggle(HEARTBEAT_LED);
    faultStateActivated = 1;
}


int main(){

    HAL_Init();
    SystemClock_Config();

    faultStateActivated = 0;

    statusLeds_init();

    faultBits_init();
    command_line_init(); // only make calls to printf after the scheduler started

    xTaskCreateStatic(runningThread, 
                     "runningThread",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 1,
                     runningThreadTaskStack,
                     &runningThreadTaskBuffer);


    xTaskCreateStatic(
                    faultState,
                    "Fault State Task",
                    TASK_FAULT_STATE_STACK_SIZE,
                    (void*)NULL,
                    TASK_FAULT_STATE_PRIO,
                    Task_FaultState_Stack_Array, 
                    &Task_FaultState_Buffer
   );

    vTaskStartScheduler();

    while(1){
        // should never reach here
    }


}