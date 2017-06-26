//******************************************************************************
/// \Filename: i2c1.c
/// \Author: Hoang Tran
/// \Date: May 2017 
/// \Brief: provide a function that initializes, sends, and receives using
///			I2C1 peripheral to interface with PCF8574T
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "hw_i2c.h"
#include "i2c1.h"

#define I2C(i) (1<<i)		
#define GPIO(i) (1<<i)

//******************************************************************************
///
///		\PUBLIC \FUNCTION
///
//******************************************************************************
void I2C1_Init(void)
{
	SYSCTL->RCGCI2C |= I2C(1);					// activate I2C1
	while ((SYSCTL->PRI2C & I2C(1)) == 0){};				// ready?
	SYSCTL->RCGCGPIO |= GPIO(0);				// activate GPIOA
	while ((SYSCTL->PRGPIO & GPIO(0)) == 0){};	// ready?

	GPIOA->DEN |= (1<<6)|(1<<7);							// 5) enable digital I/O on PA7:6
	GPIOA->AFSEL |= (1<<6)|(1<<7);							// 3) enable alt funct on PA7:6
	GPIOA->ODR |= (1<<7);             						// 4) enable open drain on SDA
	GPIOA->PCTL = (GPIOA->PCTL & 0x00FFFFFF) + 0x33000000;	// 6) configure PA7:6 as I2C
	GPIOA->AMSEL &= ~((1<<6)|(1<<7));						// disable analog functionality on PB2,3

	I2C1->MCR = I2C_MCR_MFE;								// 9) I2C Master Function Enable
	I2C1->MTPR = 39;										// 8) configure for 100 kbps clock
	//TPR = (SysClk / (2*(LP+HP)*CLK)) - 1
	//LP+ HP = 10, CLK= 100kbps for Fast Mode, 	SysClk = 16Mhz
	//NVIC_EnableIRQ(I2C1_IRQn);
	//NVIC_SetPriority(I2C1_IRQn, (3<<5));
	//__enable_irq();
}

uint8_t I2C1_Recv1(int8_t slave, uint8_t *data)
{
	while (I2C1->MCS & I2C_MCS_BUSY){};		// wait for I2C ready

	I2C1->MSA = slave & I2C_MSA_SA_M;		// MSA[7:1] is slave address
	I2C1->MSA |= I2C_MSA_RS;				// MSA[0] is 1 for receive
	I2C1->MCS = (0
				| I2C_MCS_STOP				// Data Acknowledge Enable
				| I2C_MCS_START				// Generate START
				| I2C_MCS_RUN);				// I2C Master Enable
	while (I2C1->MCS & I2C_MCS_BUSY){};		// wait for transmission done
	if ((I2C1->MCS & I2C_MCS_ERROR) != 0)	// check error bits
	{
		I2C1->MCS = (0	// send stop if nonzero
					| I2C_MCS_STOP);	// stop

		return 0;	// return error bits if nonzero
	}
	*data = I2C1->MDR & 0xFF;				// Start reading data
	
	return 1;				// usually returns 0xFFFF on error
}

uint8_t I2C1_Send1(uint8_t slave, uint8_t data)
{
	uint32_t result;

	while (I2C1->MCS & I2C_MCS_BUSY){};		// wait for I2C ready

	I2C1->MSA = slave & I2C_MSA_SA_M;	// MSA[7:1] is slave address
	I2C1->MSA &= ~I2C_MSA_RS;				// MSA[0] is 0 for send
	I2C1->MDR = data & 0xFF;         // prepare first byte
	I2C1->MCS = (0
				| I2C_MCS_STOP		// Data Acknowledge Enable
				| I2C_MCS_START    // generate start/restart
				| I2C_MCS_RUN);    // master enable

	while (I2C1->MCS & I2C_MCS_BUSY){};// wait for transmission done	
	result = (I2C1->MCS & (I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
	if (result != 0)	// check error bits
	{
		I2C1->MCS = (0					// send stop if nonzero
					| I2C_MCS_STOP);     // stop

		return (uint8_t)result;	// return error bits if nonzero
	}

	return 1;	// return error bits
}
