#include "addressableLED.h"
#include "statusLeds.h"

StaticTask_t gamerLEDTaskBuffer;
StackType_t gamerLEDTaskStack[configMINIMAL_STACK_SIZE];

#define LED_SEQUENCE_SPEED 250
void led_sequence(){
    ws2812b_set_all_leds(&wsHandle, WS2812B_SOLID_OFF, portMAX_DELAY);

    ws2812b_color_t colors[] = {
        WS2812B_SOLID_RED,
        WS2812B_SOLID_BLUE,
        WS2812B_SOLID_GREEN,
        WS2812B_SOLID_PURPLE,
        WS2812B_SOLID_YELLOW,
        WS2812B_SOLID_BURNT_ORANGE
    };

    // dim brightness
    for(int i=0; i<sizeof(colors)/sizeof(ws2812b_color_t); i++){
        colors[i].blue = colors[i].blue / 6;
        colors[i].red = colors[i].red / 6;
        colors[i].green = colors[i].green / 6;
    }

    for(int i=0; i<MAX_RGB_LEDS; i++){
        ws2812b_set_color(&wsHandle, i, colors[i % (sizeof(colors)/sizeof(ws2812b_color_t))], portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(LED_SEQUENCE_SPEED));
        ws2812b_set_color(&wsHandle, i, WS2812B_SOLID_OFF, portMAX_DELAY);
    }

    for(int j=0; j<2; j++){
        for(int i=0; i<MAX_RGB_LEDS; i++){
            ws2812b_set_color(&wsHandle, i, colors[i % (sizeof(colors)/sizeof(ws2812b_color_t))], portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(LED_SEQUENCE_SPEED/2));
        ws2812b_set_all_leds(&wsHandle, WS2812B_SOLID_OFF, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(LED_SEQUENCE_SPEED/2));
    }
}

void gamerLED(void *argument){

    addressableLED_init();

    while(1){
        // led_sequence();
        ws2812b_set_all_leds(&wsHandle,WS2812B_SOLID_OFF, portMAX_DELAY);
        statusLeds_toggle(LSOM_HEARTBEAT_LED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(){
    HAL_Init();
    SystemClock_Config();
    statusLeds_init();

    xTaskCreateStatic(gamerLED, 
                     "gamer LED",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     gamerLEDTaskStack,
                     &gamerLEDTaskBuffer);

    vTaskStartScheduler();

    // should never get here
    while (1) {

    }
}
