#include"funk.c"

void blinky1(void *pvParameter);
static bool json_pars(void);
static void ds18b20_start(void *pvParameter);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_app_start(void);
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *even_data);
static void connect_wifi(void);
// static bool ds18b20_start(void);
// static bool ds18b20_start(void );