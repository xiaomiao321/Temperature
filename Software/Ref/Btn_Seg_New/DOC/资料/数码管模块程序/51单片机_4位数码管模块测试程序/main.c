//4位数码管测试程序
//上电全亮，然后数码管显示数字，并递增

#include"STC12C5A60S2.h"

sbit SDI = P1^0;
sbit CLK = P1^1;
sbit LAT = P1^2;

#define uchar unsigned char
#define uint  unsigned int

unsigned char Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei;	//千位、百位、十位和个位


//数码管编码
code unsigned char table[] = 
{0xc0,0xf9,0xa4,0xb0,0x99,
0x92,0x82,0xf8,0x80,0x90,
0xff,0x00,0x90,0x96,0xaf,
0xc0,0x89,0xc7,0x8e,0xc1,0x7f};
//0,1,2,3,4,5,6,7,8,9,全暗，全亮,g,E,r,O,H,L,F,U，小数点，共阳数码管使用


void HC595_Display(unsigned char Qian, unsigned char Bai, unsigned char Shi, unsigned char Ge)
{
	//uint s = 0;
	uchar k;
	uchar n = Ge;
	uchar m = Shi;
	uchar x = Bai;
	uchar y = Qian;
	for(k=0;k<8;k++)
	{
		if(n&0x80)SDI =1;
		else SDI =0;    //判断当前发送位数据
		n <<= 1;      	//左移一位
		CLK = 0;
		CLK = 1;       	//移位寄存器操作	
	}		
	for(k=0;k<8;k++)
	{
		if(m&0x80)SDI =1;
		else SDI =0;    //判断当前发送位数据
		m <<= 1;      	//左移一位
		CLK = 0;
		CLK = 1;       	//移位寄存器操作	
	}
	for(k=0;k<8;k++)
	{
		if(x&0x80)SDI =1;
		else SDI =0;    //判断当前发送位数据
		x <<= 1;      	//左移一位
		CLK = 0;
		CLK = 1;       	//移位寄存器操作	
	}
	for(k=0;k<8;k++)
	{
		if(y&0x80)SDI =1;
		else SDI =0;    //判断当前发送位数据
		y <<= 1;      	//左移一位
		CLK = 0;
		CLK = 1;       	//移位寄存器操作	
	}
	LAT = 1;
	LAT = 0;			//锁存数据

}


//毫秒延时函数
void delay_ms(uint x)
{
	uint i,j;
	for(i=0;i<x;i++)
		for(j=0;j<250;j++);	
}
 
//微秒延时函数
//void delay_us_110592(uint x)//延时时间为 1us * x	 晶振是11.0592M	 
//{
//	int i;
//	for(i = 0; i < x; i++)
//		_nop_();	
//}

void main()
{
	int a,b,c,d,e;
	b=0;
	c=0;
	d=0;
	e=0;
	delay_ms(2000);//等待系统稳定
	Ge_Wei = table[20];//第一位取小数点码
	Shi_Wei = table[20];//第二位取小数点码
	Bai_Wei = table[20];//第三位取小数点码
	Qian_Wei = table[20];//第四位取小数点码
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);//显示第一位各位小数点
	delay_ms(2000);
	Ge_Wei = table[10];
	Shi_Wei = table[10];
	Bai_Wei = table[10];
	Qian_Wei = table[10];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);//显示第一位各位小数点
	delay_ms(2000);//等待系统稳定
	Ge_Wei = table[10];
	Shi_Wei = table[10];
	Bai_Wei = table[10];
	Qian_Wei = table[11];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(2000);//等待系统稳定
	Ge_Wei = table[10];
	Shi_Wei = table[10];
	Bai_Wei = table[11];
	Qian_Wei = table[10];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(2000);//等待系统稳定
	Ge_Wei = table[10];
	Shi_Wei = table[11];
	Bai_Wei = table[10];
	Qian_Wei = table[10];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(2000);//等待系统稳定
	Ge_Wei = table[11];
	Shi_Wei = table[10];
	Bai_Wei = table[10];
	Qian_Wei = table[10];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(2000);
	Ge_Wei = table[10];
	Shi_Wei = table[10];
	Bai_Wei = table[10];
	Qian_Wei = table[10];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);

	delay_ms(2000);
	Ge_Wei = table[0];
	Shi_Wei = table[0];
	Bai_Wei = table[0];
	Qian_Wei = table[0];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[1];
	Shi_Wei = table[1];
	Bai_Wei = table[1];
	Qian_Wei = table[1];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[2];
	Shi_Wei = table[2];
	Bai_Wei = table[2];
	Qian_Wei = table[2];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[3];
	Shi_Wei = table[3];
	Bai_Wei = table[3];
	Qian_Wei = table[3];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[4];
	Shi_Wei = table[4];
	Bai_Wei = table[4];
	Qian_Wei = table[4];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[5];
	Shi_Wei = table[5];
	Bai_Wei = table[5];
	Qian_Wei = table[5];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[6];
	Shi_Wei = table[6];
	Bai_Wei = table[6];
	Qian_Wei = table[6];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[7];
	Shi_Wei = table[7];
	Bai_Wei = table[7];
	Qian_Wei = table[7];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[8];
	Shi_Wei = table[8];
	Bai_Wei = table[8];
	Qian_Wei = table[8];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	Ge_Wei = table[9];
	Shi_Wei = table[9];
	Bai_Wei = table[9];
	Qian_Wei = table[9];
	HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	delay_ms(1000);
	while(1)
	{
		a++;

		if(a==500)
		{
			a = 0;
			b++;
			if(b == 10)
			{
				b = 0;
			 	c++;
			}
			if(c == 10)
			{
				c = 0;
				d++;			
			}
			if(d == 10)
			{
				d = 0;
				e++;
			}
			if(e == 10)e= 0;
		}
		Qian_Wei = table[e];
		Bai_Wei = table[d];
		Shi_Wei = table[c];
		Ge_Wei = table[b];
		HC595_Display(Qian_Wei,Bai_Wei,Shi_Wei,Ge_Wei);
	
	}

}







