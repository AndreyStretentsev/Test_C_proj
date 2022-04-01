#include "string.h"
#include "math.h"
#include "internal_palette.h"
#include "ansiGraphic2.1.h"
#include "gif.h"

#define MAX_PALETTE_SIZE		256
#define INTERNAL_PALETTE_SIZE	216

#define FILE_BUF_SIZE			512

typedef struct {
	uint16_t width;
	uint16_t height;
	union {
		uint8_t byte;
		struct {
			unsigned size: 3;
			unsigned is_sorted: 1;
			unsigned color_resolution: 3;
			unsigned have_global: 1;
			} __attribute__ ((packed)) bits;
	} __attribute__ ((packed)) palette_flags;
	uint8_t bg_color_ind;
	uint8_t aspect;
} __attribute__ ((packed)) screen_descriptor_t;

typedef struct {
	uint8_t id;
	uint16_t x_offset;
	uint16_t y_offset;
	uint16_t width;
	uint16_t heigth;
	union {
		uint8_t byte;
		struct {
			unsigned size: 3;
			unsigned reserved: 2;
			unsigned is_sorted: 1;
			unsigned is_interlaced: 1;
			unsigned have_local: 1;
			} __attribute__ ((packed)) bits;
	} __attribute__ ((packed)) palette_flags;
} __attribute__ ((packed)) image_descriptor_t;

typedef struct {
	char sig[3];
	char ver[3];
	screen_descriptor_t scr;
} __attribute__ ((packed)) gif_header_t;	

static uint8_t __attribute__((aligned((4)))) file_buf[FILE_BUF_SIZE];
static uint8_t frame_buf[DISP_COLS_NUM][DISP_ROWS_NUM][DISP_LEDS_NUM];

static struct {
	uint8_t *next_block;
	uint8_t *cur_block;
	
	uint8_t i_palette[INTERNAL_PALETTE_SIZE][DISP_LEDS_NUM];
	
	uint8_t g_palette[MAX_PALETTE_SIZE][DISP_LEDS_NUM];
	uint16_t g_palette_size;
	
	uint8_t l_palette[MAX_PALETTE_SIZE][DISP_LEDS_NUM];
	uint16_t l_palette_size;
	
	uint16_t scr_width;
	uint16_t scr_height;

	uint8_t color_resolution;
	
	uint8_t bg_color_ind;
	uint8_t bg_color[DISP_LEDS_NUM];

	uint32_t file_buf_addr;
	uint16_t file_buf_offset;

} gif = {
	.next_block = NULL,
	.cur_block = NULL,
	
	.scr_width = DISP_COLS_NUM,
	.scr_height = DISP_ROWS_NUM,
	
	.i_palette = GIF_INTERNAL_PALETTE,
	
	.g_palette = {0},
	.g_palette_size = 0,
	
	.l_palette = {0},
	.l_palette_size = 0,
	
	.color_resolution = 1,

	.bg_color_ind = 0,

	.file_buf_addr = 0,
	.file_buf_offset = 0,
};

bool is_gif_file(file_t *file) {
	if (file == NULL)
		return false;
	
	storage_file_set_cursor(file, 0, S_SET);
	storage_file_read(file, (uint32_t *)file_buf, 8);
	gif_header_t *f = (gif_header_t *)file_buf;
	
	if (memcmp(f->sig, "GIF", 3) != 0)
		return false;
	
	if (memcmp(f->ver, "89a", 3) != 0)
		return false;
	
	return true;
}


void print_gpalette() {
	
	for (uint16_t i = 0; i < gif.g_palette_size; i ++) {
		printf(
			"[%02X%02X%02X]\n", 
			gif.g_palette[i][0], 
			gif.g_palette[i][1], 
			gif.g_palette[i][2]
			);
	}
}


gif_error_t gif_open(file_t *file) {
	gif_error_t ret = G_OK;
	if (file == NULL)
		return G_NOT_GIF_FILE;

	gif.file_buf_addr = 0;
	storage_file_set_cursor(file, gif.file_buf_addr, S_SET);
	storage_file_read(file, (uint32_t *)file_buf, FILE_BUF_SIZE);

	gif_header_t *hgif = (gif_header_t *)file_buf;
	gif.scr_width = hgif->scr.width;
	gif.scr_height = hgif->scr.height;
	
	if (hgif->scr.palette_flags.bits.have_global) {
		gif.g_palette_size = 1 << (hgif->scr.palette_flags.bits.size + 1);
		gif.color_resolution = hgif->scr.palette_flags.bits.color_resolution;
		gif.bg_color_ind = hgif->scr.bg_color_ind;
		memcpy(
			gif.g_palette, 
			&file_buf[sizeof(gif_header_t)], 
			gif.g_palette_size * DISP_LEDS_NUM
		);
		memcpy(gif.bg_color, gif.g_palette[gif.bg_color_ind], DISP_LEDS_NUM);
		gif.file_buf_offset = sizeof(gif_header_t) + gif.g_palette_size * DISP_LEDS_NUM;
	} else {
		gif.file_buf_offset = sizeof(gif_header_t);
		gif.g_palette_size = 0;
	}

	return ret;
}

gif_error_t gif_get_frame() {
	
}

void *console_create_display(int width, int height) {
	ansigraphic_image_RGB_t *screen = 
	ansigraphic_newImage_RGB(width * 2, height);
	return screen;
}

void console_display_image(void *display, uint8_t *raw_image) {
	ansigraphic_image_RGB_t *screen = (ansigraphic_image_RGB_t *)display;

	ansigraphic_ivector2_t xy;
	for (xy.y = 0; xy.y < screen->height; xy.y++) {
		for (xy.x = 0; xy.x < screen->width; xy.x++) {
			ansigraphic_color_RGB_t color;
			ansigraphic_color_RGB_set(
				&color, 
				raw_image[((screen->height - xy.y - 1) * screen->width + xy.x >> 1) * DISP_LEDS_NUM], 
				raw_image[((screen->height - xy.y - 1) * screen->width + xy.x >> 1) * DISP_LEDS_NUM + 1], 
				raw_image[((screen->height - xy.y - 1) * screen->width + xy.x >> 1) * DISP_LEDS_NUM + 2]
			);
			ansigraphic_pixelSetColor_RGB(screen, &xy, &color, &color);
		}
	}
	ansigraphic_imagePrint_RGB(screen);
}
