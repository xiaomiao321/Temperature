#include "leds_595.h"
#include "bsp.h"

uint16_t leds_595_display_buf[21];
uint16_t leds_flash_timer;


void leds_595_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO  , ENABLE); 

	//595  C0  PC   9  10  11	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 |  GPIO_Pin_11 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);   
	GPIO_ResetBits(GPIOC,  GPIO_Pin_9 | GPIO_Pin_10 |  GPIO_Pin_11 );	
	
	
	//595  C1  PB 5 6 7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);   
	GPIO_ResetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7 );	
	
	//595  C2  PB   9  10  11		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 |  GPIO_Pin_11 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);   
	GPIO_ResetBits(GPIOB,  GPIO_Pin_9 | GPIO_Pin_10 |  GPIO_Pin_11 );	
	//595  C3  PB   13  14  15	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 |  GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);   
	GPIO_ResetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 |  GPIO_Pin_15 );	
	//595  C4  PC   1  2  3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 |  GPIO_Pin_3 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);   
	GPIO_ResetBits(GPIOC,GPIO_Pin_1 | GPIO_Pin_2 |  GPIO_Pin_3 );	
	
	//595  C5  PC  5 6 7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);   
	GPIO_ResetBits(GPIOC, GPIO_Pin_5 | GPIO_Pin_6 |  GPIO_Pin_7 );	
	
	
	
	leds_flash_timer=0;
}


void leds_595_C0_WR_byte(uint8_t value)
{
	//595  C0  PC   9  10  11  load clk din
	uint8_t i;
	for(i=0;i<8;i++)  
	{
		GPIO_ResetBits(GPIOC,GPIO_Pin_10);
		delay_us(100);
		if(0x80&value)
			GPIO_SetBits(GPIOC,GPIO_Pin_11);
		else
			GPIO_ResetBits(GPIOC,GPIO_Pin_11);
		value = value <<1 ;
		delay_us(100);
		GPIO_SetBits(GPIOC,GPIO_Pin_10);
		delay_us(100);
	}
	GPIO_ResetBits(GPIOC,GPIO_Pin_10);
	GPIO_ResetBits(GPIOC,GPIO_Pin_11);	 
}


void leds_595_C1_WR_byte(uint8_t value)
{
	//595  C1  PB 5 6 7 load clk din
	uint8_t i;
	for(i=0;i<8;i++)  
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_6);
		delay_us(100);
		if(0x80&value)
			GPIO_SetBits(GPIOB,GPIO_Pin_7);
		else
			GPIO_ResetBits(GPIOB,GPIO_Pin_7);
		value = value <<1 ;
		delay_us(100);
		GPIO_SetBits(GPIOB,GPIO_Pin_6);
		delay_us(100);
	}
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
	GPIO_ResetBits(GPIOB,GPIO_Pin_7);	 
}


void leds_595_C2_WR_byte(uint8_t value)
{
	//595  C2  PB   9  10  11	 load clk din
	uint8_t i;
	for(i=0;i<8;i++)  
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_10);
		delay_us(100);
		if(0x80&value)
			GPIO_SetBits(GPIOB,GPIO_Pin_11);
		else
			GPIO_ResetBits(GPIOB,GPIO_Pin_11);
		value = value << 1 ;
		delay_us(100);
		GPIO_SetBits(GPIOB,GPIO_Pin_10);
		delay_us(100);
	}
	GPIO_ResetBits(GPIOB,GPIO_Pin_10);
	GPIO_ResetBits(GPIOB,GPIO_Pin_11); 
}


void leds_595_C3_WR_byte(uint8_t value)
{
	//595  C3  PB   13  14  15		 load clk din
	uint8_t i;
	for(i=0;i<8;i++)  
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_14);
		delay_us(100);
		if(0x80&value)
			GPIO_SetBits(GPIOB,GPIO_Pin_15);
		else
			GPIO_ResetBits(GPIOB,GPIO_Pin_15);
		value = value << 1 ;
		delay_us(100);
		GPIO_SetBits(GPIOB,GPIO_Pin_14);
		delay_us(100);
	}
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);
	GPIO_ResetBits(GPIOB,GPIO_Pin_15); 
}

void leds_595_C4_WR_byte(uint8_t value)
{
	//595  C4  PC   1  2  3		 load clk din
	uint8_t i;
	for(i=0;i<8;i++)  
	{
		GPIO_ResetBits(GPIOC,GPIO_Pin_2);
		delay_us(100);
		if(0x80&value)
			GPIO_SetBits(GPIOC,GPIO_Pin_3);
		else
			GPIO_ResetBits(GPIOC,GPIO_Pin_3);
		value = value << 1 ;
		delay_us(100);
		GPIO_SetBits(GPIOC,GPIO_Pin_2);
		delay_us(100);
	}
	GPIO_ResetBits(GPIOC,GPIO_Pin_2);
	GPIO_ResetBits(GPIOC,GPIO_Pin_3);	
}

void leds_595_C5_WR_byte(uint8_t value)
{
	//595  C5  PC  5 6 7 load clk din
	uint8_t i;
	for(i=0;i<8;i++)  
	{
		GPIO_ResetBits(GPIOC,GPIO_Pin_6);
		delay_us(100);
		if(0x80&value)
			GPIO_SetBits(GPIOC,GPIO_Pin_7);
		else
			GPIO_ResetBits(GPIOC,GPIO_Pin_7);
		value = value <<1 ;
		delay_us(100);
		GPIO_SetBits(GPIOC,GPIO_Pin_6);
		delay_us(100);
	}
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);	 
}

void leds_display_buf_set(uint16_t *buf)
{
	uint8_t i,j;
	uint16_t temp,temp2;;
	//leds_595_display_buf
	for(i=0;i<21;i++)
	{
		temp=buf[i];
		if(temp>14)
			temp=14;
		temp2=0;
		for(j=0;j<temp;j++)
		{
			temp2 |= (0x2000>>j);
		}
		leds_595_display_buf[i]=~temp2;
	}
}


void leds_flash(void)
{
	uint8_t i;
	//leds_595_display_buf
	for(i=0;i<21;i++)
	{
		switch(i)
		{
			case 0:
				
				leds_595_C0_WR_byte(leds_595_display_buf[i]>>8);
				leds_595_C0_WR_byte(leds_595_display_buf[i]&0xff);
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				leds_595_C1_WR_byte(leds_595_display_buf[i]>>8);
				leds_595_C1_WR_byte(leds_595_display_buf[i]&0xff);
				break;
			case 5:
			case 6:
			case 7:
			case 8:			
				leds_595_C4_WR_byte(leds_595_display_buf[i]>>8);
				leds_595_C4_WR_byte(leds_595_display_buf[i]&0xff);
				break;
			case 9:
			case 10:
			case 11:
			case 12:
				leds_595_C5_WR_byte(leds_595_display_buf[i]>>8);
				leds_595_C5_WR_byte(leds_595_display_buf[i]&0xff);
				break;
			case 13:
			case 14:
			case 15:
			case 16:
				leds_595_C3_WR_byte(leds_595_display_buf[i]>>8);
				leds_595_C3_WR_byte(leds_595_display_buf[i]&0xff);
				break;
			case 17:
			case 18:
			case 19:
			case 20:
				leds_595_C2_WR_byte(leds_595_display_buf[i]>>8);
				leds_595_C2_WR_byte(leds_595_display_buf[i]&0xff);
				break;
		}
	}
	GPIO_SetBits(GPIOC,GPIO_Pin_9);
	GPIO_SetBits(GPIOB,GPIO_Pin_5);
	GPIO_SetBits(GPIOB,GPIO_Pin_9);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	GPIO_SetBits(GPIOC,GPIO_Pin_1);
	GPIO_SetBits(GPIOC,GPIO_Pin_5);
	
	delay_ms(100);
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_9);
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);
	GPIO_ResetBits(GPIOB,GPIO_Pin_9);
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	GPIO_ResetBits(GPIOC,GPIO_Pin_1);
	GPIO_ResetBits(GPIOC,GPIO_Pin_5);
	
	
}

void leds_595_test()
{
	leds_595_C0_WR_byte(0xaa);
	leds_595_C0_WR_byte(0xaa);
	GPIO_SetBits(GPIOC,GPIO_Pin_9);
	delay_ms(100);
	GPIO_ResetBits(GPIOC,GPIO_Pin_9);
}

	
              