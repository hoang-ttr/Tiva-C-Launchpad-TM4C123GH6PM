//******************************************************************************
/// \Filename: main.c
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: using ADC with interrupt handler
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "driver/pll.h"
#include "driver/uart0.h"

#define SS1 1	//sample sequencer 1

volatile uint8_t pTemp;

//******************************************************************************
/// \brief initialize ADC0.SS1 and GPIPOA to ouput ADC result
/// \param[in] none
/// \return none
//******************************************************************************
void ADC0_Init()
{
	
	// connect clock to gpioA, gipoE and adc0 module
	SYSCTL->RCGCGPIO |= (1<<0)|(1<<4);
	while ((SYSCTL->PRGPIO & ((1<<0)|(1<<4))) == 0){};
	SYSCTL->RCGCADC |= (1<<0);
	while ((SYSCTL->PRADC & (1<<0)) == 0){};

	// set gpio as output and enable digital fucntion
	GPIOA->DIR |= 0xFF;
	GPIOA->DEN |= 0xFF;

	//Initialize adc module A3~PE0
	GPIOE->AFSEL |= 0x01;								// set pins as alternate function
	GPIOE->DEN &= ~0x01;								// disable digital function
	GPIOE->AMSEL |= 0x01;								// enable analog mode (disable analog isolation circuitry)
	GPIOE->ADCCTL = 0;									// NOT using GPIO as external source

	//Initialize sample sequencer SS1 of ADC0
	ADC0->ACTSS &= ~(1<<SS1);							// disable SS1 for configuration
	ADC0->EMUX &= 0;									// select default trigger source by processor
	ADC0->SSMUX1 |= (3<<0);								// choose sampling location for each sample3:0 at A3
	ADC0->SSCTL1 |= (1<<1)|(1<<2);						// signal END at 1st sample with interrupt
	ADC0->IM |=	(1<<SS1);								// interrupt request is sent to NVIC for processing
	ADC0->ACTSS |= (1<<SS1);							// enable SS1
	
	NVIC_SetPriority(ADC0SS1_IRQn, (3<<5));				// set priority for ADC0SS1
	NVIC_EnableIRQ(ADC0SS1_IRQn);						// enable ADC0SS1 IRQ for interrupt
	__enable_irq();
	
	ADC0->PSSI |= (1<<SS1);								// initiate ADC0.SS1
}

int main(void)
{
	PLL_Init(Bus80MHz);
	ADC0_Init();
	while (1){/*Infinite loop*/};
}

void ADC0SS1_Handler(void)
{
	ADC0->ISC = (1<<SS1);			// clear interrupt to prepare for the next sample
	pTemp = (ADC0->SSFIFO1 >> 4);	// store value from the sequence, each FIFO is 12-bit wide
	GPIOA->DATA = (pTemp);			// output the value to LED
	ADC0->PSSI |= (1<<SS1);			// initiate ADC0.SS1
}
