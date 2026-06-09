/*
 * @file     key.h
 * @brief    按键驱动模块 (基于 MultiButton)
 * @version  v2.0
 */

#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include <stdint.h>
#include "multi_button.h"

/* 按键 ID 定义 */
typedef enum {
    KEY_ID_SET = 1,      /* SET 确认键 - Btn1 (PB3) */
    KEY_ID_ADD = 2,      /* 加键 - Btn2 (PB4) */
    KEY_ID_SUB = 3,      /* 减键 - Btn3 (PB0) */
    KEY_ID_SHIFT = 4,    /* 左移切换键 - Btn4 (PB1) */
    KEY_ID_RESET = 5,    /* 复位键 - Reset_Btn (PA15) */
    KEY_ID_MANUAL = 6,   /* 手动报警键 - Manual_Btn (PA12) */
} KeyId_t;

/* 按键句柄 */
extern Button btn_set;
extern Button btn_add;
extern Button btn_sub;
extern Button btn_shift;
extern Button btn_reset;
extern Button btn_manual;

/* 按键事件标志 (供外部查询) */
extern volatile uint8_t key_set_pressed;
extern volatile uint8_t key_add_pressed;
extern volatile uint8_t key_sub_pressed;
extern volatile uint8_t key_shift_pressed;
extern volatile uint8_t key_reset_pressed;
extern volatile uint8_t key_manual_pressed;

/* 函数声明 */
void KEY_Init(void);
void KEY_Handler(void);  // 在主循环中调用，处理按键事件

/* GPIO 读取函数 - MultiButton 需要 */
uint8_t KEY_ReadPin(uint8_t button_id);

#endif /* __KEY_H */
