/*
 * @file     hc595.h
 * @brief    74HC595 驱动数码管 - 基于 HAL 库
 * @version  v1.0
 * @date     2026-05-17
 */
#ifndef __HC595_H
#define __HC595_H

#include "main.h"
#include <stdint.h>

/* ========== 用户配置区 ========== */
/* 引脚定义 (与 CubeMX 配置一致) */
#define HC595_PORT        GPIOA
#define HC595_DS_PIN      GPIO_PIN_5      /* 串行数据输入 DS */
#define HC595_SHCP_PIN    GPIO_PIN_6      /* 移位寄存器时钟 SHCP */
#define HC595_STCP_PIN    GPIO_PIN_7      /* 输出寄存器时钟 STCP */

/* 数码管位数配置 */
#define SEG_DIGIT_COUNT   3               /* 3 位数码管 */
/* ================================ */

/* 共阳极数码管段码表：0-9, A-F, 灭，点 */
extern const uint8_t SEG_CODE[];

/* 函数声明 */
void HC595_Init(void);
void HC595_SendByte(uint8_t data);
void HC595_Latch(void);
void HC595_Display(uint8_t num);
void HC595_DisplayRaw(uint8_t seg_code);
void HC595_SendMultiBytes(uint8_t *data, uint8_t count);

/* 3 位数码管显示函数 */
void HC595_Display3Digits(uint8_t d1, uint8_t d2, uint8_t d3);
void HC595_DisplayNumber(uint16_t num);

/* 带闪烁效果的显示函数 (用于设置模式) */
/* blinkPos: 0-百位，1-十位，2-个位，3-不闪烁 */
/* blinkState: 1-显示，0-熄灭 (闪烁用) */
void HC595_Display3DigitsWithBlink(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t blinkPos, uint8_t blinkState);

/* 测试函数 */
void HC595_TestAllOn(void);    /* 全亮测试 */
void HC595_TestAllOff(void);   /* 全灭测试 */
void HC595_Test888(void);      /* 显示 888 */

#endif /* __HC595_H */
