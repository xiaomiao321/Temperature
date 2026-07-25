/*
 * @file     report.c
 * @brief    串口调试输出 (UART1): 上电横幅 + 运行日志
 *
 * 从 main.c 抽出。阈值直接取 temp_setting 的 getter, 不再依赖全局拷贝。
 */
#include "report.h"
#include "main.h"
#include "board_config.h"
#include "temperature.h"
#include "temp_setting.h"
#include "usart.h"
#include <stdio.h>

/* 上电横幅: 上电/复位后打印一次, 现场插上串口即可确认程序在跑。
 * __DATE__/__TIME__ 为编译时间戳, 便于核对烧录的是哪一版固件。 */
void Report_Banner(void)
{
    char buf[256];
    int len = sprintf(buf,
        "\r\n==============================\r\n"
        " 8CH PT100 Temp Monitor (Delivery)\r\n"
        " Build : %s %s\r\n"
        " Debug : UART1 @115200 8N1\r\n"
        " Relay : K1=Alarm K2=Fan K3=Gas K4=Damper\r\n"
        "==============================\r\n",
        __DATE__, __TIME__);
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 100);
}

/* 运行日志: 主循环每 1s 定时调用一次 (与收帧解耦, 没接温度模块也照打)。
 * 格式: t=秒.毫秒 T1..T8 TH1/TH2 ALM MAN LAT BM/BR K=K1K2K3K4 */
void Report_Temperature(void)
{
    char buf[224];
    int len = 0;

    /* 时间戳: 开机秒数 (秒.毫秒) */
    uint32_t ms = HAL_GetTick();
    len += sprintf(&buf[len], "t=%lu.%03lu ", (unsigned long)(ms / 1000),
                   (unsigned long)(ms % 1000));

    /* 8 路温度: 有效输出 x.x, 故障输出 ERR (整数方式, 避免浮点 printf) */
    for (int i = 0; i < 8; i++) {
        if (temp_valid[i]) {
            int temp_int = (int)temperature[i];
            int temp_frac = (int)((temperature[i] - temp_int) * 10);
            if (temp_frac < 0)
                temp_frac = -temp_frac;
            len += sprintf(&buf[len], "T%d=%d.%d ", i + 1, temp_int, temp_frac);
        } else {
            len += sprintf(&buf[len], "T%d=ERR ", i + 1);
        }
    }

    /* 阈值: TH1 用于 CH1-CH7, TH2 用于 CH8 */
    len += sprintf(&buf[len], "TH1=%u TH2=%u ",
                   (unsigned)TEMP_SETTING_GetValue1(),
                   (unsigned)TEMP_SETTING_GetValue2());

    /* 综合报警状态 (与继电器输出一致) */
    unsigned alm = (manual_alarm || temp_alarm_latched) ? 1u : 0u;
    len += sprintf(&buf[len], "ALM=%u ", alm);

    /* 按键调试: MAN=手动报警标志 LAT=温度锁定; BM/BR=手动(PA12)/复位(PA15)引脚原始电平 */
    unsigned bm = HAL_GPIO_ReadPin(Manual_Btn_GPIO_Port, Manual_Btn_Pin);
    unsigned br = HAL_GPIO_ReadPin(Reset_Btn_GPIO_Port, Reset_Btn_Pin);
    len += sprintf(&buf[len], "MAN=%u LAT=%u BM=%u BR=%u ", (unsigned)manual_alarm,
                   (unsigned)temp_alarm_latched, bm, br);

    /* 继电器实际输出电平 K1 K2 K3 K4 (1=吸合) */
    unsigned k1 = HAL_GPIO_ReadPin(K1_ALARM_Port, K1_ALARM_Pin);
    unsigned k2 = HAL_GPIO_ReadPin(K2_FAN_Port, K2_FAN_Pin);
    unsigned k3 = HAL_GPIO_ReadPin(K3_GAS_Port, K3_GAS_Pin);
    unsigned k4 = HAL_GPIO_ReadPin(K4_DAMPER_Port, K4_DAMPER_Pin);
    len += sprintf(&buf[len], "K=%u%u%u%u\r\n", k1, k2, k3, k4);

    HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 100);
}
