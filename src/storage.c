
#include "storage.h"
#include <string.h>
#include "flash.h"
#include <stdio.h>

#define DATA_FLASH_PAGES_NUM	(DATA_FLASH_SIZE / FMC_FLASH_PAGE_SIZE)
#define FILES_NUM_MAX	DATA_FLASH_PAGES_NUM

typedef struct {
	char name[4];
	uint32_t id;
	uint32_t size;
} file_header_t;

#define PAGE_IND_TO_ADDR(ind)	(ind * FMC_FLASH_PAGE_SIZE + DATA_FLASH_BASE)
#define SET_FILE_HEADER(hfile, fid, fsize)	\
	do { \
		(hfile).id = (fid); \
		(hfile).size = (fsize); \
		(hfile).name[0] = 'F'; \
		(hfile).name[1] = 'I'; \
		(hfile).name[2] = 'L'; \
		(hfile).name[3] = 'E'; \
	} while(0)
	
#define FLASH_PAGE_SIZE_Msk	0x0FFF
#define FLASH_PAGE_SIZE_Pos	12
#define SIZE_TO_PAGES_ROUND_UP(size)	(((size) >> FLASH_PAGE_SIZE_Pos) + ((size) & FLASH_PAGE_SIZE_Msk ? 1 : 0))
#define SIZE_TO_PAGES_ROUND_DW(size)	((size) >> FLASH_PAGE_SIZE_Pos)
#define SIZE_TO_PAGES_W_HEADERS(size) 	(SIZE_TO_PAGES_ROUND_UP(SIZE_TO_PAGES_ROUND_UP(size) * sizeof(file_header_t) + (size)))
#define SIZE_TO_SIZE_W_HEADERS(size)	(((SIZE_TO_PAGES_ROUND_DW(SIZE_TO_PAGES_ROUND_UP(size) * sizeof(file_header_t) + (size))) << FLASH_PAGE_SIZE_Pos) + ((size) & FLASH_PAGE_SIZE_Msk))

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


f_error_t storage_get_file_by_id(file_t *file, uint16_t id) {
	int first_file_page_ind = storage_get_next_page_index_by_id(id, 0);
	if (first_file_page_ind < 0)
		return FILE_NOT_FOUND;
	file->id = f_table.page_file_id[first_file_page_ind];
	file->st_addr = PAGE_IND_TO_ADDR(first_file_page_ind);
	file_header_t hfile;
	storage_read_page_wa((uint32_t *)&hfile, file->st_addr, sizeof(hfile));
	file->size = hfile.size;
	file->cur_addr = file->st_addr;
	return FILE_OK;
}

#if 0
uint32_t tmp_page[FMC_FLASH_PAGE_SIZE / 4];

static void storage_record_move(record_t *record, uint32_t addr) {
	for (uint8_t page = 0; 
		page < record->size / FMC_FLASH_PAGE_SIZE; 
		page++) {
		storage_read_wa(
			tmp_page, 
			record->addr + FMC_FLASH_PAGE_SIZE * page, 
			FMC_FLASH_PAGE_SIZE
		);
		storage_write_in_page_wa(
			tmp_page, 
			addr + FMC_FLASH_PAGE_SIZE * page, 
			FMC_FLASH_PAGE_SIZE
		);
	}
	record->addr = addr;
}


static void storage_fragmentate_sorted() {
	uint32_t prev_addr = DATA_FLASH_BASE;
	for (uint8_t i = 0; i < FILES_NUM_MAX; i++) {
		if (f_table.records[i].id == 0)
			continue;
		if (f_table.records[i].addr > prev_addr) {
			storage_record_move(&f_table.records[i], prev_addr);
		}
		prev_addr = f_table.records[i].addr + f_table.records[i].size;
	}
}


static uint8_t storage_find_free_blocks_sorted() {
	memset(free_list, 0, sizeof(free_list));
	uint8_t blocks_num = 0;
	for (uint8_t i = 0; i < FILES_NUM_MAX; i++) {
		if (f_table.records[i].id == 0)
			continue;
		if (f_table.records[i].addr > free_list[blocks_num].addr) {
			free_list[blocks_num].size = 
				f_table.records[i].addr - free_list[blocks_num].addr;
			blocks_num++;                         
		} 
		free_list[blocks_num].addr = 
			f_table.records[i].addr + f_table.records[i].size;
	}
	if (free_list[blocks_num].addr < DATA_FLASH_END) {
		free_list[blocks_num].size = 
			DATA_FLASH_END - free_list[blocks_num].addr;
	} else {
		free_list[blocks_num].addr = 0;
		blocks_num--;
	}
	return blocks_num + 1;
}


static inline void swap_records(record_t *r1, record_t *r2) {
	record_t tmp;
	memcpy(&tmp, r1, sizeof(record_t));
	memcpy(r1, r2, sizeof(record_t));
	memcpy(r2, &tmp, sizeof(record_t));
}


static void storage_sort_records() {
	for (int i = 0; i < FILES_NUM_MAX; i++) {
		bool sorted = true;
		for (int j = 0; j < FILES_NUM_MAX - (i + 1); j++) { 
			if (f_table.records[j].addr > f_table.records[j + 1].addr) {
				sorted = false;
				swap_records(&f_table.records[j], &f_table.records[j + 1]);
			}
		}
		if (sorted) 
			break;
	}
}


static uint32_t storage_get_block(uint32_t block_size) {
	uint8_t free_blocks_num = storage_find_free_blocks_sorted();
	uint32_t min_size = DATA_FLASH_SIZE;
	int min_fitting_block_ind = -1;
	for (uint8_t i = 0; i < free_blocks_num; i++) {
		if (free_list[i].size < block_size ||
			free_list[i].size >= min_size)
			continue;
		min_size = free_list[i].size;
		min_fitting_block_ind = i;
	}
	if (min_fitting_block_ind < 0)
		return 0;
	return free_list[min_fitting_block_ind].addr;
}
#endif


f_error_t storage_file_create(file_t *file, uint16_t id, uint32_t size) {
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
	
	f_error_t ret = FILE_OK;
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
	file->cur_addr = file->st_addr + sizeof(file_header_t);
	file->size = size;
	f_table.free_pages -= file_size_in_pages;
	if (ret != FILE_OK) {
		storage_file_delete(file);
	}
	
	return ret;
}


f_error_t storage_file_delete(file_t *file) {
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


f_error_t storage_file_write(file_t *file, uint32_t *data, int len) {
	f_error_t ret = FILE_OK;
	uint32_t end_writing_addr = file->cur_addr + SIZE_TO_SIZE_W_HEADERS(len);
	uint32_t end_of_file_addr = file->st_addr + SIZE_TO_SIZE_W_HEADERS(file->size);
	printf("end_writing_addr = 0x%08X(%d), end_of_file_addr = 0x%08X(%d)\n", 
		end_writing_addr, end_writing_addr,
		end_of_file_addr, end_of_file_addr
	);
	if (end_writing_addr >= end_of_file_addr) {
		len -= end_writing_addr - end_of_file_addr;
		ret = FILE_EOF;
	}
	uint32_t page_st_addr = file->cur_addr & ~FLASH_PAGE_SIZE_Msk;
	uint8_t page_ind = (page_st_addr - DATA_FLASH_BASE) >> FLASH_PAGE_SIZE_Pos;
	
	uint32_t data_offset = file->cur_addr - page_st_addr;
	
	if (len + data_offset <= FMC_FLASH_PAGE_SIZE) {
		storage_write_in_page_wa(data, file->cur_addr, len);
		file->cur_addr += len;
		return ret;
	}
	
	int first_page_len = FMC_FLASH_PAGE_SIZE - data_offset;
	storage_write_in_page_wa(data, file->cur_addr, first_page_len);
	
	len -= first_page_len;
	page_ind = storage_get_next_page_index_by_id(file->id, page_ind);
	file->cur_addr = PAGE_IND_TO_ADDR(page_ind) + sizeof(file_header_t);
	data += first_page_len;
	
	uint32_t page_size_wo_header = FMC_FLASH_PAGE_SIZE - sizeof(file_header_t);
	while (len >= page_size_wo_header) {
		storage_write_in_page_wa(data, file->cur_addr, page_size_wo_header);
		len -= page_size_wo_header;
		page_ind = storage_get_next_page_index_by_id(file->id, page_ind);
		file->cur_addr = PAGE_IND_TO_ADDR(page_ind) + sizeof(file_header_t);
		data += page_size_wo_header;
	}
	storage_write_in_page_wa(data, file->cur_addr, len);
	file->cur_addr += len;
	return ret;
}


f_error_t storage_file_read(file_t *file, uint32_t *data, int len) {
	f_error_t ret = FILE_OK;
	uint32_t end_reading_addr = file->cur_addr + SIZE_TO_SIZE_W_HEADERS(len);
	uint32_t end_of_file_addr = file->st_addr + SIZE_TO_SIZE_W_HEADERS(file->size);
	printf("end_reading_addr = 0x%08X(%d), end_of_file_addr = 0x%08X(%d)\n", 
		end_reading_addr, end_reading_addr,
		end_of_file_addr, end_of_file_addr
	);
	if (end_reading_addr >= end_of_file_addr) {
		len -= end_reading_addr - end_of_file_addr;
		return FILE_EOF;
	}
	uint32_t page_st_addr = file->cur_addr & ~FLASH_PAGE_SIZE_Msk;
	uint8_t page_ind = (page_st_addr - DATA_FLASH_BASE) >> FLASH_PAGE_SIZE_Pos;
	
	uint32_t data_offset = file->cur_addr - page_st_addr;
	
	if (len + data_offset <= FMC_FLASH_PAGE_SIZE) {
		storage_read_page_wa(data, file->cur_addr, len);
		file->cur_addr += len;
		return ret;
	}
	
	int first_page_len = FMC_FLASH_PAGE_SIZE - data_offset;
	storage_read_page_wa(data, file->cur_addr, first_page_len);
	
	len -= first_page_len;
	page_ind = storage_get_next_page_index_by_id(file->id, page_ind);
	file->cur_addr = PAGE_IND_TO_ADDR(page_ind) + sizeof(file_header_t);
	data += first_page_len;
	
	uint32_t page_size_wo_header = FMC_FLASH_PAGE_SIZE - sizeof(file_header_t);
	while (len >= page_size_wo_header) {
		storage_read_page_wa(data, file->cur_addr, page_size_wo_header);
		len -= page_size_wo_header;
		page_ind = storage_get_next_page_index_by_id(file->id, page_ind);
		file->cur_addr = PAGE_IND_TO_ADDR(page_ind) + sizeof(file_header_t);
		data += page_size_wo_header;
	}
	storage_read_page_wa(data, file->cur_addr, len);
	file->cur_addr += len;
	return ret;
}


void storage_file_set_cursor(file_t *file, uint32_t cursor) {
	file->cur_addr = file->st_addr + sizeof(file_header_t) + 
		(cursor / (FMC_FLASH_PAGE_SIZE - sizeof(file_header_t))) * FMC_FLASH_PAGE_SIZE +
		cursor % (FMC_FLASH_PAGE_SIZE - sizeof(file_header_t));
}

