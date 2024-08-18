#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "device_ctrl.h"
#include "env_sensor.h"
#include "water_sensor.h"
#include "main.h"
#include "wifi_app.h"
#include "nvsflash_app.h"
#include "mqtt_app.h"
#include "oled_device.h"
#include "motor_device.h"
#include "rtc_device.h"
#include "key_app.h"
#include "oled_app.h"

static const char *TAG = "cyp";

static RunModeParam_ST g_stRunModeParam;


/**
 * @brief:  IDLE模式处理，循环判断进行模式切换
 * @param: void 
 * @return: RunMode_E 当前需要运行的模式
 * @note:   
*/
RunMode_E IdleModeProcess(void)
{
    RunMode_E eRunMode = RUNMODE_IDLE;

    //水盘无水，净水箱有水的时候进入加水模式
    if (getClearWaterLevel() == 0 && getDrinkWaterLevelLow() == 1)
    {
        eRunMode = RUNMODE_ADDWATER;
    }
    //水盘有水，且热释电红外传感器检测到宠物在喝水，开启水循环
    else if (getDrinkWaterLevelLow() == 0 && get_ir_sw() == 1)
    {
        eRunMode = RUNMODE_CYCLE;
    }
    //时间达到24H时进入排水模式
    else if (g_stRunModeParam.ulDrainageTime == 0)
    {
        eRunMode = RUNMODE_DRAINAGE;
    }
    return eRunMode;
}

/**
 * @brief:  水循环模式处理
 * @param: void 
 * @return: RunMode_E
 * @note:   
*/
RunMode_E CycleModeProcess(void)
{    
    RunMode_E eRunMode = RUNMODE_IDLE;

    //关闭净水箱水泵
    device_ctrl_bump1_close();
    //开启水循环水泵
    device_ctrl_bump2_open();
    //关闭水阀
    motor_device_switch(0);
    return eRunMode;
}

/**
 * @brief:  排水模式处理
 * @param: void 
 * @return: RunMode_E
 * @note:   
*/
RunMode_E DrainageModeProcess(void)
{
    RunMode_E eRunMode = RUNMODE_IDLE;
    static uint16_t ulWaterAllCleanTime;     //排水到低水位后延时等待水排完
    static uint8_t ucflag;

    if(g_stRunModeParam.ulDrainageTime == 0)
    {
        //关闭所有水泵开关
        device_ctrl_bump1_close();
        device_ctrl_bump2_close();
        //开启排水的水阀
        if(ucflag == 0)
        {
            if(getDrainWaterLevelHigh() == 0)
            {
                motor_device_switch(1);
                ucflag = 1;
            }
        }

        if(getDrainWaterLevelHigh() == 1)
        {//废水箱满了，停止排水
            ulWaterAllCleanTime = 0;
            motor_device_switch(0);
            g_stRunModeParam.ulDrainageTime = DEFAULT_DRAINAGE_TIME;
            ucflag = 0;
            eRunMode = RUNMODE_IDLE;
        }
        else if(getDrinkWaterLevelLow() == 1)
        {//达到低水位后开始计时 等待最后一点盆地下的水排完
            ulWaterAllCleanTime++;
            if(ulWaterAllCleanTime > DEFAULT_WATER_ALL_CLEAN_TIME)
            {
                ulWaterAllCleanTime = 0;
                motor_device_switch(0);
                g_stRunModeParam.ulDrainageTime = DEFAULT_DRAINAGE_TIME;
                ucflag = 0;
                eRunMode = RUNMODE_IDLE;
            }
            else
            {
                eRunMode = RUNMODE_DRAINAGE;
            }
        }
    }

    return eRunMode;
}

/**
 * @brief:  加水模式处理
 * @param: void 
 * @return: RunMode_E
 * @note:   
*/
RunMode_E AddWaterModeProcess(void)
{
    RunMode_E eRunMode = RUNMODE_IDLE;
    static uint32_t ulAddWaterTime;//加水时间计时

    //净水箱没水，关闭净水水泵，直接停止加水
    if(getClearWaterLevel() == 0)
    {
        eRunMode = RUNMODE_IDLE;
        device_ctrl_bump1_close();

    }
    // 水盆还没到高水位，继续加水
    else if(getDrinkWaterLevelHigh() == 0)
    {
        device_ctrl_bump1_open();
        device_ctrl_bump2_close();
        //device_ctrl_valve_close();

        ulAddWaterTime ++;
        if(ulAddWaterTime > MAX_ADDWATER_TIME)
        {
            ulAddWaterTime = 0;
            g_stRunModeParam.eRunMode = RUNMODE_IDLE;
            device_ctrl_bump1_close();
            device_ctrl_bump2_close();
            //device_ctrl_valve_close();
        }
        else
        {
            eRunMode = RUNMODE_ADDWATER;
        }
    }
    //已经到高水位关闭水泵退出加水模式
    else if(getDrinkWaterLevelHigh() == 1)
    {
        device_ctrl_bump1_close();
        device_ctrl_bump2_close();
        //device_ctrl_valve_close();
        g_stRunModeParam.eRunMode = RUNMODE_IDLE;
    }
    return eRunMode;

}

/**
 * @brief:  其他循环处理的控制，例如开关紫外线灯
 * @param: void 
 * @return: void
 * @note:   
*/
void OtherProcess(void)
{
    //有水且宠物不在时开紫外线灯
    if(getDrinkWaterLevelLow() == 0 && get_ir_sw() == 0)
    {
        if(get_night_status() != 1)
        {
            device_ctrl_uv_switch(1);
        }
    }
    else
    {
        device_ctrl_uv_switch(0);
    }
    
}

/**
 * @brief:  创建主任务,循环处理加水、水循环等主要任务
 * @param: void * pvParameters
 * @return: void
 * @note:   
*/
void mainTask(void *pvParameters)
{
    g_stRunModeParam.ulDrainageTime = DEFAULT_DRAINAGE_TIME;
    key_msg_t key_msg;

    while (1)
    {
        if(mqtt_get_debug_status() == 1)
        {
            vTaskDelay(MAIN_TASK_PERIOD*2); // 500ms
        }
        else
        {
            OtherProcess();
            //排水按键处理(高优先级)
            if(get_key_water_msg(&key_msg) == pdTRUE)
            {
                if(key_msg.key_state == KEY_STATE_LONG)
                {
                    g_stRunModeParam.eRunMode = RUNMODE_DRAINAGE;
                    g_stRunModeParam.ulDrainageTime = 0;
                }
            }


            //ESP_LOGI(TAG, "This is a main task message");
            switch (g_stRunModeParam.eRunMode)
            {
            case RUNMODE_IDLE:
                //ESP_LOGI(TAG, "RUNMODE_IDLE");
                g_stRunModeParam.eRunMode = IdleModeProcess();
                break;
            case RUNMODE_CYCLE:
                g_stRunModeParam.eRunMode = CycleModeProcess();
                break;
            case RUNMODE_DRAINAGE:
                ESP_LOGI(TAG, "RUNMODE_DRAINAGE");
                g_stRunModeParam.eRunMode = DrainageModeProcess();
                break;
            case RUNMODE_ADDWATER:
                g_stRunModeParam.eRunMode = AddWaterModeProcess();
                break;
            default:
                break;
            }    
            //排水时间倒计时
            if (g_stRunModeParam.ulDrainageTime > 0)
            {
                g_stRunModeParam.ulDrainageTime--;
            }

            vTaskDelay(MAIN_TASK_PERIOD); // 500ms

        }
    }
}

void MqttTask(void *pvParameters)
{
    uint16_t ulCount = 0;
    wifi_app_init();
    mqtt_app_init();
    while (1)
    {            
        //每30s连接一次wifi
        if (ulCount%30 == 0)
        {
            if(getWifiConnected() == 0)
            {
                wifi_conn_start();
            }
        }
        //ESP_LOGI(TAG, "This is a test task message");
        if(getWifiConnected() == 1)
        {        
            vTaskDelay(2000); // 暂停20秒钟

            if(mqtt_get_client_status() == 0)
            {
                mqtt_app_start();
            }
        }
        else 
        {
            mqtt_set_client_status(0);
            ESP_LOGI(TAG, "wifi not connected");
            if(getWifiConnected()!=2)
            {
                //smartconfig_start();
            }
        }
        vTaskDelay(1000); // 暂停20秒钟
    }
}

void app_main(void)
{
    static uint32_t ulFlag;
    //初始化nvss flash
    nvsflash_app_init();
    //初始化触摸传感器
    water_sensor_init();
    oled_device_init();
    env_sensor_init();
    device_ctrl_init();
    motor_device_init();
    rtc_device_init();
    key_init();
    // 初始化FreeRTOS
    xTaskCreate(mainTask, "mainTask", configMINIMAL_STACK_SIZE * 40, NULL, 1, NULL);
    //测试
    xTaskCreate(MqttTask, "MqttTask", configMINIMAL_STACK_SIZE * 20, NULL, 1, NULL);
    // 启动任务调度器
    //vTaskDelay(10000); // 暂停1秒钟
    oled_init();
    while (1)
    {      
          
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pin_bit_mask = (1ULL << 36);

        gpio_config(&io_conf);
        ulFlag = !ulFlag;
        gpio_set_level(36, ulFlag);
        if(mqtt_get_client_status() == 1)
        {//mqtt已经连接
            //mqtt_send_log("cyptest");
        }
        //oled_app_test();
        //rtc_device_test();
        vTaskDelay(1000);
    }
}
