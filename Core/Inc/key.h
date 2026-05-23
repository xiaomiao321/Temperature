/*
 * @file     key.h
 * @brief    按键驱动模块
 * @version  v1.0
 */

#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include <stdint.h>

/* 按键消抖延时 (ms) */
#define KEY_DEBOUNCE_DELAY  20

/* 按键状态 */
typedef enum {
    KEY_STATE_RELEASED = 1,  /* 按键释放 */
    KEY_STATE_PRESSED = 0    /* 按键按下 (低电平有效) */
} KeyState_t;

/* 按键 ID 定义 */
typedef enum {
    KEY_ID_SET = 0,    /* SET 确认键 - Btn1 */
    KEY_ID_ADD,        /* 加键 - Btn2 */
    KEY_ID_SUB,        /* 减键 - Btn3 */
    KEY_ID_SHIFT,      /* 左移切换键 - Btn4 */
    KEY_ID_RESET,      /* 复位键 - Reset_Btn */
    KEY_ID_COUNT       /* 按键总数 */
} KeyId_t;

/* 函数声明 */
void KEY_Init(void);
KeyState_t KEY_Read(KeyId_t key_id);
uint8_t KEY_Scan(KeyId_t key_id);  /* 扫描按键，返回 1 表示检测到按下 */

#endif /* __KEY_H */
