//******************************************************************************
/// \Filename: lcd1602.c
/// \Author: Hoang Tran
/// \Date: May 2017 
/// \Brief: interface LCD1602 using I2C PCF8574AT
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "lcd1602_i2c.h"
#include "i2c1.h"

/// Definitions on how the PCF8574 is connected to the LCD
/// These are Bit-Masks for the special signals and background light
#define RS		(1<<0)
#define RW		(1<<1)
#define E		(1<<2)

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

//******************************************************************************
///
///		\PRIVATE \FUNCTION \PROTOTYPE
///
//******************************************************************************

// LCD1602 Write Commend to LCD module
static void Cmd_Write(uint8_t command);
// LCD1602 Write DATA to LCD module
static void Data_Write(uint8_t data);

//******************************************************************************
///
///		\PUBLIC \FUNCTION
///
//******************************************************************************

void LCD1602_Init(void)
{
	I2C1_Init();

	//osDelay(16);								//Delay 15ms
	Cmd_Write((LCD_FUNCTIONSET|LCD_8BIT)>>4);
	//osDelay(5);								//Delay 5ms
	Cmd_Write((LCD_FUNCTIONSET|LCD_8BIT)>>4);
	//osDelay(1);								//Delay 150us
	Cmd_Write((LCD_FUNCTIONSET|LCD_8BIT)>>4);

	Cmd_Write((LCD_FUNCTIONSET|LCD_4BIT)>>4);
	Cmd_Write(LCD_FUNCTIONSET
				| LCD_4BIT
				| LCD_2LINE
				| LCD_5x8);
	Cmd_Write(LCD_DISPLAYCONTROL
				| LCD_DISPLAYON
				| LCD_CURSOROFF
				| LCD_BLINKOFF);
	Cmd_Write(LCD_ENTRYMODE
				| LCD_NOSHIFT
				| LCD_INCREMENT);
	Cmd_Write(LCD_CLEARDISPLAY);
}

void LCD1602_Clear(void)
{
	Cmd_Write(LCD_CLEARDISPLAY);
}

void LCD1602_Send_Cmd(char command)
{
	Cmd_Write(command);
}
void LCD1602_SetString(char *pStr)
{
	while (*pStr)
	{
		Data_Write(*pStr++);
	}
}

void LCD1602_SetChar(char character)
{
	Data_Write(character);
}

void LCD1602_SetUDec(uint8_t number)
{
	if (number >= 10)
	{
		LCD1602_SetChar(number / 10);
		number = number % 10;
	}
	LCD1602_SetChar(number + '0');
}

void LCD1602_Goto(uint8_t line, uint8_t digit)
{
	Cmd_Write(line + digit);
}

//******************************************************************************
///
///		\PRIVATE \FUNCTION \PROTOTYPE
///
//******************************************************************************

static void Cmd_Write(uint8_t command)
{
	uint8_t cmdHi = ((command & 0xF0) | LCD_BACKLIGHT),
			cmdLo = (((command<<4) & 0xF0) | LCD_BACKLIGHT);

	I2C1_Send1(LCD_ADDR, cmdHi);
	I2C1_Send1(LCD_ADDR, (cmdHi | E));
	I2C1_Send1(LCD_ADDR, (cmdHi & ~E));

	I2C1_Send1(LCD_ADDR, cmdLo);
	I2C1_Send1(LCD_ADDR, (cmdLo | E));
	I2C1_Send1(LCD_ADDR, (cmdLo & ~E));
}

static void Data_Write(uint8_t data)
{
	uint8_t dataHi = ((data & 0xF0) | LCD_BACKLIGHT),
			dataLo = (((data<<4) & 0xF0) | LCD_BACKLIGHT);

	I2C1_Send1(LCD_ADDR, dataHi);
	I2C1_Send1(LCD_ADDR, ((dataHi | RS) | E ));
	I2C1_Send1(LCD_ADDR, ((dataHi | RS) & ~E));

	I2C1_Send1(LCD_ADDR, dataLo);
	I2C1_Send1(LCD_ADDR, ((dataLo | RS) | E ));
	I2C1_Send1(LCD_ADDR, ((dataLo | RS) & ~E));
}
