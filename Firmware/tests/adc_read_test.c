#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "adc_sense.h"
#include "statusLeds.h"
#include "commandLine.h"
#include "supplementalMonitor.h"
#include "tasksConfig.h"

StaticTask_t adcTaskBuffer;
StackType_t adcTaskStack[configMINIMAL_STACK_SIZE];

void adc_print_status(adc_status_t readStat){
    printf("Status: ");
    switch (readStat){
        case ADC_OK:
            printf("ADC OK\n\r");
            break;
        case ADC_CHANNEL_CONFIG_FAIL:
            printf("ADC CHANNEL CONFIG FAIL\n\r");
            break;
        case ADC_INTERRUPT_BUSY:
            printf("ADC INTERRUPT BUSY\n\r");
            break;
        case ADC_INTERRUPT_ERROR:
            printf("ADC INTERRUPT ERROR\n\r");
            break;
        default:
            printf("OTHER \n\r");
            break;
    }
}

void adcTask(void *argument){

    command_line_init();

    adc_status_t readStat;

    while(1){
        /* TEST SUPP BATTERY VOLTAGE READING*/
        readStat = adc_start_read(SUPPLEMENTAL_BATTERY_VOLTAGE, portMAX_DELAY);
        if(readStat == ADC_OK){
            uint32_t result;
            BaseType_t returnStatus = adc_read_value(SUPPLEMENTAL_BATTERY_VOLTAGE, &result, portMAX_DELAY);
            if(returnStatus == pdTRUE){
                printf("Supp battery voltage: %ld counts \n\r", result);
                printf("Supp battery voltage: %ld mV \n\r", adc_to_SuppVoltage(result));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
         /* TEST SUPP BATTERY CURRENT READING*/
        readStat = adc_start_read(SUPPLEMENTAL_BATTERY_CURRENT, portMAX_DELAY);
        if(readStat == ADC_OK){
            uint32_t result;
            BaseType_t returnStatus = adc_read_value(SUPPLEMENTAL_BATTERY_CURRENT, &result, portMAX_DELAY);
            if(returnStatus == pdTRUE){
                printf("Supp battery current: %ld counts \n\r", result);
                printf("Supp battery current: %d mA \n\r", adc_To_Hall(result));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        /* TEST SUPP REG VOLTAGE READING*/
        readStat = adc_start_read(REGULATED_BATTERY_VOLTAGE, portMAX_DELAY);
        if(readStat == ADC_OK){
            uint32_t result;
            BaseType_t returnStatus = adc_read_value(REGULATED_BATTERY_VOLTAGE, &result, portMAX_DELAY);
            if(returnStatus == pdTRUE){
                printf("Supp regulated voltage: %ld counts \n\r", result);
                printf("Supp regulated voltage: %ld mV \n\r", adc_to_SuppVoltage(result));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        /* TEST SUPP REG CURRENT READING*/
        readStat = adc_start_read(REGULATED_BATTERY_CURRENT, portMAX_DELAY);
        if(readStat == ADC_OK){
            uint32_t result;
            BaseType_t returnStatus = adc_read_value(REGULATED_BATTERY_CURRENT, &result, portMAX_DELAY);
            if(returnStatus == pdTRUE){
                printf("Supp regulated current: %ld counts\n\r", result);
                printf("Supp regulated current: %d mA\n\r", adc_To_Hall(result));
            }
        }
        printf("=============================================\n\r");
        statusLeds_toggle(LSOM_HEARTBEAT_LED);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

int main(){

    HAL_Init();
    SystemClock_Config();
    statusLeds_init();

    adc_status_t adcInitStat = adc_sense_init();
    if(adcInitStat == ADC_OK){

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
