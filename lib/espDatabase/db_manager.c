#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "stdbool.h"
#include "esp_wifi.h"
#include "esp_crt_bundle.h"
#include "db_manager.h"

// Proje ayarları
#include "config.h"

static const char *TAG = "DB_MANAGER";
static const char *ESP_NAME = "ESP32_FURKAN";
// --- PROTOTİPLER ---
static void _send_json_package(cJSON *json_root);
static void _send_http_string(const char *key, const char *value);
static void _send_http_bool(const char *key, bool value);





static void _send_json_package(cJSON *json_root) {
    
    if (!is_wifi_connected()) {
        ESP_LOGW(TAG, "WiFi bagli degil! Log gonderilemedi.");
        return;
    }

    char *post_data = cJSON_PrintUnformatted(json_root);
    
    // GÜVENLİK 2: JSON oluşturulamadıysa işlemi iptal et (Crash önleyici)
    if (post_data == NULL) {
        ESP_LOGE(TAG, "JSON olusturma hatasi (Bellek yetersiz olabilir)");
        return;
    }

    esp_http_client_config_t config = {
        .url = DATABASE_URL,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 5000, // 5 saniye zaman aşımı koyalım ki sonsuza kadar 
        
        .buffer_size = 1024,     // Gelen veri (Response) için 4KB yer aç (YETMİYORDU)
        .buffer_size_tx = 2048,  // Giden veri için 2KB yer aç

        .disable_auto_redirect = true
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "HTTP Client baslatilamadi");
        free(post_data);
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "apikey", DATABASE_KEY);

    char auth_header[300];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", DATABASE_KEY);
    esp_http_client_set_header(client, "Authorization", auth_header);

    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Log Gonderildi. Kod: %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "Log Hatasi: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    free(post_data);
}



static void _send_http_bool(const char *key, bool value) {
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) return;

    cJSON_AddStringToObject(root,"esp_device",ESP_NAME);
    cJSON_AddStringToObject(root, "variable_name", key);
    cJSON_AddBoolToObject(root, "value", value);   

    _send_json_package(root); 
    cJSON_Delete(root); 
}


void update_variable_string (const char* key, const char *value){
    nvs_handle_t my_handle;
    char saved_str[64] = {0}; // Boyutu büyüttük
    size_t len = sizeof(saved_str);
    esp_err_t err;

    err = nvs_open (NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err == ESP_OK) {
        err = nvs_get_str(my_handle, key, saved_str, &len);
        
        // Veri yoksa (NOT_FOUND) veya değer farklıysa
        if (err == ESP_ERR_NVS_NOT_FOUND || strcmp(saved_str, value) != 0) {
            
            ESP_LOGI(TAG, "Deger guncelleniyor: %s", key);
            nvs_set_str(my_handle, key, value);
            nvs_commit(my_handle);
            
            // Sadece WiFi varsa göndermeyi dene (Fonksiyon içinde kontrol var ama buraya da ekleyebilirsin)
            _send_http_string(key, value);
        }
        nvs_close(my_handle);
    } else {
        ESP_LOGE(TAG, "NVS Acilamadi! Hata: %s", esp_err_to_name(err));
    }
}

void update_variable_bool(const char* key, bool value){
    nvs_handle_t my_handle;
    uint8_t saved_u8 = 2; // Başlangıç değeri
    esp_err_t err;

    err = nvs_open (NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err == ESP_OK) {
        err = nvs_get_u8(my_handle, key, &saved_u8);
        bool current_bool_val = (saved_u8 == 1);

        if (err == ESP_ERR_NVS_NOT_FOUND || current_bool_val != value) {
            ESP_LOGI(TAG, "Bool degisti: %s -> %d", key, value);
            
            nvs_set_u8(my_handle, key, (value ? 1 : 0));
            nvs_commit(my_handle);

            _send_http_bool(key, value);
        }
        nvs_close(my_handle);
    }
}

