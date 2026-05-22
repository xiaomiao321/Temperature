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
};

/* 按键上次状态 */
static KeyState_t key_last_state[KEY_ID_COUNT] = {
    KEY_STATE_RELEASED,
    KEY_STATE_RELEASED,
    KEY_STATE_RELEASED,
    KEY_STATE_RELEASED
};

/**
 * @brief  初始化按键 GPIO
 */
void KEY_Init(void)
{
    /* GPIO 已在 MX_GPIO_Init() 中初始化 */
    /* 这里只需重置状态 */
    for (int i = 0; i < KEY_ID_COUNT; i++)
    {
        key_last_state[i] = KEY_STATE_RELEASED;
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
 * @brief  扫描按键，检测按下事件
 * @param  key_id: 按键 ID
 * @retval 1 = 检测到按下，0 = 无按下
 */
uint8_t KEY_Scan(KeyId_t key_id)
{
    KeyState_t curr_state;
    
    if (key_id >= KEY_ID_COUNT)
    {
        return 0;
    }
    
    curr_state = KEY_Read(key_id);
    
    /* 检测下降沿 (从释放到按下) */
    if (key_last_state[key_id] == KEY_STATE_RELEASED && curr_state == KEY_STATE_PRESSED)
    {
        HAL_Delay(KEY_DEBOUNCE_DELAY);  /* 消抖 */
        
        if (KEY_Read(key_id) == KEY_STATE_PRESSED)
        {
            key_last_state[key_id] = KEY_STATE_PRESSED;
            return 1;  /* 检测到按下 */
        }
    }
    else if (curr_state == KEY_STATE_RELEASED)
    {
        key_last_state[key_id] = KEY_STATE_RELEASED;
    }
    
    return 0;  /* 无按下 */
}
