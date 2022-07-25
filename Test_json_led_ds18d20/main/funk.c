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
float temperature = 0;

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_mosquitto_org_crt_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_mosquitto_org_crt_end");
int msg_id;

static void blinky1(void *pvParameter){
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

static bool ds18b20_start(void){
    if (ds18b20_init(TEMP_BUS) == true) {
  		printf("Init\n");
	} else {
  	    printf("Failed!\n");
	}
    // while (1){
        if (ds18b20_get_temperature(&temperature, NULL) == true) {
            printf("Temp:%0.1f\n", temperature);
        } else {
            printf("Not temp!\n");
        }
        // vTaskDelay(1500 / portTICK_RATE_MS);
    // }
    return 0;
}

static bool json_pars(void){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "Temp:", ds18b20_start());

	char *my_json_string = cJSON_Print(root);
	ESP_LOGI(TAG, "\n%s",my_json_string);
	cJSON_Delete(root);
    return 0;
 }

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "/higth/test_esp32/", 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    
        // msg_id = esp_mqtt_client_publish(client, "/higth/test_esp32/", json_pars(), 0, 0, 0); //Topic and sen date!!!
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
       
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
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

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *even_data){
    switch (event_id){
        case WIFI_EVENT_STA_START:
            printf("Wifi connecting...\n");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            printf("Wifi connected...\n");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            printf("Wifi lost connection...\n");
            break;
        case IP_EVENT_STA_GOT_IP:
            printf("Wifi got IP...\n\n");
            break;
        default:
            break;
    }
}

static void connect_wifi(void){
    esp_netif_init();                                                       //TCP-IP init
    esp_event_loop_create_default();                                        //Event loop
    esp_netif_create_default_wifi_sta();                                    //Wifi station
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);                                        //Config Phase wifi
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS
        }
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_start();                                                       //Wifi start phase
    esp_wifi_connect();                                                    //Wifi connect phase
}