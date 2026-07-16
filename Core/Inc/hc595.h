/*
 * @file     hc595.h
 * @brief    74HC595 驱动数码管 - 支持两组 3 位数码管
 * @version  v2.1
 */
#ifndef __HC595_H
#define __HC595_H

#include "main.h"
#include <stdint.h>

/* ========== 用户配置区 ========== */
/* 第一组 74HC595 引脚定义 (PA5-PA7) */
#define HC595_1_PORT      GPIOA
#define HC595_1_DS_PIN    SDI1_Pin    /* 串行数据输入 DS */
#define HC595_1_SHCP_PIN  SCLK1_Pin   /* 移位寄存器时钟 SHCP */
#define HC595_1_STCP_PIN  LOAD1_Pin   /* 输出寄存器时钟 STCP */

/* 第二组 74HC595 引脚定义 (PB7-PB9) */
#define HC595_2_PORT      GPIOB
#define HC595_2_DS_PIN    SDI2_Pin    /* 串行数据输入 DS */
#define HC595_2_SHCP_PIN  SCLK2_Pin   /* 移位寄存器时钟 SHCP */
#define HC595_2_STCP_PIN  LOAD2_Pin   /* 输出寄存器时钟 STCP */

/* 数码管位数配置 */
#define SEG_DIGIT_COUNT   3           /* 每组 3 位数码管 */
/* ================================ */

/* 共阳极数码管段码表：0-9, A-F, 灭，点 */
extern const uint8_t SEG_CODE[];

/* 段码表索引 (共 29 项, 索引 0-28) */
#define SEG_COUNT         29
#define SEG_DOT_MASK      0x7F  /* 与段码 AND 可点亮小数点 */

/* 段码索引宏 (方便 display.c 等模块使用) */
#define SEG_idx_0         0
#define SEG_idx_1         1
#define SEG_idx_2         2
#define SEG_idx_3         3
#define SEG_idx_4         4
#define SEG_idx_5         5
#define SEG_idx_6         6
#define SEG_idx_7         7
#define SEG_idx_8         8
#define SEG_idx_9         9
#define SEG_idx_A         10
#define SEG_idx_b         11
#define SEG_idx_C         12
#define SEG_idx_d         13
#define SEG_idx_E         14
#define SEG_idx_F         15
#define SEG_idx_OFF       16
#define SEG_idx_DOT       17
#define SEG_idx_H         18
#define SEG_idx_L         19
#define SEG_idx_r         20
#define SEG_idx_n         21
#define SEG_idx_y         22
#define SEG_idx_MINUS     23
#define SEG_idx_I         24
#define SEG_idx_G         25
#define SEG_idx_o         26
#define SEG_idx_N         27
#define SEG_idx_M         28

/* 函数声明 - 第一组数码管 */
void HC595_1_Init(void);
void HC595_1_SendByte(uint8_t data);
void HC595_1_Latch(void);
void HC595_1_Display3Digits(uint8_t d1, uint8_t d2, uint8_t d3);
void HC595_1_DisplayNumber(uint16_t num);
void HC595_1_Display3DigitsWithBlink(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t blinkPos, uint8_t blinkState);
void HC595_1_Display3DigitsWithDot(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t dotPos);
void HC595_1_DisplayChannel(uint8_t channel);

/* 函数声明 - 第二组数码管 */
void HC595_2_Init(void);
void HC595_2_SendByte(uint8_t data);
void HC595_2_Latch(void);
void HC595_2_Display3Digits(uint8_t d1, uint8_t d2, uint8_t d3);
void HC595_2_DisplayNumber(uint16_t num);
void HC595_2_Display3DigitsWithBlink(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t blinkPos, uint8_t blinkState);
void HC595_2_Display3DigitsWithDot(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t dotPos);
void HC595_2_DisplayChannel(uint8_t channel);

/* 测试函数 */
void HC595_TestAllOn(void);    /* 全亮测试 */
void HC595_TestAllOff(void);   /* 全灭测试 */
void HC595_Test888(void);      /* 显示 888 */

#endif /* __HC595_H__ */
