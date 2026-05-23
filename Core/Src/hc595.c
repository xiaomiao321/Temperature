/*
 * @file     hc595.c
 * @brief    74HC595 驱动数码管 - 支持两组 3 位数码管
 * @version  v2.0
 */
#include "hc595.h"

/* 共阳极段码表：0-9, A-F, 灭，点 */
/* 段顺序：a,b,c,d,e,f,g,dp (dp 为小数点) */
/* 共阳极：0 点亮，1 熄灭 */
const uint8_t SEG_CODE[] = {
    0xC0,  /* 0: a,b,c,d,e,f 亮 */
    0xF9,  /* 1: b,c 亮 */
    0xA4,  /* 2: a,b,d,e,g 亮 */
    0xB0,  /* 3: a,b,c,d,g 亮 */
    0x99,  /* 4: b,c,f,g 亮 */
    0x92,  /* 5: a,c,d,f,g 亮 */
    0x82,  /* 6: a,c,d,e,f,g 亮 */
    0xF8,  /* 7: a,b,c 亮 */
    0x80,  /* 8: 全亮 */
    0x90,  /* 9: a,b,c,d,f,g 亮 */
    0x88,  /* A: a,b,c,e,f,g 亮 */
    0x83,  /* B: c,d,e,f,g 亮 (小写 b) */
    0xC6,  /* C: a,d,e,f 亮 */
    0xA1,  /* D: b,c,d,e,f 亮 (小写 d) */
    0x86,  /* E: a,d,e,f,g 亮 */
    0x8E,  /* F: a,e,f,g 亮 */
    0xFF,  /* 灭：全灭 */
    0x7F   /* 点：只有小数点 */
};

/**
 * @brief  短延时 (用于 74HC595 时序)
 */
static void HC595_Delay(void)
{
    __NOP();
    __NOP();
    __NOP();
}

/* ========== 第一组数码管 (PA5-PA7) ========== */

/**
 * @brief  初始化第一组 74HC595 引脚
 */
void HC595_1_Init(void)
{
    /* 先拉低所有引脚 */
    HAL_GPIO_WritePin(HC595_1_PORT, HC595_1_DS_PIN | HC595_1_SHCP_PIN | HC595_1_STCP_PIN, GPIO_PIN_RESET);
    
    /* 发送 3 字节 0，清零移位寄存器 */
    HC595_1_SendByte(0x00);
    HC595_1_SendByte(0x00);
    HC595_1_SendByte(0x00);
    HC595_1_Latch();
}

/**
 * @brief  发送 1 字节数据到第一组 74HC595 (MSB First)
 */
void HC595_1_SendByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(HC595_1_PORT, HC595_1_SHCP_PIN, GPIO_PIN_RESET);
        if (data & 0x80)
            HAL_GPIO_WritePin(HC595_1_PORT, HC595_1_DS_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(HC595_1_PORT, HC595_1_DS_PIN, GPIO_PIN_RESET);
        data <<= 1;
        HC595_Delay();
        HAL_GPIO_WritePin(HC595_1_PORT, HC595_1_SHCP_PIN, GPIO_PIN_SET);
        HC595_Delay();
    }
    HAL_GPIO_WritePin(HC595_1_PORT, HC595_1_SHCP_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  第一组锁存数据到输出
 */
void HC595_1_Latch(void)
{
    HAL_GPIO_WritePin(HC595_1_PORT, HC595_1_STCP_PIN, GPIO_PIN_SET);
    HC595_Delay();
    HAL_GPIO_WritePin(HC595_1_PORT, HC595_1_STCP_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  显示 3 位数码管 - 第一组
 */
void HC595_1_Display3Digits(uint8_t d1, uint8_t d2, uint8_t d3)
{
    if (d1 > 16) d1 = 16;
    if (d2 > 16) d2 = 16;
    if (d3 > 16) d3 = 16;
    
    HC595_1_SendByte(SEG_CODE[d3]);
    HC595_1_SendByte(SEG_CODE[d2]);
    HC595_1_SendByte(SEG_CODE[d1]);
    HC595_1_Latch();
}

/**
 * @brief  显示数字 (0-999) - 第一组
 */
void HC595_1_DisplayNumber(uint16_t num)
{
    if (num > 999) num = 999;
    uint8_t d3 = num % 10;
    uint8_t d2 = (num / 10) % 10;
    uint8_t d1 = (num / 100) % 10;
    HC595_1_Display3Digits(d1, d2, d3);
}

/**
 * @brief  带闪烁效果的显示 - 第一组
 */
void HC595_1_Display3DigitsWithBlink(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t blinkPos, uint8_t blinkState)
{
    uint8_t show_d1 = d1, show_d2 = d2, show_d3 = d3;
    
    if (d1 > 16) d1 = 16;
    if (d2 > 16) d2 = 16;
    if (d3 > 16) d3 = 16;
    
    if (blinkState == 0) {
        switch (blinkPos) {
            case 0: show_d1 = 16; break;
            case 1: show_d2 = 16; break;
            case 2: show_d3 = 16; break;
        }
    }
    
    HC595_1_SendByte(SEG_CODE[show_d3]);
    HC595_1_SendByte(SEG_CODE[show_d2]);
    HC595_1_SendByte(SEG_CODE[show_d1]);
    HC595_1_Latch();
}

/* ========== 第二组数码管 (PB7-PB9) ========== */

/**
 * @brief  初始化第二组 74HC595 引脚
 */
void HC595_2_Init(void)
{
    /* 先拉低所有引脚 */
    HAL_GPIO_WritePin(HC595_2_PORT, HC595_2_DS_PIN | HC595_2_SHCP_PIN | HC595_2_STCP_PIN, GPIO_PIN_RESET);
    
    /* 发送 3 字节 0，清零移位寄存器 */
    HC595_2_SendByte(0x00);
    HC595_2_SendByte(0x00);
    HC595_2_SendByte(0x00);
    HC595_2_Latch();
}

/**
 * @brief  发送 1 字节数据到第二组 74HC595 (MSB First)
 */
void HC595_2_SendByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(HC595_2_PORT, HC595_2_SHCP_PIN, GPIO_PIN_RESET);
        if (data & 0x80)
            HAL_GPIO_WritePin(HC595_2_PORT, HC595_2_DS_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(HC595_2_PORT, HC595_2_DS_PIN, GPIO_PIN_RESET);
        data <<= 1;
        HC595_Delay();
        HAL_GPIO_WritePin(HC595_2_PORT, HC595_2_SHCP_PIN, GPIO_PIN_SET);
        HC595_Delay();
    }
    HAL_GPIO_WritePin(HC595_2_PORT, HC595_2_SHCP_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  第二组锁存数据到输出
 */
void HC595_2_Latch(void)
{
    HAL_GPIO_WritePin(HC595_2_PORT, HC595_2_STCP_PIN, GPIO_PIN_SET);
    HC595_Delay();
    HAL_GPIO_WritePin(HC595_2_PORT, HC595_2_STCP_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  显示 3 位数码管 - 第二组
 */
void HC595_2_Display3Digits(uint8_t d1, uint8_t d2, uint8_t d3)
{
    if (d1 > 16) d1 = 16;
    if (d2 > 16) d2 = 16;
    if (d3 > 16) d3 = 16;
    
    HC595_2_SendByte(SEG_CODE[d3]);
    HC595_2_SendByte(SEG_CODE[d2]);
    HC595_2_SendByte(SEG_CODE[d1]);
    HC595_2_Latch();
}

/**
 * @brief  显示数字 (0-999) - 第二组
 */
void HC595_2_DisplayNumber(uint16_t num)
{
    if (num > 999) num = 999;
    uint8_t d3 = num % 10;
    uint8_t d2 = (num / 10) % 10;
    uint8_t d1 = (num / 100) % 10;
    HC595_2_Display3Digits(d1, d2, d3);
}

/**
 * @brief  带闪烁效果的显示 - 第二组
 */
void HC595_2_Display3DigitsWithBlink(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t blinkPos, uint8_t blinkState)
{
    uint8_t show_d1 = d1, show_d2 = d2, show_d3 = d3;
    
    if (d1 > 16) d1 = 16;
    if (d2 > 16) d2 = 16;
    if (d3 > 16) d3 = 16;
    
    if (blinkState == 0) {
        switch (blinkPos) {
            case 0: show_d1 = 16; break;
            case 1: show_d2 = 16; break;
            case 2: show_d3 = 16; break;
        }
    }
    
    HC595_2_SendByte(SEG_CODE[show_d3]);
    HC595_2_SendByte(SEG_CODE[show_d2]);
    HC595_2_SendByte(SEG_CODE[show_d1]);
    HC595_2_Latch();
}

/* ========== 公共测试函数 ========== */

/**
 * @brief  测试函数：显示全 8 字 (两组同时显示)
 */
void HC595_Test888(void)
{
    /* 第一组 */
    HC595_1_SendByte(0x80);
    HC595_1_SendByte(0x80);
    HC595_1_SendByte(0x80);
    HC595_1_Latch();
    
    /* 第二组 */
    HC595_2_SendByte(0x80);
    HC595_2_SendByte(0x80);
    HC595_2_SendByte(0x80);
    HC595_2_Latch();
}

/**
 * @brief  测试函数：全灭 (两组同时)
 */
void HC595_TestAllOff(void)
{
    /* 第一组 */
    HC595_1_SendByte(0xFF);
    HC595_1_SendByte(0xFF);
    HC595_1_SendByte(0xFF);
    HC595_1_Latch();
    
    /* 第二组 */
    HC595_2_SendByte(0xFF);
    HC595_2_SendByte(0xFF);
    HC595_2_SendByte(0xFF);
    HC595_2_Latch();
}

/**
 * @brief  测试函数：全亮 (两组同时)
 */
void HC595_TestAllOn(void)
{
    /* 第一组 */
    HC595_1_SendByte(0x00);
    HC595_1_SendByte(0x00);
    HC595_1_SendByte(0x00);
    HC595_1_Latch();
    
    /* 第二组 */
    HC595_2_SendByte(0x00);
    HC595_2_SendByte(0x00);
    HC595_2_SendByte(0x00);
    HC595_2_Latch();
}
