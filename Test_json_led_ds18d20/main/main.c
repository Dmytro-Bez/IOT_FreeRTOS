#include"main.h"

void app_main(void){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

   
    // connect_wifi();
    // mqtt_app_start();
 

    xTaskCreate(&blinky1, "blinky1", 2048,NULL,5,NULL);   
    if(xTaskCreate(&ds18b20_start, "Temp:/n", configMINIMAL_STACK_SIZE + 4096,NULL,5,xSensorTask) != pdTRUE){
        ESP_LOGE("Error", "Error sensor \n");
        while(1);
    }
    while (true){
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}