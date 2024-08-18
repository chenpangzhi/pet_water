/**
 * @file  env_sensor.c
 * @brief 读取红外传感器等环境传感器的值
 * @date  2024-03-10
 * @author  cyp
 * @copyright  MIT
 * @history
 * <2024-03-10, cyp, V1.0, Create>
 * 
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "env_sensor.h"
#include "driver/gpio.h"

static const char *TAG = "env_sensor";

//热释电红外传感器IO定义
#define GPIO_IR_SW GPIO_NUM_36


/**
 * @brief:  初始化环境传感器
 * @param: void
 * @return: void
 * @note:
 * 初始化热释电红外传感器 初始化IO5
*/
void env_sensor_init(void)
{
    ESP_LOGI(TAG, "env_sensor_init");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = (1ULL << GPIO_IR_SW);

    gpio_config(&io_conf);
}

/**
 * @brief:  获取红外传感器值
 * @param: void
 * @return: 0:没猫 1:有猫
 * @note:
*/
int get_ir_sw(void)
{
    int level = 0;
 
    level = gpio_get_level(GPIO_IR_SW);
    return level;
}

