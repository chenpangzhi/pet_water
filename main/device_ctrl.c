/**
 *  @file  device_ctrl.c
 *  @brief 用于控制设备里的水泵、水阀等模块
 *  @date  2024-03-10
 *  @author  cyp
 *  @copyright  MIT
 *  @note
 *  @history
 *  <2024-03-10, cyp, V1.0, Create>
 * 
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "device_ctrl.h"
#include "driver/gpio.h"

static const char *TAG = "device_ctrl";



//水泵开关
#define GPIO_BUMP1_SW GPIO_NUM_33
//水泵2开关
#define GPIO_BUMP2_SW GPIO_NUM_34

//紫外线灯开关
#define GPIO_UV_SW GPIO_NUM_21
/**
 * @brief:  初始化水泵和水阀的IO
 * @param: void
 * @return: void
 * 
*/
void device_ctrl_init(void)
{
    ESP_LOGI(TAG, "device_ctrl_init");

    //初始化水泵1
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = (1ULL << GPIO_BUMP1_SW);
    gpio_config(&io_conf);
    gpio_set_level(GPIO_BUMP1_SW, 0);

    //初始化水泵2
    gpio_config_t io_conf2;
    io_conf2.intr_type = GPIO_INTR_DISABLE;
    io_conf2.mode = GPIO_MODE_OUTPUT;
    io_conf2.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf2.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf2.pin_bit_mask = (1ULL << GPIO_BUMP2_SW);
    gpio_config(&io_conf2);
    gpio_set_level(GPIO_BUMP2_SW, 0);



    // 初始化紫外线灯IO
    gpio_config_t io_conf4;
    io_conf4.intr_type = GPIO_INTR_DISABLE;
    io_conf4.mode = GPIO_MODE_OUTPUT;
    io_conf4.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf4.pin_bit_mask = (1ULL << GPIO_UV_SW);
    gpio_config(&io_conf4);
    gpio_set_level(GPIO_UV_SW, 0);

}

/**
 * @brief:  控制水泵1开
 * @param: void
 * @return: void
 * 
*/
void device_ctrl_bump1_open(void)
{
    gpio_set_level(GPIO_BUMP1_SW, 1);
}

/**
 * @brief:  控制水泵1关
 * @param: void
 * @return: void
 * 
*/
void device_ctrl_bump1_close(void)
{
    gpio_set_level(GPIO_BUMP1_SW, 0);
}

/**
 * @brief:  控制水泵2开
 * @param: void
 * @return: void
 * 
*/
void device_ctrl_bump2_open(void)
{
    gpio_set_level(GPIO_BUMP2_SW, 1);
}

/**
 * @brief:  控制水泵2关
 * @param: void
 * @return: void
 * 
*/
void device_ctrl_bump2_close(void)
{
    gpio_set_level(GPIO_BUMP2_SW, 0);
}


/**
 * @brief:  控制紫外线灯开关
 * @param: 0 关 1 开
 * @return: void
 * 
*/
void device_ctrl_uv_switch(int iState)
{
    gpio_set_level( GPIO_UV_SW, iState);
}