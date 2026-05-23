/*
 * @file     hc595.h
 * @brief    74HC595 驱动数码管 - 支持两组 3 位数码管
 * @version  v2.0
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

/* 函数声明 - 第一组数码管 */
void HC595_1_Init(void);
void HC595_1_SendByte(uint8_t data);
void HC595_1_Latch(void);
void HC595_1_Display3Digits(uint8_t d1, uint8_t d2, uint8_t d3);
void HC595_1_DisplayNumber(uint16_t num);
void HC595_1_Display3DigitsWithBlink(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t blinkPos, uint8_t blinkState);

/* 函数声明 - 第二组数码管 */
void HC595_2_Init(void);
void HC595_2_SendByte(uint8_t data);
void HC595_2_Latch(void);
void HC595_2_Display3Digits(uint8_t d1, uint8_t d2, uint8_t d3);
void HC595_2_DisplayNumber(uint16_t num);
void HC595_2_Display3DigitsWithBlink(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t blinkPos, uint8_t blinkState);

/* 测试函数 */
void HC595_TestAllOn(void);    /* 全亮测试 */
void HC595_TestAllOff(void);   /* 全灭测试 */
void HC595_Test888(void);      /* 显示 888 */

#endif /* __HC595_H */
