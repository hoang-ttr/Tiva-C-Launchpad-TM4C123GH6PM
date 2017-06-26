#include "stdio.h"
#include "cmsis_os.h"
#include "TM4C123GH6PM.h"
#include "LCD.h"

/*------------------------------Local function prototype------------------------------*/

// LCD1602 Initial GPIOB, GBIOA
static void gpio_init(void);
//LCD1602 Write Commend to LCD module
static void write_command(uint8_t command);
//LCD1602 Write DATA to LCD module
static void write_data(uint8_t data);

/*------------------------------Main function use------------------------------*/

//LCD1602 Initialize
void lcd_init(void)
{
	gpio_init();
	osDelay(16);				//Delay 15ms
	write_command(0x03);
	osDelay(5);					//Delay 5ms
	write_command(0x03);
	osDelay(1);					//Delay 150us
	write_command(0x03);

	write_command(0x02);
	write_command(0x28);
	write_command(0x0C);
	write_command(0x06);

	osDelay(50);				//Delay 50ms
}

//LCD1602 Clear Screen
void lcd_clear(void)
{
	write_command(0x01);
}

//LCD1602 Display String to Screen
void lcd_setstring(char *pStr)
{
	while (*pStr > 0)
	{
		write_data(*pStr++);
	}
}

//LCD1602 Display Char to Screen
void lcd_setchar(char character)
{
	write_data(character);
}

//LCD1602 Display Decimal number to Screen
void lcd_setnumber(uint8_t number)
{
	if (number >= 10)
	{
		lcd_setchar(number / 10);
		number = number % 10;
	}
	lcd_setchar(number + '0');
}

//LCD1602 Set Cursor Position on the Screen
void lcd_goto(uint8_t line, uint8_t digit)
{
	write_command(line + digit);
}

/*------------------------------Local function------------------------------*/

static void gpio_init(void)
{
	SYSCTL->RCGCGPIO |= (1<<0);
	osDelay(1);
	GPIOA->AMSEL &= ~0xFE;
	GPIOA->PCTL &= ~0xFFF00000;
	GPIOA->DIR |= 0xFE;
	GPIOA->AFSEL &= ~0xFE;
	GPIOA->DEN |= 0xFE;
	GPIOA->DR8R |= 0xFE;
}

static void write_command(uint8_t command)
{
	GPIOA->DATA &= ~(RS+RW+E);     //RS=0,RW=0,E=0

	GPIOA->DATA = (command >> 4);         //Write Command
	GPIOA->DATA |= E;                  //RS=0,RW=0,E=1
	osDelay(1);                       //Enable width 230 ns
	GPIOA->DATA &= ~E;     //RS=0,RW=0,E=0
	osDelay(1);                     //Delay 1 ms

	GPIOA->DATA = (command & 0x0F);         //Write Command
	GPIOA->DATA |= E;                  //RS=0,RW=0,E=1
	osDelay(1);                       //Enable width 230 ns
	GPIOA->DATA &= ~E;     //RS=0,RW=0,E=0
	osDelay(1);                     //Delay 1 ms
}

static void write_data(uint8_t data)
{
	GPIOA->DATA |= RS;               //RS=1,RW=0,E=1
	GPIOA->DATA &= ~(RW + E);

	GPIOA->DATA = (data >> 4);            //Write Data
	GPIOA->DATA |= E;               //RS=1,RW=0,E=1
	osDelay(1);                      //Enable width 230 ns
	GPIOA->DATA &= ~E;     //RS=0,RW=0,E=0
	osDelay(1);                     //Delay 1 ms

	GPIOA->DATA = (data & 0x0F);            //Write Data
	GPIOA->DATA |= E;               //RS=1,RW=0,E=1
	osDelay(1);                      //Enable width 230 ns
	GPIOA->DATA &= ~E;     //RS=0,RW=0,E=0
	osDelay(1);                     //Delay 1 ms
}
