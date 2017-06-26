//******************************************************************************
/// \Filename: main.c
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: Demonstration of SD Card including a RTC for precise time.
//******************************************************************************

// **********MCU and SDC*******************
// CS connected to PA3  (SSI0CS)
// SCK connected to PA2 (SSI0Clk)
// MOSI connected to PA5 (SSI0Tx)
// MISO connected to PA4 (SSI0Rx)
// VCC connected to +3.3V
// Gnd connected to ground

#include <string.h>
#include <TM4C123GH6PM.h>
#include "driver/PLL.h"
#include "driver/sdcard/ff.h"
#include "driver/sdcard/rtc/DS1307.h"
#include "driver/uart0/uart0.h"

void diskError(char* errtype, unsigned long n)
{
	GPIOF->DATA = 0x00;      // turn LED off to indicate error
	UART0_OutString(errtype);
	UART0_OutString(": ");
	UART0_OutUDec(n);
	UART0_OutString("\n");

	while (1){/*Infinite loop*/};
}

void PortF_Init(void)
{
	unsigned long volatile delay;
	SYSCTL->RCGCGPIO |= 0x20;	// activate port F
	delay = SYSCTL->RCGCGPIO;
	delay = SYSCTL->RCGCGPIO;
	GPIOF->LOCK = 0x4C4F434B;	// 2) unlock PortF PF0
	//GPIOF->CR |= 0x1F;			// allow changes to PF4-0
	GPIOF->DIR = 0x0E;			// make PF3-1 output (PF3-1 built-in LEDs),PF4,0 input
	GPIOF->PUR = 0x11;			// PF4,0 have pullup
	GPIOF->AFSEL = 0x00;		// disable alt funct on PF4-0
	GPIOF->DEN = 0x1F;			// enable digital I/O on PF4-0
	GPIOF->PCTL = 0x00000000;
	GPIOF->AMSEL = 0;			// disable analog functionality on PF
}

static FATFS testingFatFs;
FIL file1;

int main(void)
{
	PLL_Init(Bus80MHz);
	PortF_Init();
	UART0_Init();
	DS1307_Init();

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

	if (DS1307_SetTime(0xD0, storedValue))
	{
		UART0_OutString("\nInitializing RTC succeeded");
	}

	UART0_OutString("\nInitializing SDC...");

	FRESULT result_ff;
	result_ff = f_mount(&testingFatFs, "", 1);			// Initializes and mounts the SD card
	switch (result_ff)
	{
		case FR_OK:
			UART0_OutString("\nGood to go, logical drive mounted");
			GPIOF->DATA = (7<<1);	// Turns all LEDs on
			break;
		default:
			diskError("\nSomething wrong", result_ff);
			break;
	}

	result_ff = f_open(&file1, "testfile.txt", FA_WRITE | FA_OPEN_ALWAYS | FA_READ);	// Opens a file, if already exists, overrides
	switch (result_ff)
	{
		case FR_OK:
			UART0_OutString("\nFile opened!");
			break;
		default:
			diskError("\nSomething wrong", result_ff);
			break;
	}

	UINT successfulCounts;
	const char *buff = "anh yeu em nhieu lam em co biet khong, de coi xem no write duoc bao nhieu dong la maximum nhen thu xem naoooooo\r\n";	// string to write
	uint8_t stringlength = strlen(buff);
	for (uint8_t i = 0; i<10; i++)
	{
		result_ff = f_write(&file1, buff, strlen(buff), &successfulCounts);
		switch (result_ff)
		{
			case FR_OK:
				UART0_OutString("\nWriting succeeded: ");
				UART0_OutUDec(successfulCounts);
				break;
			default:
				diskError("\nSomething wrong", result_ff);
				break;
		}
	}

	f_lseek(&file1, 0);
	char buffer_read[17];
	result_ff = f_read(&file1, &buffer_read, strlen(buff), &successfulCounts);
	switch (result_ff)
	{
		case FR_OK:
			UART0_OutString("\nData read: ");
			UART0_OutString(buffer_read);
			break;
		default:
			diskError("\nSomething wrong", result_ff);
			break;
	}

	result_ff = f_close(&file1);
	switch (result_ff)
	{
		case FR_OK:
			UART0_OutString("\nFile closed!");
			break;
		default:
			diskError("\nSomething wrong", result_ff);
			break;
	}

	while (1){}
}
