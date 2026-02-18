#include "supplementalMonitor.h"

void supplementalMonitor(){

    while(1){
        // statusLeds_toggle(LSOM_HEARTBEAT_LED);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
}