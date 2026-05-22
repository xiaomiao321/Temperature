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

/* 温度阈值 (单位：℃)，超过此值 LED 点亮 */
#define TEMP_THRESHOLD      30.0f

float temperature[TEMP_CHANNEL_NUM] = {0};
uint8_t temp_valid[TEMP_CHANNEL_NUM] = {0};  /* 1=有效，0=传感器故障 */

/* 温度告警标志 */
uint8_t temp_alarm = 0;  /* 1=有通道超温，0=正常 */

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
            
            /* 检查是否超过阈值 */
            if (temperature[i] >= TEMP_THRESHOLD)
            {
                alarm_flag = 1;
            }
        }
    }
    
    /* 更新告警标志 */
    temp_alarm = alarm_flag;

    /* 根据告警状态控制 LED (低电平点亮) */
    if (temp_alarm)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);  /* 点亮 LED */
    }
    else
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);  /* 熄灭 LED */
    }
    
    return 0;  /* 解析成功 */
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
