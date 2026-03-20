#include "stm32xx_hal.h"
#include "bq25756e.h"
#include "statusLeds.h"
#include "commandLine.h"
#include "ltc4421.h"
#include "pinDefs.h"
#include "faultBits.h"

#define MS_DELAY_100 pdMS_TO_TICKS(100) 
#define MS_DELAY_500 pdMS_TO_TICKS(500) 

// Run test w/ arbitrary fault after ~10s
#define FAULT_TEST

/* BQ Driver Setup */
// User's BQ Handle
static BQ_HandleTypeDef bq_handle;

// I2C Handle
I2C_HandleTypeDef hi2c;

// Task buffers
StaticTask_t bqTaskBuffer;
StackType_t bqTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t faultTaskBuffer;
StackType_t faultTaskStack[configMINIMAL_STACK_SIZE];

void BqTask(void *argument){
    faultBits_init();
    // give chip a bit to power on
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    bq25756e_preReqBit_wait(BQ25756E_PREREQ_LTC_VALID, portMAX_DELAY);

    statusLeds_toggle(LSOM_HEARTBEAT_LED);

    bq25756e_charge_status_t charge_state=BQ25756E_NOT_CHRG;
    bq25756e_charge(portMAX_DELAY, BQ25756E_1A);

    while (1) {
        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        #ifdef FAULT_TEST
        // Only run fault test
        if (faultBit_wait(FAULT_SUPPREG_UNDERVOLTAGE, pdMS_TO_TICKS(200)) != pdFALSE) {
            bq25756e_charge(portMAX_DELAY, BQ25756E_1A);
            vTaskDelete(NULL);
        }  
        #endif

        bq25756e_charge(portMAX_DELAY, BQ25756E_1A);

        // // Dump status and continue
        bq25756e_dump_status(&charge_state, BQ25756E_SERIAL_ENABLE, portMAX_DELAY); 
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
    bq25756e_init(&bq_handle, &hi2c);
    command_line_init();
    
    // Start in disabled state to be safeeeee
    bq25756e_write_ce(BQ25756E_LOGIC_LOW);

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