//******************************************************************************
/// \Filename: ssi0.c
/// \Author: Hoang Tran
/// \Date: June 2017 
/// \Brief: SSI0 module for diskio.h to interface with SD card
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "hw_ssi.h"
#include "ssi0.h"

extern void disk_timerproc(void);

//******************************************************************************
///
///		\PUBLIC \FUNCTION
///
//******************************************************************************
void Timer5_Init(void)
{
	volatile uint16_t delay;

	SYSCTL->RCGCTIMER |= 0x20;
	delay = SYSCTL->RCGCTIMER;
	delay = SYSCTL->RCGCTIMER;
	TIMER5->CTL = 0x00000000;				// 1) disable timer5A during setup
	TIMER5->CFG = 0x00000000;				// 2) configure for 32-bit mode
	TIMER5->TAMR = 0x00000002;				// 3) configure for periodic mode, default down-count settings
	TIMER5->TAILR = 7999999;				// 4) reload value, 100 ms, 80 MHz clock
	TIMER5->TAPR = 0;						// 5) bus clock resolution
	TIMER5->ICR = 0x00000001;				// 6) clear timer5A timeout flag
	TIMER5->ICR = 0x00000001;				// 7) arm timeout interrupt

	NVIC_SetPriority(TIMER5A_IRQn, 0x40);	// 8) priority 2
	NVIC_EnableIRQ(TIMER5A_IRQn);			// 9) enable interrupt 92 in NVIC
	__enable_irq();

	TIMER5->CTL = 0x00000001;				// 10) enable timer5A
}


void SSI0_Init(unsigned long CPSDVSR)
{
	Timer5_Init();

	SYSCTL->RCGCGPIO |= (1<<0);	// activate GPIOA
	while ((SYSCTL->RCGCGPIO & (1<<0)) == 0){}; // allow time for clock to stabilize
	SYSCTL->RCGCSSI |= (1<<0);	// activate SSI0
	while((SYSCTL->PRSSI & (1<<0)) == 0){};  // allow time for clock to stabilize

	GPIOA->AFSEL |= 0x34;			// enable alt funct on GPIOA5.4.2
	GPIOA->AFSEL &= ~0x08;			// disable alt funct on GPIOA3
	GPIOA->PUR |= 0x3C;				// enable weak pullup on  GPIOA5:2
	GPIOA->DEN |= 0x3C;				// enable digital I/O on GPIOA5:2
	GPIOA->DIR |= 0x08;				// GPIOA3 output
	GPIOA->DR4R |= 0x3C;			// 4mA Drive
	GPIOA->DATA |= 0x08;			// GPIOA3 high (disable SDCard)
	GPIOA->AMSEL &= ~0x3C;          // disable analog functionality on GPIOA5:2
	GPIOA->PCTL = (GPIOA->PCTL & 0xFF00F0FF) + 0x00220200;	// configure  GPIOA5.4.2 as SSI
	GPIOA->PCTL = (GPIOA->PCTL & 0xFFFF0FFF) + 0x00000000;	// configure  GPIOA3 as default GPIO

	SSI0->CR1 &= ~SSI_CR1_SSE;		// SSI Synchronous Serial Port Disable
	SSI0->CR1 &= ~SSI_CR1_MS;		// SSI Master/Slave Select
	SSI0->CC = (SSI0->CC & ~SSI_CC_CS_M) + SSI_CC_CS_PIOSC;	// configure for clock from source PIOSC for baud clock source

	// clock divider for 8 MHz SSIClk (assumes 16 MHz PIOSC)
	SSI0->CPSR = (SSI0->CPSR & ~SSI_CPSR_CPSDVSR_M) + CPSDVSR;	// CPSDVSR must be even from 2 to 254

	SSI0->CR0 &= ~(SSI_CR0_SCR_M |									// SSI Serial Clock Rate: no additional prescaler
					SSI_CR0_SPH |									// SPH = 0
					SSI_CR0_SPO);									// SPO = 0
	SSI0->CR0 = (SSI0->CR0 & ~SSI_CR0_FRF_M) + SSI_CR0_FRF_MOTO;	// Freescale SPI Frame Format
	SSI0->CR0 = (SSI0->CR0 & ~SSI_CR0_DSS_M) + SSI_CR0_DSS_8;		// 8-bit data
	SSI0->CR1 |= SSI_CR1_SSE;		// SSI Synchronous Serial Port Enable
}

void set_max_speed(void)
{
	SSI0->CR1 &= ~SSI_CR1_SSE;		// SSI Synchronous Serial Port Disable
	SSI0->CPSR = (SSI0->CPSR & ~SSI_CPSR_CPSDVSR_M) + 8;	// 10,000,000 bps (could run at 12 MHz)
	SSI0->CR1 |= SSI_CR1_SSE;		// SSI Synchronous Serial Port Enable
}

void SELECT(void)
{
	GPIOA->DATA &= ~0x08;
	//xchg_spi(0xFF);	/* Dummy clock (force DO enabled) */
}

void DESELECT(void)
{
	GPIOA->DATA |= 0x08;
	xchg_spi(0xFF);	/* Dummy clock (force DO hi-z for multiple slave SPI) */
}

BYTE xchg_spi(BYTE dat)
{
	DWORD volatile rcvdat;
	while ((SSI0->SR & SSI_SR_TNF) == 0){};	// SSI Transmit FIFO Not Full
	SSI0->DR = dat;							// data out
	while ((SSI0->SR & SSI_SR_RNE) == 0){};	// wait until Rx FIFO NOT empty
	return (BYTE)SSI0->DR;                // read received data
}

void rcvr_spi_m(BYTE *dst)
{
	*dst = xchg_spi(0xFF);
}

/* Timer5A Interrupt Handler */
void Timer5A_Handler(void)	// Executed every 10 ms
{
	TIMER5->ICR = 0x00000001;	// acknowledge timer5A timeout
	disk_timerproc();
}
