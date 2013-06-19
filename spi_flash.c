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

#include "spi_flash.h"
#include "xspi_l.h"

static XSpi Spi;
static u8 ReadBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES + 4];
static u8 WriteBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];

int spi_flash_init(u32 dev_id){
	int Status;
	XSpi_Config *ConfigPtr;

	ConfigPtr = XSpi_LookupConfig(dev_id);
	if (ConfigPtr == NULL)
		return XST_DEVICE_NOT_FOUND;

	Status = XSpi_CfgInitialize(&Spi, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	Status = XSpi_SetOptions(&Spi, XSP_MASTER_OPTION |
				 XSP_MANUAL_SSELECT_OPTION);
	if(Status != XST_SUCCESS)
		return XST_FAILURE;

	Status = XSpi_SetSlaveSelect(&Spi, SPI_SELECT);
	if(Status != XST_SUCCESS)
		return XST_FAILURE;

	XSpi_Start(&Spi);
	XSpi_IntrGlobalDisable(&Spi);	// Pooled mode

	return XST_SUCCESS;
}
/*******************************************************************************
 * Read data from SPI Flash
 * @param	u32 Address
 * @param	u32 Count
 * @param	u8* Buffer
 * @return	Operation Status
 ******************************************************************************/
int spi_flash_read(u32 spi_addr, u32 count, u8* buf){
	u32 i, block_size, page_rem, buf_rem;
	u32 buf_addr = 0;

	while(buf_addr < count){
		page_rem = 0x0100 - (spi_addr & 0x00ff);
		buf_rem = count - buf_addr;
		block_size = (buf_rem > page_rem) ? page_rem : buf_rem;
		if(spi_flash_read_l(spi_addr, block_size) != XST_SUCCESS)
			return XST_FAILURE;
		for(i = 0; i < block_size; i++)
			buf[buf_addr + i] = ReadBuffer[i + READ_WRITE_EXTRA_BYTES];
		spi_addr += block_size;
		buf_addr += block_size;
	}
	return XST_SUCCESS;
}

int spi_flash_read_l(u32 Addr, u32 ByteCount){
	int Status;

	Status = spi_flash_wait();
	if(Status != XST_SUCCESS)
		return XST_FAILURE;

	WriteBuffer[0] = SPI_READ;
	WriteBuffer[1] = (u8) (Addr >> 16);
	WriteBuffer[2] = (u8) (Addr >> 8);
	WriteBuffer[3] = (u8) Addr;
	return XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer,
			(ByteCount + READ_WRITE_EXTRA_BYTES));
}
/*******************************************************************************
 * Read flash status register
 * @param	Spi Driver Instance
 * @return	Status register contents
 ******************************************************************************/
int spi_flash_status(void){
	WriteBuffer[0] = SPI_RDSR1;
	return XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer, RDSR1_BYTES);
}

int spi_flash_wait(void){
	while(1) {
		if(spi_flash_status() != XST_SUCCESS)
			return XST_FAILURE;
		if((ReadBuffer[1] & FLASH_SR_IS_READY_MASK) == 0)
			break;
	}
	return XST_SUCCESS;
}

void spi_flash_set_write_enable(void){
	WriteBuffer[0] = SPI_WREN;
	XSpi_Transfer(&Spi, WriteBuffer, NULL, WREN_BYTES);
}

void spi_flash_sector_erase(u8 sector){
	spi_flash_set_write_enable();
	WriteBuffer[0] = SPI_SE;
	WriteBuffer[1] = sector;
	WriteBuffer[2] = 0;
	WriteBuffer[3] = 0;
	XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer, SE_BYTES);
	spi_flash_wait();

}

int spi_flash_write_buf(u32 spi_addr, u32 count, u8* buf){
	int i;

	spi_flash_wait();
	spi_flash_set_write_enable();
	WriteBuffer[0] = SPI_WRITE;
	WriteBuffer[1] = (spi_addr >> 16) & 0xFF;
	WriteBuffer[2] = (spi_addr >>  8) & 0xFF;
	WriteBuffer[3] = spi_addr & 0xFF;
	for(i = 0; i < count; i++){
		WriteBuffer[4 + i] = buf[i];
	}
	XSpi_Transfer(&Spi, WriteBuffer, NULL, (count + READ_WRITE_EXTRA_BYTES));
	return XST_SUCCESS;
}

int spi_flash_write(u32 spi_addr, u32 count, u8* buf){
	u32 cnt = count;
	u32 pointer = 0;
	u32 page_rem, block_size;
	u32 cur_addr = spi_addr;

	while(cnt){
		if((cur_addr & 0xFFFF) == 0x0000){
			spi_flash_sector_erase(cur_addr >> 16);
		}
		page_rem = PAGE_SIZE - (cur_addr & 0xFF);
		block_size = (cnt > page_rem) ? page_rem : cnt;
		spi_flash_write_buf((cur_addr+pointer), block_size, (buf+pointer));
		pointer += block_size;
		cnt -= block_size;
	}
	return XST_SUCCESS;
}

int spi_flash_get_id(void){
	u32 id;
	WriteBuffer[0] = SPI_RDID;
	XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer, 4);
	id = (ReadBuffer[1] << 16) | (ReadBuffer[2] << 8) | ReadBuffer[3];
	return id;
}
