#ifndef FLASH_H
#define FLASH_H

#include "main.h"

#define FMC_FLASH_PAGE_SIZE 0x1000UL

#define DATA_FLASH_BASE	0x00020000
#define DATA_FLASH_END	0x00080000
#define DATA_FLASH_SIZE	(DATA_FLASH_END - DATA_FLASH_BASE)

void flash_init();
void flash_write(uint32_t addr, uint32_t data);
uint32_t flash_read(uint32_t addr);
uint8_t flash_read_direct(uint32_t addr);
void flash_erase(uint32_t addr);
#endif // FLASH_H