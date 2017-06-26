//******************************************************************************
/// \Filename: diskio.h
/// \Author: Hoang Tran
/// \Date: May 2017 
/// \Brief: Low-level interface with MMC/SD Card using SSI
//******************************************************************************

#include <TM4C123GH6PM.h>
#include "ssi0.h"
#include "diskio.h"
#include "rtc/DS1307.h"

/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD41	(0x40+41)	/* SEND_OP_COND (ACMD) */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */


extern void UART0_OutString(char *pt);
//******************************************************************************
///
///		\PRIVATE \FUNCTION \PROTOTYPE
///
//******************************************************************************

static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */
static volatile BYTE Timer1, Timer2;		/* 100Hz decrement timer */
static BYTE CardType;						/* b0:MMC, b1:SDC, b2:Block addressing */

static BYTE wait_ready(void);
static bool rcvr_datablock (BYTE *buff,	/* Data buffer to store received data */
							UINT btr);	/* Byte count (must be even number) */
#if _READONLY == 0

static bool xmit_datablock(	const BYTE *buff,	/* 512 byte data block to be transmitted */
							BYTE token);		/* Data/Stop token */
#endif /* _READONLY */

static BYTE send_cmd (	BYTE cmd,	/* Command byte */
						DWORD arg);	/* Argument */

//******************************************************************************
///
///		\PUBLIC \FUNCTION
///
//******************************************************************************

DSTATUS disk_initialize(BYTE drv)	/* Physical drive nmuber (0) */
{
	BYTE n, ty, ocr[4];

	if (drv) return STA_NOINIT;	/* Supports only single drive */

	SSI0_Init(200); // 400,000 bps, slow mode
	DESELECT();	/* Ensure CS is held high. */

	/// 10ms ready wait time, @note this doesnt seem to work
	//for (Timer1 = 10; Timer1;){};

	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	/* Send 10 bytes over the SSI. This causes the clock to wiggle the */
	/* required number of times. */
	for(uint8_t i = 0 ; i < 10 ; i++) xchg_spi(0xFF);	// write dummy data, flush return data
	
	SELECT();
	ty = 0;
	if (send_cmd(CMD0, 0) == 1)	/* Put the card SPI mode */
	{
		Timer1 = 10;	/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1)
		{    /* SDC Ver2+ */
			for (n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);
			if (ocr[2] == 0x01 && ocr[3] == 0xAA)	/* The card can work at vdd range of 2.7-3.6V */
			{
				do
				{
					if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 1UL << 30) == 0)	break;    /* ACMD41 with HCS bit */
				} while (Timer1);
				if (Timer1 && send_cmd(CMD58, 0) == 0)
				{    /* Check CCS bit */
					for(n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);
					ty = (ocr[0] & 0x40) ? 6 : 2;
				}
			}
		}
		else	/* SDC Ver1 or MMC */
		{
			ty = (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) <= 1) ? 2 : 1;	/* SDC : MMC */
			do
			{
				if (ty == 2)
				{
					if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) == 0) break;	/* ACMD41 */
				}
				else
				{
					if (send_cmd(CMD1, 0) == 0) break;	/* CMD1 */
				}
			} while (Timer1);
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	ty = 0;	/* Select R/W block length */
		}
	}
	CardType = ty;
	DESELECT();

	if (ty)	/* Initialization succeded */
	{
		Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT */
		set_max_speed();
	}
	else	/* Initialization failed */
	{
		/*---*/
	}

	return Stat;
}

DSTATUS disk_status (BYTE drv) /* Physical drive nmuber (0) */
{
	if (drv) return STA_NOINIT;	/* Supports only single drive */

	return Stat;
}

DRESULT disk_read (	BYTE drv,		/* Physical drive nmuber (0) */
					BYTE *buff,		/* Pointer to the data buffer to store read data */
					DWORD sector,	/* Start sector number (LBA) */
					BYTE count)		/* Sector count (1..255) */
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & 4)) sector *= 512;	/* Convert to byte address if needed */

	SELECT();

	if (count == 1)	/* Single block read */
	{
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else	/* Multiple block read */
	{
		if (send_cmd(CMD18, sector) == 0)	/* READ_MULTIPLE_BLOCK */
		{
			do
			{
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);	/* STOP_TRANSMISSION */
		}
	}

	DESELECT();

	return count ? RES_ERROR : RES_OK;
}

#if _READONLY == 0

DRESULT disk_write (BYTE drv,			/* Physical drive nmuber (0) */
					const BYTE *buff,	/* Pointer to the data to be written */
					DWORD sector,		/* Start sector number (LBA) */
					BYTE count)			/* Sector count (1..255) */
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if ( !(CardType & 4) ) sector *= 512;    /* Convert to byte address if needed */

	SELECT();

	if (count == 1)	/* Single block write */
	{
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
		count = 0;
	}
	else	/* Multiple block write */
	{
		if (CardType & 2)
		{
			send_cmd(CMD55, 0);	/* ACMD23 */
			send_cmd(CMD23, count);
		}
		if (send_cmd(CMD25, sector) == 0)	/* WRITE_MULTIPLE_BLOCK */
		{
			do
			{
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			
			if (!xmit_datablock(0, 0xFD)) count = 1;	/* STOP_TRAN token */
		}
	}

	DESELECT();            /* CS = H */

	return count ? RES_ERROR : RES_OK;
}


#endif /* _READONLY */

DRESULT disk_ioctl (BYTE drv,	/* Physical drive nmuber (0) */
					BYTE ctrl,	/* Control code */
					void *buff)	/* Buffer to send/receive control data */
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	WORD csize;

	if (drv) return RES_PARERR;
	res = RES_ERROR;

	if (Stat & STA_NOINIT) return RES_NOTRDY;

	SELECT();

	switch (ctrl)
	{
		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
			{
				if ((csd[0] >> 6) == 1)	/* SDC ver 2.00 */
				{
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				}
				else	/* MMC or SDC ver 1.XX */
				{
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;
		case GET_SECTOR_SIZE :    /* Get sectors on the disk (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;
		case CTRL_SYNC :    /* Make sure that data has been written */
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;
		case MMC_GET_CSD :    /* Receive CSD as a data block (16 bytes) */
			if (send_cmd(CMD9, 0) == 0 && rcvr_datablock(ptr, 16))	/* READ_CSD */
				res = RES_OK;
			break;
		case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
			if (send_cmd(CMD10, 0) == 0 && rcvr_datablock(ptr, 16))/* READ_CID */
				res = RES_OK;
			break;
		case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0)	/* READ_OCR */
			{
				for (n = 0; n < 4; n++) *ptr++ = xchg_spi(0xFF);
				res = RES_OK;
			}
			break;
		case MMC_GET_TYPE:	/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;
		default:
			res = RES_PARERR;
	}

	DESELECT();

	return res;
}

void disk_timerproc(void)/* This function must be called in period of 10ms */
{
	if(Timer1) Timer1--;
	if(Timer2) Timer2--;
}

DWORD get_fattime (void)
{
	RTC_Time presentTime;

	if (DS1307_GetTime(0xD0, &presentTime))
	{
		return	(((presentTime.Year+2000)-1980) << 25) |
				(presentTime.Month << 21) |
				(presentTime.Date << 16) |
				(presentTime.Hour << 11) |
				(presentTime.Min << 5) |
				(presentTime.Sec >> 1)
				;
	}
	/* If there is anything wrong with RTC, use this instead */ 
	return	((2017-1980) << 25) |					// Year = 2017
			(1 << 21) |								// Month = January
			(1 << 16) |								// Day = 1
			(0 << 11) |								// Hour = 0
			(0 << 5) |								// Min = 0
			(0 >> 1)								// Sec = 0
			;
}

//******************************************************************************
///
///		\PRIVATE \FUNCTION
///
//******************************************************************************

/* Wait for card ready */
static BYTE wait_ready(void)
{
	BYTE res;
	Timer2 = 5;    /* Wait for ready in timeout of 500ms */

	xchg_spi(0xFF);
	do
	{
		res = xchg_spi(0xFF);
	}
	while ((res != 0xFF) && Timer2);

	return res;
}

/* Receive a data packet from MMC */
static bool rcvr_datablock(BYTE *buff,	/* Data buffer to store received data */
							UINT btr)	/* Byte count (must be even number) */
{
	BYTE token;
	Timer1 = 1;
	do	/* Wait for data packet in timeout of 100ms */
	{
		token = xchg_spi(0xFF);
	} while ((token == 0xFF) && Timer1);

	if (token != 0xFE) return false;	/* If not valid data token, retutn with error */

	do						/* Receive the data block into buffer */
	{
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 2);

	xchg_spi(0xFF);	/* Discard CRC */
	xchg_spi(0xFF);

	return true;	/* Return with success */
}

#if _READONLY == 0

/* Send a data packet to MMC */
static bool xmit_datablock(	const BYTE *buff,	/* 512 byte data block to be transmitted */
							BYTE token)			/* Data/Stop token */
{
	BYTE resp, wc;

	if (wait_ready() != 0xFF)
		return false;

	xchg_spi(token);	/* Xmit data token */
	if (token != 0xFD)				/* Is data token */
	{
		wc = 0;
		do							/* Xmit the 512 byte data block to MMC */
		{
			xchg_spi(*buff++);
			xchg_spi(*buff++);
		} while (--wc);

		xchg_spi(0xFF);				/* CRC (Dummy) */
		xchg_spi(0xFF);

		resp = xchg_spi(0xFF);		/* Reveive data response */
		if ((resp & 0x1F) != 0x05)	/* If not accepted, return with error */
			return false;
	}

	return true;
}
#endif /* _READONLY */

/* Send a command packet to MMC */
static BYTE send_cmd (	BYTE cmd,		/* Command byte */
						DWORD arg)		/* Argument */
{
	BYTE n, res;

	if (wait_ready() != 0xFF) return 0xFF;

	/* Send command packet */
	xchg_spi(cmd);						/* Command */
	xchg_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	xchg_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	xchg_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
	xchg_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0;
	if (cmd == CMD0) n = 0x95;			/* CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* CRC for CMD8(0x1AA) */
	xchg_spi(n);

    /* Receive command response */
	if (cmd == CMD12) xchg_spi(0xFF);	/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
	{
		res = xchg_spi(0xFF);
	} while ((res & 0x80) && --n);

	return res;							/* Return with the response value */
}
