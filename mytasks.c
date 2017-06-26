//******************************************************************************
/// \Filename: 
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: 
//******************************************************************************

#include <stdio.h>
#include <TM4C123GH6PM.h>
#include <FreeRTOS.h>
#include <task.h>

#include "driver/rdm6300/rdm6300.h"
#include "driver/rdm6300/uart0.h"
#include "driver/rdm6300/uart1.h"
#include "driver/lcd_i2c/lcd1602_i2c.h"
#include "driver/sdcard/rtc/ds1307.h"
#include "driver/sdcard/ff.h"
#include "semphr.h"
#include "mytasks.h"

/* SD Card Module */
extern RTC_Time	presentValue,
				storedValue;

FIL logger1;

/* RDM6300 Module */
char myTag[SequenceLength];

/* Semaphore Declaration */
SemaphoreHandle_t xSDCtoLCD1_BiSem;
SemaphoreHandle_t xSDCtoLCD2_BiSem;

void vRTC_Print(void *pvParameters)
{
	portTickType lastWake = xTaskGetTickCount();

	for (;;)
	{
		if (DS1307_GetTime(0xD0, &presentValue))
		{
			UART0_OutString("\nTime: ");	UART0_OutUDec(presentValue.Hour);
			UART0_OutString(":");			UART0_OutUDec(presentValue.Min);
			UART0_OutString(":");			UART0_OutUDec(presentValue.Sec);
			if (presentValue.Mode12)
			{
				if (presentValue.AmPm)		UART0_OutString(" AM, ");
				else						UART0_OutString(" PM, ");
			}
			UART0_OutString("Date: ");	UART0_OutUDec(presentValue.Day);
			UART0_OutString(", ");			UART0_OutUDec(presentValue.Date);
			UART0_OutString("/");			UART0_OutUDec(presentValue.Month);
			UART0_OutString("/20");			UART0_OutUDec(presentValue.Year);
		}
		else
		{
			UART0_OutString("\nThere's an error");
			while (1){};
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void vCard_Read(void *vParameters)
{
	portTickType lastWake = xTaskGetTickCount();
	FRESULT result_ff;
	static FSIZE_t currentFilePtr;
	INT successfulCounts;
	uint32_t TagNumber;
	char *accessStat;	// string to write

	for (;;)
	{
		if(Read_Tag(myTag))
		{
			if (Analyze_Tag(myTag, &TagNumber)) /* Access Granted */
			{
				accessStat = "\r\nAccess granted at: ";
				UART0_OutString("\n***Access granted***");
				xSemaphoreGive(xSDCtoLCD1_BiSem);
				
			}
			else /* Access Denied */
			{
				accessStat = "\r\nAccess denied at: ";
				UART0_OutString("\n***Access denied***");
				xSemaphoreGive(xSDCtoLCD2_BiSem);
			}
			result_ff = f_open(&logger1, "logger1.txt", FA_WRITE | FA_OPEN_ALWAYS);	// Opens the file if it is existing. If not, a new file will be created.
			switch (result_ff)
			{
				case FR_OK:
					UART0_OutString("\nFile opened!");
					break;
				default:
					UART0_OutString("\nSomething wrong f_open");
					break;
			}

			result_ff = f_lseek(&logger1, currentFilePtr);
			switch (result_ff)
			{
				case FR_OK:
					UART0_OutString("-Pointer moved!");
					break;
				default:
					UART0_OutString("-Something wrong f_lseek");
					break;
			}

			successfulCounts = f_puts(accessStat ,&logger1);
			switch (successfulCounts)
			{
				case (-1):
					UART0_OutString("-Something wrong f_puts");
					break;
				default:
					UART0_OutString("-Writing succeeded: ");
					UART0_OutUDec(successfulCounts);
					break;
			}

			successfulCounts = f_printf(&logger1, "%d:%d:%d, %d/%d/20%d. Requesting TagID: %ld", presentValue.Hour,
																									presentValue.Min,
																									presentValue.Sec,
																									presentValue.Date,
																									presentValue.Month,
																									presentValue.Year,
																									TagNumber);
			switch (successfulCounts)
			{
				case (-1):
					UART0_OutString("-Something wrong f_printf");
					break;
				default:
					UART0_OutString("-Writing succeeded: ");
					UART0_OutUDec(successfulCounts);
					break;
			}

			currentFilePtr = f_tell(&logger1); // Save the last known pointer position for later use

			result_ff = f_close(&logger1);
			switch (result_ff)
			{
				case FR_OK:
					UART0_OutString("-File closed!");
					break;
				default:
					UART0_OutString("-Something wrong f_close");
					break;
			}
		}
		vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000));
	}
}

void vProcess_Door_Succeed(void *vParameters)
{
	for (;;)
	{
		if (xSemaphoreTake(xSDCtoLCD1_BiSem, portMAX_DELAY) == pdPASS)
		{
			LCD1602_Clear();
			LCD1602_Goto(LINE1, 0);
			LCD1602_SetString("-----Access-----");
			LCD1602_Goto(LINE2, 0);
			LCD1602_SetString("-----Granted----");
			GPIOF->DATA |= (1<<3);
		}
		vTaskDelay(pdMS_TO_TICKS(1500));
		GPIOF->DATA &= ~(1<<3);
		LCD1602_Clear();
	}
}

void vProcess_Door_Fail(void *vParameters)
{
	for (;;)
	{
		if (xSemaphoreTake(xSDCtoLCD2_BiSem, portMAX_DELAY) == pdPASS)
		{
			LCD1602_Clear();
			LCD1602_Goto(LINE1, 0);
			LCD1602_SetString("-----Access-----");
			LCD1602_Goto(LINE2, 0);
			LCD1602_SetString("-----Denied-----");
			GPIOF->DATA |= (1<<1);
		}
		vTaskDelay(pdMS_TO_TICKS(1500));
		GPIOF->DATA &= ~(1<<1);
		LCD1602_Clear();
	 }
}
