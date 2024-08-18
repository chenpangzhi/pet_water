
file(GLOB_RECURSE MAIN_SOURCES "*.c")
idf_component_register(SRCS ${MAIN_SOURCES}
                            INCLUDE_DIRS ".")


#idf_component_register(SRCS "u8g2_myfont_chinese.c" "u8g2_d_setup.c" "u8x8_d_ssd1306_128x64_noname.c" "u8x8_gpio.c" "u8g2_d_memory.c" "u8x8_setup.c" "u8x8_display.c" "u8g2_setup.c" "key_app.c" "ssd1306.c" "oled_device.c" "nvsflash_app.c" "mqtt_app.c" "wifi_app.c" "env_sensor.c" "device_ctrl.c" "water_sensor.c" "main.c" "ssd1306_spi.c" "motor_device.c" "rtc_device.c" "key_app.c" "oled_app.c" "u8x8_byte.c"
#                       INCLUDE_DIRS ".")
