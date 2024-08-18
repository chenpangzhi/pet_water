#include <stdio.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "oled_device.h"

// SPI pins
#define PIN_NUM_D1_MOSI 11
#define PIN_NUM_D0_CLK  12
#define PIN_NUM_CS   10
#define PIN_NUM_DC   13
#define PIN_NUM_RST  14

#define TAG "OLED_DEVICE"

static SSD1306_t dev;

void oled_device_init() 
{
    ESP_LOGI(TAG, "oled_device_init");

    spi_master_init(&dev, PIN_NUM_D1_MOSI, PIN_NUM_D0_CLK, PIN_NUM_CS, PIN_NUM_DC, PIN_NUM_RST);
    // SSD1306 initialization
    ssd1306_init(&dev, 128, 64);

    // Clear screen
    ssd1306_clear_screen(&dev, false);
}

void oled_Set_Pos(unsigned char x, unsigned char y) 
{ 
	spi_master_write_command(&dev, 0xb0+y);
	spi_master_write_command(&dev, ((x&0xf0)>>4)|0x10);
	spi_master_write_command(&dev, (x&0x0f)|0x01); 
}   	  
// Set pixel to internal buffer. Not show it.
void OLED_SetPixel (int xpos, int ypos, uint iColor)
{
	uint8_t _page = (ypos / 8);
	uint8_t _bits = (ypos % 8);
	uint8_t _seg = xpos;
	uint8_t wk0 = dev._page[_page]._segs[_seg];
	uint8_t wk1 = 1 << _bits;
	ESP_LOGD(TAG, "ypos=%d _page=%d _bits=%d wk0=0x%02x wk1=0x%02x", ypos, _page, _bits, wk0, wk1);
	if (iColor == 0) {
		wk0 = wk0 & ~wk1;
	} else {
		wk0 = wk0 | wk1;
	}
	if (dev._flip) wk0 = ssd1306_rotate_byte(wk0);
	ESP_LOGD(TAG, "wk0=0x%02x wk1=0x%02x", wk0, wk1);
	dev._page[_page]._segs[_seg] = wk0;
    oled_Set_Pos(_seg, _page);  
    spi_master_write_data(&dev, &wk0, 1);

}
int	OLED_GetPixel(int iPosX, int iPosY)
{
	uint8_t _page = (iPosY / 8);
	uint8_t _bits = (iPosY % 8);
	uint8_t _seg = iPosX;
	uint8_t wk0 = 0;
	uint8_t wk1 = 1 << _bits;
    int ret = 0;

    wk0 = dev._page[_page]._segs[_seg];
    if (dev._flip) wk0 = ssd1306_rotate_byte(wk0);
    ret = ((wk0 & wk1)>0) ? 1 : 0;
    return ret;
}
void oled_test()
{
    uint8_t x = 0;


    for(uint16_t j=0;j<64;j++)
    {
        OLED_SetPixel(j, j, 1);
    }
    x = 64;
    for(uint16_t j=64;j>0;j--)
    {
        OLED_SetPixel(x, j, 1);
        x++;
    }

}
