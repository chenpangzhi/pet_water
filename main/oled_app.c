/**
 * @file oled_app.c
 * @brief oled显示应用程序，使用u8g2库
 * @author chenyupeng
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>

#include "oled_app.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "u8g2_myfont_chinese.h"
#include "key_app.h"
#include "wifi_app.h"
#include "mqtt_app.h"
#include "rtc_device.h"
#include "nvsflash_app.h"
#include "wifi_app.h"
// SPI pins
#define PIN_NUM_D1_MOSI 11
#define PIN_NUM_D0_CLK  12
#define PIN_NUM_CS   10
#define PIN_NUM_DC   13
#define PIN_NUM_RST  14

#define TAG "OLED_APP"

static u8g2_t oled_handle;
static UI_PageST stPage;
static char dispbuf[128] = {0};

// UI菜单个数
#define UI_MENU_NUM (sizeof(stMenuTab) / sizeof(UIMenuST))
// UI菜单
const UIMenuST stMenuTab[]=
{
    {"紫外灯设置", "UV Lamp", NULL},
    {"排水时间设置", "Drainage Time", NULL},
    {"语言切换", "Language", NULL},
    {"网络连接", "Network", NULL}
};

/**
 * @brief oled初始化
 * @param 无
 */
void oled_gpio_init(void)
{
    //初始化IO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_NUM_D0_CLK) | (1ULL << PIN_NUM_D1_MOSI) | (1ULL << PIN_NUM_RST) | (1ULL << PIN_NUM_DC) | (1ULL << PIN_NUM_CS),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE          // GPIO interrupt type 
    };
    gpio_config(&io_conf);
    gpio_set_level(PIN_NUM_CS, 0);
}

uint8_t gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        oled_gpio_init();                    
        break;
    case U8X8_MSG_GPIO_SPI_DATA:
        gpio_set_level(PIN_NUM_D1_MOSI, arg_int);
        break;
    case U8X8_MSG_GPIO_SPI_CLOCK:
        gpio_set_level(PIN_NUM_D0_CLK, arg_int);
        break;        
    case U8X8_MSG_GPIO_CS:  // CS默认接地
        break;
    case U8X8_MSG_GPIO_DC:
        gpio_set_level(PIN_NUM_DC, arg_int);
        break;
    case U8X8_MSG_GPIO_RESET:
        gpio_set_level(PIN_NUM_RST, arg_int);
        break;
    case U8X8_MSG_DELAY_MILLI:
        vTaskDelay(arg_int);	// 1000hz
        break;
    default:
        return 0;   //A message was received which is not implemented, return 0 to indicate an error
    }
  return 1;
}

/**
 * @brief 页面跳转
 * @param 跳转索引
 */
void oled_page_jump(uint8_t u8PageIndex)
{
    stPage.u8CurrPage = u8PageIndex;
    stPage.u8PageInitFlag = 0;
    stPage.u8CurrIndex = 0;
}

/**
 * @brief 按键处理
 * @param 无
 */
void oled_key_handle(void)
{

    key_msg_t key_msg;

    get_key_msg(&key_msg);
    if(stPage.u8CurrPage == 0)
    {//主页
        if(key_msg.key_state == KEY_STATE_SHORT)
        {//短按切换到下一页
            oled_page_jump(1);
        }
    }
    else if (stPage.u8CurrPage == 1)
    {//菜单页
        if(key_msg.key_state == KEY_STATE_LONG)
        {
            stPage.u8LongPressCnt += 4;
            if(stPage.u8LongPressCnt > 100)
            {
                stPage.u8LongPressCnt = 0;
                stPage.u8CurrPage = 2;
                if(stPage.u8CurrIndex == 3)
                {
                    smartconfig_start();
                }
                ESP_LOGI(TAG, "ENTER press");
            }        
        }
        if(key_msg.key_state == KEY_STATE_SHORT)
        {//短按切换到下一页
            stPage.u8LongPressCnt = 0;
            stPage.u8CurrIndex++;
            ESP_LOGI(TAG, "SHORTR press %d", stPage.u8CurrIndex);
            if(stPage.u8CurrIndex >= stPage.u8ItemTotalNum)
            {//
                oled_page_jump(0);
            }

        }
    }
    
}




// UI页面显示

/**
 * @brief 主页页面
 */
void oled_page_main(void)
{
    struct tm timeinfo;

    u8g2_ClearBuffer(&oled_handle);                // 清空缓冲区的内容  

    //初始化
    if(stPage.u8PageInitFlag == 0)
    {
        u8g2_SetFont(&oled_handle, u8g2_font_streamline_all_t); // 设置英文字体
        //显示wifi连接标志
        if(getWifiConnected() == 1)
        {
            u8g2_DrawGlyph(&oled_handle, 100, 22, 509);           
        }
        else
        {
            u8g2_DrawGlyph(&oled_handle, 100, 22, 399);           
        }
        //显示MQTT已连接标志
        if(mqtt_get_client_status() == 1)
        {
            u8g2_DrawGlyph(&oled_handle, 75, 22, 516);           
        }
        else
        {
            u8g2_DrawGlyph(&oled_handle, 75, 22, 518);           
        }
        //时间显示
        getcurrtime(&timeinfo);
        sprintf(dispbuf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        u8g2_SetFont(&oled_handle, u8g2_myfont_chinese); // 设置英文字体
        u8g2_DrawUTF8(&oled_handle, 0, 22, dispbuf);  
        //初始化完成
        stPage.u8PageInitFlag = 1;             
    }
    else
    {

        u8g2_SetFont(&oled_handle, u8g2_font_streamline_all_t); // 设置英文字体
        //显示wifi连接标志
        if(getWifiConnected() == 1)
        {
            u8g2_DrawGlyph(&oled_handle, 100, 22, 509);           
        }
        else
        {
            u8g2_DrawGlyph(&oled_handle, 100, 22, 400);           
        }
        //显示MQTT已连接标志
        if(mqtt_get_client_status() == 1)
        {
            u8g2_DrawGlyph(&oled_handle, 75, 22, 516);           
        }
        else
        {
            u8g2_DrawGlyph(&oled_handle, 75, 22, 518);           
        }
        //公司LOGO
        u8g2_DrawGlyph(&oled_handle, 16, 46, 585); 
        u8g2_SetFont(&oled_handle, u8g2_myfont_chinese); // 设置英文字体
        if(g_system_param.ucLanguage == 0)
        {//中文
            u8g2_DrawUTF8(&oled_handle, 40, 44, "宠物饮水机"); 
        }
        else
        {//英文
            u8g2_DrawUTF8(&oled_handle, 40, 44, "Pet Water"); 
        }
          
        //时间显示
        if(stPage.u8RefreshCnt%50 == 0)
        {
            getcurrtime(&timeinfo);
            sprintf(dispbuf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        }
        u8g2_SetFont(&oled_handle, u8g2_myfont_chinese); // 设置英文字体
        u8g2_DrawUTF8(&oled_handle, 0, 16, dispbuf); 


        

    }
    u8g2_SendBuffer(&oled_handle);		// 一定要发送buffer
}

/**
 * @brief 配网界面显示
 */
void oled_page_wifi_config(void)
{
    static uint16_t u8Cnt;

    u8g2_ClearBuffer(&oled_handle);
    u8g2_SetFont(&oled_handle, u8g2_font_streamline_all_t); // 图标
    u8g2_DrawGlyphX2(&oled_handle, 54, 42, 511);
    u8g2_SetFont(&oled_handle, u8g2_myfont_chinese);
    if(getWifiConnected() == 1)
    {
        u8g2_DrawUTF8(&oled_handle, 10, 60, "Connected!"); 
        if(u8Cnt > 1000)
        {
            u8Cnt = 0;
        }
        u8Cnt++;
        if(u8Cnt > 100)
        {
            u8Cnt = 0;
            stPage.u8CurrPage = 1; //回到菜单页面
        }          
    }
    else if (getWifiConnected() == 2)
    {
        u8g2_DrawUTF8(&oled_handle, 10, 60, "Connecting……"); 
        u8Cnt++;
        if(u8Cnt > 6100)
        {
            u8Cnt = 0;
            stPage.u8CurrPage = 1; //回到菜单页面
        }
        else if(u8Cnt > 6000)
        {
            u8g2_DrawUTF8(&oled_handle, 50, 60, "fail!"); 
        }
    }
    u8g2_SendBuffer(&oled_handle);		// 一定要发送buffer    
}

/**
 * @brief 菜单页显示
 */
void oled_page_menu(void)
{
    uint8_t u8StartMenuIndex = 0;
    uint8_t u8MenuPage = 0;

    u8StartMenuIndex = stPage.u8CurrIndex/3 * 3;

    u8g2_ClearBuffer(&oled_handle);
    u8g2_SetFont(&oled_handle, u8g2_myfont_chinese);
    if(g_system_param.ucLanguage == 0)
    {//中文
        {
            for(uint8_t i = 0; i < 3; i++)
            {
                if((u8StartMenuIndex+i) < stPage.u8ItemTotalNum)
                {
                    u8g2_DrawUTF8(&oled_handle, 24, 18+i*22, stMenuTab[u8StartMenuIndex+i].pcName); 
                }
            }

            u8g2_SetFont(&oled_handle, u8g2_font_streamline_all_t); // 图标
            u8g2_DrawGlyph(&oled_handle, 0, 22+(stPage.u8CurrIndex%3)*22, 445); 
        }

    }
    else
    {//英文
        {
            for(uint8_t i = 0; i < 3; i++)
            {
                if((u8StartMenuIndex+i) < stPage.u8ItemTotalNum)
                {
                    u8g2_DrawUTF8(&oled_handle, 24, 18+i*22, stMenuTab[u8StartMenuIndex+i].pcNameEn); 
                }
            }

            u8g2_SetFont(&oled_handle, u8g2_font_streamline_all_t); // 图标
            u8g2_DrawGlyph(&oled_handle, 0, 22+(stPage.u8CurrIndex%3)*22, 445); 
        }

    }
    // 确认进度
    u8g2_DrawBox(&oled_handle, 24, (stPage.u8CurrIndex%3)*22, stPage.u8LongPressCnt, 22);
    u8g2_SendBuffer(&oled_handle);		// 一定要发送buffer

}

void oled_app_task(void *pvParameters)
{
    stPage.u8ItemTotalNum = UI_MENU_NUM;
    ESP_LOGI(TAG, "oled_app_task[%d]", stPage.u8ItemTotalNum);
    while(1)
    {
        //按键处理
        oled_key_handle();
        if(stPage.u8CurrPage == 0)
        {
            oled_page_main();
        }
        else if(stPage.u8CurrPage == 1)
        {
            oled_page_menu();
        }
        else if(stPage.u8CurrPage == 2)
        {
            if(stPage.u8CurrIndex == 3)
            {
                oled_page_wifi_config();
            }
        }
        stPage.u8RefreshCnt++;
        if(stPage.u8RefreshCnt > 200)
        {//刷新
            stPage.u8RefreshCnt = 0;
        }
        vTaskDelay(10);
    }
}

/**
 * @brief oled app初始化
 * @param 无
 */
void oled_init(void)
{
    u8g2_Setup_ssd1306_128x64_noname_f(&oled_handle, U8G2_R0, u8x8_byte_4wire_sw_spi, gpio_and_delay_cb);  // 初始化u8g2
    u8g2_InitDisplay(&oled_handle);              // 初始化显示器
    u8g2_SetPowerSave(&oled_handle, 0);            // 唤醒显示器
    u8g2_ClearBuffer(&oled_handle);                // 清空缓冲区的内容       
    //创建显示线程
    xTaskCreate(oled_app_task, "oled_app_task", 4096, NULL, 5, NULL);                     
}

void oled_app_test(void)
{
    static uint16_t i;
    char str[200] = {0};
    ESP_LOGI(TAG, "oled app test");
    u8g2_ClearBuffer(&oled_handle);                // 清空缓冲区的内容                            
    //u8g2_DrawCircle(&oled_handle, 64, 32, i, U8G2_DRAW_ALL);    
    //u8g2_DrawCircle(&oled_handle, 32, 32, i, U8G2_DRAW_ALL);
    //u8g2_DrawCircle(&oled_handle, 96, 32, i, U8G2_DRAW_ALL);

    i++;
    if(i>688)
    {
        i = 32;
    }
    else if(i == 0)
    {
        i = 48;
    }
    //u8g2_DrawBox(&oled_handle, 0, 0, 128,64);
    //memset(oled_handle.tile_buf_ptr, 0xff, 512);
    sprintf(str, "排水周期:%d", i);
    u8g2_SetFont(&oled_handle, u8g2_myfont_chinese); // 设置英文字体
    u8g2_DrawUTF8(&oled_handle, 0, 16, str);
    u8g2_SetFont(&oled_handle, u8g2_font_streamline_all_t); // 设置英文字体
    u8g2_DrawGlyph(&oled_handle, 0, 32, i);

    u8g2_SendBuffer(&oled_handle);		// 一定要发送buffer

}


