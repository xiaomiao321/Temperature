#include "STC12C2052AD.h"
#include <intrins.h>
#include "HC595.h"

#define uint  unsigned int
#define uchar unsigned char

//void HC595_Delay_ms(unsigned int n)
//{
//	unsigned int  i,j;
//	for(i=0;i<n;i++)
//		for(j=0;j<123;j++);
//}

void HC595_Display(unsigned char Shi, unsigned char Ge)
{
	//uint s = 0;
	uchar k;
	uchar n = Shi;
	uchar m = Ge;
	for(k=0;k<8;k++)
	{
		if(n&0x80)SDI =1;
		else SDI =0;    //判断当前发送位数据
		n <<= 1;      	//左移一位
		CLK = 0;
		//SM16126_Delay_ms(1);
		CLK = 1;       	//移位寄存器操作	
	}		
	for(k=0;k<8;k++)
	{
		if(m&0x80)SDI =1;
		else SDI =0;    //判断当前发送位数据
		m <<= 1;      	//左移一位
		CLK = 0;
		//SM16126_Delay_ms(1);
		CLK = 1;       	//移位寄存器操作	
	}

	LE = 1;
	//SM16126_Delay_ms(1);
	LE = 0;			//锁存数据
	//SM16126_Delay_ms(1);
	//OE = 0;			//OE为低电平时显示启动输出，OE为高电平时输出关闭
	//Delay_ms(1000);
	//OE = 1;		//这里不关闭显示，让数码管静态长亮
	//Delay_ms(1000);
}

//void HC595_Off(void)
//{
//	//OE = 1;		
//}