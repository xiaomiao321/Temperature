/*
 * @file     display.h
 * @brief    数码管显示管理模块
 * @version  v1.0
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "hc595.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 显示模式枚举 */
typedef enum {
    DISP_MODE_ROTATION = 0,    /* 轮询显示（默认） */
    DISP_MODE_ALARM,           /* 报警显示 */
    DISP_MODE_SETTING          /* 设置模式 */
} DispMode_t;

/* 函数声明 */
void Display_Init(void);
void Display_Handler(void);           /* 主循环调用 */
void Display_SetMode(DispMode_t mode);
DispMode_t Display_GetMode(void);
void Display_NotifyKeyPress(void);    /* 通知按键按下 */
void Display_CheckAlarm(void);        /* 检查是否进入报警模式 */

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H__ */
