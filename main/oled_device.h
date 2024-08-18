/* oled_device.h */
#ifndef OLED_DEVICE_H
#define OLED_DEVICE_H

extern void oled_device_init();
extern void oled_test();
extern void OLED_SetPixel (int xpos, int ypos, uint iColor);
extern int	OLED_GetPixel(int iPosX, int iPosY);


#endif