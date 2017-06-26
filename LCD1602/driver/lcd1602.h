//******************************************************************************
/// \Filename: 
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: 
//******************************************************************************
/*--------------------------------------------------*/
/*Hardware connection between Launchpad and LCD1602 module
||	Lauchpad		<==>	LCD1602
||		PA6			<==>		RS
||		PA5			<==>		R/W, To GND is only enable to write
||		PA7			<==>		E
||		PB0			<==>		D4
||		PB1			<==>		D5
||		PB2			<==>		D6
||		PB3			<==>		D7					*/
/*--------------------------------------------------*/

#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>

#define LINE1 0x80	//First Line of LCD1602
#define LINE2 0xC0	//Second Line of LCD1602
#define RW (1<<1)	//PA1
#define RS (1<<2)	//PA2 
#define E  (1<<3)	//PA3

//----------------------------------------------------------------------//

//LCD1602 Initialize
void lcd_init(void);

//LCD1602 Clear Screen
void lcd_clear(void);

//LCD1602 Display Char to Screen
void lcd_setchar(char character);

//LCD1602 Display String to Screen
void lcd_setstring(char *pStr);

//LCD1602 Display Decimal number to Screen
void lcd_setnumber(uint8_t number);

//LCD1602 Set Cursor Position on the Screen
//	Line : LINE1 or LINE2
//	Digit : from 0 ~ 15
void lcd_goto(uint8_t line, uint8_t digit);

#endif /* _LCD_H_ */
