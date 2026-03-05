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
    ltc4421_shdn_enable(ON);
    faultBits_init();
    
    printf("wha\n\r");
    statusLeds_toggle(LSOM_HEARTBEAT_LED);

    uint8_t buffer[1];
    bq25756e_status_t stat;

    // Pend on fault bitmap
    // faultBit_wait(MAX_FAULT_BITS, portMAX_DELAY);

    // uint8_t STAT;
    printf("yur!\n\r");
    stat=bq25756e_charge(BQ25756E_CHRG_START);
    if (stat != BQ25756E_OK) printf("Cooked:  %d \n\r", stat);

    while (1) {
        printf("in her3!\n\r");
        if (faultBit_wait(FAULT_SUPPREG_UNDERVOLTAGE, pdMS_TO_TICKS(200)) != pdFALSE) {
            bq25756e_write_ce(BQ25756E_LOGIC_HIGH);
            bq25756e_charge(BQ25756E_CHRG_STOP);
            vTaskDelete(NULL);
        }  

        // Dump status
        bq25756e_read_reg(REG_CHARGE_STATUS_1, buffer);
        printf("Charge status reg: %d\n\r", buffer[0]);

        printf("toggling!\n\r");
        statusLeds_toggle(LSOM_HEARTBEAT_LED);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void FaultTask(void *argument){
    // Trip fault after 10s
    vTaskDelay(pdMS_TO_TICKS(10000)); 
    // throw a random fault
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