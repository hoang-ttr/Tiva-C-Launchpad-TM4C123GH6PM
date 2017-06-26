//******************************************************************************
/// \Filename: main.c
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: writes a pre-defined time and then reads it back on an UART
//******************************************************************************

// I2C0SCL connected to PB2
// I2C0SDA connected to PB3

#include <TM4C123GH6PM.h>
#include "driver/pll.h"
#include "driver/uart0.h"
#include "driver/DS1307.h"

RTC_Time	presentValue,
			storedValue;

int main(void)
{

	RTC_Time storedValue =
	{
		.Sec = 10,
		.Min = 11,
		.Hour = 18,
		.AmPm = 1,
		.Mode12 = 1,
		.Day = 1,
		.Date = 3,
		.Month = 9,
		.Year = 26,
	};

	PLL_Init(Bus80MHz);
	UART0_Init();
	DS1307_Init();
	UART0_OutString("\nInitializing...");

	if (DS1307_SetTime(0xD0, storedValue))
	{
		UART0_OutString("\nWrite succeeded");
	}
	else
	{
		UART0_OutString("\nThere's an error");
		while (1){};
	}

	if (DS1307_GetTime(0xD0, &presentValue))
	{
		UART0_OutString("\nRead succeeded");
		UART0_OutString("\nTime: ");	UART0_OutUDec(presentValue.Hour);
		UART0_OutString(":");			UART0_OutUDec(presentValue.Min);
		UART0_OutString(":");			UART0_OutUDec(presentValue.Sec);
		if (presentValue.Mode12)
		{
			if (presentValue.AmPm)		UART0_OutString(" AM");
			else						UART0_OutString(" PM");
		}
		UART0_OutString("\nDate: ");	UART0_OutUDec(presentValue.Day);
		UART0_OutString(", ");			UART0_OutUDec(presentValue.Date);
		UART0_OutString("/");			UART0_OutUDec(presentValue.Month);
		UART0_OutString("/20");			UART0_OutUDec(presentValue.Year);
	}
	else
	{
		UART0_OutString("\nThere's an error");
		while (1){};
	}
	uint8_t besttime;
	DS1307_GetTime1(0xD0, 4, &besttime);
	UART0_OutString("\nSingleTime: ");	UART0_OutUDec(besttime);

	while (1){};
}
