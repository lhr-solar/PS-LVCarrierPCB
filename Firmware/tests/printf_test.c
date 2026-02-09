#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "UART.h"
#include "printf.h"

StaticTask_t txTaskBuffer;
StackType_t txTaskStack[configMINIMAL_STACK_SIZE];

int main(){
    HAL_Init();
    SystemClock_Config();

    // xTaskCreateStatic(TxTask, 
    //                  "TX",
    //                  configMINIMAL_STACK_SIZE,
    //                  NULL,
    //                  tskIDLE_PRIORITY + 2,
    //                  txTaskStack,
    //                  &txTaskBuffer);

    vTaskStartScheduler();

    // should never get here
    while (1) {

    }
}