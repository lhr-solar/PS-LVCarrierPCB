#include "stm32xx_hal.h"
#include "statusLeds.h"
#include "ltc4421.h"
#include "pinDefs.h"
#include "common.h"
#include "commandLine.h"


StaticTask_t ltc4421TaskBuffer;
StackType_t ltc4421TaskStack[configMINIMAL_STACK_SIZE];

void ltc4421Task(void *argument){
    command_line_init(); // initializes printf

    while(1){
        printf("==================================================\n\r");
        ltc4421_channel_t selectedChannel = ltc4421_channel_selected();
        if(selectedChannel == CHANNEL1){
            printf("Channel 1 is selected\n\r");
        }
        else if(selectedChannel == CHANNEL2){
            printf("Channel 2 is selected\n\r");
        }
        else{
            printf("No channel is selected\n\r");
        }

        pin_state_t channelState = ltc4421_read_valid(CHANNEL1);
        if(channelState == ON){
            printf("Channel 1 is valid\n\r");
        }
        channelState = ltc4421_read_valid(CHANNEL2);
        if(channelState == ON){
            printf("Channel 2 is valid\n\r");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));

        pin_state_t channelFaultState = ltc4421_read_fault(CHANNEL1);
        if(channelFaultState == ON){
            printf("Channel 1 has fault\n\r");
        }
        channelFaultState = ltc4421_read_fault(CHANNEL2);
        if(channelFaultState == ON){
            printf("Channel 2 has fault\n\r");
        }
        printf("==================================================\n\r");

        statusLeds_toggle(LSOM_HEARTBEAT_LED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(){
    HAL_Init();
    SystemClock_Config();

    ltc4421_gpio_init();
    statusLeds_init();

    ltc4421_shdn_enable(OFF);
    HAL_Delay(5000);
    ltc4421_shdn_enable(ON);

    xTaskCreateStatic(ltc4421Task, 
                     "LTC4421Task",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     ltc4421TaskStack,
                     &ltc4421TaskBuffer);

    vTaskStartScheduler();

    // should never get here
    while (1) {

    }


}