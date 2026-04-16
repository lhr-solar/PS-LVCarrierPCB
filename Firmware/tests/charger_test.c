#include "stm32xx_hal.h"
#include "bq25756e.h"
#include "statusLeds.h"
#include "commandLine.h"
#include "ltc4421.h"
#include "pinDefs.h"
#include "faultBits.h"
#include "suppCharger.h"
#include "supplementalMonitor.h"


#define MS_DELAY_100 pdMS_TO_TICKS(100) 
#define MS_DELAY_500 pdMS_TO_TICKS(500) 


// Task buffers
StaticTask_t bqTaskBuffer;
StackType_t bqTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t faultTaskBuffer;
StackType_t faultTaskStack[configMINIMAL_STACK_SIZE];

void BqTask(void *argument){
    faultBits_init();

    // give chip a bit to power on
    vTaskDelay(pdMS_TO_TICKS(5000));

    ltc4421_shdn_enable(ON);
    
    statusLeds_toggle(LSOM_HEARTBEAT_LED);

    bq25756e_charge_status_t charge_state = BQ25756E_NOT_STARTED;
    bq25756e_charge(portMAX_DELAY, 2000);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    adc_status_t readStat;

    uint32_t failedCount = 0;

    while (1) {
        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        // Dump status and continue
        bq25756e_dump_status(&charge_state, BQ25756E_SERIAL_ENABLE, portMAX_DELAY); 
        bq25756e_pet_wdg(portMAX_DELAY);

        readStat = adc_start_read(SUPPLEMENTAL_BATTERY_VOLTAGE, pdMS_TO_TICKS(100));
        if(readStat == ADC_OK){
            uint32_t result;
            BaseType_t returnStatus = adc_read_value(SUPPLEMENTAL_BATTERY_VOLTAGE, &result, portMAX_DELAY);
            if(returnStatus == pdTRUE){
                printf("Supp battery voltage: %ld counts \n\r", result);
                printf("Supp battery voltage: %ld mV \n\r", adc_to_SuppVoltage(result));
            }
            failedCount = 0;
        }
        else{
            printf("ADC read failed: %d\n\r", failedCount);
            failedCount++;
        }

        vTaskDelayUntil(&xLastWakeTime , pdMS_TO_TICKS(1000));
    }
}   

int main()
{
    HAL_Init();
    SystemClock_Config();

    statusLeds_init();
    bq25756e_init(&bq_handle, &hi2c);
    command_line_init();

    ltc4421_shdn_enable(OFF);
    ltc4421_gpio_init();

    adc_sense_init();

    xTaskCreateStatic(BqTask, 
                     "BQ test",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     bqTaskStack,
                     &bqTaskBuffer);


    // xTaskCreateStatic(FaultTask, 
    //                  "Fault test",
    //                  configMINIMAL_STACK_SIZE,
    //                  NULL,
    //                  tskIDLE_PRIORITY + 2,
    //                  faultTaskStack,
    //                  &faultTaskBuffer);

    vTaskStartScheduler();
    
    while (1)
    {
        // should never be here
    }
}