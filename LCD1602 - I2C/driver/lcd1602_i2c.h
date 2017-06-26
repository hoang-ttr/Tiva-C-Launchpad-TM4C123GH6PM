//******************************************************************************
/// \Filename: lcd1602.h
/// \Author: Hoang Tran
/// \Date: May 2017 
/// \Brief: interface LCD1602 using I2C PCF8574AT
//******************************************************************************

/*--------------------------------------------------*/
/*Hardware connection between Launchpad and LCD1602 using I2C PCF8574AT
||	Lauchpad		<==>	PCF8574T
||		PA6			<==>		SCL
||		PA7			<==>		SDA					*/
/*--------------------------------------------------*/

#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>

#define LCD_ADDR	0x7E	// LCD1602 address for I2C (obtained from PCF8574AT

#define LINE1	0x80		// first Line of LCD1602
#define LINE2	0xC0		// second Line of LCD1602

// commands for use with Cmd_Write()
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODE 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORDISPLAYSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for LCD_ENTRYMODE
#define LCD_NOSHIFT 0x00
#define LCD_SHIFT 0x01
#define LCD_DECREMENT 0x00
#define LCD_INCREMENT 0x02

// flags for LCD_DISPLAYCONTROL
#define LCD_DISPLAYOFF 0x00
#define LCD_DISPLAYON 0x04
#define LCD_CURSOROFF 0x00
#define LCD_CURSORON 0x02
#define LCD_BLINKOFF 0x00
#define LCD_BLINKON 0x01

// flags for LCD_CURSORDISPLAYSHIFT
#define LCD_CURSORMOVE 0x00
#define LCD_DISPLAYMOVE 0x08
#define LCD_MOVELEFT 0x00
#define LCD_MOVERIGHT 0x04

// flags for LCD_FUNCTIONSET
#define LCD_4BIT 0x00
#define LCD_8BIT 0x10
#define LCD_1LINE 0x00
#define LCD_2LINE 0x08
#define LCD_5x8 0x00
#define LCD_5x10 0x04

//******************************************************************************
/// \brief 
/// \param[in] 
/// \param[out] 
/// \return 
//******************************************************************************
void LCD1602_Init(void);

//******************************************************************************
/// \brief 
/// \param[in] 
/// \param[out] 
/// \return 
//******************************************************************************
void LCD1602_Send_Cmd(char command);

//******************************************************************************
/// \brief 
/// \param[in] 
/// \param[out] 
/// \return 
//******************************************************************************
void LCD1602_Clear(void);

//******************************************************************************
/// \brief 
/// \param[in] 
/// \param[out] 
/// \return 
//******************************************************************************
void LCD1602_SetChar(char character);

//******************************************************************************
/// \brief 
/// \param[in] 
/// \param[out] 
/// \return 
//******************************************************************************
void LCD1602_SetString(char *pStr);

//******************************************************************************
/// \brief 
/// \param[in] 
/// \param[out] 
/// \return 
//******************************************************************************
void LCD1602_SetUDec(uint8_t number);

//******************************************************************************
/// \brief LCD1602 Set Cursor Position on the Screen
///			Line : LINE1 or LINE2
///			Digit : from 0 ~ 15
/// \param[in] 
/// \param[out] 
/// \return 
//******************************************************************************
void LCD1602_Goto(uint8_t line, uint8_t digit);

#endif /* _LCD_H_ */
