#include <stdio.h>
#include "ds18b20.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdlib.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>

#define TEMP_BUS 15
#define LOGGING_ENABLED     1 // logging enabled

void app_main(void){
	if (ds18b20_init(TEMP_BUS) == true) {
  		printf("Init\n");
		ESP_LOGW("Main", "Init");
	} else {
  		printf( "Failed!\n");
		ESP_LOGE("Main", "Failed");
	}
	while (1){
		float temperature;
		if (ds18b20_get_temperature(&temperature, NULL) == true) {
			ESP_LOGW("Main", "Temperature: %0.1f", temperature);
		} else {
			ESP_LOGW("Main", "Error reading temperature!");
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
