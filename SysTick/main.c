//******************************************************************************
/// \Filename: 
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: 
//******************************************************************************

#include "TM4C123GH6PM.h"
#include "driver/pll.h"
#include "driver/systick/systick.h"

int main(void)
{
	PLL_Init(Bus80MHz);         // set system clock to 50 MHz
	SysTick_Init();             // initialize SysTick timer

	SYSCTL->RCGCGPIO |= (1<<5);  // activate port F
	GPIOF->DIR |= (1<<2);   // make PF2 out (built-in blue LED)
	GPIOF->DEN |= (1<<2);   // enable digital I/O on PF2
	GPIOF->PCTL = (GPIOF->PCTL&0xFFFFF0FF)+0x00000000;	// configure PF2 as GPIO

	for (;;)
	{
		GPIOF->DATA ^= (1<<2); // toggle PF2
		//    SysTick_Wait(1);        // approximately 720 ns
		//    SysTick_Wait(2);        // approximately 720 ns
		//    SysTick_Wait(10000);    // approximately 0.2 ms
		SysTick_Wait10ms(1);      // approximately 10 ms
	}
}
