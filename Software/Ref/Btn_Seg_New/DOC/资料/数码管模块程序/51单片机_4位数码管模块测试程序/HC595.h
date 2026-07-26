#ifndef __HC595_H
#define __HC595_H

#include "STC12C2052AD.h"
#include <intrins.h>

//IO设置
sbit SDI = P3^7;		//串行数据输入
sbit CLK = P1^0;		//时钟上升沿移位数据
sbit LE	 = P1^1;		//高电平更新，低电平锁存
//sbit OE	 = P1^2;		//OE为低电平时显示启动输出，OE为高电平时输出关闭 

extern void HC595_Display(unsigned char Shi, unsigned char Ge);//数码管显示函数
extern void HC595_Off(void);									 //关闭显示

#endif