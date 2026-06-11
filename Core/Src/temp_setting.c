/*
 * @file     temp_setting.c
 * @brief    温度阈值设置模块 - 支持两组独立阈值
 * @version  v2.1
 */

#include "temp_setting.h"
#include "key.h"
#include "hc595.h"
#include "main.h"
#include "temperature.h"
#include "stm32f1xx_hal.h"

/* ========== Flash 存储配置 ========== */
/* 使用 Flash 最后一页存储阈值 (STM32F103xB Flash 大小 64KB) */
#define FLASH_SAVE_ADDR  (0x08000000 + 0xFC00)  /* 最后 1KB */
#define FLASH_MAGIC_NUM  0xA5A5                  /* 魔数，标识数据已保存 */

/* 全局变量 */
static SetMode_t g_setMode = SET_MODE_NORMAL;     /* 当前模式 */
static SetPos_t g_setPos = SET_POS_UNIT;          /* 当前设置位 */
static SetGroup_t g_currentGroup = SET_GROUP_1;   /* 当前设置的组 */
static uint16_t g_setValue1 = 30;                 /* 第一组设置值 (默认 30℃) */
static uint16_t g_setValue2 = 30;                 /* 第二组设置值 (默认 30℃) */
static uint8_t g_blinkState = 1;                  /* 闪烁状态 (1-亮，0-灭) */
static uint32_t g_lastBlinkTime = 0;              /* 上次闪烁时间 */
static uint32_t g_setPressTime = 0;               /* SET 键按下时间 */
static uint8_t g_longPressDetected = 0;           /* 长按标志 */

/**
 * @brief  从 Flash 加载阈值
 */
static void TEMP_SETTING_LoadFromFlash(void)
{
    uint16_t magic = *(__IO uint16_t*)FLASH_SAVE_ADDR;
    
    if (magic == FLASH_MAGIC_NUM) {
        /* 数据有效，加载阈值 */
        uint16_t val1 = *(__IO uint16_t*)(FLASH_SAVE_ADDR + 2);
        uint16_t val2 = *(__IO uint16_t*)(FLASH_SAVE_ADDR + 4);
        
        /* 验证范围 */
        if (val1 <= 999) g_setValue1 = val1;
        if (val2 <= 999) g_setValue2 = val2;
    }
    /* 否则使用默认值 30 */
}

/**
 * @brief  保存阈值到 Flash
 */
static void TEMP_SETTING_SaveToFlash(void)
{
    uint32_t pageError;
    FLASH_EraseInitTypeDef eraseInit;
    
    /* 解锁 Flash */
    HAL_FLASH_Unlock();
    
    /* 擦除页面 */
    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.PageAddress = FLASH_SAVE_ADDR;
    eraseInit.NbPages = 1;
    HAL_FLASHEx_Erase(&eraseInit, &pageError);
    
    /* 写入魔数 */
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_SAVE_ADDR, FLASH_MAGIC_NUM);
    
    /* 写入第一组阈值 */
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_SAVE_ADDR + 2, g_setValue1);
    
    /* 写入第二组阈值 */
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_SAVE_ADDR + 4, g_setValue2);
    
    /* 锁定 Flash */
    HAL_FLASH_Lock();
}

/* 调试用全局变量 */
uint16_t DBG_setValue1 = 30;      /* 第一组设置值 */
uint16_t DBG_setValue2 = 30;      /* 第二组设置值 */
uint8_t DBG_setMode = 0;          /* 0=正常模式，1=设置模式 */
uint8_t DBG_setPos = 2;           /* 0=百位，1=十位，2=个位 */
uint8_t DBG_blinkState = 1;       /* 闪烁状态 */
uint8_t DBG_currentGroup = 0;     /* 当前设置的组 */
uint8_t DBG_seg_d1 = 0;           /* 百位显示值 */
uint8_t DBG_seg_d2 = 0;           /* 十位显示值 */
uint8_t DBG_seg_d3 = 0;           /* 个位显示值 */

/* 外部温度变量 */
extern float temperature[8];
extern uint8_t temp_alarm;

/**
 * @brief  初始化设置模块
 */
void TEMP_SETTING_Init(void)
{
    /* 从 Flash 加载阈值 */
    TEMP_SETTING_LoadFromFlash();
    
    /* 上电自检：显示测试序列 */
    HC595_Test888();      /* 显示 888 */
    HAL_Delay(500);
    HC595_TestAllOff();   /* 全灭 */
    HAL_Delay(200);
    HC595_1_DisplayNumber(123);  /* 显示 123 */
    HC595_2_DisplayNumber(456);  /* 第二组显示不同值 */
    HAL_Delay(500);
    HC595_1_DisplayNumber(30);   /* 显示默认阈值 1 */
    HC595_2_DisplayNumber(30);   /* 显示默认阈值 2 */
    HAL_Delay(300);

    g_setMode = SET_MODE_NORMAL;
    g_setPos = SET_POS_UNIT;
    g_currentGroup = SET_GROUP_1;
    g_blinkState = 1;
    g_lastBlinkTime = HAL_GetTick();
}

/**
 * @brief  获取当前组的设置值指针
 */
static uint16_t* get_current_value_ptr(void)
{
    return (g_currentGroup == SET_GROUP_1) ? &g_setValue1 : &g_setValue2;
}

/**
 * @brief  处理 SET 键 (确认/切换模式/长按切换组)
 */
static void handle_set_key(void)
{
    if (key_set_pressed)
    {
        key_set_pressed = 0;  /* 清除标志 */

        if (g_setMode == SET_MODE_NORMAL)
        {
            /* 进入设置模式，从个位开始 */
            g_setMode = SET_MODE_EDIT;
            g_setPos = SET_POS_UNIT;
            g_blinkState = 1;
            g_lastBlinkTime = HAL_GetTick();
            g_longPressDetected = 0;
            g_setPressTime = HAL_GetTick();
        }
        else
        {
            /* 确认设置，返回正常模式 */
            g_setMode = SET_MODE_NORMAL;
            g_blinkState = 1;  /* 停止闪烁，全亮 */
            
            /* 保存阈值到 Flash */
            TEMP_SETTING_SaveToFlash();
        }
    }
}

/**
 * @brief  检测 SET 键长按
 */
static void check_long_press(void)
{
    if (g_setMode == SET_MODE_EDIT && !g_longPressDetected)
    {
        /* 检查 SET 键是否仍被按下 */
        if (KEY_ReadPin(KEY_ID_SET) == 0)  /* 低电平表示按下 */
        {
            if (HAL_GetTick() - g_setPressTime >= LONG_PRESS_THRESHOLD)
            {
                g_longPressDetected = 1;
                /* 切换组 */
                g_currentGroup = (g_currentGroup == SET_GROUP_1) ? SET_GROUP_2 : SET_GROUP_1;
                /* 重置设置位 */
                g_setPos = SET_POS_UNIT;
                g_blinkState = 1;
                g_lastBlinkTime = HAL_GetTick();
            }
        }
    }
}

/**
 * @brief  处理加键
 */
static void handle_add_key(void)
{
    uint16_t *val = get_current_value_ptr();
    
    if (g_setMode != SET_MODE_EDIT)
    {
        return;
    }

    if (key_add_pressed)
    {
        key_add_pressed = 0;  /* 清除标志 */
        
        uint8_t digit;
        switch (g_setPos)
        {
            case SET_POS_HUNDRED:
                digit = (*val / 100) % 10;
                digit = (digit + 1) % 10;
                *val = (*val % 100) + digit * 100;
                break;
            case SET_POS_TEN:
                digit = (*val / 10) % 10;
                digit = (digit + 1) % 10;
                *val = (*val % 10) + digit * 10 + (*val / 100) * 100;
                break;
            case SET_POS_UNIT:
                digit = *val % 10;
                digit = (digit + 1) % 10;
                *val = (*val / 10) * 10 + digit;
                break;
        }
    }
}

/**
 * @brief  处理减键
 */
static void handle_sub_key(void)
{
    uint16_t *val = get_current_value_ptr();
    
    if (g_setMode != SET_MODE_EDIT)
    {
        return;
    }

    if (key_sub_pressed)
    {
        key_sub_pressed = 0;  /* 清除标志 */
        
        uint8_t digit;
        switch (g_setPos)
        {
            case SET_POS_HUNDRED:
                digit = (*val / 100) % 10;
                digit = (digit + 9) % 10;  /* -1 等效于 +9 再取模 */
                *val = (*val % 100) + digit * 100;
                break;
            case SET_POS_TEN:
                digit = (*val / 10) % 10;
                digit = (digit + 9) % 10;
                *val = (*val % 10) + digit * 10 + (*val / 100) * 100;
                break;
            case SET_POS_UNIT:
                digit = *val % 10;
                digit = (digit + 9) % 10;
                *val = (*val / 10) * 10 + digit;
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

    if (key_shift_pressed)
    {
        key_shift_pressed = 0;  /* 清除标志 */
        
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
    uint16_t val1 = g_setValue1;
    uint16_t val2 = g_setValue2;

    /* 更新调试变量 */
    DBG_setValue1 = val1;
    DBG_setValue2 = val2;
    DBG_setMode = (uint8_t)g_setMode;
    DBG_setPos = (uint8_t)g_setPos;
    DBG_blinkState = g_blinkState;
    DBG_currentGroup = (uint8_t)g_currentGroup;

    if (g_setMode == SET_MODE_NORMAL)
    {
        /* 正常模式 - 两组分别显示各自的阈值 */
        d3 = val1 % 10; d2 = (val1 / 10) % 10; d1 = (val1 / 100) % 10;
        HC595_1_Display3Digits(d1, d2, d3);
        
        d3 = val2 % 10; d2 = (val2 / 10) % 10; d1 = (val2 / 100) % 10;
        HC595_2_Display3Digits(d1, d2, d3);
    }
    else
    {
        /* 设置模式 - 当前设置的组闪烁，另一组正常显示 */
        uint16_t editVal = *get_current_value_ptr();
        uint16_t otherVal = (g_currentGroup == SET_GROUP_1) ? val2 : val1;
        
        /* 显示另一组 (正常显示) */
        d3 = otherVal % 10; d2 = (otherVal / 10) % 10; d1 = (otherVal / 100) % 10;
        if (g_currentGroup == SET_GROUP_1)
            HC595_2_Display3Digits(d1, d2, d3);
        else
            HC595_1_Display3Digits(d1, d2, d3);
        
        /* 显示当前设置组 (带闪烁) */
        d3 = editVal % 10; d2 = (editVal / 10) % 10; d1 = (editVal / 100) % 10;
        DBG_seg_d1 = d1; DBG_seg_d2 = d2; DBG_seg_d3 = d3;
        if (g_currentGroup == SET_GROUP_1)
            HC595_1_Display3DigitsWithBlink(d1, d2, d3, (uint8_t)g_setPos, g_blinkState);
        else
            HC595_2_Display3DigitsWithBlink(d1, d2, d3, (uint8_t)g_setPos, g_blinkState);
    }
}

/**
 * @brief  按键处理主函数
 */
void TEMP_SETTING_Handler(void)
{
    handle_set_key();
    check_long_press();  /* 检查长按 */
    handle_add_key();
    handle_sub_key();
    handle_shift_key();
    /* handle_reset_key();  -- 复位键已在 TEMP_Alarm_Handler 中处理 */
    handle_blink();
}

/**
 * @brief  获取第一组设置值
 */
uint16_t TEMP_SETTING_GetValue1(void)
{
    return g_setValue1;
}

/**
 * @brief  获取第二组设置值
 */
uint16_t TEMP_SETTING_GetValue2(void)
{
    return g_setValue2;
}

/**
 * @brief  获取当前设置的组
 */
SetGroup_t TEMP_SETTING_GetCurrentGroup(void)
{
    return g_currentGroup;
}
