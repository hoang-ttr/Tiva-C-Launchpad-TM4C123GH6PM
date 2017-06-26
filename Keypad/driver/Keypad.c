#include "Keypad.h"

void Keypad_Ini(void)
{
	SYSCTL->RCGCGPIO |= (1<<0)|(1<<1)|(1<<3);
	
	GPIOA->DIR &= ~0x3C;
	GPIOA->DEN |= 0x3C;
	GPIOB->DIR |= 0x0F;
	GPIOB->DEN |= 0x0F;
	GPIOD->DIR |= 0x0F;
	GPIOD->DEN |= 0x0F;
}

char Keypad_Read(void)
{
	uint8_t row=0, col=0;
	const uint8_t rowNum=4;
	const char row_sel[]={0x01, 0x02, 0x04, 0x08};
	
	while(row<rowNum)
	{
		GPIOD->DIR &= ~0x0F;
		GPIOD->DIR |= row_sel[row];
		GPIO->DATA |= 0x0F;
		col = GPIOA->DATA & 0x3C;
		if(col!=0) {break;}
		row++;
	}
	
	GPIOD->DIR &= ~0x0F;
	if(col==0) {return col;}
	if(col==0x04) {return (row*4+1);}
	if(col==0x08) {return (row*4+2);}
	if(col==0x10) {return (row*4+3);}
	if(col==0x20) {return (row*4+4);}
}
