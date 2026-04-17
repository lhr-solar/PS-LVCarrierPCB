#include "powerMuxMonitor.h"

StaticTask_t Task_PowerMux_Buffer;
StackType_t Task_PowerMux_Stack_Array[ TASK_POWER_MUX_MON_STACK_SIZE ];

void powerMuxMonitor(){
        
    // initialize pins for the power mux
    ltc4421_gpio_init();

    // initialize pins to read the state of the LV enable signals
    lvEnable_gpio_init();

    ltc4421_shdn_enable(OFF);

    // turn on shutdown after 1 second
    vTaskDelay(pdMS_TO_TICKS(1000));
    ltc4421_shdn_enable(ON);

    while(1){
        // statusLeds_toggle(LSOM_HEARTBEAT_LED);

        // char *lvEnableSupp = (get_lvEnable_supp() == ON) ? "ON" : "OFF"; 
        // printf("LV Enable Supp %s\n", lvEnableSupp);

        // char *lvEnablePS = (get_lvEnable_powerSupply() == ON) ? "ON" : "OFF"; 
        // printf("LV Enable Power Supply %s\n", lvEnablePS);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}