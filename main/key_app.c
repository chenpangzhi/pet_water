/**
 * @file key_app.c
 * @brief 按键处理程序
 * @version 0.1
 * @date 2024-06-01
 */
#include "key_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#define TAG "key_app"

/* 排水按键引脚定义 */
#define KEY_WATER_IO  1
// 显示切换按键
#define KEY_DISP_IO   2

//显示按键消息队列
QueueHandle_t key_msg_queue;
//排水按键消息队列
QueueHandle_t key_water_msg_queue;

/**
 * @brief 按键任务
 * @param 无
 * @return 无
 */
void key_task(void *pvParameters)
{
    key_msg_t key_msg;
    uint16_t key_water_count = 0;
    uint16_t key_disp_count = 0;

    while(1)
    {

        //按键滤波 短按判断
        if(gpio_get_level(KEY_WATER_IO) == 0)
        {
            key_water_count ++;
            if((key_water_count > 15) && (key_water_count < 30) )            
            {
                key_water_count = 300;
                key_msg.key_id = KEY_ID_WATER;
                key_msg.key_state = KEY_STATE_LONG;                 
                xQueueSend(key_water_msg_queue, &key_msg, portMAX_DELAY);
            }
        }
        else
        {
            key_water_count = 0;
        }

        if(gpio_get_level(KEY_DISP_IO) == 0)
        {
            key_disp_count ++;
            if(key_disp_count > 15)
            {
                key_disp_count = 15;
                key_msg.key_id = KEY_ID_DISPLAY;
                key_msg.key_state = KEY_STATE_LONG; 
                xQueueSend(key_msg_queue, &key_msg, portMAX_DELAY);

            }
        }
        else
        {
            if(key_disp_count > 2)
            {//短按
                key_msg.key_id = KEY_ID_DISPLAY;
                key_msg.key_state = KEY_STATE_SHORT;
                xQueueSend(key_msg_queue, &key_msg, portMAX_DELAY);
            }
            else if((key_disp_count > 15) && (key_disp_count < 30) )
            {
                key_msg.key_id = KEY_ID_DISPLAY;
                key_msg.key_state = KEY_STATE_LONG;
                xQueueSend(key_msg_queue, &key_msg, portMAX_DELAY);
            }
            key_disp_count = 0;
        }

        

        //任务延时10ma
        vTaskDelay(100);
    }
}

/**
 * @brief 获取显示按键消息
 * @param 无
 * @return 无
 */
void get_key_msg(key_msg_t *key_msg)
{
    xQueueReceive(key_msg_queue, key_msg, 0);
}

/**
 * @brief 获取排水按键消息
 * @param key_msg_t *key_msg 获取到的按键消息
 * @return 是否接收到消息队列
 */
int get_key_water_msg(key_msg_t *key_msg)
{
    return xQueueReceive(key_water_msg_queue, key_msg, 0);
}


/**
 * @brief 按键初始化
 * @param 无
 * @return 无
 */
void key_init(void)
{
    gpio_config_t io_conf = {0};
    io_conf.intr_type = GPIO_INTR_DISABLE; 
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << KEY_WATER_IO) | (1ULL << KEY_DISP_IO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // 按键消息队列初始化
    key_msg_queue = xQueueCreate(5, sizeof(key_msg_t));
    //排水按键消息队列
    key_water_msg_queue = xQueueCreate(5, sizeof(key_msg_t));
    
    //  创建按键任务
    xTaskCreate(key_task, "key_task", 4096, NULL, 10, NULL);
}

