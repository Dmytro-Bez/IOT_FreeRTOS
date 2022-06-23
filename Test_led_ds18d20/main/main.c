/*================================================Libr. includes================================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "ds18b20.h"
/*===================================================Defines=================================================*/
#define SSID "EE"
#define PASS "EE@05kilogram"
#define TAG "test_esp32"
#define BLINK_GPIO1 13
#define BLINK_GPIO2 15
#define TEMP_BUS 4
#define LOGGING_ENABLED 1 

static void init_s_ds18d20(void){                                   //Funk. init water sensor
    if (ds18b20_init(TEMP_BUS) == true) {
  		printf("Init\n");
		ESP_LOGW("Main", "Init");
	} else {
  		printf( "Failed!\n");
		ESP_LOGE("Main", "Failed");
	}
}

void blinky1(void *pvParameter){                                    //Funk. init and work first led
    gpio_pad_select_gpio(BLINK_GPIO1);
    gpio_set_direction(BLINK_GPIO1, GPIO_MODE_OUTPUT);              //Set the GPIO as a push/pull output
    while(1) {
        gpio_set_level(BLINK_GPIO1, 0);                             //Blink off (output low)
        // printf("Led1 LOW\n");
        vTaskDelay(2500 / portTICK_RATE_MS);                         //Blink on (output high)
        gpio_set_level(BLINK_GPIO1, 1);
        // printf("Led1 HIGH\n");
        vTaskDelay(2500 / portTICK_RATE_MS);
    }
}

void blinky2(void *pvParameter){                                    //Funk. init and work second led
    gpio_pad_select_gpio(BLINK_GPIO2);
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_set_level(BLINK_GPIO2, 0);
        // printf("Led2 LOW\n");
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_set_level(BLINK_GPIO2, 1);
        // printf("Led2 HIGH\n");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main(){
    init_s_ds18d20();

    // xTaskCreate(&blinky1, "blinky1", 2048,NULL,5,NULL);
    xTaskCreate(&blinky2, "blinky2", 1024,NULL,5,NULL );

	while (1){
		float temperature;
		if (ds18b20_get_temperature(&temperature, NULL) == true) {
			ESP_LOGW("Main", "Temperature: %0.1f", temperature);
            xTaskCreate(&blinky1, "blinky1", 2048,NULL,5,NULL);
		} else {
			ESP_LOGW("Main", "Error reading temperature!");
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}