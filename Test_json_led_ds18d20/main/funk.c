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
#include "mqtt_client.h"
#include "esp_wifi.h"

#define SSID "EE"
#define PASS "EE@05kilogram"
#define TAG "test32"
#define BLINK_GPIO1 2
#define TEMP_BUS 15
#define LOGGING_ENABLED 1 

static const char *topic_mqtt_data = "test_esp32/";
float temperature = 0;
int msg_id;
TaskHandle_t xPublishTask;
TaskHandle_t xSensorTask;
TaskHandle_t xLedTask;

QueueHandle_t xSensor_Control = 0;
QueueHandle_t xLed_Control = 0;

char *my_json_string = NULL;
char *pr_led = NULL;

esp_mqtt_client_handle_t client;
cJSON *root;

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_mosquitto_org_crt_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_mosquitto_org_crt_end");

static void blinky1(void *pvParameter){
    gpio_pad_select_gpio(BLINK_GPIO1);
    gpio_set_direction(BLINK_GPIO1, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_set_level(BLINK_GPIO1, 0);
        vTaskDelay(1500 / portTICK_RATE_MS);
        gpio_set_level(BLINK_GPIO1, 1);
        vTaskDelay(1500 / portTICK_RATE_MS);
    }
}

static void ds18b20_start(void *pvParameter){
    if (ds18b20_init(TEMP_BUS) == true) {
  		printf("Init\n");
	} else {
  	    printf("Failed!\n");
	}
    while (1){
        if (ds18b20_get_temperature(&temperature, NULL) == true) {
            printf("Temp:%0.1f\n", temperature);
            root = cJSON_CreateObject();
            cJSON_AddNumberToObject(root, "Temp:", temperature);
            
            my_json_string = cJSON_Print(root);
            ESP_LOGW(TAG, "\n%s",my_json_string);
            cJSON_Delete(root);
            vTaskDelay(pdMS_TO_TICKS(2000));
        } else {
            printf("Not temp!\n");
        }
        // vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGW(TAG, "MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGW(TAG, "MQTT_EVENT_PUBLISHED");
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        break;
    }
}

static void mqtt_app_start(void){
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtts://a25h60s1inklac-ats.iot.us-east-1.amazonaws.com:8883",
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
        .cert_pem = (const char *)server_cert_pem_start,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

// static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *even_data){
//     switch (event_id){
//         case WIFI_EVENT_STA_START:
//             printf("Wifi connecting...\n");
//             break;
//         case WIFI_EVENT_STA_CONNECTED:
//             printf("Wifi connected...\n");
//             break;
//         case WIFI_EVENT_STA_DISCONNECTED:
//             printf("Wifi lost connection...\n");
//             break;
//         case IP_EVENT_STA_GOT_IP:
//             printf("Wifi got IP...\n\n");
//             break;
//         default:
//             break;
//     }
// }

static void connect_wifi(void){
    tcpip_adapter_init();
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t sta_wifi_config = {
        .sta = {
            .ssid = SSID, 
            .password = PASS
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_wifi_config));
    
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
    ESP_LOGI(TAG, "Connect to WiFi.");
}

static void publish_mqtt(void *pvParameter){
    while (1){
        ESP_LOGW(TAG, "Sending data to topic %s...", topic_mqtt_data);
        if(xQueueReceive(xSensor_Control, &my_json_string, 6000)){
            ESP_LOGW(TAG, "Error send dates sensor.\n");
        }
        esp_mqtt_client_publish(client, topic_mqtt_data, my_json_string, 0, 0, 0);
        free(my_json_string);
    }
//    vTaskDelay(5000 / portTICK_RATE_MS);
vTaskDelay(pdMS_TO_TICKS(2000));
}