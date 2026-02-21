#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "statusLeds.h"
#include "pinDefs.h"
#include "commandLine.h"
#include "tasksConfig.h"
 

void initThread(){

    command_line_init();

    statusLeds_toggle(LSOM_HEARTBEAT_LED);

     xTaskCreateStatic(
                    powerMuxMonitor,
                    "Power Mux Monitor Task",
                    TASK_POWER_MUX_MON_STACK_SIZE,
                    (void*)NULL,
                    TASK_POWER_MUX_MON_PRIO,
                    Task_PowerMux_Stack_Array, 
                    &Task_PowerMux_Buffer
   );

    xTaskCreateStatic(
                    supplementalMonitor,
                    "Supplemental Monitor Task",
                    TASK_SUPP_MON_STACK_SIZE,
                    (void*)NULL,
                    TASK_SUPP_MON_STACK_SIZE,
                    Task_SuppMon_Stack_Array, 
                    &Task_SuppMon_Buffer
   );

   vTaskDelete(NULL);

   while(1){

   }
}