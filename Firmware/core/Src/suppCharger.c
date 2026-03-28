#include "suppCharger.h"

StackType_t Task_SuppCharger_Stack_Array[ TASK_SUPP_CHARGING_STACK_SIZE ];
StaticTask_t Task_SuppCharger_Buffer;

// User's BQ Handle
static BQ_HandleTypeDef bq_handle;

// I2C Handle
I2C_HandleTypeDef hi2c;

void suppCharger(){

    bq25756e_init(&bq_handle, &hi2c);

    bq25756e_charge_status_t charge_state = BQ25756E_NOT_STARTED;

     // give chip a bit to power on
    vTaskDelay(pdMS_TO_TICKS(1000));


    while(1){

        // todo, make all of these not port max delay
        bq25756e_preReqBit_wait(BQ25756E_NUM_PREREQS, portMAX_DELAY);

        // start charging at 2A
        bq25756e_charge(portMAX_DELAY, 2000);

        bq25756e_dump_status(&charge_state, BQ25756E_SERIAL_ENABLE, portMAX_DELAY); 
        bq25756e_pet_wdg(portMAX_DELAY);

        vTaskDelay(SUPPLEMENTAL_CHARGER_THREAD_DELAY_TICKS);
    }
}