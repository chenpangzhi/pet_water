#ifndef _MOTOR_DEVICE_H_
#define _MOTOR_DEVICE_H_

extern void motor_device_init(void);

extern void motor_device_test(int steps, int flag);
extern void motor_device_switch(int flag);
extern int get_hall_status(void);


#endif