#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "UART.h"
#include "statusLeds.h"
#include "commandLine.h"

StaticTask_t txTaskBuffer;
StackType_t txTaskStack[configMINIMAL_STACK_SIZE];

void TxTask(void *argument){
    command_line_init();

    while(1){
        printf("Hello World! %s %d\n\r", "Test String", 5);
        statusLeds_toggle(LSOM_HEARTBEAT_LED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(){
    HAL_Init();
    SystemClock_Config();
    statusLeds_init();

    xTaskCreateStatic(TxTask, 
                     "TX",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     txTaskStack,
                     &txTaskBuffer);

    vTaskStartScheduler();

    // should never get here
    while (1) {

    }
}
