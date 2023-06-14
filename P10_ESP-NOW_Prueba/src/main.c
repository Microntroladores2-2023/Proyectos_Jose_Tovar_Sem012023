#include <stdio.h>
#include "string.h"
#include "freertos/freeRTOS.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "ESP_NOW_INIT";

static esp_err_t init_wifi(void)
{
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

    esp_netif_init();
    esp_event_loop_create_default();
    nvs_flash_init();
    esp_wifi_init(&wifi_init_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);

    ESP_LOGI(TAG, "wifi_init_completed");
    return ESP_OK;
}


void recive_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    ESP_LOGI(TAG, "Data recived" MACSTR "%s", MACSTR(esp_now_peer_info->peer_addr), data);
}

void send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
if (status == ESP_NOW_SEND_SUCCESS)
{
    ESP_LOGI(TAG,"ESP_NOW_SEND_SUCCESS");
}
else{
ESP_LOGW(TAG, "ESP_NOW_SEND_FAIL");
}

}

static esp_err_t init_esp_now(void)
{
    esp_now_init();
    esp_now_register_recv_cb(recive_cb);
    esp_now_register_send_cb(send_cb);

    ESP_LOGI(TAG, "esp now init completed");

    return ESP_OK;
}
void app_main()
{
    ESP_ERROR_CHECK(init_wifi());
}