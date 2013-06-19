/*
Copyright (C) 2013 Trenz Electronic GmbH

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
IN THE SOFTWARE.
*/
#ifndef __SPI_FLASH_H_
#define __SPI_FLASH_H_

#include "xparameters.h"
#include "xspi.h"
#include "xil_exception.h"

#define SPI_DEVICE_ID			XPAR_QSPI_FLASH_DEVICE_ID
#define SPI_SELECT 				0x01
#define	SPI_WRITE				0x02
#define SPI_READ				0x03
#define SPI_RDSR1				0x05
#define	SPI_WREN				0x06
#define	SPI_SE					0xD8
#define SPI_RDID				0x9F

#define READ_WRITE_EXTRA_BYTES	4
#define RDSR1_BYTES				2
#define WREN_BYTES				1
#define SE_BYTES				4
#define FLASH_SR_IS_READY_MASK	0x01
#define PAGE_SIZE				256

#define SPI_DATA_REG			0
#define SPI_CTRL_REG			1
#define SPI_STS_REG				2

#define SPI_CTRL_RXRST			0x01
#define SPI_CTRL_TXRST			0x02

#define SPI_STS_RDV				0x01
#define SPI_STS_WRV				0x02
#define SPI_STS_WRB				0x04

#define STAT_OK					0x0A
#define STAT_ERR				0x11
#define CMD_SET_FPWM			0x01
#define CMD_SET_SPWM			0x02
#define CMD_SET_SCDAC			0x03
#define CMD_SET_SVDAC			0x04
#define CMD_RD_V_DCDC			0x05
#define CMD_RD_V_OUT			0x06
#define CMD_RD_I_OUT			0x07
#define CMD_RD_I_SEN			0x08
#define CMD_RD_SW				0x09


#define SPI_DRV_CS				0x100
#define SPI_DRV_RD				0x200
#define SPI_FLASH_SS			0x400
#define SPI_IO_SS				0x800

int spi_flash_init(u32 dev_id);
int spi_flash_read(u32 addr, u32 count, u8* buf);
int spi_flash_read_l(u32 Addr, u32 ByteCount);
int spi_flash_wait(void);
void spi_flash_sector_erase(u8 sector);
void spi_flash_set_write_enable(void);
int spi_flash_write(u32 spi_addr, u32 count, u8* buf);
int spi_flash_get_id(void);

#endif
