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
    
    while(1){
        if(faultStateActivated == 0){
            printf("No faults, we're straight chilling\n\r");
        }
        else{
            // this indiciates that we did not stay in fault state
            printf("Left fault and came back \n\r");
        }
    }
}

void faultState_hook(EventBits_t pending){
    
    printf("zooweeemama, we're in fault state\r\n");
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
                     tskIDLE_PRIORITY + 2,
                     runningThreadTaskStack,
                     &runningThreadTaskBuffer);

    vTaskStartScheduler();

    while(1){
        // should never reach here
    }


}