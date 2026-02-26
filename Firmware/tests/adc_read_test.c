#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "adc_sense.h"
#include "statusLeds.h"
#include "commandLine.h"

StaticTask_t adcTaskBuffer;
StackType_t adcTaskStack[configMINIMAL_STACK_SIZE];

void adcTask(void *argument){

    command_line_init();

    while(1){
        printf("woop woop\n\r");

        adc_status_t readStat = adc_start_read(SUPPLEMENTAL_BATTERY_VOLTAGE);
        // if(readStat == ADC_OK){
        //     printf("Succesfully started read\n\r");
        // }
        statusLeds_toggle(LSOM_HEARTBEAT_LED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(){

    HAL_Init();
    SystemClock_Config();
    statusLeds_init();

    adc_status_t adcInitStat = adc_sense_init();
    if(adcInitStat == ADC_OK){
        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        xTaskCreateStatic(adcTask, 
                     "adcTask",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     adcTaskStack,
                     &adcTaskBuffer);

        vTaskStartScheduler();
    }

    // should never get here
    while (1) {

    }

    return 0;
}
