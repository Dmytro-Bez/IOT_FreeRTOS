#include <stdlib.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_mqtt.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <string.h>
#include <string.h>
#include <ds18b20.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


#define TEMP_BUS 26                 // Temp Sensors are on GPIO26
#define LED1 2
#define LED2 15
#define HIGH 1
#define LOW 0
#define digitalWrite gpio_set_level

#define WIFI_SSID "EE"
#define WIFI_PASS "EE@05kilogram"
#define MQTT_HOST "a25h60s1inklac-ats.iot.us-east-1.amazonaws.com"
#define MQTT_PORT "1883"
#define MQTTS_PORT "8883"
#define MQTT_USER "public"
#define MQTT_PASS "public"
#define PUBLISH_INTERVAL 1000
#define RESTART_INTERVAL 20000

extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[] asm("_binary_server_root_cert_pem_end");

DeviceAddress tempSensors[2];

/*Creating a ds18b20 temperature sensor data read function*/
void getTempAddresses(DeviceAddress *tempSensorAddresses) {
	unsigned int numberFound = 0;
	reset_search();
	while (search(tempSensorAddresses[numberFound],true)) {     // search for 2 addresses on the oneWire protocol
		numberFound++;
		if (numberFound == 2) break;
	}
	while (numberFound != 2) {                                  // if 2 addresses aren't found then flash the LED rapidly
		numberFound = 0;
		digitalWrite(LED2, HIGH);
		vTaskDelay(100 / portTICK_PERIOD_MS);
		digitalWrite(LED2, LOW);
		vTaskDelay(100 / portTICK_PERIOD_MS);
		reset_search();                                         // search in the loop for the temp sensors as they may hook them up
		while (search(tempSensorAddresses[numberFound],true)) {
			numberFound++;
			if (numberFound == 2) break;
		}
	}
	return;
}

// static void status_callback_aws(esp_mqtt_status_t status) {
//     switch (status) {
//         case ESP_MQTT_STATUS_CONNECTED:
//             esp_mqtt_subscribe("/he", 2);  // subscribe
//             printf("Subscribe topic.\n");
//             break;
//         case ESP_MQTT_STATUS_DISCONNECTED:
//             printf("Disconnect topic.\n");
//         default:
//             break;
//   }
// }

// static void message_callback_aws(const char *topic, uint8_t *payload, size_t len) {
//     printf("Work message aws.\n\n");
//     ESP_LOGI("test", "incoming: %s => %s (%d)", topic, payload, (int)len);
//     printf("Work message aws.\n");
// }

// static void connect_mqtt() {
//     static bool use_tls = false;
//     use_tls = !use_tls;
//     ESP_LOGI("test", "starting mqtt with tls=%d", use_tls); // start mqtt
//     // esp_mqtt_tls(use_tls, true, server_root_cert_pem_start, server_root_cert_pem_end - server_root_cert_pem_start); //Creades key
//     esp_mqtt_start(MQTT_HOST, use_tls ? MQTTS_PORT : MQTT_PORT, "esp-mqtt", MQTT_USER, MQTT_PASS);
// }

// static void process_mqtt_publish(void *p) {
//     for (;;) {
//         esp_mqtt_publish("/hello", (uint8_t *)"world", 5, 2, false);    // publish every second
//         vTaskDelay(PUBLISH_INTERVAL / portTICK_PERIOD_MS);
//     }
// }

// static void start_work_net(void *_) {
//     connect_mqtt();    // initial start
//     for (;;) {
//         vTaskDelay(RESTART_INTERVAL / portTICK_PERIOD_MS);  // restart periodically
//         esp_mqtt_stop();
//         connect_mqtt();
//     }
// }

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT) {
      switch (event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());  // connect to ap
            printf("Connect to wifi.\n");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());  // reconnect Wi-Fi
            printf("Reconnect to wifi.\n");
            break;
        default:
            break;
    }
  }
}

void app_main(void){
    gpio_reset_pin(LED2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);

	ds18b20_init(TEMP_BUS);
	// getTempAddresses(tempSensors);
	// ds18b20_setResolution(tempSensors,2,10);

	// printf("Address 0: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", tempSensors[0][0],tempSensors[0][1],tempSensors[0][2],tempSensors[0][3],tempSensors[0][4],tempSensors[0][5],tempSensors[0][6],tempSensors[0][7]);
	// printf("Address 1: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", tempSensors[1][0],tempSensors[1][1],tempSensors[1][2],tempSensors[1][3],tempSensors[1][4],tempSensors[1][5],tempSensors[1][6],tempSensors[1][7]);

	// while (1) {
	// 	ds18b20_requestTemperatures();
	// 	float temp1 = ds18b20_getTempF((DeviceAddress *)tempSensors[0]);
	// 	float temp2 = ds18b20_getTempF((DeviceAddress *)tempSensors[1]);
	// 	float temp3 = ds18b20_getTempC((DeviceAddress *)tempSensors[0]);
	// 	float temp4 = ds18b20_getTempC((DeviceAddress *)tempSensors[1]);
	// 	printf("Temperatures: %0.1fF %0.1fF\n", temp1,temp2);
	// 	printf("Temperatures: %0.1fC %0.1fC\n", temp3,temp4);

	// 	float cTemp = ds18b20_get_temp();
	// 	printf("Temperature: %0.1fC\n", cTemp);
    //     // ESP_LOGW("Main", "Temperature: %0.1fC\n", cTemp);
	// 	vTaskDelay(1000 / portTICK_PERIOD_MS);
	// }

    ESP_ERROR_CHECK(nvs_flash_init());  // initialize NVS flash
    // printf("Initialize NVS flash.\n");
    ESP_LOGW("Main", "Initialize NVS flash."); 

    ESP_ERROR_CHECK(esp_netif_init());    // initialize networking
    // printf("Initialize networking.\n");
    ESP_LOGW("Main", "Initialize networking.");   
  
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // create default event loop
    // printf("Create default event loop.\n");
    ESP_LOGW("Main", "Create default event loop.");

    esp_netif_create_default_wifi_sta();  // enable Wi-Fi
    // printf("Enable Wi-Fi.\n");
    ESP_LOGW("Main", "Enable Wi-Fi.");
 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // initialize Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // printf("Initialize Wi-Fi.\n");
    ESP_LOGW("Main", "Initialize Wi-Fi.");

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));  // set Wi-Fi storage to ram
    // printf("Set Wi-Fi storage to ram.");
    ESP_LOGW("Main", "Set Wi-Fi storage to ram.");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));  // set wifi mode
    // printf("Set wifi mode.\n");
    ESP_LOGW("Main", "Set wifi mode.");

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));  // register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
    // printf("Register event handlers.\n");
    ESP_LOGW("Main", "Prepare Wi-Fi config.");

    wifi_config_t wifi_config = {.sta = {.ssid = WIFI_SSID, .password = WIFI_PASS}};  // prepare Wi-Fi config
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // printf("Prepare Wi-Fi config.\n");
    ESP_LOGW("Main", "Register event handlers.");

    ESP_ERROR_CHECK(esp_wifi_start());    // start Wi-Fi
    // printf("Start Wi-Fi.\n");
    ESP_LOGW("Main", "Start Wi-Fi.");

    // esp_mqtt_init(status_callback_aws, message_callback_aws, 256, 2000);   // initialize mqtt
    // printf("Initialize mqtt.\n");

    // xTaskCreatePinnedToCore(process_mqtt_publish, "process_mqtt_publish", 2048, NULL, 10, NULL, 1); // create tasks
    // xTaskCreatePinnedToCore(start_work_net, "start work net", 2048, NULL, 10, NULL, 1);
}
