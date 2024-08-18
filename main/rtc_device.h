#ifndef _RTC_DEVICE_H_
#define _RTC_DEVICE_H_

typedef struct 
{
    unsigned char seconds;
    unsigned char  minutes;
    unsigned char  hours;
    unsigned char  day;
    unsigned char  month;
    unsigned char  year;
} ds1302_time_t;

extern void rtc_device_test(void);
extern void rtc_device_init(void);
extern void setcurrtime(int year, int month, int day, int hour, int minute, int second);
extern void getcurrtime(struct tm *timeinfo);
extern int get_night_status(void);

#endif