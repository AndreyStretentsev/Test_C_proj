#ifndef GIF_H
#define GIF_H

#include "main.h"
#include "storage.h"

typedef enum {
	G_OK = 0,
	G_NOT_GIF_FILE = -1,
} gif_error_t;

bool is_gif_file(file_t *file);
gif_error_t gif_execute(file_t *file);

#endif // GIF_H