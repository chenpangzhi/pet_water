#ifndef __OLED_APP_H_
#define __OLED_APP_H_

#include "u8g2.h"
#include "u8x8.h"

//页面显示回调函数
typedef void (*OLED_APP_PAGE_CB)(uint16_t u16Para);
// 菜单管理结构体
typedef struct {
    //菜单中文名称
    const char *pcName;
    //菜单英文名称
    const char *pcNameEn;
    // 菜单数字输入页面回调函数
    OLED_APP_PAGE_CB pfnPageCb; //当前页面回调函数


} UIMenuST;

// 页面管理结构体
typedef struct {
    //当前页面
    uint8_t u8CurrPage;
    //页面初始化标志
    uint8_t u8PageInitFlag;
    //当前菜单选择位置
    uint8_t u8CurrIndex;
    //菜单项数
    uint8_t u8ItemTotalNum;
    //长按选中计时
    uint8_t u8LongPressCnt;
    //刷新计时
    uint8_t u8RefreshCnt;
} UI_PageST;

extern void oled_init(void);
extern void oled_app_test(void);

#endif