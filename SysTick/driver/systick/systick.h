//******************************************************************************
/// \Filename: systick.h
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: A System Tick timer
//******************************************************************************

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include "stdint.h"

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void);

// Time delay using busy wait.
// The delay parameter is in units of the core clock. (units of 20 nsec for 50 MHz clock)
void SysTick_Wait(uint32_t delay);

// Time delay using busy wait.
// This assumes 50 MHz system clock.
void SysTick_Wait10ms(uint32_t delay);

#endif /* _SYSTICK_H_ */
