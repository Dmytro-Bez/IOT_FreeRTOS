#include"main.h"

void app_main(void){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    connect_wifi();
    mqtt_app_start();
    vTaskDelay(2000 / portTICK_RATE_MS);

    // xTaskCreate(&blinky1, "blinky1", 2048,NULL,5,NULL);   
    // xTaskCreate(&ds18b20_start, "Temp:/n", 2048,NULL,6,NULL);  
    
    json_pars();  
    // vTaskDelay(2000 / portTICK_RATE_MS);
}