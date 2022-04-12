#ifndef GIF_H
#define GIF_H

#include "main.h"
#include "storage.h"
#include "internal_palette.h"

#define MAX_PALETTE_SIZE		256

#define FILE_BUF_SIZE			512

typedef enum {
	G_OK = 0,
	G_NOT_GIF_FILE = -1,
	G_TRAILER = -2,
	G_ERROR = -3,
} gif_err_t;

typedef struct gif_palette_t {
    int size;
    uint8_t colors[MAX_PALETTE_SIZE * DISP_LEDS_NUM];
} gif_palette_t;

typedef struct gif_gce_t {
    uint16_t delay;
    uint8_t tindex;
    uint8_t disposal;
    int input;
    int transparency;
} gif_gce_t;

typedef struct gif_t {
    file_t *fd;
    uint32_t anim_start;
    uint16_t width, height;
    uint16_t depth;
    uint16_t loop_count;
    gif_gce_t gce;
    gif_palette_t *palette;
    gif_palette_t lct, gct;
    void (*plain_text)(
        struct gif_t *gif, uint16_t tx, uint16_t ty,
        uint16_t tw, uint16_t th, uint8_t cw, uint8_t ch,
        uint8_t fg, uint8_t bg
    );
    void (*comment)(struct gif_t *gif);
    void (*application)(struct gif_t *gif, char id[8], char auth[3]);
    uint16_t fx, fy, fw, fh;
    uint8_t bgindex;
    uint8_t frame[DISP_ROWS_NUM][DISP_COLS_NUM];
} gif_t;

bool is_gif_file(file_t *file);
gif_err_t gif_open(file_t *file, gif_t *gif);
int gif_get_frame(gif_t *gif);
void gif_decode_n_render(gif_t *gif, uint8_t *buffer);
int gif_is_bgcolor(gif_t *gif, uint8_t color[DISP_LEDS_NUM]);
void gif_rewind(gif_t *gif);

void *console_create_display(int width, int height);
void console_display_image(void *display, uint8_t *raw_image);
void console_delete_display(void *screen);

#endif // GIF_H