//******************************************************************************
/// \Filename: RDM6300.c
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: used to interface with RFID module RDM6300, with 2 processing
///			functions and 2 result functions
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "uart0.h"
#include "uart1.h"
#include "RDM6300.h"

#define numberOfTag 4	// amount of tags

const uint32_t AuthorizedTagID[numberOfTag] = {5902508, 11690608, 25698756, 63598525};	// authorized TagID

//******************************************************************************
///
///		\PUBLIC \FUNCTION
///
//******************************************************************************

/// @todo adding CRC verification, if fail return 0
uint8_t Read_Tag(char *codeSequence) // the read tag is codeSequence
{
	uint8_t flag = 0,
			done = 0;
	char temp;

	for (uint8_t codePosition = 0; !done; codePosition++)	// reading tag until reaches end of text
	{
		temp = UART1_InChar();
		if (temp == STX)	// start of text
		{
			UART0_OutString("\nStarted...");
			flag = 1;			// set flag to start storing code
			codePosition = 0;	// reset to store at start of array
		}
		else if (flag)	// detect the flag and recieve data
		{
			if (temp == ETX)	// end of text
			{
				flag = 0;	// reset for next function call
				done = 1;	// complete the reading process, set done
				UART0_OutString("\nDone");
			}
			codeSequence[codePosition] = temp;
		}
	}
	
	return 1;
}

void Analyze_Tag(char *codeSequence)
{
	uint32_t Num = 0;	// the requesting tagID

	UART0_OutString("\nDecrypted Tag Code: ");
	// convert Hex base to Decimal base. Refer to ASCII chart. Only take character from 2nd to 10th of Code[]
	for (uint8_t i = 1; i < (SequenceLength-3); i++)
	{
		UART0_OutChar(codeSequence[i]);
		Num<<=4;	// make space for the next hex number
		if(codeSequence[i] > 64)  Num += (codeSequence[i] - 55);	//('A'=65)
		else Num += (codeSequence[i] - 48);	//char 0:9 <-> int 48:57
	}

	UART0_OutString("\nRequesting Tag: ");  UART0_OutUDec(Num);	// num is the codeSequence converted into Decimal

	for (uint8_t i = 0; i < numberOfTag; i++)	// scanning through the authorized tagID
	{
		if (Num == AuthorizedTagID[i])	// matches a tagID
		{
			AccessGranted();
			break;
		}
		else if (i == (numberOfTag-1))	// if matches no tagID
		{
			AccessDenied();
		}
	}
}

void AccessGranted(void)
{
	UART0_OutString("\nAccess Granted");
	UART0_OutString("\n--------------------");
	
	GPIOF->DATA |= (1<<1);
	for (uint16_t i=0; i<0xFFFF; i++){};
	GPIOF->DATA &= ~(1<<1);
}

void AccessDenied(void)
{
	UART0_OutString("\nAccess Denied");
	UART0_OutString("\n--------------------");
	
	GPIOF->DATA |= (1<<2);
	for (uint16_t i=0; i<0xFFFF; i++){};
	GPIOF->DATA &= ~(1<<2);
}
