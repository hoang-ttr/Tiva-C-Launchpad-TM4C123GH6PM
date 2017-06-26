//******************************************************************************
/// \Filename: uart0.c
/// \Author: Hoang Tran
/// \Date: May 2017
/// \Brief: Use UART0 to implement bidirectional data transfer to and from a
///			computer running HyperTerminal.  This time, interrupts and FIFOs
///			are used
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "hw_uart.h"
#include "fifo.h"
#include "uart0.h"

#define UART(i) (1<<i)
#define GPIO(i) (1<<i)

// create index implementation FIFO (see FIFO.h)
AddIndexFifo(Rx, FIFOSIZE, char, FIFOSUCCESS, FIFOFAIL)
AddIndexFifo(Tx, FIFOSIZE, char, FIFOSUCCESS, FIFOFAIL)
	
//******************************************************************************
///
///		\PRIVATE \FUNCTION \PROTOTYPE
///
//******************************************************************************

// copy from hardware RX FIFO to software RX FIFO
// stop when hardware RX FIFO is empty or software RX FIFO is full
static void copyHardwareToSoftware(void);
// copy from software TX FIFO to hardware TX FIFO
// stop when software TX FIFO is empty or hardware TX FIFO is full
static void copySoftwareToHardware(void);

//******************************************************************************
///
///		\PUBLIC \FUNCTION
///
//******************************************************************************

void UART0_Init(void)
{
	SYSCTL->RCGCUART |= UART(0);	// activate UART0
	SYSCTL->RCGCGPIO |= GPIO(0);	// activate port A

	GPIOA->AFSEL |= (1<<1)|(1<<0);							// enable alt funct on PA1-0
	GPIOA->DEN |= (1<<1)|(1<<0);							// enable digital I/O on PA1-0
	GPIOA->PCTL = (GPIOA->PCTL & 0xFFFFFF00)+0x00000011;	// configure PA1-0 as UART
	GPIOA->AMSEL = ~((1<<1)|(1<<0));						// disable analog functionality on PA

	RxFifo_Init();	// initialize empty FIFOs
	TxFifo_Init();

	UART0->CTL &= ~UART_CTL_UARTEN;						// UART Disable
	UART0->IBRD = 43;									// IBRD = int(80,000,000 / (16 * 115,200)) = int(43.402777)
	UART0->FBRD = 26;									// FBRD = int(0.402777 * 64 + 0.5) = int(26.2777)
	UART0->LCRH = (UART_LCRH_WLEN_8						// 8-N-1 frame
					|UART_LCRH_FEN);					// UART Enable FIFOs
	UART0->IFLS &= ~(UART_IFLS_RX_M|UART_IFLS_TX_M);	// clear TX and RX interrupt FIFO level fields
	UART0->IFLS += (UART_IFLS_TX1_8						// TX FIFO <= 1/8 full
					| UART_IFLS_RX1_8);					// RX FIFO >= 1/8 full
	UART0->IM |= (UART_IM_RXIM							// UART Receive Interrupt Mask
					| UART_IM_TXIM						// UART Transmit Interrupt Mask
					| UART_IM_RTIM);					// UART Receive Time-Out Interrupt
	UART0->CTL |= (UART_CTL_RXE							// UART Receive Enable
					| UART_CTL_TXE						// UART Transmit Enable
					| UART_CTL_UARTEN);					// UART Enable

	NVIC_SetPriority(UART0_IRQn, (2<<5));	// UART0=priority 2
	NVIC_EnableIRQ(UART0_IRQn);
	__enable_irq();
}


char UART0_InChar(void)
{
	char letter;

	while (RxFifo_Get(&letter) == FIFOFAIL){};

	return letter;
}
void UART0_OutChar(char data)
{
	while (TxFifo_Put(data) == FIFOFAIL){};

	UART0->IM &= ~UART_IM_TXIM;	// disable TX FIFO interrupt to safely transfer data from SFIFO to HFIFO
	copySoftwareToHardware();	// pass data into hardware FIFO
	UART0->IM |= UART_IM_TXIM;	// enable TX FIFO interrupt
}
void UART0_OutString(char *pt)
{
	while (*pt)
	{
		UART0_OutChar(*pt++);
	}
}
uint32_t UART0_InUDec(void)
{
	uint32_t	number=0,
				length=0;
	char character;
	
	character = UART0_InChar();
	
	while (character != CR)
	{ // accepts until <enter> is typed
	// The next line checks that the input is a digit, 0-9.
	// If the character is not 0-9, it is ignored and not echoed
		if ((character>='0') && (character<='9'))
		{
			number = 10*number+(character-'0');   // this line overflows if above 4294967295
			length++;
			UART0_OutChar(character);
		}
		// If the input is a backspace, then the return number is
		// changed and a backspace is outputted to the screen
		else if ((character==BS) && length)	
		{
			number /= 10;
			length--;
			UART0_OutChar(character);
		}
		character = UART0_InChar();
	}

	return number;
}
void UART0_OutUDec(uint32_t n)
{
	// This function uses recursion to convert decimal number
	//   of unspecified length as an ASCII string
	if (n >= 10)
	{
		UART0_OutUDec(n/10);
		n = n%10;
	}
	UART0_OutChar(n+'0'); /* n is between 0 and 9 */
}
uint32_t UART0_InUHex(void)
{
	uint32_t	number=0,
				digit,
				length=0;
	char character;
	
	character = UART0_InChar();
	while (character != CR)
	{
		digit = 0x10; // assume bad
		if ((character>='0') && (character<='9'))
		{
			digit = character-'0';
		}
		else if ((character>='A') && (character<='F'))
		{
			digit = (character-'A')+0xA;
		}
		else if ((character>='a') && (character<='f'))
		{
			digit = (character-'a')+0xA;
		}
			// If the character is not 0-9 or A-F, it is ignored and not echoed
		if (digit <= 0xF)
		{
			number = number*0x10+digit;
			length++;
			UART0_OutChar(character);
		}
			// Backspace outputted and return value changed if a backspace is inputted
		else if ((character==BS) && length)
		{
			number /= 0x10;
			length--;
			UART0_OutChar(character);
		}
		character = UART0_InChar();
	}

	return number;
}
void UART0_OutUHex(uint32_t number)
{
	// This function uses recursion to convert the number of
	//   unspecified length as an ASCII string
	if (number >= 0x10)
	{
		UART0_OutUHex(number/0x10);
		UART0_OutUHex(number%0x10);
	}
	else
	{
		if (number < 0xA)
		{
			UART0_OutChar(number+'0');
		}
		else
		{
			UART0_OutChar((number-0x0A)+'A');
		}
	}
}
void UART0_InString(char *bufPt, uint16_t max)
{
	uint16_t length=0;
	char character;
	
	character = UART0_InChar();	// get the character
	while (character != CR)		// if not carriage return
	{
		if (character == BS)	// if not delete
		{
			if (length)			// there is charater to delete
			{
				bufPt--;
				length--;
				UART0_OutChar(BS);
			}
		}
		else if (length < max)	// there is still space available
		{
			*bufPt = character;
			bufPt++;
			length++;
			UART0_OutChar (character);
		}
		character = UART0_InChar();	// get the next charater
	}
}

//******************************************************************************
///
///		\PRIVATE \FUNCTION
///
//******************************************************************************

static void copyHardwareToSoftware(void)
{
	char letter;
	
	while (((UART0->FR & UART_FR_RXFE) == 0) && (RxFifo_Size() < (FIFOSIZE - 1)))
	{
		letter = UART0->DR;
		RxFifo_Put(letter);
	}
}
static void copySoftwareToHardware(void)
{
	char letter;

	while (((UART0->FR & UART_FR_TXFF) == 0) && (TxFifo_Size() > 0))
	{
		TxFifo_Get(&letter);
		UART0->DR = letter;
		if (letter ==  '\n')
			UART0->DR = '\r';
	}
}

// at least one of three things has happened:
// hardware TX FIFO goes from 3 to 2 or less items
// hardware RX FIFO goes from 1 to 2 or more items
// UART receiver has timed out
void UART0_Handler(void)
{
	if (UART0->RIS & UART_RIS_TXRIS)	// hardware TX FIFO <= 2 items
	{
		UART0->ICR = UART_ICR_TXIC;	// acknowledge TX FIFO
		copySoftwareToHardware();

	}
	else if (UART0->RIS & UART_RIS_RXRIS)	// hardware RX FIFO >= 2 items
	{
	
		UART0->ICR = UART_ICR_RXIC;	// acknowledge RX FIFO
		copyHardwareToSoftware();
	}
	else if (UART0->RIS & UART_RIS_RTRIS)	// receiver timed out
	{
		UART0->ICR = UART_ICR_RTIC;	// acknowledge receiver time out
		copyHardwareToSoftware();
	}
}
