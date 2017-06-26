//******************************************************************************
/// \Filename: diskio.h
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: Low-level interface with MMC/SD Card using SPI
//******************************************************************************


// **********MCU and SDC*******************
// CS connected to PA3  (SSI0CS)
// SCK connected to PA2 (SSI0Clk)
// MOSI connected to PA5 (SSI0Tx)
// MISO connected to PA4 (SSI0Rx)
// VCC connected to +3.3V
// Gnd connected to ground

/// @NOTE if the SDCard takes too long to respond, try touching near the CS pin

#ifndef _DISKIO_H_
#define _DISKIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "integer.h"
#include "stdbool.h"

#define _READONLY 0	/* 1: Read-only mode */

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT			0x01	/* Drive not initialized */
#define STA_NODISK			0x02	/* No medium in the drive */
#define STA_PROTECT			0x04	/* Write protected */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at _FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at _USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at _MAX_SS != _MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at _USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */

/* Generic command (Not used by FatFs) */
#define CTRL_POWER		5	/* Get/Set power status */
#define CTRL_LOCK		6	/* Lock/Unlock media removal */
#define CTRL_EJECT		7	/* Eject media */
#define CTRL_FORMAT		8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */
#define ISDIO_READ			55	/* Read data form SD iSDIO register */
#define ISDIO_WRITE			56	/* Write data to SD iSDIO register */
#define ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum
{
	RES_OK = 0,	/* 0: Successful */
	RES_ERROR,	/* 1: R/W Error */
	RES_WRPRT,	/* 2: Write Protected */
	RES_NOTRDY,	/* 3: Not Ready */
	RES_PARERR	/* 4: Invalid Parameter */
} DRESULT;


//******************************************************************************
/// \brief Initialize the interface between microcontroller and the SD card.
///			Since this program initializes the disk, it must run with 
///			the disk periodic task operating
/// \param[in] drive number (only drive 0 is supported)
/// \return DSTATUS
//******************************************************************************
DSTATUS disk_initialize(BYTE drive);

//******************************************************************************
/// \brief Check the status of the SD card
/// \param[in] drive number (only drive 0 is supported)
/// \return DSTATUS
//******************************************************************************
DSTATUS disk_status(BYTE drive);

//******************************************************************************
/// \brief Read data from the SD card (write to RAM)
/// \param[in]	drv: drive number (only drive 0 is supported)
///				sector: sector number of SD card to read: 0,1,2,...
///				count: number of sectors to read
/// \param[out]	*buff: pointer to an empty RAM buffer
/// \return DRESULT
//******************************************************************************
DRESULT disk_read(	BYTE drv,		/* Physical drive number (0) */
					BYTE *buff,		/* Pointer to buffer to read data */
					DWORD sector,	/* Start sector number (LBA) */
					BYTE count);	/* Sector count (1..255) */

#if	_READONLY == 0

//******************************************************************************
/// \brief Write data to the SD card
/// \param[in]	drv: drive number (only drive 0 is supported)
///				sector: start sector of SD card to write
///				count: number of sectors to write
/// \param[out] *buff: pointer to RAM buffer with information
/// \return DRESULT
//******************************************************************************
DRESULT disk_write(	BYTE drv,			/* Physical drive number (0) */
					const BYTE *buff,	/* Pointer to the data to be written */
					DWORD sector,		/* Start sector number (LBA) */
					BYTE count);		/* Sector count (1..255) */

#endif	/* _READONLY_ */

//******************************************************************************
/// \brief Misellanious function
/// \param[in]	@drv: Physical drive nmuber (0)
///				@ctrl: Control code
/// \param[out] @*buff Buffer to send/receive control data
/// \return DRESULT
//******************************************************************************
DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff);

//******************************************************************************
/// \brief Timer driven procedure. This function must be called in period of 10ms
/// \param[in] none
/// \return none
//******************************************************************************
void disk_timerproc(void);
					
//******************************************************************************
/// \brief User Provided Timer Function for FatFs module
///			This is a real time clock service to be called from FatFs module.
///			Any valid time must be returned even if the system does not support
///			 a real time clock.
/// \param[in] none
/// \return a 32-bit of time data encoded
//******************************************************************************
DWORD get_fattime(void);

#ifdef __cplusplus
}
#endif

#endif	/*_DISKIO_H_*/
