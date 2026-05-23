/*
 * @file     key.c
 * @brief    按键驱动模块
 * @version  v1.0
 */

#include "key.h"

/* GPIO 引脚定义结构 */
typedef struct {
    uint16_t pin;
    GPIO_TypeDef *port;
} GPIO_PinDef_t;

/* 按键引脚映射表 */
static const GPIO_PinDef_t key_pins[KEY_ID_COUNT] = {
    {Btn1_Pin, GPIOB},     /* KEY_ID_SET */
    {Btn2_Pin, GPIOB},     /* KEY_ID_ADD */
    {Btn3_Pin, GPIOB},     /* KEY_ID_SUB */
    {Btn4_Pin, GPIOB},     /* KEY_ID_SHIFT */
    {Reset_Btn_Pin, GPIOA},/* KEY_ID_RESET - PA15 */
};

/* 按键状态机结构 */
typedef struct {
    KeyState_t state;           /* 当前状态 */
    KeyState_t last_state;      /* 上次状态 */
    uint32_t press_time;        /* 按下时刻 (ms) */
    uint8_t trigger_flag;       /* 触发标志 */
} KeyStatus_t;

/* 按键状态数组 */
static KeyStatus_t key_status[KEY_ID_COUNT] = {0};

/**
 * @brief  初始化按键 GPIO
 */
void KEY_Init(void)
{
    for (int i = 0; i < KEY_ID_COUNT; i++)
    {
        key_status[i].state = KEY_STATE_RELEASED;
        key_status[i].last_state = KEY_STATE_RELEASED;
        key_status[i].press_time = 0;
        key_status[i].trigger_flag = 0;
    }
}

/**
 * @brief  读取按键状态 (低电平有效)
 * @param  key_id: 按键 ID
 * @retval 按键状态 (KEY_STATE_PRESSED 或 KEY_STATE_RELEASED)
 */
KeyState_t KEY_Read(KeyId_t key_id)
{
    if (key_id >= KEY_ID_COUNT)
    {
        return KEY_STATE_RELEASED;
    }

    return (KeyState_t)HAL_GPIO_ReadPin(key_pins[key_id].port, key_pins[key_id].pin);
}

/**
 * @brief  扫描按键，检测按下事件 (非阻塞方式)
 * @param  key_id: 按键 ID
 * @retval 1 = 检测到按下，0 = 无按下
 */
uint8_t KEY_Scan(KeyId_t key_id)
{
    KeyState_t curr_state;
    uint32_t curr_time;

    if (key_id >= KEY_ID_COUNT)
    {
        return 0;
    }

    curr_state = KEY_Read(key_id);
    curr_time = HAL_GetTick();

    /* 状态机处理 */
    switch (key_status[key_id].state)
    {
        case KEY_STATE_RELEASED:
            if (curr_state == KEY_STATE_PRESSED)
            {
                /* 检测到按下，记录时间 */
                key_status[key_id].press_time = curr_time;
                key_status[key_id].state = KEY_STATE_PRESSED;
            }
            break;

        case KEY_STATE_PRESSED:
            /* 检查是否已稳定按下 (消抖时间) */
            if (curr_time - key_status[key_id].press_time >= KEY_DEBOUNCE_DELAY)
            {
                /* 再次确认按键状态 */
                if (KEY_Read(key_id) == KEY_STATE_PRESSED)
                {
                    /* 确认有效按下，设置触发标志 */
                    key_status[key_id].trigger_flag = 1;
                }
                else
                {
                    /* 误触发，返回释放状态 */
                    key_status[key_id].state = KEY_STATE_RELEASED;
                }
            }
            break;

        default:
            break;
    }

    /* 检测按键释放 */
    if (curr_state == KEY_STATE_RELEASED && key_status[key_id].state == KEY_STATE_PRESSED)
    {
        key_status[key_id].state = KEY_STATE_RELEASED;
    }

    /* 如果有触发标志，清除并返回 1 */
    if (key_status[key_id].trigger_flag)
    {
        key_status[key_id].trigger_flag = 0;
        return 1;
    }

    return 0;
}
