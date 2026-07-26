
#include "stm32f10x.h"

unsigned char  Tab[] = {0xc0,0xcf,0xa4,0xb0,0x99,0x92,0x82, 0xf8,0x80,
							0x90, 0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff,0x7f};//0 1 2 3 4 5 6 7 8 9 A B C D F Ãð . //  ¹²ÑôÊýÂë¹Ü



void delay10ms(uint16_t temp)
{
	uint16_t  i,j,t;
	t=temp;
	while(t!=0)
    {
		for(i=200;i>0;i--)
       	for(j=248;j>0;j--);
		t--;
	}
}




void led_595_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO  , ENABLE); 

	//595  C0  Pb   5 6 7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);   
	GPIO_ResetBits(GPIOB,  GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7  );	
}

void SPI_595(uint8_t value)
{
	//595  C0  PC   Pb   5 6 7  load clk din
	uint8_t i;
	for(i=0;i<8;i++)  
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_6);
		delay10ms(1);
		if(0x80&value)
			GPIO_SetBits(GPIOB,GPIO_Pin_7);
		else
			GPIO_ResetBits(GPIOB,GPIO_Pin_7);
		value = value <<1 ;
		delay10ms(1);
		GPIO_SetBits(GPIOB,GPIO_Pin_6);
		delay10ms(1);
	}
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
	GPIO_ResetBits(GPIOB,GPIO_Pin_7);	

	
}
void load(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_5);
	delay10ms(1);
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);
}


int main(void)
{  
	
	led_595_io_init();
    while(1)
    {  

	delay10ms(500);
	SPI_595(Tab[0]);
	SPI_595(Tab[0]);
	SPI_595(Tab[0]);
	SPI_595(Tab[0]);
	SPI_595(Tab[0]);
    SPI_595(Tab[0]);
	SPI_595(Tab[0]); 
	SPI_595(Tab[0]);
	load();
	delay10ms(500);
	SPI_595(Tab[1]);
	SPI_595(Tab[1]);
	SPI_595(Tab[1]);
	SPI_595(Tab[1]);
	SPI_595(Tab[1]);
    SPI_595(Tab[1]);
	SPI_595(Tab[1]); 
	SPI_595(Tab[1]);
	load();
	delay10ms(500);
	SPI_595(Tab[2]);
	SPI_595(Tab[2]);
	SPI_595(Tab[2]);
	SPI_595(Tab[2]);
	SPI_595(Tab[2]);
    SPI_595(Tab[2]);
	SPI_595(Tab[2]); 
	SPI_595(Tab[2]);
	load();
	delay10ms(500);
	SPI_595(Tab[3]);
	SPI_595(Tab[3]);
	SPI_595(Tab[3]);
	SPI_595(Tab[3]);
	SPI_595(Tab[3]);
    SPI_595(Tab[3]);
	SPI_595(Tab[3]); 
	SPI_595(Tab[3]);
	load();
		delay10ms(500);
	SPI_595(Tab[4]);
	SPI_595(Tab[4]);
	SPI_595(Tab[4]);
	SPI_595(Tab[4]);
	SPI_595(Tab[4]);
    SPI_595(Tab[4]);
	SPI_595(Tab[4]); 
	SPI_595(Tab[4]);
	load();
		delay10ms(500);
	SPI_595(Tab[5]);
	SPI_595(Tab[5]);
	SPI_595(Tab[5]);
	SPI_595(Tab[5]);
	SPI_595(Tab[5]);
    SPI_595(Tab[5]);
	SPI_595(Tab[5]); 
	SPI_595(Tab[5]);
	load();
		delay10ms(500);
	SPI_595(Tab[6]);
	SPI_595(Tab[6]);
	SPI_595(Tab[6]);
	SPI_595(Tab[6]);
	SPI_595(Tab[6]);
    SPI_595(Tab[6]);
	SPI_595(Tab[6]); 
	SPI_595(Tab[6]);
		load();
		delay10ms(500);
	SPI_595(Tab[7]);
	SPI_595(Tab[7]);
	SPI_595(Tab[7]);
	SPI_595(Tab[7]);
	SPI_595(Tab[7]);
    SPI_595(Tab[7]);
	SPI_595(Tab[7]); 
	SPI_595(Tab[7]);
 	load();
		delay10ms(500);
	SPI_595(Tab[8]);
	SPI_595(Tab[8]);
	SPI_595(Tab[8]);
	SPI_595(Tab[8]);
	SPI_595(Tab[8]);
    SPI_595(Tab[8]);
	SPI_595(Tab[8]); 
	SPI_595(Tab[8]);
	load();
		delay10ms(500);
	SPI_595(Tab[9]);
	SPI_595(Tab[9]);
	SPI_595(Tab[9]);
	SPI_595(Tab[9]);
	SPI_595(Tab[9]);
    SPI_595(Tab[9]);
	SPI_595(Tab[9]); 
	SPI_595(Tab[9]);
	load();
			delay10ms(500);
	SPI_595(Tab[7]);
	SPI_595(Tab[6]);
	SPI_595(Tab[5]);
	SPI_595(Tab[4]);
	SPI_595(Tab[3]);
    SPI_595(Tab[2]);
	SPI_595(Tab[1]); 
	SPI_595(Tab[0]);
;	load();
		delay10ms(200);
	SPI_595(Tab[15]);
	SPI_595(Tab[14]);
	SPI_595(Tab[13]);
	SPI_595(Tab[12]);
	SPI_595(Tab[11]);
    SPI_595(Tab[10]);
	SPI_595(Tab[9]); 
	SPI_595(Tab[8]);
;	load();
		delay10ms(500);

	} 
}

