#include"funk.c"

void blinky1(void *pvParameter);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_app_start(void);
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *even_data);
void connect_wifi();
static void init_s_ds18d20(void);
static void json_pars();