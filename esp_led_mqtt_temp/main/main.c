#include <stdlib.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_mqtt.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

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

// static void status_callback_aws(esp_mqtt_status_t status) {
//     switch (status) {
//         case ESP_MQTT_STATUS_CONNECTED:
//             esp_mqtt_subscribe("/he", 2);  // subscribe
//             printf("Subcribe topic.\n");
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
        //printf("Work message aws.\n");
// }

void app_main(void){
    
    ESP_ERROR_CHECK(nvs_flash_init());  // initialize NVS flash
    printf("Initialize NVS flash.\n");

    ESP_ERROR_CHECK(esp_netif_init());    // initialize networking
    printf("Initialize networking.\n");   
  
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // create default event loop
    printf("Create default event loop.\n");

    esp_netif_create_default_wifi_sta();  // enable Wi-Fi
    printf("Enable Wi-Fi.\n");
 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // initialize Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    printf("Initialize Wi-Fi.\n");

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));  // set Wi-Fi storage to ram
    printf("Set Wi-Fi storage to ram.\n");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));  // set wifi mode
    printf("Set wifi mode.\n");


    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));  // register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
    printf("Register event handlers.\n");

    wifi_config_t wifi_config = {.sta = {.ssid = WIFI_SSID, .password = WIFI_PASS}};  // prepare Wi-Fi config
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    printf("Prepare Wi-Fi config.\n");

    ESP_ERROR_CHECK(esp_wifi_start());    // start Wi-Fi
    printf("Start Wi-Fi.\n");

  // initialize mqtt
//   esp_mqtt_init(status_callback, message_callback, 256, 2000);

  // create tasks
//   xTaskCreatePinnedToCore(process_mqtt_publish, "process_mqtt_publish", 2048, NULL, 10, NULL, 1);
//   xTaskCreatePinnedToCore(start_work_net, "start work net", 2048, NULL, 10, NULL, 1);
}
