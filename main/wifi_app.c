/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "wifi_app.h"
#include "nvsflash_app.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;
static uint8_t ucWifiConnected; //连接wifi标志位
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "smartconfig_example";
static wifi_config_t m_stWifiConfig;

static void smartconfig_example_task(void * parm);

/**
 * @brief 获取wifi连接状态
 * @note 获取wifi连接状态
 * 
 */
uint8_t getWifiConnected(void)
{
    return ucWifiConnected;
}

/**
 * @brief 设置wifi连接状态
 * @param ucConnected 0未连接 1已连接 2连接中 
 * @return void
 * 
 * 
 */
void setWifiConnected(uint8_t ucConnected)
{
    ucWifiConnected = ucConnected;
}


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG, "event_handler:%ld", event_id);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        setWifiConnected(0);
        //esp_wifi_connect();
        //xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) 
    {
        ESP_LOGI(TAG, "Scan done");
        setWifiConnected(0);

    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) 
    {
        ESP_LOGI(TAG, "Found channel");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) 
    {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        bzero(&m_stWifiConfig, sizeof(wifi_config_t));
        memcpy(m_stWifiConfig.sta.ssid, evt->ssid, sizeof(m_stWifiConfig.sta.ssid));
        memcpy(m_stWifiConfig.sta.password, evt->password, sizeof(m_stWifiConfig.sta.password));
        m_stWifiConfig.sta.bssid_set = evt->bssid_set;
        if (m_stWifiConfig.sta.bssid_set == true) {
            memcpy(m_stWifiConfig.sta.bssid, evt->bssid, sizeof(m_stWifiConfig.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i=0; i<33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &m_stWifiConfig) );
        esp_wifi_connect();
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) 
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

static void initialise_wifi(void)
{
    //ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;

    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) {
            setWifiConnected(1);
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            //如果wifi已经连接状态，则停止smartconfig
            if( getWifiConnected() == 1)
            {            
                ESP_LOGI(TAG, "保存密码");
                save_wifi_credentials((char *)m_stWifiConfig.sta.ssid, (char *)m_stWifiConfig.sta.password);
            }
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

/**
 * @brief   启动smartconfig
 * 
*/
void smartconfig_start(void)
{
    ESP_LOGI(TAG, "smartconfig_start");
    setWifiConnected(2);
    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    esp_esptouch_set_timeout(60);
    esp_smartconfig_start(&cfg);
}

/**
 * @brief 开启WiFi连接
 * 
 */
void wifi_conn_start(void)
{
    if(m_stWifiConfig.sta.ssid[0] != '\0' && m_stWifiConfig.sta.password[0] != '\0')
    {
        esp_wifi_set_config(WIFI_IF_STA, &m_stWifiConfig);
        setWifiConnected(2);
        esp_wifi_connect();
    }
    else
    {
        ESP_LOGI(TAG, "密码没找到");
    }
}


/** 
 * @brief  初始化wifi连接，当存在wifi密码和账号的时候自动连接wifi
 * @return void 
 * @note   
 * @attention
 * 
*/
void wifi_app_init(void)
{
    ////初始化wifi
    initialise_wifi();
    load_wifi_credentials( (char *)m_stWifiConfig.sta.ssid, (char *)m_stWifiConfig.sta.password );
    ESP_LOGI(TAG, "wifi ssid:%s", m_stWifiConfig.sta.ssid);
    ESP_LOGI(TAG, "wifi password:%s", m_stWifiConfig.sta.password);

}
