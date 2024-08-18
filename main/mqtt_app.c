/*
 * @brief  MQTT 客户端应用
 * @date   2024.04.13
 * @file   mqtt_app.c
 * @author chenyupeng
 * @note   
 */

#include <stdio.h>
#include <stdlib.h>   
#include <stdint.h>
#include "mqtt_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "mqtt_client.h"
//json解析库
#include "cJSON.h"
#include "water_sensor.h"
#include "device_ctrl.h"
#include "motor_device.h"
#include "oled_device.h"
#include "rtc_device.h"
#include "env_sensor.h"

static const char *TAG = "MQTT_APP";
//订阅服务器IP地址
#define MQTT_SERVER_IP "192.168.31.213"

//订阅服务器端口
#define MQTT_SERVER_PORT 1883 

//订阅服务器用户名
#define MQTT_SERVER_USERNAME "admin"

//订阅服务器密码
#define MQTT_SERVER_PASSWORD "admin"

//订阅服务器主题
#define MQTT_SERVER_TOPIC "cyp/test"

//debug模式主题
#define MQTT_SERVER_DEBUG_TOPIC "petwater/debug"
//log模式主题
#define MQTT_SERVER_LOG_TOPIC "petwater/log"

static unsigned char usMqttClientStatus;
static uint16_t  usDebugStatus;

esp_mqtt_client_handle_t mqtt_client;

/**
 * @brief 切换debug状态 0 非debug模式  1debug模式
 * @param status debug状态
 * @return void
 */
void mqtt_set_debug_status(uint16_t status) 
{
    usDebugStatus = status;
}

/**
 * @brief 获取debug状态 0 非debug模式  1debug模式
 * @param void
 * @return void
 */
uint16_t mqtt_get_debug_status(void) 
{
    return usDebugStatus;
}

/**
 * @brief 设置MQTT连接状态
 * @param status 连接状态
 * @return 无
*/
void mqtt_set_client_status(uint16_t status) 
{
    usMqttClientStatus = status;
}

/*
 * 获取MQTT客户端连接状态
 * @param 无
 * @return 连接状态
 */
unsigned char mqtt_get_client_status(void) 
{
    return usMqttClientStatus;
}

/**
 * @brief 发送log到MQTT服务器
 * @param char
 * @return 无
*/
void mqtt_send_log(const char *log) 
{
    esp_mqtt_client_publish(mqtt_client, MQTT_SERVER_LOG_TOPIC, log, 0, 0, 0);
}
#if 0
/**
 * @brief 接收到的数据进行json解析
 */
void mqtt_parse_json(const char *json) 
{
    cJSON *json_root = cJSON_Parse(json);
    if (json_root != NULL) {
        cJSON *json_data = cJSON_GetObjectItem(json_root, "data");
        if (json_data != NULL) {
            cJSON *json_value = cJSON_GetObjectItem(json_data, "value");
            if (json_value != NULL) {
                //获取value的值
                const char *value = json_value->valuestring;
               //根据value的值进行不同的操作
            }
            cJSON_Delete(json_root);
        }
        cJSON_Delete(json_root);
     
    }
}
#endif

/**
 * @brief MQTT debug数据处理函数
 * 
 */
void mqtt_debug_data_handle(const char *data) 
{
    char caDebugData[1024] = {0};

    // 水箱检测状态
    if(memcmp(data, "waterlevel", strlen("waterlevel")) == 0)
    {
        int waterLevel = 0;
        //废水箱
        waterLevel = getDrainWaterLevelHigh();
        {
            sprintf(caDebugData, "waste Water Level:%d", waterLevel);
            mqtt_send_log(caDebugData);
        }
        //净水箱
        waterLevel = getClearWaterLevel();
        sprintf(caDebugData, "clear Water Level:%d", waterLevel);
        mqtt_send_log(caDebugData);
        //饮水盆高位
        waterLevel = getDrinkWaterLevelHigh();
        sprintf(caDebugData, "drink Water High Level:%d", waterLevel);
        mqtt_send_log(caDebugData);
        //饮水盆低位
        waterLevel = getDrinkWaterLevelLow();
        sprintf(caDebugData, "drink Water Low Level:%d", waterLevel);
        mqtt_send_log(caDebugData);
    }
    else if(memcmp(data, "huoer", strlen("huoer")) == 0)
    {

    }
    else if(memcmp(data, "env", strlen("env")) == 0)
    {//获取人体传感器值
        int envLevel = 0;

        envLevel = get_ir_sw();
        sprintf(caDebugData, "env Level:%d", envLevel);
        mqtt_send_log(caDebugData);

    }
    else if(memcmp(data, "pump1open", strlen("pump1open")) == 0)
    {
        device_ctrl_bump1_open();
        mqtt_send_log("pump1open ok");
    }
    else if(memcmp(data, "pump1close", strlen("pump1close")) == 0)
    {
        device_ctrl_bump1_close();
        mqtt_send_log("pump1close ok");
    }
    else if(memcmp(data, "pump2open", strlen("pump2open")) == 0)
    {
        device_ctrl_bump2_open();
        mqtt_send_log("pump2open ok");
    }
    else if(memcmp(data, "pump2close", strlen("pump2close")) == 0)
    {
        device_ctrl_bump2_close();
        mqtt_send_log("pump2close ok");
    }   
    else if(memcmp(data, "uvopen", strlen("uvopen")) == 0)
    {
        device_ctrl_uv_switch(1);
        mqtt_send_log("uvopen ok");
    }
    else if(memcmp(data, "uvclose", strlen("uvclose")) == 0)
    {
        device_ctrl_uv_switch(0);
        mqtt_send_log("uvclose ok");
    }   
    else if(memcmp(data, "motorshun=", strlen("motorshun=")) == 0)
    {// 步进电机步数
        int step = 0;
        step = atoi(data+strlen("motorshun="));
        motor_device_test(step, 0);
        mqtt_send_log("motorshun ok");
    }
    else if(memcmp(data, "motorni=", strlen("motorni=")) == 0)
    {// 步进电机步数
        int step = 0;
        step = atoi(data+strlen("motorni="));
        motor_device_test(step, 1);
        mqtt_send_log("motorni ok");
    }
    else if(memcmp(data, "oled", strlen("oled")) == 0)
    {// 步进电机步数
        oled_test();
        mqtt_send_log("oled ok");
    }
    else if(memcmp(data, "rtc", strlen("rtc")) == 0)
    {// 步进电机步数
        rtc_device_test();
        mqtt_send_log("rtc ok");
    }
    else if(memcmp(data, "2024", strlen("2024")) == 0)
    {   
        char temp[10] = {0};
        int month = 0;   
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;

        month = atoi(data+5);
        day = atoi(data+8);
        hour = atoi(data+11);
        minute = atoi(data+14);
        second = atoi(data+17);
        sprintf(caDebugData, "setcurrtime: %d-%d-%d %d:%d:%d", 2024, month, day, hour, minute, second);
        mqtt_send_log(caDebugData);

        setcurrtime(2024, month, day, hour, minute, second);
    }
    //帮助命令
    if(memcmp(data, "help", strlen("help")) == 0)
    {
        mqtt_send_log("help:waterlevel,huoer,pump1open,pump1close,pump2open,pump2close,uvopen,uvclose,motorshun=,motorni=,oled,rtc");
    }

}
/* 事件回调函数
 * @param event 事件结构体
 * @return 错误码
 * @note 事件回调函数
 */
static void mqtt_event_handler_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");    
            mqtt_set_client_status(1);        
            esp_mqtt_client_subscribe(mqtt_client, MQTT_SERVER_TOPIC, 0);            
            esp_mqtt_client_subscribe(mqtt_client, MQTT_SERVER_DEBUG_TOPIC, 0);

            //发布主题
            mqtt_send_log("MQTT_EVENT_CONNECTED");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            //esp_mqtt_client_disconnect(mqtt_client);
            //esp_mqtt_client_reconnect(mqtt_client);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            //ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            //ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            if(memcmp(event->topic, MQTT_SERVER_DEBUG_TOPIC, event->topic_len) == 0)
            {//debug模式数据处理
                if(mqtt_get_debug_status() == 1)
                {
                    mqtt_debug_data_handle(event->data);
                }
                else
                {
                    mqtt_send_log("debug mode is close");
                }
            }
            else if(memcmp(event->topic, MQTT_SERVER_TOPIC, event->topic_len) == 0)
            {//test模式 用于进入debug模式和烧录串号等
                if(memcmp(event->data, "debugstart", event->data_len) ==  0)
                {
                    mqtt_set_debug_status(1);
                    mqtt_send_log("debugstart");

                }
                else if(memcpy(event->data, "debugstop", event->data_len) == 0)
                {
                    mqtt_set_debug_status(0);
                    mqtt_send_log("debugstop");

                }
            }
            else if(0)
            {//正常数据接收

            }
            break;
    }
}



/** MQTT应用启动函数，连接mqtt服务器
 * @return 无
*/
void mqtt_app_start(void) {
    //mqtt连接
    mqtt_set_client_status(2);
    ESP_LOGI(TAG, "MQTT APP START");

    esp_mqtt_client_start(mqtt_client);

}

/**
 * mqtt 初始化
 * 
*/ 
void mqtt_app_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg;
    memset((char *)&mqtt_cfg, 0, sizeof(esp_mqtt_client_config_t));
    mqtt_cfg.broker.address.path = "/";
    mqtt_cfg.broker.address.uri = "mqtt://110.41.59.66:1883";
    mqtt_cfg.network.disable_auto_reconnect = false;
    mqtt_cfg.broker.address.port = 1883;
    mqtt_cfg.credentials.client_id = "cyp";
    mqtt_cfg.credentials.username = "admin";
    mqtt_cfg.credentials.authentication.password = "admin";
    mqtt_cfg.session.protocol_ver = MQTT_PROTOCOL_V_5;
    mqtt_cfg.session.last_will.msg_len = 0;
    mqtt_cfg.session.disable_keepalive = false;
    mqtt_cfg.session.keepalive = 5;
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) 
    {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return;
    }
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, mqtt_client);
}


