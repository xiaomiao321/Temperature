/*
 * temperature.c
 * PT100 温度数据解析模块
 */

#include "temperature.h"
#include <stdint.h>
#include <string.h>
#include "main.h"

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

/* 温度告警标志 */
uint8_t temp_alarm = 0;           /* 1=有通道超温，0=正常 */
uint8_t temp_alarm_latched = 0;   /* 警报锁定标志 - 按下复位键前保持警报 */
uint8_t manual_alarm = 0;         /* 手动报警标志 */
uint8_t blue_led_flash = 0;       /* 蓝灯闪烁标志（心跳指示） */
uint32_t last_temp_time = 0;      /* 上次接收到温度数据的时间 */
uint32_t blue_flash_end_time = 0; /* 蓝灯闪烁结束时间 */
uint16_t temp_data_count = 0;     /* 温度数据接收计数器（用于心跳指示） */

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

            /* 检查是否超过阈值 - 通道 8 使用阈值 2，其他通道使用阈值 1 */
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
        temp_alarm_latched = 1;  /* 锁定警报 */
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
        key_reset_pressed = 0;  /* 清除标志 */
        manual_alarm = 0;       /* 清除手动报警 */
        temp_alarm_latched = 0; /* 清除温度报警锁定 */
    }

    /* 综合报警条件：手动报警 或 温度报警锁定 */
    uint8_t alarm_active = manual_alarm || temp_alarm_latched;

    /* 控制继电器 */
    if (alarm_active)
    {
        /* 报警状态：24V 导通（蜂鸣器响），220V 关断（照明灯灭） */
        HAL_GPIO_WritePin(Relay_24_GPIO_Port, Relay_24_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Relay_220_GPIO_Port, Relay_220_Pin, GPIO_PIN_RESET);
    }
    else
    {
        /* 正常状态：24V 关断（蜂鸣器关），220V 导通（照明灯开） */
        HAL_GPIO_WritePin(Relay_24_GPIO_Port, Relay_24_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Relay_220_GPIO_Port, Relay_220_Pin, GPIO_PIN_SET);
    }

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
