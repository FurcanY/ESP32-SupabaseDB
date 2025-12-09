#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"


// ============================================================
//               AYARLAR
// ============================================================

# include "config.h"
// Wifi işlemleri için gerekli ayarlar
//#include "wifi_manager.h"

// Veritabanı işlemleri için gerekli ayarlar
#include "db_manager.h"

void nvs_initialize(void);

static const char *TAG = "ESP_32_MAIN";


#include "driver/gpio.h"
#define BLINK_GPIO 2

void app_main(void) {


    nvs_initialize(); //NVS başlatır.

    // wifi işlemleri başlatılır. (wifi işlemleri için yazılan fonksiyon- sizin yaptığınız varsayılır.)
    //wifi_init_sta(); 

    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while(1) {

        // LED YAK
        ESP_LOGI(TAG, "LED 1");
        gpio_set_level(BLINK_GPIO, 1);

        // veritabanı gönderimi
        update_variable_bool("LED",true);
        vTaskDelay(10000 / portTICK_PERIOD_MS);

        // LED SONDUR
        ESP_LOGI(TAG, "LED 0");
        gpio_set_level(BLINK_GPIO, 0);

        // veritabanı gönderimi
        update_variable_bool("LED",false);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}


void nvs_initialize(void){

    // 1. Sistem Başlatma (NVS ve Wifi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase()); 
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

}