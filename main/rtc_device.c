/**
 * @file rtc_device.c
 * @brief DS1302 rtc芯片驱动
 * @author cyp
 * @version 1.0.0
 * @date 2024-01-07
 */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "rtc_device.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/rtc_io.h"

static const char *TAG = "rtc_device";

// 引脚定义
#define SCLK_PIN GPIO_NUM_7
#define IO_PIN GPIO_NUM_8
#define RST_PIN GPIO_NUM_9

unsigned char twdata[8]={0x58,0x59,0x23,0x31,0x12,0x06,0x04,0x80};
//unsigned char twdata[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80};



void send_byte(uint8_t data) {
    gpio_set_direction(IO_PIN, GPIO_MODE_OUTPUT);
    for (int i = 0; i < 8; i++) {
        gpio_set_level(IO_PIN, (data >> i) & 0x01);
        gpio_set_level(SCLK_PIN, 1);
        gpio_set_level(SCLK_PIN, 0);
    }
}

uint8_t receive_byte() {
    uint8_t data = 0;
    gpio_set_direction(IO_PIN, GPIO_MODE_INPUT);
    for (int i = 0; i < 8; i++) {
        if (gpio_get_level(IO_PIN)) {
            data |= (1 << i);
        }
        gpio_set_level(SCLK_PIN, 1);
        gpio_set_level(SCLK_PIN, 0);
    }
    return data;
}

void write_register(uint8_t address, uint8_t data) {
    gpio_set_level(RST_PIN, 0);
    gpio_set_level(SCLK_PIN, 0);
    gpio_set_level(RST_PIN, 1);
    send_byte(address);
    send_byte(data);
    gpio_set_level(SCLK_PIN, 1);

    gpio_set_level(RST_PIN, 0);
}

uint8_t read_register(uint8_t address) {
    uint8_t data;
    gpio_set_level(RST_PIN, 0);
    gpio_set_level(SCLK_PIN, 0);
    gpio_set_level(RST_PIN, 1);
    send_byte(address);
    data = receive_byte();
    gpio_set_level(SCLK_PIN, 1);
    gpio_set_level(RST_PIN, 0);
    return data;
}

/********************************************************************
函 数 名：BurstW1302T()
功 能：往 GM1302 写入时钟数据(多字节方式)
说 明：先写地址，后写命令/数据
调 用：RTInputByte()
入口参数：pWClock: 时钟数据地址 格式为: 秒 分 时 日 月 星期 年 控制 8Byte (BCD 码)1B 1B 1B 1B 1B 1B 1B 1B
返 回 值：无
***********************************************************************/
void BurstW1302T(uint8_t *pWClock)
{ 
    write_register(0x8e,0x00); /* 控制命令,WP=0,写操作?*/ 
    gpio_set_level(RST_PIN, 0);
    gpio_set_level(SCLK_PIN, 0);
    gpio_set_level(RST_PIN, 1);
    send_byte(0xBe); /* 0xbe:时钟多字节写命令 */ 
    for (uint8_t i = 8; i>0; i--)
    {  /*8Byte = 7Byte 时钟数据 + 1Byte 控制*/ 
        send_byte(*pWClock); /* 写 1Byte 数据*/
        pWClock++; 
    }
    gpio_set_level(SCLK_PIN, 1);
    gpio_set_level(RST_PIN, 0);
}

/********************************************************************
函 数 名：BurstR1302T() 
功 能：读取 GM1302 时钟数据 
说 明：先写地址/命令，后读数据(时钟多字节方式) 
调 用：RTInputByte() , RTOutputByte() 
入口参数：pRClock: 读取时钟数据地址 格式为: 秒 分 时 日 月 星期 年 7Byte (BCD 码)1B 1B 1B 1B 1B 1B 1B 
返 回 值：无
***********************************************************************/
void BurstR1302T(uint8_t *pRClock)
{ 
    gpio_set_level(RST_PIN, 0);
    gpio_set_level(SCLK_PIN, 0);
    gpio_set_level(RST_PIN, 1);
    send_byte(0xbf); /* 0xbf:时钟多字节读命令 */ 
    for (uint8_t i=8; i>0; i--) 
    { 
        *pRClock = receive_byte(); /* 读 1Byte 数据 */ 
        pRClock++; 
    }
    gpio_set_level(SCLK_PIN, 1);
    gpio_set_level(RST_PIN, 0);
}

/**
 * @brief 初始化内部RTC和外部RTC芯片函数
 * @return void
 * @attention
 * @param void
 */
void rtc_device_init() {
    // 初始化外部RTC芯片
    gpio_set_direction(SCLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(IO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);


    BurstW1302T(twdata); /* 多字节方式设置 GM1302 时间 */

}
void setcurrtime(int year, int month, int day, int hour, int minute, int second)
{       
    // 内部RTC时间
    struct  tm tm = {
        .tm_year = year - 1900,  // 年份，从 1900 年开始
        .tm_mon = month - 1,         // 月份，从 0 开始
        .tm_mday = day,           // 日期
        .tm_hour = hour,           // 小时
        .tm_min = minute,            // 分钟
        .tm_sec = second              // 秒
    };
    
    time_t t = mktime(&tm);
    struct timeval now = {
        .tv_sec = t,
        .tv_usec = 0
    };
    
    settimeofday(&now, NULL);    
}

/**
 * @brief 获取当前时间
 * @return void
 */
void getcurrtime(struct tm *timeinfo)
{
    time_t now;
    char strftime_buf[64];

    time(&now);
    localtime_r(&now, timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", timeinfo);

}

/**
 * @brief 判断是否在夜间
 * @return 0 - 非夜间 1 - 夜间
 */
int get_night_status(void)
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    int hour, minute, second;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    //ESP_LOGI(TAG, "The current date/time is: %02d:%02d:%02d", hour, minute, second);
    if(hour >= 20 || hour <= 6)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 测试函数
*/
void rtc_device_test(void)
{
    unsigned char trdata[7] = {0};
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

//    time(&now);
//    localtime_r(&now, &timeinfo);

//    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
//    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
    BurstR1302T(trdata);
    ESP_LOGI(TAG, "The current date/time is: %02x:%02x:%02x %02x-%02x-%02x", trdata[0], trdata[1], trdata[2], trdata[3], trdata[4], trdata[5]);
    vTaskDelay(10000);

}

