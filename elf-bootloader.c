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
-------------------------------------------------------------------------------
Company: Trenz Electronics GmbH
Autor: Oleksandr Kiyenko (a.kienko@gmail.com)
-------------------------------------------------------------------------------
*/
#include "spi_flash.h"
#include <stdio.h>
//-----------------------------------------------------------------------------
#define SSB_START_ADDR			0x00860000
#define XIL_BIT_SYNC			0x665599AA
#define ELF_EHSIZE				52
#define ELF_SHENTSIZE			40
#define ELFMAG					0x464C457F
#define SHT_PROGBITS			1
#define ELF_HDR_SHNUM_OFF		48
#define ELF_HDR_SHOFF_OFF		8
#define ELF_HDR_SHTYPE_OFF		4
#define ELF_HDR_SHADDR_OFF		3
#define ELF_HDR_SHOFFSET_OFF	4
#define ELF_HDR_SHSIZE_OFF		5
#define MEM_TEST_RANGE			0x1000
#define SPI_READ_CMD			0x0300
//-----------------------------------------------------------------------------
u8	e_shnum, section, sh_type;
u32	e_shoff, sh_addr, sh_offset, sh_size;
int (*reset_func) () = 0;
u32	int_read_buf[13];
u8* char_read_buf = (u8*)int_read_buf;
u8 *mem_section;
volatile u32 *ddr_mem = (u32*)XPAR_MCB_DDR3_S0_AXI_BASEADDR;
//-----------------------------------------------------------------------------
void pause(int p){
	volatile int v;
	for(v = 0; v < (400000 * p); v++);
}
//-----------------------------------------------------------------------------
int main(void){
	int i;

	print("\r\nTrenz Electronic ELF bootloader "__DATE__" "__TIME__"\r\n\r\n");
	spi_flash_init(XPAR_QSPI_FLASH_DEVICE_ID);	// Init SPI Flash
	// Test memory before use it!
	for(i = 0; i < MEM_TEST_RANGE; i++)
		ddr_mem[i] = i;
	for(i = 0; i < MEM_TEST_RANGE; i++){
		if(ddr_mem[i] != i){
			xil_printf("ERROR: Memory test failed!\r\n");
			while(1);
		}
	}
	// Read SSB
	spi_flash_read(SSB_START_ADDR, ELF_EHSIZE, char_read_buf);
	if(int_read_buf[0] != ELFMAG){
		xil_printf("ERROR: Boot image not found\r\n");
		while(1);
	}

	e_shnum	= char_read_buf[ELF_HDR_SHNUM_OFF];
	e_shoff = int_read_buf[ELF_HDR_SHOFF_OFF];

	for(section = 0; section < e_shnum; section++){	// Sections loop
		spi_flash_read((SSB_START_ADDR + e_shoff + section * ELF_SHENTSIZE),
			ELF_SHENTSIZE, char_read_buf);

		sh_type = char_read_buf[ELF_HDR_SHTYPE_OFF];
		sh_addr = int_read_buf[ELF_HDR_SHADDR_OFF];
		sh_offset = int_read_buf[ELF_HDR_SHOFFSET_OFF];
		sh_size = int_read_buf[ELF_HDR_SHSIZE_OFF];
		mem_section = (u8*)sh_addr;

		if(sh_type == SHT_PROGBITS){	// Process only sections with data
			if(sh_addr == 0){
				if(sh_size == 8)
					spi_flash_read((SSB_START_ADDR + sh_offset), sh_size,
						mem_section);
			}
			else	// Usual sections
				spi_flash_read((SSB_START_ADDR + sh_offset), sh_size,
					mem_section);
		}
	}	// Sections loop
	xil_printf("\r\nBoot...\r\n");
	pause(1);
	reset_func();	// Start Second Stage
	while(1);
}
