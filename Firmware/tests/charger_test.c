#include "stm32xx_hal.h"
#include "bq25756e.h"
#include "statusLeds.h"
#include "commandLine.h"
#include "ltc4421.h"
#include "pinDefs.h"
#include "faultBits.h"
#include "suppCharger.h"


#define MS_DELAY_100 pdMS_TO_TICKS(100) 
#define MS_DELAY_500 pdMS_TO_TICKS(500) 

// Task buffers
StaticTask_t bqTaskBuffer;
StackType_t bqTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t faultTaskBuffer;
StackType_t faultTaskStack[configMINIMAL_STACK_SIZE];

void BqTask(void *argument){
    faultBits_init();
    int16_t  charge_current;
    uint16_t charge_limit = 450;
    uint8_t  wdg = 0;

    // give chip a bit to power on
    vTaskDelay(pdMS_TO_TICKS(5000));

    statusLeds_toggle(LSOM_HEARTBEAT_LED);


    bq25756e_charge_status_t charge_state = BQ25756E_NOT_STARTED;
    bq25756e_charge(portMAX_DELAY, charge_limit);

    while (1) {
        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        // Dump status and continue
        printf("============= \n\r");

        bq25756e_dump_status(&charge_state, BQ25756E_SERIAL_ENABLE, portMAX_DELAY); 
        bq25756e_dump_charge_current(&charge_current, BQ25756E_SERIAL_ENABLE, portMAX_DELAY); 
        bq25756e_dump_wdg(&wdg, BQ25756E_SERIAL_ENABLE, portMAX_DELAY); 

        printf("============= \n\r");
        
        bq25756e_pet_wdg(portMAX_DELAY);


        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}   


void FaultTask(void *argument){
    // Signal LTC and supp good
    vTaskDelay(pdMS_TO_TICKS(1000)); 
    bq25756e_set_preReqBit(BQ25756E_PREREQ_LTC_VALID);

    // Trip fault after 10s
    vTaskDelay(pdMS_TO_TICKS(10000)); 
    // Throw a random fault
    while (1) {  
        vTaskDelay(pdMS_TO_TICKS(500));
        set_faultBit(FAULT_SUPPREG_UNDERVOLTAGE);      
    }
}

int main()
{
    HAL_Init();
    SystemClock_Config();

    statusLeds_init();

    ltc4421_gpio_init();
    ltc4421_shdn_enable(OFF);

    HAL_Delay(100);
    
    ltc4421_shdn_enable(ON);

    bq25756e_init(&bq_handle, &hi2c);
    command_line_init();

    xTaskCreateStatic(BqTask, 
                     "BQ test",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     bqTaskStack,
                     &bqTaskBuffer);


    xTaskCreateStatic(FaultTask, 
                     "Fault test",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     faultTaskStack,
                     &faultTaskBuffer);

    vTaskStartScheduler();
    
    while (1)
    {
        // should never be here
    }
}