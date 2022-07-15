#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "cJSON.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "ds18b20.h"

#define SSID "EE"
#define PASS "EE@05kilogram"
#define TAG "test32"
#define BLINK_GPIO1 2
#define TEMP_BUS 4
#define LOGGING_ENABLED 1 

void blinky1(void *pvParameter){
    gpio_pad_select_gpio(BLINK_GPIO1);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO1, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO1, 0);
        // printf("Led1 LOW\n");
        
        vTaskDelay(500 / portTICK_RATE_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO1, 1);
        // printf("Led1 HIGH\n");
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}


 static void json_pars(){
    float temperature;

    if (ds18b20_init(TEMP_BUS) == true) {
  		printf("Init\n");
		ESP_LOGW("Main", "Init");
	} else {
  		printf( "Failed!\n");
		ESP_LOGE("Main", "Failed");
	}
	if (ds18b20_get_temperature(&temperature, NULL) == true) {
		// ESP_LOGW("Main", "Temperature: %0.1f", temperature);
        xTaskCreate(&blinky1, "blinky1", 2048,NULL,5,NULL);
	} else {
			ESP_LOGW("Main", "Error reading temperature!");
	}
    cJSON *root;
	root = cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "Temp:", temperature);
	
	char *my_json_string = cJSON_Print(root);
	ESP_LOGI(TAG, "my_json_string\n%s",my_json_string);
	cJSON_Delete(root);
 }
	

