#ifndef __MAIN_H__
#define __MAIN_H__

//主任务任务周期
#define MAIN_TASK_PERIOD 500
//排水时间默认值 24h
#define DEFAULT_DRAINAGE_TIME (86000*1000/MAIN_TASK_PERIOD)
//排完盆底水预计的时间
#define DEFAULT_WATER_ALL_CLEAN_TIME (5*1000/MAIN_TASK_PERIOD)
//加水最大的时间，防止传感器异常后加水停不下来
#define MAX_ADDWATER_TIME (60*1000/MAIN_TASK_PERIOD)

// 运行模式
typedef enum
{
    RUNMODE_IDLE = 0,
    RUNMODE_CYCLE,      //水循环模式
    RUNMODE_DRAINAGE,   //排水模式
    RUNMODE_ADDWATER,   //加水模式
} RunMode_E;

//系统运行参数结构体
typedef struct
{
    RunMode_E eRunMode;             //当前运行模式
    uint32_t ulDrainageTime;        //排水倒计时时间
} RunModeParam_ST;




#endif