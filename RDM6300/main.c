//******************************************************************************
/// \Filename: 
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: 
//******************************************************************************

#include "TM4C123GH6PM.h"
#include "driver/pll.h"
#include "driver/uart0.h"
#include "driver/uart1.h"
#include "driver/RDM6300.h"

//******************************************************************************
/// \brief initialize portF for LEDs and buttonss
/// \param[in] none
/// \return none
//******************************************************************************
void PortF_Init(void)
{
	SYSCTL->RCGCGPIO |= (1<<5);								// enable portF
	while ((SYSCTL->PRGPIO & (1<<5)) == 0){};				// wait for portF to be ready

	GPIOF->DEN |= (1<<3)|(1<<2)|(1<<1);				// digital enable
	GPIOF->PCTL &= ~0x0000FFF0;						// GPIO function
	GPIOF->DIR |= (1<<3)|(1<<2)|(1<<1);				// set output
}

char CodeSequence[SequenceLength];	//Storing ASCII data format

int main()
{
	PLL_Init(Bus80MHz);
	UART0_Init();
	UART1_Init();
	PortF_Init();

	UART0_OutString("\nRDM6300 initialized...");
	
	for (;;)
	{	
		if (Read_Tag(CodeSequence))
			Analyze_Tag(CodeSequence);
	}
}
