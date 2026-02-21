#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "statusLeds.h"
#include "pinDefs.h"
#include "ltc4421.h"
#include "lvEnable.h"
#include "tasksConfig.h"

StaticTask_t initTaskBuffer;
StackType_t initTaskStack[TASK_INIT_STACK_SIZE];

int main(){
    HAL_Init(); // reset all peripherals

    SystemClock_Config(); // configure clock
    
    statusLeds_init();

     xTaskCreateStatic(
                    initThread,
                    "Init Task",
                    TASK_INIT_STACK_SIZE,
                    (void*)NULL,
                    TASK_INIT_PRIO,
                    initTaskStack, 
                    &initTaskBuffer
   );

    // Start the scheduler
    vTaskStartScheduler();

    // should never reach here

    while(1){
        
    }

}