#ifndef __WATER_SENSOR_H__
#define __WATER_SENSOR_H__

extern void water_sensor_init(void);
extern int getClearWaterLevel(void);
extern int getDrinkWaterLevelLow(void);
extern int getDrinkWaterLevelHigh(void);
extern int getDrainWaterLevelHigh(void);

#endif