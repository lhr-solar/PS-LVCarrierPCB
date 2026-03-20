/* LV Carrier CAN Messages */
#include "stm32xx_hal.h"
#include "obbbCan.h"
#include "commandLine.h"



// send "LV_Carrier" status message on bus
int main() {
    HAL_Init();
    SystemClock_Config();
    statusLeds_init();
    command_line_init();


}