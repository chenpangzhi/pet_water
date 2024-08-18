/**
 * @file motor_device.c
 * @brief 步进电机驱动 型号:28byj-48
 * @version 0.1
 * @date 2024-06-01
 */

#include "motor_device.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"

static const char *TAG = "motor_device";

//IN1~IN4 IO定义
#define MOTOR_DEVICE_IN1_IO 15
#define MOTOR_DEVICE_IN2_IO 16
#define MOTOR_DEVICE_IN3_IO 17
#define MOTOR_DEVICE_IN4_IO 18

// 开启、关闭步数
#define MOTOR_DEVICE_STEP_NUM 100

// 霍尔传感器IO定义 用于定位电机位置
//  关闭的霍尔
#define MOTOR_DEVICE_HALL1_IO 26
// 开启的霍尔    
#define MOTOR_DEVICE_HALL2_IO 35


// 舵机配置 预留
#define SERVO_PIN   GPIO_NUM_12
// LEDC 频道配置
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_FREQUENCY 50 // SG90 的控制频率为 50 Hz

// SG90 的 PWM 信号范围
#define SERVO_MIN_PULSEWIDTH 500 // 最小脉宽，500us
#define SERVO_MAX_PULSEWIDTH 2500 // 最大脉宽，2500us
#define SERVO_MAX_DEGREE 180 // SG90 最大角度

// 定义步进电机的步序列（4步和8步）
int step_sequence[8][4] = 
{
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

#if 0
//舵机
void servo_init() {
    // 配置 LEDC 定时器
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_14_BIT, // 设置 PWM 占空比分辨率为 14 位
        .freq_hz = LEDC_FREQUENCY, // PWM 频率为 50 Hz
        .speed_mode = LEDC_MODE, // LEDC 模式
        .timer_num = LEDC_TIMER, // 定时器编号
        .clk_cfg = LEDC_AUTO_CLK // 自动选择时钟源
    };
    ledc_timer_config(&ledc_timer);

    // 配置 LEDC 通道
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL,
        .duty = 0, // 初始化占空比为 0
        .gpio_num = SERVO_PIN,
        .speed_mode = LEDC_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER
    };
    ledc_channel_config(&ledc_channel);
}

uint32_t servo_angle_to_duty(uint32_t angle) {
    uint32_t duty = (SERVO_MIN_PULSEWIDTH +
                    ((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * angle) / SERVO_MAX_DEGREE);
    return (duty * 16384) / 20000; // 计算占空比，20000us 是 50Hz 的周期
}

void servo_set_angle(uint32_t angle) {
    if (angle > SERVO_MAX_DEGREE) {
        angle = SERVO_MAX_DEGREE;
    }
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
#endif
/**
 * @brief 步进电机驱动初始化
 * 
 */
void motor_device_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << MOTOR_DEVICE_IN1_IO) | (1ULL << MOTOR_DEVICE_IN2_IO)| ( 1ULL << MOTOR_DEVICE_IN3_IO) | ( 1ULL << MOTOR_DEVICE_IN4_IO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    //初始化霍尔
    gpio_config_t io_conf_hall;
    io_conf_hall.intr_type = GPIO_INTR_DISABLE;
    io_conf_hall.mode = GPIO_MODE_INPUT;
    io_conf_hall.pin_bit_mask = (1ULL << MOTOR_DEVICE_HALL1_IO) | (1ULL << MOTOR_DEVICE_HALL2_IO);
    io_conf_hall.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf_hall.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf_hall);

}

/**
 * @brief 获取霍尔状态
 * @return 0 未到位 1 已经关闭 2 已经开启
 */
int get_hall_status(void)
{
    if(gpio_get_level(MOTOR_DEVICE_HALL1_IO)==0)
    {
        return 1;
    }
    else if(gpio_get_level(MOTOR_DEVICE_HALL2_IO)==0)
    {
        return 2;
    }
    else
    {
        return 0;
   }
}

// 执行一个步进
void step(int step_index) 
{
    gpio_set_level(MOTOR_DEVICE_IN1_IO, step_sequence[step_index][0]);
    gpio_set_level(MOTOR_DEVICE_IN2_IO, step_sequence[step_index][1]);
    gpio_set_level(MOTOR_DEVICE_IN3_IO, step_sequence[step_index][2]);
    gpio_set_level(MOTOR_DEVICE_IN4_IO, step_sequence[step_index][3]);
}

// 顺时针旋转
void rotate_clockwise(int steps, int delay_ms) 
{
    for (int i = 0; i < steps; i++) 
    {
        if(get_hall_status() == 1)
        {
            break;
        }
        for (int j = 0; j < 8; j++) {
            step(j);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }        
    }
}

// 逆时针旋转
void rotate_counterclockwise(int steps, int delay_ms) {
    for (int i = 0; i < steps; i++) 
    {
        if(get_hall_status() == 2)
        {
            break;
        }
        for (int j = 7; j >= 0; j--) {
            step(j);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }

    }
}

/**
 * @brief 用步进电机开关阀门
 * @param 0关闭 1开启
 * 
 */
void motor_device_switch(int flag)
{
    if(0==flag)
    {
        rotate_clockwise(MOTOR_DEVICE_STEP_NUM, 1);
        
    }
    else
    {
        rotate_counterclockwise(MOTOR_DEVICE_STEP_NUM, 1);
    }
}

//测试函数
void motor_device_test(int steps, int flag)
{
    if(0==flag)
    {
        rotate_clockwise(steps, 1);
    }
    else
    {
        rotate_counterclockwise(steps, 1);
    }
}