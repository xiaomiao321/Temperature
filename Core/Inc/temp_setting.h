/*
 * @file     temp_setting.h
 * @brief    温度阈值设置模块
 * @version  v1.0
 */

#ifndef __TEMP_SETTING_H
#define __TEMP_SETTING_H

#include <stdint.h>

/* 闪烁间隔 (ms) */
#define SETTING_BLINK_INTERVAL  300

/* 设置模式 */
typedef enum {
    SET_MODE_NORMAL = 0,    /* 正常显示模式 */
    SET_MODE_EDIT           /* 设置编辑模式 */
} SetMode_t;

/* 设置位置 (0-百位，1-十位，2-个位) */
typedef enum {
    SET_POS_HUNDRED = 0,
    SET_POS_TEN = 1,
    SET_POS_UNIT = 2
} SetPos_t;

/* 调试用全局变量 */
extern uint16_t DBG_setValue;       /* 当前设置值 */
extern uint8_t DBG_setMode;         /* 0=正常模式，1=设置模式 */
extern uint8_t DBG_setPos;          /* 0=百位，1=十位，2=个位 */
extern uint8_t DBG_blinkState;      /* 闪烁状态 */
extern uint8_t DBG_seg_d1;          /* 百位显示值 */
extern uint8_t DBG_seg_d2;          /* 十位显示值 */
extern uint8_t DBG_seg_d3;          /* 个位显示值 */

/* 函数声明 */
void TEMP_SETTING_Init(void);
void TEMP_SETTING_Handler(void);  /* 按键处理 */
uint16_t TEMP_SETTING_GetValue(void);  /* 获取设置值 */
void TEMP_SETTING_Display(void);  /* 更新显示 */

#endif /* __TEMP_SETTING_H */
