//******************************************************************************
/// \Filename: RDM6300.h
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: used to interface with RFID module RDM6300, with 2 processing
///			functions and 2 result functions
//******************************************************************************

#ifndef _RDM6300_H_
#define _RDM6300_H_

#include <stdint.h>

#define SequenceLength 14	// lenght of the data sequence of each tag

#define numberOfTag 4	// amount of tags

extern const uint32_t AuthorizedTagID[numberOfTag];	// authorized TagID

//******************************************************************************
/// \brief read a RFID tag by polling, storing a retrieved value into an array
///			of characters, then check for CRC
/// \param[out] array of charater to store code
/// \return 1 if CRC is correct and 0 vice versa
//******************************************************************************
uint8_t Read_Tag(char *codeSequence);
	
//******************************************************************************
/// \brief check if a stored tagID matches an authorized tagID, and then proceed
///			to execute resulting function
/// \param[in] array of charater to check
/// \return none
//******************************************************************************
void Analyze_Tag(char *codeSequence);

//******************************************************************************
/// \brief execute if a tagID matches an authorized tagID
/// \return none
//******************************************************************************
void AccessGranted(void);

//******************************************************************************
/// \brief execute if a tagID doesn't match an authorized tagID
/// \return none
//******************************************************************************
void AccessDenied(void);

#endif /*_RDM6300_H_*/
