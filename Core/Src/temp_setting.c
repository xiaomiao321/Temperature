/*
 * @file     temp_setting.c
 * @brief    温度阈值设置模块
 * @version  v1.0
 */

#include "temp_setting.h"
#include "key.h"
#include "hc595.h"
#include "main.h"

/* 全局变量 */
static SetMode_t g_setMode = SET_MODE_NORMAL;     /* 当前模式 */
static SetPos_t g_setPos = SET_POS_UNIT;          /* 当前设置位 */
static uint16_t g_setValue = 30;                  /* 设置值 (默认 30℃) */
static uint8_t g_blinkState = 1;                  /* 闪烁状态 (1-亮，0-灭) */
static uint32_t g_lastBlinkTime = 0;              /* 上次闪烁时间 */

/* 调试用全局变量 - 可在调试器中查看 */
uint16_t DBG_setValue = 30;       /* 当前设置值 */
uint8_t DBG_setMode = 0;          /* 0=正常模式，1=设置模式 */
uint8_t DBG_setPos = 2;           /* 0=百位，1=十位，2=个位 */
uint8_t DBG_blinkState = 1;       /* 闪烁状态 */
uint8_t DBG_seg_d1 = 0;           /* 百位显示值 */
uint8_t DBG_seg_d2 = 0;           /* 十位显示值 */
uint8_t DBG_seg_d3 = 0;           /* 个位显示值 */

/* 外部温度阈值变量 (在 temperature.c 中定义) */
extern float temperature[8];
extern uint8_t temp_alarm;

/**
 * @brief  初始化设置模块
 */
void TEMP_SETTING_Init(void)
{
    /* 上电自检：显示测试序列 */
    HC595_Test888();      /* 显示 888 */
    HAL_Delay(500);
    HC595_TestAllOff();   /* 全灭 */
    HAL_Delay(200);
    HC595_DisplayNumber(123);  /* 显示 123 */
    HAL_Delay(500);
    HC595_DisplayNumber(456);  /* 显示 456 */
    HAL_Delay(500);
    HC595_DisplayNumber(0);    /* 显示 000 */
    HAL_Delay(300);
    
    g_setMode = SET_MODE_NORMAL;
    g_setPos = SET_POS_UNIT;
    g_blinkState = 1;
    g_lastBlinkTime = HAL_GetTick();
}

/**
 * @brief  处理 SET 键 (确认/切换模式)
 */
static void handle_set_key(void)
{
    if (KEY_Scan(KEY_ID_SET))
    {
        if (g_setMode == SET_MODE_NORMAL)
        {
            /* 进入设置模式，从个位开始 */
            g_setMode = SET_MODE_EDIT;
            g_setPos = SET_POS_UNIT;
            g_blinkState = 1;
            g_lastBlinkTime = HAL_GetTick();
        }
        else
        {
            /* 确认设置，返回正常模式 */
            g_setMode = SET_MODE_NORMAL;
            g_blinkState = 1;  /* 停止闪烁，全亮 */
            
            /* 更新温度阈值 */
            // 在 main.c 中处理
        }
    }
}

/**
 * @brief  处理加键
 */
static void handle_add_key(void)
{
    if (g_setMode != SET_MODE_EDIT)
    {
        return;
    }
    
    if (KEY_Scan(KEY_ID_ADD))
    {
        uint8_t digit;
        switch (g_setPos)
        {
            case SET_POS_HUNDRED:
                digit = (g_setValue / 100) % 10;
                digit = (digit + 1) % 10;
                g_setValue = (g_setValue % 100) + digit * 100;
                break;
            case SET_POS_TEN:
                digit = (g_setValue / 10) % 10;
                digit = (digit + 1) % 10;
                g_setValue = (g_setValue % 10) + digit * 10 + (g_setValue / 100) * 100;
                break;
            case SET_POS_UNIT:
                digit = g_setValue % 10;
                digit = (digit + 1) % 10;
                g_setValue = (g_setValue / 10) * 10 + digit;
                break;
        }
    }
}

/**
 * @brief  处理减键
 */
static void handle_sub_key(void)
{
    if (g_setMode != SET_MODE_EDIT)
    {
        return;
    }
    
    if (KEY_Scan(KEY_ID_SUB))
    {
        uint8_t digit;
        switch (g_setPos)
        {
            case SET_POS_HUNDRED:
                digit = (g_setValue / 100) % 10;
                digit = (digit + 9) % 10;  /* -1 等效于 +9 再取模 */
                g_setValue = (g_setValue % 100) + digit * 100;
                break;
            case SET_POS_TEN:
                digit = (g_setValue / 10) % 10;
                digit = (digit + 9) % 10;
                g_setValue = (g_setValue % 10) + digit * 10 + (g_setValue / 100) * 100;
                break;
            case SET_POS_UNIT:
                digit = g_setValue % 10;
                digit = (digit + 9) % 10;
                g_setValue = (g_setValue / 10) * 10 + digit;
                break;
        }
    }
}

/**
 * @brief  处理移位键 (切换位数)
 */
static void handle_shift_key(void)
{
    if (g_setMode != SET_MODE_EDIT)
    {
        return;
    }
    
    if (KEY_Scan(KEY_ID_SHIFT))
    {
        /* 切换设置位置：个位 -> 十位 -> 百位 -> 个位 循环 */
        g_setPos = (SetPos_t)((g_setPos + 1) % 3);
        
        /* 重置闪烁状态和时间 */
        g_blinkState = 1;
        g_lastBlinkTime = HAL_GetTick();
    }
}

/**
 * @brief  处理闪烁逻辑
 */
static void handle_blink(void)
{
    if (g_setMode == SET_MODE_EDIT)
    {
        uint32_t currTime = HAL_GetTick();
        if (currTime - g_lastBlinkTime >= SETTING_BLINK_INTERVAL)
        {
            g_blinkState = !g_blinkState;
            g_lastBlinkTime = currTime;
        }
    }
}

/**
 * @brief  更新显示
 */
void TEMP_SETTING_Display(void)
{
    uint8_t d1, d2, d3;  /* 百位、十位、个位 */

    /* 分解数值 */
    d3 = g_setValue % 10;         /* 个位 */
    d2 = (g_setValue / 10) % 10;  /* 十位 */
    d1 = (g_setValue / 100) % 10; /* 百位 */

    /* 更新调试变量 */
    DBG_setValue = g_setValue;
    DBG_setMode = (uint8_t)g_setMode;
    DBG_setPos = (uint8_t)g_setPos;
    DBG_blinkState = g_blinkState;
    DBG_seg_d1 = d1;
    DBG_seg_d2 = d2;
    DBG_seg_d3 = d3;

    if (g_setMode == SET_MODE_NORMAL)
    {
        /* 正常模式 - 直接显示 */
        HC595_Display3Digits(d1, d2, d3);
    }
    else
    {
        /* 设置模式 - 带闪烁显示 */
        HC595_Display3DigitsWithBlink(d1, d2, d3, (uint8_t)g_setPos, g_blinkState);
    }
}

/**
 * @brief  按键处理主函数
 */
void TEMP_SETTING_Handler(void)
{
    handle_set_key();
    handle_add_key();
    handle_sub_key();
    handle_shift_key();
    handle_blink();
}

/**
 * @brief  获取设置值
 * @retval 设置的阈值 (0-999)
 */
uint16_t TEMP_SETTING_GetValue(void)
{
    return g_setValue;
}
