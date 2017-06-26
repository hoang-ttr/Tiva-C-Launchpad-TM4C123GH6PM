//******************************************************************************
/// \Filename: main.c
/// \Author: Hoang Tran
/// \Date: May 2017
/// \Brief: This program is used to debug UART0
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "driver/pll.h"
#include "driver/uart0.h"

int main(void)
{
	char *buffer;

	PLL_Init(Bus80MHz);
	UART0_Init();	// initialize UART
	for(char i='A'; i<='Z'; i=i+1)
	{
		UART0_OutChar(i);	// print the uppercase alphabet
	}

	while (1)
	{
		UART0_OutString("\nEnter loopback string: ");
		UART0_InString(buffer, 10);
		UART0_OutString("\nLoopback string: ");
		UART0_OutString(buffer);
	}
}
