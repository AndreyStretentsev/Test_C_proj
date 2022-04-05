#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "flash.h"

uint32_t *flash_mem = NULL;

uint8_t flash_read_direct(uint32_t addr) {
    return ((uint8_t *)flash_mem)[addr];
}

uint32_t flash_read(uint32_t addr) {
    // printf("%08X | %11d | read from | %08X | %11d\n", 
    //     flash_mem[addr >> 2], flash_mem[addr >> 2], 
    //     addr, addr
    // );
    return flash_mem[addr >> 2];
}

void flash_write(uint32_t addr, uint32_t data) {
    // printf("%08X | %11d | written to | %08X | %11d\n", 
    //     data, data, 
    //     addr, addr
    // );
    flash_mem[addr >> 2] &= data;
}

void flash_erase(uint32_t addr) {
    // printf("%08X | %11d | page erased\n", 
    //     addr, addr
    // );
    memset(&flash_mem[addr >> 2], 0xFFFFFFFF, FMC_FLASH_PAGE_SIZE);
}

void flash_init() {
    flash_mem = calloc(DATA_FLASH_SIZE + DATA_FLASH_BASE, sizeof(uint32_t));
    memset(flash_mem, 0xFF, DATA_FLASH_SIZE + DATA_FLASH_BASE);
}