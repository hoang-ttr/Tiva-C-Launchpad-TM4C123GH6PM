//******************************************************************************
/// \Filename: DS1307.h
/// \Author: Hoang Tran
/// \Date: May 2017
/// \Brief: provide a function that initializes, sends, and receives the
///			I2C module interfaced with an RTC DS1307
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "hw_i2c.h"
#include "DS1307.h"

#define I2C(i) (1<<i)
#define GPIO(i) (1<<i)

//******************************************************************************
///
///		\PRIVATE \FUNCTION \PROTOTYPE
///
//******************************************************************************

//******************************************************************************
/// \brief set inital address for following read or write
/// \param[in] address of the slave
/// \param[in] initial register (0-7)
/// \return 1 if succeed or 0 if fail
//******************************************************************************
static uint8_t DS1307_SetAddressReg(uint8_t slave, uint8_t reg);

static uint8_t BCD2Dec(uint8_t BCD);
static uint8_t Dec2BCD(uint8_t Dec);
static void DS1307_ValueUpdateToBCD(uint8_t *data ,RTC_Time timeStrc);
static void DS1307_ValueUpdateToDec(uint8_t *data , RTC_Time *timeStrc);

//******************************************************************************
///
///		\PUBLIC \FUNCTION
///
//******************************************************************************

void DS1307_Init(void)
{
	SYSCTL->RCGCI2C |= I2C(0);								// activate I2C0
	while ((SYSCTL->PRI2C & I2C(0)) == 0){};				// ready?
	SYSCTL->RCGCGPIO |= GPIO(1);							// activate port B
	while ((SYSCTL->PRGPIO & GPIO(1)) == 0){};				// ready?

	GPIOB->AFSEL |= (1<<3)|(1<<2);							// enable alt funct on PB2,3
	GPIOB->DEN |= (1<<3)|(1<<2);							// enable digital I/O on PB2,3
	GPIOB->ODR |= (1<<3);             						// enable open drain on PB3 only
	GPIOB->PCTL = (GPIOB->PCTL & 0xFFFF00FF) + 0x00003300;	// configure PB2,3 as I2C
	GPIOB->AMSEL &= ~((1<<3)|(1<<2));						// disable analog functionality on PB2,3

	I2C0->MCR = I2C_MCR_MFE;								// I2C Master Function Enable
	I2C0->MTPR = 39;										// configure for 100 kbps clock
	//TPR = (SysClk / (2*(LP+HP)*CLK)) - 1
	//LP+ HP = 10, CLK= 100kbps for Fast Mode, 	SysClk = 16Mhz

	//NVIC_EnableIRQ(I2C0_IRQn);
	//NVIC_SetPriority(I2C0_IRQn, (3<<5));
	//__enable_irq();
}

uint8_t DS1307_GetTime1(int8_t slave, uint8_t initial_reg, uint8_t *data)
{
	if(!DS1307_SetAddressReg(slave, initial_reg))
		return 0;

	I2C0->MSA = slave & I2C_MSA_SA_M;		// MSA[7:1] is slave address
	I2C0->MSA |= I2C_MSA_RS;				// MSA[0] is 1 for receive
	I2C0->MCS = (0
				| I2C_MCS_STOP				// Data Acknowledge Enable
				| I2C_MCS_START				// Generate START
				| I2C_MCS_RUN);				// I2C Master Enable
	while (I2C0->MCS & I2C_MCS_BUSY){};		// I2C Busy
	if ((I2C0->MCS & I2C_MCS_ERROR) != 0)	// Error
	{
		I2C0->MCS = (0
					| I2C_MCS_STOP);		// Generate STOP

		return 0;							// return 0 if fail
	}
	*data = I2C0->MDR & 0xFF;				// start reading data

	return 1;								// return 1 if succeed
}


uint8_t DS1307_GetTime(uint8_t slave, RTC_Time *timeStrc)
{
	uint8_t length = 7;
	uint8_t data[7];	//store value get from RTC

	if(!DS1307_SetAddressReg(slave, 0))
		return 0;

	I2C0->MSA = slave & I2C_MSA_SA_M;			// MSA[7:1] is slave address
	I2C0->MSA |= I2C_MSA_RS;					// MSA[0] is 1 for receive
	I2C0->MCS = (0
				| I2C_MCS_ACK					// Data Acknowledge Enable
				| I2C_MCS_START					// Generate START
				| I2C_MCS_RUN);					// I2C Master Enable
	while (I2C0->MCS & I2C_MCS_BUSY){};			// I2C Busy
	if ((I2C0->MCS & I2C_MCS_ERROR) != 0)		// Error
	{
		I2C0->MCS = (0
					| I2C_MCS_STOP);			// Generate STOP

		return 0;								// return 0 if fail
	}
	data[0] = I2C0->MDR & 0xFF;					// start reading data

	for (uint8_t reg = 1; reg < (length-1); reg++)
	{
		I2C0->MCS = (0
					| I2C_MCS_ACK				// Data Acknowledge Enable
					| I2C_MCS_RUN);				// I2C Master Enable
		while (I2C0->MCS & I2C_MCS_BUSY){};		// I2C Busy
		if ((I2C0->MCS & I2C_MCS_ERROR) != 0)	// Error
		{
			I2C0->MCS = (0
						| I2C_MCS_STOP);		// Generate STOP

			return 0;							// return 0 if fail
		}
		data[reg] = I2C0->MDR & 0xFF;			// almost done, near last data
	}

	I2C0->MCS = (0
				| I2C_MCS_STOP					// Generate STOP
				| I2C_MCS_RUN);					// I2C Master Enable
	while (I2C0->MCS & I2C_MCS_BUSY){};			// I2C Busy
	if ((I2C0->MCS & I2C_MCS_ERROR) != 0)		// Error
	{
		I2C0->MCS = (0
					| I2C_MCS_STOP);			// Generate STOP

		return 0;								// return 0 if fail
	}
	data[length-1] = I2C0->MDR & 0xFF;			// last byte to receive

	DS1307_ValueUpdateToDec(data, timeStrc);	// convert BCD data of RTS into decimal

	return 1;									// return 1 if succeed
}

uint8_t DS1307_SetTime(uint8_t slave, RTC_Time timeStrc)
{
	uint32_t result;		// result of each transfer
	uint8_t length = 7;		// amount of registers written
	uint8_t data[7];		// store value get from RTC

	DS1307_ValueUpdateToBCD(data, timeStrc);		// convert decimal into BCD to write into DS1307

	while (I2C0->MCS & I2C_MCS_BUSY){};				// wait for I2C ready

	I2C0->MSA = slave & I2C_MSA_SA_M;				// MSA[7:1] is slave address
	I2C0->MSA &= ~I2C_MSA_RS;						// MSA[0] is 0 for send
	I2C0->MDR = 0;									// always start from first register (second)
	I2C0->MCS = (0
				| I2C_MCS_START						// Generate START
				| I2C_MCS_RUN);						// I2C Master Enable

	while (I2C0->MCS & I2C_MCS_BUSY){};				// I2C Busy
	result = (I2C0->MCS & (I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
	if (result != 0)								// check error bits
	{
		I2C0->MCS = (0
					| I2C_MCS_STOP);				// Generate STOP

		return 0;									// return 0 if fail
	}

	for (uint8_t reg = 0; reg < (length-1); reg++)	// start sendind data
	{
		I2C0->MDR = data[reg] & 0xFF;
		I2C0->MCS = (0
					| I2C_MCS_RUN);					// I2C Master Enable
		while (I2C0->MCS & I2C_MCS_BUSY){};			// I2C Busy
		result = (I2C0->MCS & (I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
		if (result != 0)							// check error bits
		{
			I2C0->MCS = (0
						| I2C_MCS_STOP);			// Generate STOP

			return 0;								// return 0 if fail
		}
	}

	I2C0->MDR = data[length-1] & 0xFF;				// prepare last byte
	I2C0->MCS = (0
				| I2C_MCS_STOP						// Generate STOP
				| I2C_MCS_RUN);						// I2C Master Enable
	while (I2C0->MCS & I2C_MCS_BUSY){};				// I2C Busy
	result = (I2C0->MCS & (I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
	if (result != 0)								// check error bits
	{
		I2C0->MCS = (0
					| I2C_MCS_STOP);				// Generate STOP

		return 0;									// return 0 if fail
	}

	return 1;										// return 1 if succeed
}

//******************************************************************************
///
///		\PRIVATE \FUNCTION
///
//******************************************************************************

static uint8_t DS1307_SetAddressReg(uint8_t slave, uint8_t reg)
{
	while (I2C0->MCS & I2C_MCS_BUSY){};		// I2C Busy

	I2C0->MSA = slave & I2C_MSA_SA_M;		// MSA[7:1] is slave address
	I2C0->MSA &= ~I2C_MSA_RS;				// MSA[0] is 0 for send
	I2C0->MDR = reg & 0xFF;					// prepare first byte
	I2C0->MCS = (0
				| I2C_MCS_START				// Generate START
				| I2C_MCS_RUN);				// I2C Master Enable
	while (I2C0->MCS & I2C_MCS_BUSY){};		// wait for transmission done
	uint32_t result = (I2C0->MCS & (I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
	if (result != 0)						// check error bits
	{
		I2C0->MCS = (0
					| I2C_MCS_STOP);		// Generate STOP

		return 0;							// return 0 if fail
	}

	return 1;								// return 1 if succeed
}

uint8_t BCD2Dec(uint8_t BCD)
{
	uint8_t Low,High;
	Low = BCD&0x0F;
	High = (BCD>>4)*10;

	return (High+Low);
}

uint8_t Dec2BCD(uint8_t Dec)
{
	uint8_t Low,High;
	Low = Dec%10;
	High = (Dec/10)<<4;

	return (High+Low);
}

void DS1307_ValueUpdateToBCD(uint8_t *data , RTC_Time timeStrc)
{
	data[0] = Dec2BCD(timeStrc.Sec);
	data[1] = Dec2BCD(timeStrc.Min);

	if (timeStrc.Mode12 != 0)
	{
		if (timeStrc.Hour > 12)
		{
			timeStrc.Hour-=12;
			timeStrc.AmPm = 1;
		}
		data[2] = Dec2BCD(timeStrc.Hour)|(timeStrc.Mode12<<6)|(timeStrc.AmPm<<5);
	}
	else data[2] = Dec2BCD(timeStrc.Hour);

	data[3] = Dec2BCD(timeStrc.Day);
	data[4] = Dec2BCD(timeStrc.Date);
	data[5] = Dec2BCD(timeStrc.Month);
	data[6] = Dec2BCD(timeStrc.Year);
}

void DS1307_ValueUpdateToDec(uint8_t *data , RTC_Time *timeStrc)
{
	timeStrc->Sec = BCD2Dec(data[0]);
	timeStrc->Min = BCD2Dec(data[1]);

	if ((data[2] & 0x40))	// 12h mode
	{
		timeStrc->Mode12 = 1;
		if ((data[2] & 0x20))
			timeStrc->AmPm = 0;	// PM mode
		else timeStrc->AmPm = 1;
		timeStrc->Hour = BCD2Dec((data[2]&0x1F));
	}
	else
	{
		timeStrc->Mode12 = 0;
		timeStrc->Hour = BCD2Dec((data[2]&0x3F));
	}

	timeStrc->Day = BCD2Dec(data[3]);
	timeStrc->Date = BCD2Dec(data[4]);
	timeStrc->Month = BCD2Dec(data[5]);
	timeStrc->Year = BCD2Dec(data[6]);
}
