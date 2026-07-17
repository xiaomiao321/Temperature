/*
 * temperature.c
 * PT100 温度数据解析模块
 */

#include "temperature.h"
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "board_config.h"

/* 温度数据 */
#define TEMP_CHANNEL_NUM    8
#define TEMP_SENSOR_FAULT   -5000   /* 传感器故障特殊值 */

/* 外部阈值变量 (在 main.c 中设置) */
extern uint16_t g_temp_threshold1;  /* 第一组阈值 (CH1-CH4) */
extern uint16_t g_temp_threshold2;  /* 第二组阈值 (CH5-CH8) */

/* 外部按键状态 */
extern uint8_t key_reset_pressed;   /* 复位键标志 */
extern uint8_t key_manual_pressed;  /* 手动键标志 */

float temperature[TEMP_CHANNEL_NUM] = {0};
uint8_t temp_valid[TEMP_CHANNEL_NUM] = {0};  /* 1=有效，0=传感器故障 */

/* 报警冷却时间（毫秒）- 复位后在此时间内不触发新报警, 集中在 board_config.h 配置 */
#define ALARM_COOLDOWN_TIME  ALARM_COOLDOWN_MS

/* 温度告警标志 */
uint8_t temp_alarm = 0;           /* 1=有通道超温，0=正常 */
uint8_t temp_alarm_latched = 0;   /* 警报锁定标志 - 按下复位键前保持警报 */
uint8_t manual_alarm = 0;         /* 手动报警标志 */
uint8_t blue_led_flash = 0;       /* 蓝灯闪烁标志（心跳指示） */
uint32_t last_temp_time = 0;      /* 上次接收到温度数据的时间 */
uint32_t blue_flash_end_time = 0; /* 蓝灯闪烁结束时间 */
uint16_t temp_data_count = 0;     /* 温度数据接收计数器（用于心跳指示） */
uint32_t alarm_cooldown_end_time = 0;  /* 报警冷却结束时间 */

/**
 * @brief 解析 Modbus 0x04 命令返回的温度数据
 * @param buf: 接收到的数据缓冲区
 * @param len: 数据长度
 * @retval 0=解析成功，1=解析失败
 */
uint8_t Parse_Temperature_Data(uint8_t *buf, uint16_t len)
{
    /* 最小帧长度：Addr(1) + Cmd(1) + ByteCount(1) + Data(16) + CRC(2) = 21 */
    if (len < 21)
    {
        return 1;  /* 数据长度不足 */
    }

    /* 检查帧头 */
    if (buf[0] != 0x01 || buf[1] != 0x04)
    {
        return 1;  /* 帧头不正确 */
    }

    /* 检查数据字节数 (应该是 0x10 = 16 字节，对应 8 路温度) */
    if (buf[2] != 0x10)
    {
        return 1;  /* 数据长度不正确 */
    }

    /* 解析 8 路温度数据 */
    /* 数据从 buf[3] 开始，每路温度占 2 字节 (大端序) */
    uint8_t alarm_flag = 0;

    for (int i = 0; i < TEMP_CHANNEL_NUM; i++)
    {
        uint16_t offset = 3 + i * 2;
        int16_t temp_raw;

        /* 大端序组合成 16 位有符号整数 */
        temp_raw = (int16_t)((buf[offset] << 8) | buf[offset + 1]);

        /* 检查是否为故障值 */
        if (temp_raw == TEMP_SENSOR_FAULT)
        {
            temperature[i] = -500.0f;
            temp_valid[i] = 0;  /* 标记为故障 */
        }
        else
        {
            /* 转换为实际温度值 (放大 10 倍) */
            temperature[i] = (float)temp_raw / 10.0f;
            temp_valid[i] = 1;  /* 标记为有效 */

            /* 检查是否超过阈值 - CH1-CH7 使用阈值 1，CH8 使用阈值 2 */
            if (i == 7)  /* 通道 8 (索引 7) */
            {
                /* CH8 使用第二组阈值 */
                if (temperature[i] >= (float)g_temp_threshold2)
                {
                    alarm_flag = 1;
                }
            }
            else
            {
                /* CH1-CH7 使用第一组阈值 */
                if (temperature[i] >= (float)g_temp_threshold1)
                {
                    alarm_flag = 1;
                }
            }
        }
    }

    /* 更新告警标志 - 温度超温时锁定，直到复位 */
    if (alarm_flag)
    {
        /* 检查是否在冷却时间内 */
        if (HAL_GetTick() >= alarm_cooldown_end_time)
        {
            /* 冷却时间已过，允许触发新报警 */
            temp_alarm_latched = 1;  /* 锁定警报 */
            alarm_cooldown_end_time = HAL_GetTick() + ALARM_COOLDOWN_TIME;  /* 设置冷却结束时间 */
        }
        /* 否则在冷却时间内，不触发新报警，但保持当前超温状态 */
    }
    temp_alarm = alarm_flag;  /* 当前超温状态 */

    /* 心跳指示：每 3 次数据闪一次 */
    temp_data_count++;
    if (temp_data_count >= 3)
    {
        temp_data_count = 0;
        blue_led_flash = 1;
    }
    last_temp_time = HAL_GetTick();   /* 记录接收时间 */

    return 0;  /* 解析成功 */
}

/* ===== K4 防火阀状态机 =====
 * 正常: K4 释放(开阀线, 阀保持开) / 报警: K4 吸合(关阀线, 阀关) / 复位: 释放回开阀。
 * 单/双路方案由 board_config.h 的 DAMPER_HAS_LIMIT_SWITCH 切换。
 */
typedef enum {
    DAMPER_OPEN = 0,       /* 正常态: 阀开 */
#if (DAMPER_HAS_LIMIT_SWITCH == 0)
    DAMPER_CLOSING,        /* 双路: 关阀电机通电中(定时) */
    DAMPER_OPENING,        /* 双路: 开阀电机通电中(定时) */
#endif
    DAMPER_CLOSED          /* 报警态: 阀关 */
} DamperState_t;

static DamperState_t g_damperState = DAMPER_OPEN;
#if (DAMPER_HAS_LIMIT_SWITCH == 0)
static uint32_t g_damperMoveEnd = 0;   /* 电机定时释放时刻 */
#endif

/**
 * @brief  K4 防火阀驱动 (每次主循环调用)
 * @param  alarm_active: 1=报警(关阀), 0=正常(开阀)
 */
static void Damper_Handler(uint8_t alarm_active)
{
#if (DAMPER_HAS_LIMIT_SWITCH == 1)
    /* ---- 单路: 执行器带内置限位, 吸合=关阀保持, 释放=开阀, 限位自停 ---- */
    if (alarm_active) {
        g_damperState = DAMPER_CLOSED;
        HAL_GPIO_WritePin(K4_DAMPER_Port, K4_DAMPER_Pin, GPIO_PIN_SET);   /* 关阀线 */
    } else {
        g_damperState = DAMPER_OPEN;
        HAL_GPIO_WritePin(K4_DAMPER_Port, K4_DAMPER_Pin, GPIO_PIN_RESET); /* 开阀线 */
    }
#else
    /* ---- 双路: 无限位, 通电到位后按行程时间定时切断, 防电机堵转 ---- */
    switch (g_damperState) {
        case DAMPER_OPEN:
            if (alarm_active) {                       /* 开 -> 关 */
                g_damperState = DAMPER_CLOSING;
                g_damperMoveEnd = HAL_GetTick() + DAMPER_TRAVEL_MS;
                HAL_GPIO_WritePin(K5_DAMPER2_Port, K5_DAMPER2_Pin, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(K4_DAMPER_Port,  K4_DAMPER_Pin,  GPIO_PIN_SET);
            }
            break;
        case DAMPER_CLOSING:
            if (HAL_GetTick() >= g_damperMoveEnd) {   /* 关阀到位, 停机 */
                g_damperState = DAMPER_CLOSED;
                HAL_GPIO_WritePin(K4_DAMPER_Port, K4_DAMPER_Pin, GPIO_PIN_RESET);
            }
            break;
        case DAMPER_CLOSED:
            if (!alarm_active) {                      /* 关 -> 开 (复位) */
                g_damperState = DAMPER_OPENING;
                g_damperMoveEnd = HAL_GetTick() + DAMPER_TRAVEL_MS;
                HAL_GPIO_WritePin(K4_DAMPER_Port,  K4_DAMPER_Pin,  GPIO_PIN_RESET);
                HAL_GPIO_WritePin(K5_DAMPER2_Port, K5_DAMPER2_Pin, GPIO_PIN_SET);
            }
            break;
        case DAMPER_OPENING:
            if (HAL_GetTick() >= g_damperMoveEnd) {   /* 开阀到位, 停机 */
                g_damperState = DAMPER_OPEN;
                HAL_GPIO_WritePin(K5_DAMPER2_Port, K5_DAMPER2_Pin, GPIO_PIN_RESET);
            }
            break;
        default: break;
    }
#endif
}

/**
 * @brief  处理警报逻辑（包括手动按钮、温度报警、复位、LED）
 *         在主循环中调用
 */
void TEMP_Alarm_Handler(void)
{
    /* 处理手动报警按钮 - 切换开关 */
    if (key_manual_pressed)
    {
        key_manual_pressed = 0;  /* 清除标志 */
        manual_alarm = !manual_alarm;  /* 切换手动报警状态 */
    }

    /* 处理复位按钮 - 清除所有报警 */
    if (key_reset_pressed)
    {
        key_reset_pressed = 0;      /* 清除标志 */
        manual_alarm = 0;           /* 清除手动报警 */
        temp_alarm_latched = 0;     /* 清除温度报警锁定 */
        alarm_cooldown_end_time = HAL_GetTick() + ALARM_COOLDOWN_TIME;  /* 启动冷却时间 */
    }

    /* 综合报警条件：手动报警 或 温度报警锁定 */
    uint8_t alarm_active = manual_alarm || temp_alarm_latched;

    /* ===== 继电器输出 ===== */
    /* K1 警报 / K2 风机 / K3 断气阀: 报警统一吸合, 正常释放
     * (常开/常闭由客户接线决定, 程序一律"报警=吸合") */
    GPIO_PinState k123 = alarm_active ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(K1_ALARM_Port, K1_ALARM_Pin, k123);
    HAL_GPIO_WritePin(K2_FAN_Port,   K2_FAN_Pin,   k123);
    HAL_GPIO_WritePin(K3_GAS_Port,   K3_GAS_Pin,   k123);

    /* K4 防火阀: 独立状态机 (复位时 alarm_active=0, 自动切回开阀) */
    Damper_Handler(alarm_active);

    /* 控制 LED 指示灯 */
    
    /* === 红灯控制 === */
    if (temp_alarm_latched)
    {
        /* 温度报警锁定 - 红灯常亮 */
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
    }
    else if (manual_alarm)
    {
        /* 手动报警 - 红灯灭（黄灯亮） */
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);
    }
    else
    {
        /* 正常状态：检查是否有温度数据 */
        if (HAL_GetTick() - last_temp_time > 2000)
        {
            /* 超过 2 秒没收到数据 - 红灯快速闪烁 (2Hz) */
            static uint32_t red_toggle_time = 0;
            static uint8_t red_state = 0;
            
            if (HAL_GetTick() - red_toggle_time >= 250)
            {
                red_state = !red_state;
                HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, red_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
                red_toggle_time = HAL_GetTick();
            }
        }
        else
        {
            /* 正常接收数据 - 红灯灭 */
            HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);
        }
    }

    /* === 黄灯控制 === */
    if (manual_alarm)
    {
        /* 手动报警 - 黄灯亮 */
        HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_RESET);
    }

    /* === 蓝灯控制 - 长灭，收到数据闪一下 === */
    if (blue_led_flash)
    {
        /* 刚收到数据，点亮蓝灯 */
        HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, GPIO_PIN_SET);
        blue_flash_end_time = HAL_GetTick() + 30;  /* 亮 30ms */
        blue_led_flash = 0;  /* 清除标志，下次进入 else 分支 */
    }
    else if (HAL_GetTick() < blue_flash_end_time)
    {
        /* 保持点亮直到时间到 */
        HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, GPIO_PIN_SET);
    }
    else
    {
        /* 蓝灯长灭 */
        HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief 获取指定通道的温度值
 * @param channel: 通道号 (0-7)
 * @retval 温度值 (℃)，如果通道无效返回 -999.0
 */
float Get_Temperature(uint8_t channel)
{
    if (channel >= TEMP_CHANNEL_NUM)
    {
        return -999.0f;
    }
    return temperature[channel];
}

/**
 * @brief 获取指定通道的有效状态
 * @param channel: 通道号 (0-7)
 * @retval 1=有效，0=故障
 */
uint8_t Get_Temp_Valid(uint8_t channel)
{
    if (channel >= TEMP_CHANNEL_NUM)
    {
        return 0;
    }
    return temp_valid[channel];
}

/**
 * @brief 获取当前报警状态
 * @retval 1=有报警，0=正常
 */
uint8_t TEMP_Get_AlarmState(void)
{
    return temp_alarm_latched || manual_alarm;
}
