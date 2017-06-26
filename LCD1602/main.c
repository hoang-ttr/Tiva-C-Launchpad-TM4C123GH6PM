//******************************************************************************
/// \Filename: 
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: 
//******************************************************************************
#include "TM4C123GH6PM.h"
#include "driver/lcd.h"

int main(void)
{
	uint8_t counter = 0;

	SystemInit();
	SystemCoreClockUpdate();
	lcd_init();
	lcd_goto(LINE1, 0);
	lcd_setstring("dkmm DSP");
	lcd_goto(LINE2, 0);
	lcd_setstring("phuc nigga");

    while (1)
    {
		lcd_goto(LINE2, 10);
		lcd_setnumber(counter++);
		for (uint16_t i = 0; i < 0xFFF0; i++); 
    }
}
