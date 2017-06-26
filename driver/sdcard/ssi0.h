//******************************************************************************
/// \Filename: ssi0.h
/// \Author: Hoang Tran
/// \Date: June 2017 
/// \Brief: SSI0 module for diskio.h to interface with SD card
//******************************************************************************

#ifndef _SSI0_H_
#define _SSI0_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "integer.h"
#include "stdbool.h"
//******************************************************************************
/// \brief Initialize Timer5 for hardware-dependent function
///			If there is a need for porting, make sure to implement an appropriate
///			peripheral that functions like a timer.
/// \param[in] none
/// \return none
//******************************************************************************
void Timer5_Init(void);

//******************************************************************************
/// \brief Initialize SSI0 interface to SDC
/// \param[in] clock divider to set clock frequency
///				assumes: system clock rate is 80 MHz, SCR=0
///				SSIClk = SysClk / (CPSDVSR * (1 + SCR)) = 80 MHz/CPSDVSR
///				200	for 400 Kbps slow mode, used during initialization
///				8 	for 10 Mbps fast mode, used during disk I/O
/// \return none
//******************************************************************************
void SSI0_Init(unsigned long CPSDVSR);

//******************************************************************************
/// \brief set the SSI speed to the max setting
/// \param[in] none
/// \return none
//******************************************************************************
void set_max_speed(void);

//******************************************************************************
/// \brief asserts the CS pin to the card
/// \param[in] none
/// \return none
//******************************************************************************
void SELECT(void);

//******************************************************************************
/// \brief de-asserts the CS pin to the card
/// \param[in] none
/// \return none
//******************************************************************************
void DESELECT(void);

//******************************************************************************
/// \brief Transmit and receive a byte to MMC via SPI
/// \param[in] byte to send
/// \return byte received
//******************************************************************************
BYTE xchg_spi(BYTE dat);

//******************************************************************************
/// \brief Receive multiple byte from MMC via SPI  (Platform dependent)
/// \param[out] pointer to storing data variable
/// \return none
//******************************************************************************
void rcvr_spi_m(BYTE *dst);

#ifdef __cplusplus
}
#endif

#endif	/*_SSI0_H_*/
