#include "suppCharger.h"

StackType_t Task_SuppCharger_Stack_Array[ TASK_SUPP_CHARGING_STACK_SIZE ];
StaticTask_t Task_SuppCharger_Buffer;

// User's BQ Handle
BQ_HandleTypeDef bq_handle;

// I2C Handle
I2C_HandleTypeDef hi2c;

#define MAX_BQ25756E_DELAY_TICKS pdMS_TO_TICKS(100)

#define BQ25756E_PRINT_DELAY pdMS_TO_TICKS(2000)

void suppCharger(){

    bq25756e_init(&bq_handle, &hi2c);

    bq25756e_charge_status_t charge_state = BQ25756E_NOT_STARTED;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t xLastPrintTime = xTaskGetTickCount();

     // give the bq25756e chip time to startup
    vTaskDelay(pdMS_TO_TICKS(1000));

    int16_t charge_current;

    while(1){

        uint8_t suppChargerEnabled = 0;

        // check if we're able to charge
        EventBits_t bitsSet = bq25756e_preReqBit_wait(BQ25756E_NUM_PREREQS, 0);

        if(bitsSet == BQ25756E_ALL_PREREQ_BITS){
            // set 400mA charge current
            // supp vicor quiescent current is about 400mA
            bq25756e_charge(MAX_BQ25756E_DELAY_TICKS, 400);

            suppChargerEnabled = 1;
        }
        else{
            bq25756e_charge(MAX_BQ25756E_DELAY_TICKS, 0);
            suppChargerEnabled = 0;
        }

        bq25756e_serial_config_t printEnabled = BQ25756E_SERIAL_DISABLE;
        if(xLastPrintTime + BQ25756E_PRINT_DELAY <= xTaskGetTickCount()){
            xLastPrintTime = xTaskGetTickCount();
            printEnabled = BQ25756E_SERIAL_ENABLE;
        }

        
        bq25756e_dump_status(&charge_state, printEnabled, MAX_BQ25756E_DELAY_TICKS); 
        bq25756e_dump_charge_current(&charge_current, printEnabled, MAX_BQ25756E_DELAY_TICKS); 

        bq25756e_pet_wdg(MAX_BQ25756E_DELAY_TICKS);

        vTaskDelayUntil(&xLastWakeTime, SUPPLEMENTAL_CHARGER_THREAD_DELAY_TICKS);
    }
}