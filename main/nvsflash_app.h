#ifndef NVSFLASH_APP_H
#define NVSFLASH_APP_H

#include "esp_err.h"
#include "esp_event.h"

#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASSWORD_KEY "wifi_password"

//系统参数结构体定义
typedef struct {
    char ssid[32];
    char password[64];
    uint8_t ucLanguage;
} SystemParam_ST;

extern SystemParam_ST g_system_param;

extern esp_err_t nvsflash_app_init(void);
extern void save_wifi_credentials(const char *ssid, const char *password);
extern void load_wifi_credentials(char *ssid, char *password);

#endif