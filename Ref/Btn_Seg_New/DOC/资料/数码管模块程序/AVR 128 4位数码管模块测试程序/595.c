#include <iom128v.h>
#include <macros.h>

#define uchar       unsigned char
#define uint        unsigned int
#define ulint       unsigned long int

#define SDK1             PORTA|=BIT(5)
#define SDK0             PORTA&=~BIT(5)

#define CLK1             PORTA|=BIT(6)
#define CLK0             PORTA&=~BIT(6)

#define LOAD1             PORTA|=BIT(7)
#define LOAD0             PORTA&=~BIT(7)


uchar tabnum=0;
uchar Tab[]={0xc0,0xcf,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff,0x7f};
//0 1 2 3 4 5 6 7 8 9 A B C D F 灭 . //  共阳数码管



void Delay(uint times)//空延时语句500us
 {uint i;
  uint j; 
 for(i=0;i<times;i++)
  for(j=0;j<500;j++); 
     }
void Delay1(uint times)//空延时语句10us
 {uint i;
  uint j; 
 for(i=0;i<times;i++)
  for(j=0;j<10;j++); 
     }
	 
	 
void SPI_595(int out_data)
{
	int i,temp;
 	for(i=0;i<8;i++)
  	{
		
		//Delay(1);
     	temp=out_data&0x80;
     	if(temp==0x80)
        SDK1;
        else SDK0; 
		   
     	out_data=out_data<<1;
		CLK0;
		Delay(5);
     	CLK1;
		Delay(5);
	}
	
}	 

void main(void)
{ 
     DDRA=0XFF;//IO口输出
	 PORTA=0X00;//
  
    while(1)
   {
    if (tabnum==18)tabnum=0;
	
	LOAD0;	
	Delay(800);
	SPI_595(Tab[tabnum]);
	SPI_595(Tab[tabnum]);
	SPI_595(Tab[tabnum]);
	SPI_595(Tab[tabnum]);
	

	//Delay(800);
	LOAD1;
	
	
	
	Delay(800);
	
	++tabnum;
	}
}  
