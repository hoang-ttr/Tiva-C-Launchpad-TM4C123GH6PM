//******************************************************************************
/// \Filename:
/// \Author: Hoang Tran
/// \Date: 2017
/// \Brief:
//******************************************************************************

#include <TM4C123GH6PM.h>
#include <FreeRTOS.h>
#include <task.h>

#include "driver/pll.h"
#include "driver/rdm6300/rdm6300.h"
#include "driver/rdm6300/uart0.h"
#include "driver/rdm6300/uart1.h"
#include "driver/lcd_i2c/lcd1602_i2c.h"
#include "driver/sdcard/rtc/ds1307.h"
#include "driver/sdcard/ff.h"
#include "semphr.h"
#include "mytasks.h"
/* SD Card Module */
RTC_Time	presentValue,
			storedValue;

FATFS DiskVolume0;
/* Semaphore Declaration */
extern SemaphoreHandle_t xSDCtoLCD1_BiSem;
extern SemaphoreHandle_t xSDCtoLCD2_BiSem;

static void loopError(char* errtype, unsigned long n);
static void PortF_Init(void);

int main()
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
	PortF_Init();
	UART0_Init();
	UART1_Init();
	DS1307_Init();
	LCD1602_Init();

	UART0_OutString("\n**********Peripheral Initialization**********");

	if (DS1307_SetTime(0xD0, storedValue))
	{
		UART0_OutString("\nInitializing RTC succeeded");
	}
	else
	{
		loopError("\nThere's an error", 0);
		while (1){};
	}

	FRESULT result_ff;
	result_ff = f_mount(&DiskVolume0, "", 1);			// Initializes and mounts the SD card
	switch (result_ff)
	{
		case FR_OK:
			UART0_OutString("\nLogical drive mounted");
			break;
		default:
			loopError("There's an error", result_ff);
			break;
	}

	UART0_OutString("\nInitalization...Done");
	LCD1602_SetString("Init...Done");
	
	if ((xSDCtoLCD1_BiSem =  xSemaphoreCreateBinary()) == NULL) {loopError("Binary Semaphore failed", 0);};
	if ((xSDCtoLCD2_BiSem =  xSemaphoreCreateBinary()) == NULL) {loopError("Binary Semaphore failed", 0);};

	xTaskCreate(vRTC_Print, "RTCPrint", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
	xTaskCreate(vCard_Read, "RFIDRead", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL);
	xTaskCreate(vProcess_Door_Succeed, "DoorOp1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, NULL);
	xTaskCreate(vProcess_Door_Fail, "DoorOp2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, NULL);

	vTaskStartScheduler();

	while (1){};
}

static void loopError(char* errtype, unsigned long n)
{
	GPIOF->DATA = 0x0C;      // turn LED off to indicate error
	UART0_OutString(errtype);
	UART0_OutString(": ");
	UART0_OutUDec(n);
	UART0_OutString("\n");

	for (;;){/*Infinite loop*/};
}

static void PortF_Init(void)
{
	SYSCTL->RCGCGPIO |= (1<<5);								// enable portF
	while ((SYSCTL->PRGPIO & (1<<5)) == 0){};				// wait for portF to be ready

	//GPIOF->LOCK = 0x4C4F434B;								// unlock CR of portF to use PF0, always set this value
	//GPIOF->CR = 0x01;										// unlock portF
	//GPIOF->LOCK = 0;										// re-lock CR again
	GPIOF->PCTL &= ~0xFFFFFFFF;								// GPIO function
	GPIOF->DEN |= (1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0);		// digital enable
	GPIOF->DIR |= (1<<3)|(1<<2)|(1<<1);						// set output
	GPIOF->DIR &= ~((1<<4)|(1<<0));							// set input
	GPIOF->PUR |= (1<<4)|(1<<0);							// pull-down resistor
	GPIOF->DR2R |= (1<<4)|(1<<0);							// 2mA drive
}
