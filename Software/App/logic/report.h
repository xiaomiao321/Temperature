/*
 * @file     report.h
 * @brief    串口调试输出 (UART1): 上电横幅 + 运行日志
 *
 * 从 main.c 抽出, 让主循环只留骨架。日志字段含义见《交付说明》电脑串口监控一节。
 */
#ifndef __REPORT_H
#define __REPORT_H

void Report_Banner(void);       /* 上电横幅: 上电/复位后打印一次 */
void Report_Temperature(void);  /* 运行日志: 主循环每 1s 调一次 */

#endif /* __REPORT_H */
