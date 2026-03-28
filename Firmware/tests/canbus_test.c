#include "canbus.h"
#include "stm32xx_hal.h"
#include "statusLeds.h"

StaticTask_t task_buffer;
StackType_t task_stack[512];

static void task(void *pvParameters){

    int test_id = 0x321;
    
    // send x1234 to 0x11
    uint8_t tx_data[8] = {0};
    tx_data[0] = 0x12;
    tx_data[1] = 0x34;
    tx_data[2] = 0x56;
    tx_data[3] = 0x78;
    tx_data[4] = 0x9A;
    tx_data[5] = 0xBC;
    tx_data[6] = 0xDE;
    tx_data[7] = 0xFF;
    
    while(1){

        if (canbus_send(test_id, tx_data, portMAX_DELAY) == CAN_ERR){
            Error_Handler();
        }
        else{
            statusLeds_toggle(LSOM_HEARTBEAT_LED);
        }

    }
}

void can_error_handler(){
    while(1){
        statusLeds_write(SUPPBAT_FAULT_LED, ON);
    }
}

int main(){
    HAL_Init();

    SystemClock_Config();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    statusLeds_init();

    if(canbus_init() != CAN_OK){
        can_error_handler();
    }

    xTaskCreateStatic(
                task,
                "task",
                512,
                NULL,
                tskIDLE_PRIORITY + 2,
                task_stack,
                &task_buffer);

    
    vTaskStartScheduler();

    while(1){

    }
}