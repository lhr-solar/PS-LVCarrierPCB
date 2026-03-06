#include "stm32xx_hal.h"
#include "bq25756e.h"
#include "statusLeds.h"
#include "commandLine.h"
#include "ltc4421.h"
#include "pinDefs.h"
#include "faultBits.h"

/* Define Charge Limit */  
#define CHG_LIM_1   // 1 amp

StaticTask_t bqTaskBuffer;
StackType_t bqTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t faultTaskBuffer;
StackType_t faultTaskStack[configMINIMAL_STACK_SIZE];

void BqTask(void *argument){
    // ltc4421_shdn_enable(ON);
    // faultBits_init();

    // faultBit_wait(BQ25756E_PREREQ_LTC_VALID, portMAX_DELAY);
    // faultBit_wait(BQ25756E_PREREQ_SUPP_VALID, portMAX_DELAY);

    bq25756e_charge(BQ25756E_CHRG_START);

    while (1) {
        // todo: check for more faults
        // if (faultBit_wait(FAULT_SUPPREG_UNDERVOLTAGE, pdMS_TO_TICKS(portMAX_DELAY)) != pdFALSE) {
        //     bq25756e_charge(BQ25756E_CHRG_STOP);
        //     vTaskDelete(NULL);
        // }  

        // // Dump status and continue
        // bq25756e_charge(BQ25756E_CHRG_DUMP);
        // statusLeds_toggle(LSOM_HEARTBEAT_LED);
        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void FaultTask(void *argument){
    // Signal LTC and supp good
    // xEventGroupSetBits(BQ25756E_preReqBits, FAULT_BIT(BQ25756E_PREREQ_LTC_VALID)); 
    // xEventGroupSetBits(BQ25756E_preReqBits, FAULT_BIT(BQ25756E_PREREQ_SUPP_VALID));

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
    bq25756e_init();
    command_line_init();
    ltc4421_gpio_init();

    bq25756e_write_ce(BQ25756E_LOGIC_HIGH);
    ltc4421_shdn_enable(OFF);

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