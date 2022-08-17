#include"main.h"

void app_main(void){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    connect_wifi();

    mqtt_app_start();
    vTaskDelay(5000 / portTICK_RATE_MS);

    xSensor_Control = xQueueCreate(10, sizeof(int));
    if(xSensor_Control == NULL) ESP_LOGI(TAG, "Error xQueueCreate.\n");
    if(xTaskCreate(&publish_mqtt, "Publish_mqtt.", configMINIMAL_STACK_SIZE + 4096,NULL,5,xPublishTask) != pdTRUE){
        ESP_LOGI("Error", "Error publish \n");
        while(1);
    }
     
    if(xTaskCreate(&ds18b20_start, "Temp:\n", configMINIMAL_STACK_SIZE + 4096,NULL,5,xSensorTask) != pdTRUE){
        ESP_LOGE("Error", "Error sensor \n");
        while(1);
    }
        
    xTaskCreate(&blinky1, "blinky1", 2048,NULL,5,NULL);  
    
    while (true){
        ESP_LOGW(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}