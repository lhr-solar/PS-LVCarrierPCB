#include "bq25756e.h"
#include "commandLine.h"
#include "statusLeds.h"

int main()
{
    HAL_Init();
    SystemClock_Config();
    statusLeds_init();
    bq25756e_init();
    // command_line_init();
    bq25756e_write_ce(LOW);
    HAL_Delay(50);
    bq25756e_write_ce(HIGH);
    
    while (1)
    {
        bq25756e_write_ce(HIGH);
        bq25756e_charge(START);

        HAL_GPIO_TogglePin(LSOM_HEARTBEAT_LED_PORT, LSOM_HEARTBEAT_LED_PIN);
        HAL_Delay(500);

        // printf("wsp\n");
    }
}