#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "statusLeds.h"
#include "pinDefs.h"
#include "commandLine.h"
#include "tasksConfig.h"
#include "faultState.h"
 

void initThread(){

    command_line_init();

    statusLeds_toggle(LSOM_HEARTBEAT_LED);

    faultBits_init();

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
                    TASK_SUPP_MON_PRIO,
                    Task_SuppMon_Stack_Array, 
                    &Task_SuppMon_Buffer
   );

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
                    faultState,
                    "Fault State Task",
                    TASK_FAULT_STATE_STACK_SIZE,
                    (void*)NULL,
                    TASK_FAULT_STATE_PRIO,
                    Task_FaultState_Stack_Array, 
                    &Task_FaultState_Buffer
   );



   vTaskDelete(NULL);

   while(1){

   }
}