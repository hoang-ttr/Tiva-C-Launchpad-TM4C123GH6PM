#include "TM4C123GH6PM.h"



int main()
{
	SystemInit();
	SystemCoreClockUpdate();
	
	uint32_t pw, RCC;
	SYSCTL->RCGCPWM |= 0x20;
	SYSCTL->RCGCGPIO |= 0x22;
	SYSCTL->RCGC0 |= 0x02;
		
	GPIOF->LOCK = 1;		//Unlock GPIOF
	GPIOF->CR |= 0x02;	//En
	GPIOF->LOCK = 0;
	//PWM1_2B - PF1 - M1PWM5 pin - Module 1 Generator2 - pwm2B
	PWM1->_2_CTL = 0;
	PWM1->_2_GENB = 0x0000080C;
	PWM1->_2_LOAD = 3999;
	PWM1->_2_CTL = 0x01;
	PWM1->ENABLE = 0x20;
	
	GPIOF->DIR |= 0x02;
	GPIOF->DEN |= 0x02;
	GPIOF->AFSEL |= 0X02;
	GPIOF->PCTL |= ;
	GPIOF->AMSEL &= ~0x02;
	GPIOB->DIR |= 0x03;
	GPIOB->DEN |= 0x03;
	GPIOB->DATA |= 0x01;
	
	while(1){}
}
