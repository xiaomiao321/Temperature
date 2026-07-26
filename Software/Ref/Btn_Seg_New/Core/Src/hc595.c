/*
 * @file     hc595.c
 * @brief    74HC595 驱动数码管 - 基于 HAL 库
 * @version  v1.0
 * @date     2026-05-17
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

/**
 * @brief  初始化 74HC595 引脚
 */
void HC595_Init(void)
{
    /* CubeMX 已初始化 GPIO，确保输出低电平 */
    HAL_GPIO_WritePin(HC595_PORT, HC595_DS_PIN | HC595_SHCP_PIN | HC595_STCP_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  发送 1 字节数据到 74HC595 (MSB First)
 * @param  data: 要发送的数据
 */
void HC595_SendByte(uint8_t data)
{
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        /* 1. 拉低移位时钟 */
        HAL_GPIO_WritePin(HC595_PORT, HC595_SHCP_PIN, GPIO_PIN_RESET);

        /* 2. 设置数据位 (MSB First) */
        if(data & 0x80)
            HAL_GPIO_WritePin(HC595_PORT, HC595_DS_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(HC595_PORT, HC595_DS_PIN, GPIO_PIN_RESET);

        /* 3. 数据移位 */
        data <<= 1;

        /* 4. 短延时 */
        HC595_Delay();

        /* 5. 拉高移位时钟 (上升沿移位) */
        HAL_GPIO_WritePin(HC595_PORT, HC595_SHCP_PIN, GPIO_PIN_SET);

        /* 6. 短延时 */
        HC595_Delay();
    }

    /* 确保最终状态 */
    HAL_GPIO_WritePin(HC595_PORT, HC595_SHCP_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  锁存数据到输出
 */
void HC595_Latch(void)
{
    HAL_GPIO_WritePin(HC595_PORT, HC595_STCP_PIN, GPIO_PIN_SET);
    HC595_Delay();
    HAL_GPIO_WritePin(HC595_PORT, HC595_STCP_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  显示单个数字 (0-15)
 * @param  num: 要显示的数字 (0-15)
 */
void HC595_Display(uint8_t num)
{
    if(num > 15) num = 15;
    HC595_SendByte(SEG_CODE[num]);
    HC595_Latch();
}

/**
 * @brief  显示原始段码
 * @param  seg_code: 段码值
 */
void HC595_DisplayRaw(uint8_t seg_code)
{
    HC595_SendByte(seg_code);
    HC595_Latch();
}

/**
 * @brief  发送多字节 (用于级联)
 * @param  data: 数据缓冲区
 * @param  count: 字节数
 */
void HC595_SendMultiBytes(uint8_t *data, uint8_t count)
{
    uint8_t i;
    for(i = 0; i < count; i++)
    {
        HC595_SendByte(data[i]);
    }
    HC595_Latch();
}

/**
 * @brief  显示 3 位数码管 (3 片 74HC595 级联)
 * @param  d1: 第 1 位数字 (最左边，百位)
 * @param  d2: 第 2 位数字 (中间，十位)
 * @param  d3: 第 3 位数字 (最右边，个位)
 * @note   级联时，后发送的数据在最远端
 *         顺序：个位 (最近) → 十位 → 百位 (最远)
 */
void HC595_Display3Digits(uint8_t d1, uint8_t d2, uint8_t d3)
{
    /* 限制数字范围 0-15 */
    if(d1 > 15) d1 = 15;
    if(d2 > 15) d2 = 15;
    if(d3 > 15) d3 = 15;

    /* 发送段码数据 - 先发送最近端 */
    HC595_SendByte(SEG_CODE[d3]);  /* 个位 - 第 3 片 (最近端) */
    HC595_SendByte(SEG_CODE[d2]);  /* 十位 - 第 2 片 */
    HC595_SendByte(SEG_CODE[d1]);  /* 百位 - 第 1 片 (最远端) */
    HC595_Latch();
}

/**
 * @brief  显示 3 位数字 (0-999)
 * @param  num: 要显示的数字 (0-999)
 */
void HC595_DisplayNumber(uint16_t num)
{
    uint8_t d1, d2, d3;

    if(num > 999) num = 999;

    d3 = num % 10;        /* 个位 */
    d2 = (num / 10) % 10; /* 十位 */
    d1 = (num / 100) % 10;/* 百位 */

    HC595_Display3Digits(d1, d2, d3);
}

/**
 * @brief  带闪烁效果的 3 位数码管显示 (用于设置模式)
 * @param  d1: 第 1 位数字 (百位)
 * @param  d2: 第 2 位数字 (十位)
 * @param  d3: 第 3 位数字 (个位)
 * @param  blinkPos: 闪烁位置 (0-百位，1-十位，2-个位，3-不闪烁)
 * @param  blinkState: 闪烁状态 (1-显示，0-熄灭)
 * @note   当 blinkState=0 且 blinkPos 在有效范围内时，对应位熄灭实现闪烁效果
 */
void HC595_Display3DigitsWithBlink(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t blinkPos, uint8_t blinkState)
{
    uint8_t show_d1 = d1, show_d2 = d2, show_d3 = d3;

    /* 限制数字范围 0-16(16 是灭码) */
    if(d1 > 16) d1 = 16;
    if(d2 > 16) d2 = 16;
    if(d3 > 16) d3 = 16;

    /* 根据闪烁位置和状态，决定是否熄灭对应位 */
    if(blinkState == 0)
    {
        switch(blinkPos)
        {
            case 0: show_d1 = 16; break;  /* 百位闪烁 - 熄灭 (16 是灭码) */
            case 1: show_d2 = 16; break;  /* 十位闪烁 - 熄灭 */
            case 2: show_d3 = 16; break;  /* 个位闪烁 - 熄灭 */
            default: break;               /* 不闪烁 */
        }
    }

    /* 发送段码数据 - 先发送最近端 */
    HC595_SendByte(SEG_CODE[show_d3]);  /* 个位 */
    HC595_SendByte(SEG_CODE[show_d2]);  /* 十位 */
    HC595_SendByte(SEG_CODE[show_d1]);  /* 百位 */
    HC595_Latch();
}

/**
 * @brief  测试函数：显示全 8 字 (所有段点亮)
 */
void HC595_TestAllOn(void)
{
    HC595_SendByte(0x00);  /* 0x00 = 所有段点亮 (共阳) */
    HC595_SendByte(0x00);
    HC595_SendByte(0x00);
    HC595_Latch();
}

/**
 * @brief  测试函数：显示全灭
 */
void HC595_TestAllOff(void)
{
    HC595_SendByte(0xFF);  /* 0xFF = 所有段熄灭 (共阳) */
    HC595_SendByte(0xFF);
    HC595_SendByte(0xFF);
    HC595_Latch();
}

/**
 * @brief  测试函数：显示 888
 */
void HC595_Test888(void)
{
    HC595_SendByte(0x80);  /* 8 */
    HC595_SendByte(0x80);
    HC595_SendByte(0x80);
    HC595_Latch();
}
