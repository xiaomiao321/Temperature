/*
 * temperature.h
 * PT100 温度数据解析模块
 */

#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include <stdint.h>

/* 外部变量声明 */
extern float temperature[8];
extern uint8_t temp_valid[8];
extern uint8_t temp_alarm;
extern uint8_t temp_alarm_latched;  /* 警报锁定标志 */

/* 函数声明 */
uint8_t Parse_Temperature_Data(uint8_t *buf, uint16_t len);
void TEMP_Alarm_Handler(void);      /* 警报处理（手动/复位/LED） */
float Get_Temperature(uint8_t channel);
uint8_t Get_Temp_Valid(uint8_t channel);
uint8_t TEMP_Get_AlarmState(void);  /* 获取报警状态 */

#endif /* __TEMPERATURE_H__ */
