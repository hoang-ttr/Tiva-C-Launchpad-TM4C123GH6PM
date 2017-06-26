//******************************************************************************
/// \Filename: uart0.h
/// \Author: Hoang Tran
/// \Date: May 2017
/// \Brief: Use UART0 to implement bidirectional data transfer to and from a
///			computer running HyperTerminal. This time, interrupts and FIFOs
///			are used
//******************************************************************************

#ifndef _UART0_H_
#define _UART0_H_

#include <stdint.h>

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1

// standard ASCII symbols
#define STX	0x02	//Start of text
#define ETX	0x03	//End of text
#define BS	0x08	//Backspace
#define LF	0x0A	//Line feed
#define CR	0x0D	//Carriage return
#define ESC	0x1B	//Escape
#define SP	0x20	//Space
#define DEL	0x7F	//Delete

// create index implementation FIFO (see FIFO.h)
#define FIFOSIZE   16         // size of the FIFOs (must be power of 2)
#define FIFOSUCCESS 1         // return value on success
#define FIFOFAIL    0         // return value on failure

//******************************************************************************
/// \brief initialize the UART for 115,200 baud rate (assuming 80 MHz clock)
///			8-N-1 frame, FIFOs enabled
/// \param[in] none
/// \return none
//******************************************************************************
void UART0_Init(void);

//******************************************************************************
/// \brief wait for serial port input
/// \param[in] none
/// \return ASCII character
//******************************************************************************
char UART0_InChar(void);

//******************************************************************************
/// \brief output 8-bit to serial port
/// \param[in] letter is an 8-bit ASCII character to be transferred
/// \return none
//******************************************************************************
void UART0_OutChar(char data);

//******************************************************************************
/// \brief output a string to terminal
/// \param[in] string (NULL termination)
/// \param[in] pointer to a NULL-terminated string to be transferred
/// \return none
//******************************************************************************
void UART0_OutString(char *pt);

//******************************************************************************
/// \brief accepts ASCII input in unsigned decimal format
///			and converts to a 32-bit unsigned number
///			valid range is 0 to 4294967295 (2^32-1)
///	\note if you enter a number above 4294967295, it will return an incorrect
///			value. Backspace will remove last digit typed
/// \param[in] none
/// \return 32-bit unsigned number

//******************************************************************************
uint32_t UART0_InUDec(void);

//******************************************************************************
/// \brief a 32-bit number in unsigned decimal format
///			variable format 1-10 digits with no space before or after
/// \param[in] 32-bit number to be transferred
/// \return none
//******************************************************************************
void UART0_OutUDec(uint32_t n);

//******************************************************************************
/// \brief accepts ASCII input in unsigned hexadecimal (base 16) format,
///			it will convert lower case a-f to uppercase A-F,
//			and converts to a 16 bit unsigned number
//			value range is 0 to FFFFFFFF,
//			number above FFFFFFFF will return incorrect value,
//			backspace will remove last digit typed.
///			no '$' or '0x' need be entered, just the 1 to 8 hex digits
/// \param[in] none
/// \return 32-bit unsigned number
//******************************************************************************
uint32_t UART0_InUHex(void);

//******************************************************************************
/// \brief a 32-bit number in unsigned hexadecimal format
///			variable format 1 to 8 digits with no space before or after
/// \param[in] 32-bit number to be transferred
/// \return none
//******************************************************************************
void UART0_OutUHex(uint32_t number);

//******************************************************************************
/// \brief accepts ASCII characters from the serial port
///			and adds them to a string until <enter> is typed
///			or until max length of the string is reached.
///			it echoes each character as it is inputted.
///			if a backspace is inputted, the string is modified
///			and the backspace is echoed
///			terminates the string with a null character
///			uses busy-waiting synchronization on RDRF
/// \param[in] *bufPt: pointer to empty buffer, size of buffer
///	\param[in] max: size of buffer
/// \return none
//******************************************************************************-
void UART0_InString(char *bufPt, uint16_t max);

#endif /*_UART0_H_*/
