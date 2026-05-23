/*
 * @file     key.c
 * @brief    按键驱动模块 (基于 MultiButton)
 * @version  v2.0
 */

#include "key.h"
#include "temperature.h"

/* 按键句柄实例 */
Button btn_set;
Button btn_add;
Button btn_sub;
Button btn_shift;
Button btn_reset;

/* 按键事件标志 */
volatile uint8_t key_set_pressed = 0;
volatile uint8_t key_add_pressed = 0;
volatile uint8_t key_sub_pressed = 0;
volatile uint8_t key_shift_pressed = 0;
volatile uint8_t key_reset_pressed = 0;

/**
 * @brief  GPIO 读取函数 - MultiButton 调用
 * @param  button_id: 按键 ID
 * @retval GPIO 电平 (0 或 1)
 */
uint8_t KEY_ReadPin(uint8_t button_id)
{
    switch (button_id) {
        case KEY_ID_SET:
            return HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
        case KEY_ID_ADD:
            return HAL_GPIO_ReadPin(GPIOB, Btn2_Pin);
        case KEY_ID_SUB:
            return HAL_GPIO_ReadPin(GPIOB, Btn3_Pin);
        case KEY_ID_SHIFT:
            return HAL_GPIO_ReadPin(GPIOB, Btn4_Pin);
        case KEY_ID_RESET:
            return HAL_GPIO_ReadPin(GPIOA, Reset_Btn_Pin);
        default:
            return 1;
    }
}

/* 按键按下回调 - 设置标志 */
static void on_press_down(Button* btn, void* user_data)
{
    (void)btn;
    KeyId_t key_id = (KeyId_t)(uintptr_t)user_data;
    switch (key_id) {
        case KEY_ID_SET: key_set_pressed = 1; break;
        case KEY_ID_ADD: key_add_pressed = 1; break;
        case KEY_ID_SUB: key_sub_pressed = 1; break;
        case KEY_ID_SHIFT: key_shift_pressed = 1; break;
        case KEY_ID_RESET: key_reset_pressed = 1; break;
    }
}

/**
 * @brief  初始化按键
 */
void KEY_Init(void)
{
    /* 初始化所有按键 (低电平有效) */
    button_init(&btn_set, KEY_ReadPin, 0, KEY_ID_SET);
    button_init(&btn_add, KEY_ReadPin, 0, KEY_ID_ADD);
    button_init(&btn_sub, KEY_ReadPin, 0, KEY_ID_SUB);
    button_init(&btn_shift, KEY_ReadPin, 0, KEY_ID_SHIFT);
    button_init(&btn_reset, KEY_ReadPin, 0, KEY_ID_RESET);

    /* 注册按下事件回调 */
    button_attach(&btn_set, BTN_PRESS_DOWN, on_press_down, (void*)(uintptr_t)KEY_ID_SET);
    button_attach(&btn_add, BTN_PRESS_DOWN, on_press_down, (void*)(uintptr_t)KEY_ID_ADD);
    button_attach(&btn_sub, BTN_PRESS_DOWN, on_press_down, (void*)(uintptr_t)KEY_ID_SUB);
    button_attach(&btn_shift, BTN_PRESS_DOWN, on_press_down, (void*)(uintptr_t)KEY_ID_SHIFT);
    button_attach(&btn_reset, BTN_PRESS_DOWN, on_press_down, (void*)(uintptr_t)KEY_ID_RESET);

    /* 启动按键处理 */
    button_start(&btn_set);
    button_start(&btn_add);
    button_start(&btn_sub);
    button_start(&btn_shift);
    button_start(&btn_reset);
}

/**
 * @brief  按键事件处理 - 在主循环中调用
 */
void KEY_Handler(void)
{
    /* 调用 MultiButton 状态机处理 */
    button_ticks();
}
