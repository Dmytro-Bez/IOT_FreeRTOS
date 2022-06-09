#include <stdio.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "my_data.h"

#define SSID "EE"
#define PASS "EE@05kilogram"

extern const uint8_t cert_start[] asm("_binary_AmazonRootCA1_pem_start");
extern const uint8_t cert_end[] asm("_binary_AmazonRootCA1_pem_end");
extern const uint8_t certificate_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_end[] asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_start[] asm("_binary_private_pem_key_start");
extern const uint8_t private_end[] asm("_binary_private_pem_key_end");

/* Функція обробник події з підключенням Wifi */
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
/* Функція підключенням Wifi мережі */
void connect_wifi(){
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
    esp_wifi_connect();                                                     //Wifi connect phase
}
/* Функція обробник події роботи з клієнтом */
esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt){
    switch (evt->event_id){
        case HTTP_EVENT_ON_DATA:
            printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len,  (char *)evt->data);
            break;
        default:
            break;
    }
    return ESP_OK;
}
/* Функція відправлення даних */
static void post_rest_function(){
    esp_http_client_config_t config_post = {
        .url = "https://a25h60s1inklac-ats.iot.us-east-1.amazonaws.com",
        .method = HTTP_METHOD_POST,
        .cert_pem = (const char*) cert_start,
        .client_cert_pem = (const char*) certificate_start,
        .client_key_pem = (const char*) private_start,
        .event_handler = client_event_post_handler
    };
    esp_http_client_handle_t client = esp_http_client_init(&config_post);
    char *post_data = "{\"start\":{\"Memory\":{\"doubleValue\":\"2\"},\"Name\":{\"stringValue\":\"Additional old ESP32\"}}}";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

void app_main(void){
    nvs_flash_init();
    connect_wifi();
    vTaskDelay(2000/portTICK_PERIOD_MS);
    printf("Wifi was initiated...\n\n");
    printf("Move connect MQTT...\n");
    post_rest_function();
}