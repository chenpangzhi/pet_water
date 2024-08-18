/** @file nvs_flash_app.c
 * @brief NVS Flash API
 * @author cyp 
 * @date 2024-05-01
 * @version 1.0
 * @copyright Copyright (c) 2024, cyp
*/
#include "esp_system.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvsflash_app.h"
#include "esp_log.h"

static const char *TAG = "nvs_flash_app";
//NVS分区表名称
#define NVS_PARTITION_NAME "para"

SystemParam_ST g_system_param = {
    .password = "12345678",
    .ssid = "cyp",
    .ucLanguage = 0
};

/**
 * @brief nvsflash_app_init
 * @return esp_err_t
 * @note 初始化nvsflash
 * @attention
 * @param void
*/
esp_err_t nvsflash_app_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_ini
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        ESP_ERROR_CHECK(err);
    }
    return err;
}

// 保存WiFi凭据到NVS
void save_wifi_credentials(const char *ssid, const char *password)
{
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open(NVS_PARTITION_NAME, NVS_READWRITE, &nvs_handle));

    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, WIFI_SSID_KEY, ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, WIFI_PASSWORD_KEY, password));

    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "保存密码成功");

}

#if 0
/**
 * @brief 保存参数到flash
 * @return ret -1 失败，0成功
 * @note
*/
int save_param_to_flash(void *param, int param_size)
{
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open(NVS_PARTITION_NAME, NVS_READWRITE, &nvs_handle));

    ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, "param", param, param_size));

    ESP_ERROR_CHECK(nvs_commit());
}
#endif

// 从NVS加载WiFi凭据
void load_wifi_credentials(char *ssid, char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = ESP_OK;

    nvs_open(NVS_PARTITION_NAME, NVS_READWRITE, &nvs_handle);

    size_t ssid_size = 32;
    size_t password_size = 32;

    // 从NVS获取SSID
    err = nvs_get_str(nvs_handle, WIFI_SSID_KEY, ssid, &ssid_size);
    if (err != ESP_OK) {
        strcpy(ssid, ""); // 如果没有找到，将SSID设置为空字符串
    }

    // 从NVS获取密码
    err = nvs_get_str(nvs_handle, WIFI_PASSWORD_KEY, password, &password_size);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "密码未找到");
        //strcpy(password, ""); // 如果没有找到，将密码设置为空字符串
    }
    ESP_LOGI(TAG, "size:%d,%d", ssid_size, password_size);

    nvs_close(nvs_handle);
}