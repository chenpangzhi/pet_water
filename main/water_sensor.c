/*
 * @brief:  读取触摸传感器值检测水位
 * @version: V1.0
 * @date:    2024-03-09
 * @author:   cyp
 * @company:  
 * @description:
 * @copyright:   Copyright (c) 2024 cyp
 * @license:    MIT
 * @note: 
 * @history:
 * <2024-03-09, cyp, V1.0, Create>
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "water_sensor.h"

static const char *TAG = "water_sensor";

//废水箱检测IO
#define WASTE_WATER_LEVEL_IO      GPIO_NUM_3
//净水箱检测IO
#define CLEAR_WATER_LEVEL_IO      GPIO_NUM_4
//饮水盆低水位IO
#define DRAIN_WATER_LEVEL_LOW_IO      GPIO_NUM_5
//饮水盆高水位IO
#define DRAIN_WATER_LEVEL_HIGH_IO      GPIO_NUM_6

/**
 * @brief:  初始化触摸传感器
 * @param: void
 * @return: void
 * @note:
 * 
*/
void water_sensor_init(void)
{
// iO初始化
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << WASTE_WATER_LEVEL_IO) | (1ULL << CLEAR_WATER_LEVEL_IO) | (1ULL << DRAIN_WATER_LEVEL_LOW_IO) | (1ULL << DRAIN_WATER_LEVEL_HIGH_IO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

    gpio_config(&io_conf);
}

/*
 * @brief:  获取净水箱水位是否过低标志
 * @param: void
 * @return: 0:水位正常 1:水位过低
 * @note:
*/
int getClearWaterLevel(void)
{
    int level = 0;
    
    if(gpio_get_level(CLEAR_WATER_LEVEL_IO) == 1)
    {
        level = 1;
    }
    

    return level;
}

/**
 * @brief:  获取饮水盆水位是否过低标志
 * @param: void
 * @return: 0:水位正常 1:水位过低
 * @note:
*/
int getDrinkWaterLevelLow(void)
{
    int level = 0;

    if(gpio_get_level(DRAIN_WATER_LEVEL_LOW_IO) == 1) 
    {
        level = 1;
    }   

    return level;
}

/**
 * @brief:  获取饮水盆水位是否过高标志
 * @param: void
 * @return: 0:水位正常 1:水位过高
 * @note:
*/
int getDrinkWaterLevelHigh(void)
{
    int level = 0;
    
    if( gpio_get_level(DRAIN_WATER_LEVEL_HIGH_IO) == 0)
    {
        level = 1;
    }
    return level;
}

/**
 * @brief:  获取废水箱水位是否过高标志
 * @param: void
 * @return: 0:水位正常 1:水位过高
 * @note:
*/
int getDrainWaterLevelHigh(void)
{
    int level = 0;
    
    if(gpio_get_level(WASTE_WATER_LEVEL_IO) == 0)
    {
        level = 1;
    }
    return level;
}

