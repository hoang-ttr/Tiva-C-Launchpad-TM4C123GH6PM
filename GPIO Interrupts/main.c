//******************************************************************************
/// \Filename: main.c
/// \Author: Hoang Tran
/// \Date: May 2017
/// \Brief: initialize portF to use 3 RGB LEDs and button,
///			implementing GPIO interrupt
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "driver/pll.h"

//******************************************************************************
/// \brief configure PF4:0 to use LEDs and buttons
/// \param[in] none
/// \return none
//******************************************************************************
void PORTF_Init(void)
{
	SYSCTL->RCGCGPIO |= (1<<5);								// enable portF
	while ((SYSCTL->PRGPIO & (1<<5)) == 0){};				// wait for portF to be ready

	GPIOF->LOCK = 0x4C4F434B;								// unlock CR of portF to use PF0, always set this value
	GPIOF->CR = 0x01;										// unlock portF
	GPIOF->LOCK = 0;										// re-lock CR again
	GPIOF->AMSEL &= ~((1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0));	// disable analog functionality on PD3-0
	GPIOF->AFSEL &= ~((1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0));	// regular port function
	GPIOF->DEN |= (1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0);		// digital enable
	GPIOF->PCTL &= ~0x000FFFFF;								// GPIO function
	GPIOF->DIR |= (1<<3)|(1<<2)|(1<<1);						// set output
	GPIOF->DIR &= ~((1<<4)|(1<<0));							// set input
	GPIOF->PUR |= (1<<4)|(1<<0);							// pull-down resistor
	GPIOF->DR2R |= (1<<4)|(1<<0);							// 2mA drive

	GPIOF->IM &= ~(1<<4)|(1<<0);							// mask (enable) interrupt PF4, PF1
	GPIOF->IS &= ~(1<<4)|(1<<0);							// egde sensitive
	GPIOF->IBE &= ~(1<<4)|(1<<0);							// trigger controlled by IEV
	GPIOF->IEV &= ~(1<<4)|(1<<0);							// falling egde
	GPIOF->ICR |= (1<<4)|(1<<0);							// prior interrupt clear
	GPIOF->IM |= (1<<4)|(1<<0);								// unmask (enable) interrupt PF4, PF1

	NVIC_SetPriority(GPIOF_IRQn, (3<<5));					// set priority for portF to 3
	NVIC_EnableIRQ(GPIOF_IRQn);								// enable  portF IRQn
	__enable_irq();											// global interrupt enable
}

int main(void)
{
	PLL_Init(Bus80MHz);
	PORTF_Init();
	GPIOF->DATA = (1<<1);

	while (1){/*Infinite loop*/};
}

void GPIOF_Handler(void)
{
	if ((GPIOF->MIS & (1<<4)))	// check for exact interrupt source
	{
		GPIOF->ICR |= (1<<4);	// clear masked interrupt status
		GPIOF->DATA ^= (1<<2);
	}
	else if ((GPIOF->MIS & (1<<0)))	// check for exact interrupt source
	{
		GPIOF->ICR |= (1<<0);	// clear masked interrupt status
		GPIOF->DATA ^= (1<<3);
	}
}
