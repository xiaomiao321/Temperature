/*
 * @file     temp_setting.h
 * @brief    温度阈值设置模块 - 支持两组独立阈值
 * @version  v2.0
 */

#ifndef __TEMP_SETTING_H
#define __TEMP_SETTING_H

#include <stdint.h>

/* 闪烁间隔 (ms) */
#define SETTING_BLINK_INTERVAL  300

/* 长按阈值 (ms) */
#define LONG_PRESS_THRESHOLD    1000

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

/* 当前设置的数码管组 (0-第一组，1-第二组) */
typedef enum {
    SET_GROUP_1 = 0,
    SET_GROUP_2 = 1
} SetGroup_t;

/* 调试用全局变量 */
extern uint16_t DBG_setValue1;      /* 第一组设置值 */
extern uint16_t DBG_setValue2;      /* 第二组设置值 */
extern uint8_t DBG_setMode;         /* 0=正常模式，1=设置模式 */
extern uint8_t DBG_setPos;          /* 0=百位，1=十位，2=个位 */
extern uint8_t DBG_blinkState;      /* 闪烁状态 */
extern uint8_t DBG_currentGroup;    /* 当前设置的组 */
extern uint8_t DBG_seg_d1;          /* 百位显示值 */
extern uint8_t DBG_seg_d2;          /* 十位显示值 */
extern uint8_t DBG_seg_d3;          /* 个位显示值 */

/* 函数声明 */
void TEMP_SETTING_Init(void);
void TEMP_SETTING_Handler(void);  /* 按键处理 */
uint16_t TEMP_SETTING_GetValue1(void);  /* 获取第一组设置值 */
uint16_t TEMP_SETTING_GetValue2(void);  /* 获取第二组设置值 */
void TEMP_SETTING_Display(void);  /* 更新显示 */
SetGroup_t TEMP_SETTING_GetCurrentGroup(void);  /* 获取当前设置的组 */

#endif /* __TEMP_SETTING_H */
