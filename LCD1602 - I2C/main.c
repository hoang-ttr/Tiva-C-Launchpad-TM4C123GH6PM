//******************************************************************************
/// \Filename: main.c 
/// \Author: Hoang Tran
/// \Date: May 2017
/// \Brief: Demo for interfacing with LCD1602 using I2C1
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "driver/pll.h"
#include "driver/lcd1602_i2c.h"
#include "driver/uart0/uart0.h"

int main(void)
{
	PLL_Init(Bus80MHz);
	UART0_Init();
	LCD1602_Init();

	LCD1602_Goto(LINE1, 1);
	LCD1602_SetString("testing...");
	LCD1602_Goto(LINE2, 1);
	LCD1602_SetString("embedded boi");
    while (1){};
}
