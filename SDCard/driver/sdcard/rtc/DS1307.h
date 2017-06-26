//******************************************************************************
/// \Filename: DS1307.h
/// \Author: Hoang Tran
/// \Date: May 2017 
/// \Brief: provide a function that initializes, sends, and receives the
///			I2C module interfaced with an RTC DS1307
//******************************************************************************

#ifndef _DS1307_H_
#define _DS1307_H_

#include <stdint.h>

// I2C0SCL connected to PB2
// I2C0SDA connected to PB3

typedef struct
{
	uint8_t	Sec;
	uint8_t	Min;
	uint8_t	Hour;
	uint8_t AmPm;
	uint8_t Mode12;
	uint8_t	Day;
	uint8_t	Date;
	uint8_t	Month;
	uint8_t	Year;
} RTC_Time;

//******************************************************************************
/// \brief Initialize I2C to interface with DS1307
/// \param[in] none
/// \return none
//******************************************************************************
void DS1307_Init(void);

//******************************************************************************
/// \brief receives one byte from specified slave
/// \param[in] slave: address of the slave
/// \param[in] initial_reg: address of byte to read
/// \param[out] *data: pointer to variable to store data
/// \return 1 if succeed or 0 if fail
//******************************************************************************
uint8_t DS1307_GetTime1(int8_t slave, uint8_t initial_reg, uint8_t *data);

//******************************************************************************
/// \brief receives multiple bytes from specified slave
/// \param[in] slave: address of the slave
/// \param[out] *timeStrc: structure to store value
/// \return 1 if succeed or 0 if fail
//******************************************************************************
uint8_t DS1307_GetTime(uint8_t slave, RTC_Time *timeStrc);

//******************************************************************************
/// \brief send multiple bytes from specified slave
/// \param[in] slave: address of the slave
/// \param[in] timeStrc: structure to write value into RTC
/// \return 1 if succeed or 0 if fail
//******************************************************************************
uint8_t DS1307_SetTime(uint8_t slave, RTC_Time timeStrc);

#endif /*_DS1307_H_*/
