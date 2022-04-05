
#include <string.h>
#include <stdio.h>
#include "flash.h"

#include "storage.h"

#define DATA_FLASH_PAGES_NUM	(DATA_FLASH_SIZE / FMC_FLASH_PAGE_SIZE)
#define FILES_NUM_MAX	DATA_FLASH_PAGES_NUM

typedef struct {
	char name[4];
	uint32_t id;
	uint32_t size;
} file_header_t;

#define FLASH_PAGE_SIZE_Msk	0x0FFF
#define FLASH_PAGE_SIZE_Pos	12

#define PAGE_IND_TO_ADDR(ind)	((ind) * FMC_FLASH_PAGE_SIZE + DATA_FLASH_BASE)
#define ADDR_TO_PAGE_IND(addr)	((addr) >> FLASH_PAGE_SIZE_Pos)

#define SET_FILE_HEADER(hfile, fid, fsize)	\
	do { \
		(hfile).id = (fid); \
		(hfile).size = (fsize); \
		(hfile).name[0] = 'F'; \
		(hfile).name[1] = 'I'; \
		(hfile).name[2] = 'L'; \
		(hfile).name[3] = 'E'; \
	} while(0)

#define SIZE_TO_PAGES_ROUND_UP(size)	\
	(((size) >> FLASH_PAGE_SIZE_Pos) + ((size) & FLASH_PAGE_SIZE_Msk ? 1 : 0))

#define SIZE_TO_PAGES_ROUND_DW(size)	((size) >> FLASH_PAGE_SIZE_Pos)

#define SIZE_TO_PAGES_W_HEADERS(size) 	\
	(SIZE_TO_PAGES_ROUND_UP(SIZE_TO_PAGES_ROUND_UP(size) * sizeof(file_header_t) + (size)))

#define SIZE_TO_SIZE_W_HEADERS(size)	\
	(((SIZE_TO_PAGES_ROUND_DW(SIZE_TO_PAGES_ROUND_UP(size) * sizeof(file_header_t) + (size))) \
	<< FLASH_PAGE_SIZE_Pos) \
	 + ((size) & FLASH_PAGE_SIZE_Msk) + SIZE_TO_PAGES_ROUND_DW(size) * sizeof(file_header_t))

#define FLASH_PAGE_SIZE_WO_HEADER	(FMC_FLASH_PAGE_SIZE - sizeof(file_header_t))

static struct {
	uint32_t free_pages;
	uint32_t page_file_id[DATA_FLASH_PAGES_NUM];
} f_table = {0};


static void storage_read_page_wa(uint32_t *data, uint32_t addr, int len) {
	// TODO: ASSERT
	while (len > 0) {
		*data++ = flash_read(addr);
		addr += sizeof(uint32_t);
		len -= sizeof(uint32_t);
	}
}


static void storage_read(uint8_t *data, uint32_t addr, int len) {
	while (len > 0) {
		*data++ = flash_read_direct(addr++);
		// *data++ = *((uint8_t *)addr++);
		len--;
	}
}


static inline void storage_erase_page(uint32_t addr) {
	flash_erase(addr);
}


static void storage_write_in_page_wa(uint32_t *data, uint32_t addr, int len) {
	while (len > 0) {
		flash_write(addr, *data++);
		addr += sizeof(uint32_t);
		len -= sizeof(uint32_t);
	}
}


static void storage_write_wa(uint32_t *data, uint32_t addr, int len) {
	
}


static bool is_file(file_header_t *hfile) {
	return !(hfile->name[0] != 'F' ||
			hfile->name[1] != 'I' ||
			hfile->name[2] != 'L' ||
			hfile->name[3] != 'E' ||
			hfile->id == 0 ||
			hfile->id > 65535);
}


static void storage_scan() {
	file_header_t hfile;
	memset(&f_table, 0, sizeof(f_table));
	for (uint32_t i = 0; i < DATA_FLASH_PAGES_NUM; i++) {
		storage_read_page_wa((uint32_t *)&hfile, PAGE_IND_TO_ADDR(i), sizeof(hfile));
		if (is_file(&hfile)) {
			f_table.page_file_id[i] = hfile.id;
		} else
			f_table.free_pages++;
	}
}


void storage_init() {
	flash_init();
	storage_scan();
}


static int storage_get_next_page_index_by_id(uint32_t id, uint8_t from_index) {
	int page_cnt = 0;
	int page_index = -1;
	for (uint8_t page = from_index; page < DATA_FLASH_PAGES_NUM; page++) {
		if (f_table.page_file_id[page] != id)
			continue;
		page_index = page;
		break;			
	}
	return page_index;
}

uint16_t storage_generate_id() {
	uint16_t id = 0;
	while (id == 0) {
		id = rand();
		if (storage_get_next_page_index_by_id(id, 0) >= 0)
			id = 0;
	}
	return id;
}

f_err_t storage_get_file_by_id(file_t *file, uint16_t id) {
	int first_file_page_ind = storage_get_next_page_index_by_id(id, 0);
	if (first_file_page_ind < 0)
		return FILE_NOT_FOUND;
	file->id = f_table.page_file_id[first_file_page_ind];
	file->st_addr = PAGE_IND_TO_ADDR(first_file_page_ind);
	file_header_t hfile;
	storage_read_page_wa((uint32_t *)&hfile, file->st_addr, sizeof(hfile));
	file->size = hfile.size;
	file->cur_addr = sizeof(file_header_t);
	return FILE_OK;
}


f_err_t storage_file_create(file_t *file, uint16_t id, uint32_t size) {
	uint32_t file_size_in_pages = SIZE_TO_PAGES_W_HEADERS(size);
	printf("file_size_in_pages = %d\n", file_size_in_pages);
	if (file_size_in_pages > f_table.free_pages)
		return FILE_NOT_ENOUGH_SPACE;
	
	if (storage_get_file_by_id(file, id) == FILE_OK)
		return FILE_ALREADY_EXISTS;
	
	file_header_t hfile;
	SET_FILE_HEADER(hfile, id, size);
	int free_page_ind = storage_get_next_page_index_by_id(0, 0);
	file->st_addr = PAGE_IND_TO_ADDR(free_page_ind);
	f_table.page_file_id[free_page_ind] = id;
	storage_write_in_page_wa((uint32_t *)&hfile, file->st_addr, sizeof(hfile));
	
	f_err_t ret = FILE_OK;
	for (uint8_t page = 1; page < file_size_in_pages; page++) {
		free_page_ind = storage_get_next_page_index_by_id(0, free_page_ind);
		if (free_page_ind < 0) {
			ret = FILE_NOT_ENOUGH_SPACE;
			break;
		}
		f_table.page_file_id[free_page_ind] = id;
		storage_write_in_page_wa(
			(uint32_t *)&hfile, 
			PAGE_IND_TO_ADDR(free_page_ind), 
			sizeof(hfile)
		);
	}
	file->id = id;
	file->cur_addr = sizeof(file_header_t);
	file->size = size;
	f_table.free_pages -= file_size_in_pages;
	if (ret != FILE_OK) {
		storage_file_delete(file);
	}
	
	return ret;
}


f_err_t storage_file_delete(file_t *file) {
	int page_ind = 0;
	
	uint8_t file_size_in_pages = SIZE_TO_PAGES_W_HEADERS(file->size);
	for (uint8_t page = 0; page < file_size_in_pages; page++) {
		page_ind = storage_get_next_page_index_by_id(file->id, page_ind);
		if (page_ind < 0)
			return FILE_NOT_FOUND;
		f_table.page_file_id[page_ind] = 0;
		f_table.free_pages++;
		storage_erase_page(PAGE_IND_TO_ADDR(page_ind));
	}
	return FILE_OK;
}


int storage_file_write(file_t *file, uint32_t *data, int len) {
	int written = len;
	uint32_t data_offset = file->cur_addr & FLASH_PAGE_SIZE_Msk;
	uint32_t end_writing_addr = file->cur_addr + SIZE_TO_SIZE_W_HEADERS(len);
	if (data_offset + (len & FLASH_PAGE_SIZE_Msk) >= FMC_FLASH_PAGE_SIZE)
		end_writing_addr += sizeof(file_header_t);
	uint32_t end_of_file_addr = SIZE_TO_SIZE_W_HEADERS(file->size) + sizeof(file_header_t);
	printf("end_writing_addr = 0x%08X(%d), end_of_file_addr = 0x%08X(%d)\n", 
		end_writing_addr, end_writing_addr,
		end_of_file_addr, end_of_file_addr
	);
	if (file->cur_addr == end_of_file_addr) 
		return 0;
	if (end_writing_addr > end_of_file_addr) {
		len -= end_writing_addr - end_of_file_addr;
		end_writing_addr = end_of_file_addr;
		written = len;
	}

	uint8_t page_ind = ADDR_TO_PAGE_IND(file->st_addr - DATA_FLASH_BASE);
	for (uint8_t i = 0; i < ADDR_TO_PAGE_IND(file->cur_addr); i++)
		page_ind = storage_get_next_page_index_by_id(
			file->id, 
			page_ind + 1
		);
	
	if (len + data_offset <= FMC_FLASH_PAGE_SIZE) {
		storage_write_in_page_wa(
			data, 
			PAGE_IND_TO_ADDR(page_ind) + data_offset,
			len
		);
		file->cur_addr = end_writing_addr;
		return written;
	}
	
	int first_page_len = FMC_FLASH_PAGE_SIZE - data_offset;
	storage_write_in_page_wa(
		data, 
		PAGE_IND_TO_ADDR(page_ind) + data_offset,
		first_page_len
	);

	len -= first_page_len;
	page_ind = storage_get_next_page_index_by_id(file->id, page_ind + 1);
	file->cur_addr += first_page_len + sizeof(file_header_t);
	data += first_page_len >> 2;
	
	while (len >= FLASH_PAGE_SIZE_WO_HEADER) {
		storage_write_in_page_wa(
			data, 
			PAGE_IND_TO_ADDR(page_ind) + sizeof(file_header_t), 
			FLASH_PAGE_SIZE_WO_HEADER
		);
		len -= FLASH_PAGE_SIZE_WO_HEADER;
		page_ind = storage_get_next_page_index_by_id(file->id, page_ind + 1);
		file->cur_addr += FMC_FLASH_PAGE_SIZE;
		data += FLASH_PAGE_SIZE_WO_HEADER >> 2;
	}
	storage_write_in_page_wa(
		data, 
		PAGE_IND_TO_ADDR(page_ind) + sizeof(file_header_t), 
		len
	);
	file->cur_addr = end_writing_addr;
	return written;
}


int storage_file_read(file_t *file, uint8_t *data, int len) {
	int read = len;
	uint32_t data_offset = file->cur_addr & FLASH_PAGE_SIZE_Msk;
	uint32_t end_reading_addr = file->cur_addr + SIZE_TO_SIZE_W_HEADERS(len);
	if (data_offset + (len & FLASH_PAGE_SIZE_Msk) >= FMC_FLASH_PAGE_SIZE)
		end_reading_addr += sizeof(file_header_t);
	uint32_t end_of_file_addr = SIZE_TO_SIZE_W_HEADERS(file->size) + sizeof(file_header_t);
	printf("end_reading_addr = 0x%08X(%d), end_of_file_addr = 0x%08X(%d)\n", 
		end_reading_addr, end_reading_addr,
		end_of_file_addr, end_of_file_addr
	);
	if (file->cur_addr == end_of_file_addr) 
		return 0;
	if (end_reading_addr > end_of_file_addr) {
		len -= end_reading_addr - end_of_file_addr;
		end_reading_addr = end_of_file_addr;
		read = len;
	}
	uint8_t page_ind = ADDR_TO_PAGE_IND(file->st_addr - DATA_FLASH_BASE);
	for (uint8_t i = 0; i < ADDR_TO_PAGE_IND(file->cur_addr); i++)
		page_ind = storage_get_next_page_index_by_id(
			file->id, 
			page_ind + 1
		);
	
	if (len + data_offset <= FMC_FLASH_PAGE_SIZE) {
		storage_read(
			data, 
			PAGE_IND_TO_ADDR(page_ind) + data_offset,
			len
		);
		file->cur_addr = end_reading_addr;
		return read;
	}
	
	int first_page_len = FMC_FLASH_PAGE_SIZE - data_offset;
	storage_read(
		data, 
		PAGE_IND_TO_ADDR(page_ind) + data_offset,
		first_page_len
	);

	len -= first_page_len;
	page_ind = storage_get_next_page_index_by_id(file->id, page_ind + 1);
	file->cur_addr += first_page_len + sizeof(file_header_t);
	data += first_page_len >> 2;
	
	while (len >= FLASH_PAGE_SIZE_WO_HEADER) {
		storage_read(
			data, 
			PAGE_IND_TO_ADDR(page_ind) + sizeof(file_header_t), 
			FLASH_PAGE_SIZE_WO_HEADER
		);
		len -= FLASH_PAGE_SIZE_WO_HEADER;
		page_ind = storage_get_next_page_index_by_id(file->id, page_ind + 1);
		file->cur_addr += FMC_FLASH_PAGE_SIZE;
		data += FLASH_PAGE_SIZE_WO_HEADER >> 2;
	}
	storage_read(
		data, 
		PAGE_IND_TO_ADDR(page_ind) + sizeof(file_header_t), 
		len
	);
	file->cur_addr = end_reading_addr;
	return read;
}


int storage_file_set_cursor(file_t *file, int cursor, int origin) {
	int ret = 0;
	if (cursor >= file->size)
		return FILE_EOF;
	switch (origin) {
		case S_CUR: {
			uint32_t data_offset = file->cur_addr & FLASH_PAGE_SIZE_Msk;
			uint32_t setting_addr = file->cur_addr + SIZE_TO_SIZE_W_HEADERS(cursor);
			if (data_offset + (cursor & FLASH_PAGE_SIZE_Msk) >= FMC_FLASH_PAGE_SIZE)
				setting_addr += sizeof(file_header_t);
			file->cur_addr = setting_addr;
			break;
		}
		case S_END: {
			if (cursor > 0) 
				return FILE_EOF;
			cursor = - cursor;
			uint32_t data_offset = (SIZE_TO_SIZE_W_HEADERS(file->size) + sizeof(file_header_t)) & 
				FLASH_PAGE_SIZE_Msk;
			uint32_t setting_addr = (SIZE_TO_SIZE_W_HEADERS(file->size) + sizeof(file_header_t)) -
				SIZE_TO_SIZE_W_HEADERS(cursor);
			if (data_offset - (cursor & FLASH_PAGE_SIZE_Msk) < sizeof(file_header_t))
				setting_addr -= sizeof(file_header_t);
			file->cur_addr = setting_addr;
			break;
		}
		case S_SET: {
			file->cur_addr = SIZE_TO_SIZE_W_HEADERS(cursor) + sizeof(file_header_t);
			break;
		}
		default:
			return FILE_INVALID_PARAM;
			break;
	}
	ret = file->cur_addr - SIZE_TO_PAGES_ROUND_UP(file->cur_addr) * sizeof(file_header_t);
	return ret;
}

