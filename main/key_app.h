#ifndef _KEY_APP_H_
#define _KEY_APP_H_

//按键消息队列结构体
typedef struct
{
    char key_id;      //按键ID
    char key_state;   //按键状态
}key_msg_t;

//按键ID枚举 排水和显示
enum
{
    KEY_ID_WATER = 0,
    KEY_ID_DISPLAY,
    KEY_ID_MAX
};
//按键状态枚举 短按 长按
enum
{
    KEY_STATE_SHORT = 0,
    KEY_STATE_LONG,
    KEY_STATE_MAX，
};

extern void key_init(void);
extern void get_key_msg(key_msg_t *key_msg);
extern int get_key_water_msg(key_msg_t *key_msg);

#endif