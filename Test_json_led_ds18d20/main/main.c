#include"main.h"

void app_main(void){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // while (1){
    //     // xTaskCreate(&blinky1, "blinky1", 2048,NULL,5,NULL);
    // }
    json_pars();
    vTaskDelay(1000 / portTICK_RATE_MS);
}
