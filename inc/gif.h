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
void *console_create_display(int width, int height);
void console_display_image(void *display, uint8_t *raw_image);

#endif // GIF_H