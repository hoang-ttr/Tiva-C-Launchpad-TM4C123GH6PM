//******************************************************************************
/// \Filename: i2c1.h
/// \Author: Hoang Tran
/// \Date: May 2017 
/// \Brief: provide a function that initializes, sends, and receives using
///			I2C1 peripheral to interface with PCF8574T
//******************************************************************************

// I2C1SCL connected to PA6
// I2C1SDA connected to PA7

#ifndef _I2C1_H_
#define _I2C1_H_

#include <stdint.h>

//******************************************************************************
/// \brief Initialize I2C to interface with PCF8574T
/// \param[in] none
/// \return none
//******************************************************************************
void I2C1_Init(void);

//******************************************************************************
/// \brief receives one byte from specified slave
/// \param[in] slave: address of the slave
/// \param[out] *data: pointer to variable to store data
/// \return 1 if succeed or 0 if fail
//******************************************************************************
uint8_t I2C1_Recv1(int8_t slave, uint8_t *data);

//******************************************************************************
/// \brief send one byte from specified slave
/// \param[in] slave: address of the slave
/// \param[out] data: data to be sent
/// \return 1 if succeed or 0 if fail
//******************************************************************************
uint8_t I2C1_Send1(uint8_t slave, uint8_t data);

#endif /*_I2C01_H_*/
