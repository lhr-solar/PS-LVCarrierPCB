#include "stm32xx_hal.h"
#include "bq25756e.h"
#include "statusLeds.h"
#include "commandLine.h"
#include "ltc4421.h"
#include "pinDefs.h"


StaticTask_t bqTaskBuffer;
StackType_t bqTaskStack[configMINIMAL_STACK_SIZE];

void BqTask(void *argument){
    command_line_init();

    bq25756e_write_ce(LOW);
    vTaskDelay(pdMS_TO_TICKS(50));
    bq25756e_write_ce(HIGH);
    ltc4421_shdn_enable(ON);

    while(1){
        printf("BALLS \r\n");

        bq25756e_write_ce(HIGH);
        bq25756e_charge(START);

        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        vTaskDelay(pdMS_TO_TICKS(500));

    }
}

int main()
{
    HAL_Init();
    SystemClock_Config();
    statusLeds_init();
    bq25756e_init();
    // command_line_init();
    bq25756e_write_ce(LOW);
    // HAL_Delay(50);
    bq25756e_write_ce(HIGH);
    ltc4421_gpio_init();

    ltc4421_shdn_enable(OFF);

    xTaskCreateStatic(BqTask, 
                     "BQ test",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     bqTaskStack,
                     &bqTaskBuffer);

    vTaskStartScheduler();
    
    while (1)
    {
        bq25756e_write_ce(HIGH);
        bq25756e_charge(START);

        HAL_GPIO_TogglePin(LSOM_HEARTBEAT_LED_PORT, LSOM_HEARTBEAT_LED_PIN);
        HAL_Delay(500);

        // printf("wsp\n");
    }
}