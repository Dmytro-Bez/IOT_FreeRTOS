#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#define BLINK_GPIO1 2
#define BLINK_GPIO2 15

void blinky1(void *pvParameter){
    gpio_pad_select_gpio(BLINK_GPIO1);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO1, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO1, 0);
        printf("Led1 LOW\n");
        vTaskDelay(500 / portTICK_RATE_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO1, 1);
        printf("Led1 HIGH\n");
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

void blinky2(void *pvParameter){
    gpio_pad_select_gpio(BLINK_GPIO2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO2, 0);
        printf("Led2 LOW\n");
        vTaskDelay(100 / portTICK_RATE_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO2, 1);
        printf("Led2 HIGH\n");
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void app_main()
{
    // xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
    xTaskCreate(&blinky1, "blinky1", 2048,NULL,5,NULL);
    xTaskCreate(&blinky2, "blinky2", 1024,NULL,5,NULL );
}
