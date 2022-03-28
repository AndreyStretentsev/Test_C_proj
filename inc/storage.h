#ifndef STORAGE_H
#define STORAGE_H

#include "main.h"

typedef struct {
	uint16_t id;
	uint32_t st_addr;
	uint32_t size;
	uint32_t cur_addr;
} file_t;


typedef enum {
	FILE_OK = 0,
	FILE_NOT_FOUND = -1,
	FILE_NOT_ENOUGH_SPACE = -2,
	FILE_ALREADY_EXISTS = -3,
	FILE_EOF = -4,
	FILE_INTERNAL_ERROR = -5,
} f_error_t;


void storage_init();
uint16_t storage_generate_id();
f_error_t storage_get_file_by_id(file_t *file, uint16_t id);
f_error_t storage_file_create(file_t *file, uint16_t id, uint32_t size);
int storage_file_write(file_t *file, uint32_t *data, int len);
int storage_file_read(file_t *file, uint32_t *data, int len);
f_error_t storage_file_delete(file_t *file);
f_error_t storage_file_set_cursor(file_t *file, uint32_t cursor);
#endif // STORAGE_H
