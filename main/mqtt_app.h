#ifndef __MQTT_APP_H__
#define __MQTT_APP_H__




extern void mqtt_app_start(void);
extern unsigned char mqtt_get_client_status(void);
extern void mqtt_set_client_status(uint16_t status);
extern uint16_t mqtt_get_debug_status(void);

extern void mqtt_send_log(const char *log);
extern void mqtt_app_init(void);;

#endif