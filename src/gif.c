#include "string.h"
#include "math.h"
#include "internal_palette.h"
#include "gif.h"

#define MAX_PALETTE_SIZE		256
#define INTERNAL_PALETTE_SIZE	216

#define FILE_BUF_SIZE			256

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
	uint8_t pixel_proportion;
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

static uint8_t __attribute__((aligned((4)))) file_buf[FILE_BUF_SIZE] = {0};

static struct {
	uint8_t *next_block;
	uint8_t *cur_block;
	
	uint8_t i_palette[INTERNAL_PALETTE_SIZE][DISP_LEDS_NUM];
	
	uint8_t g_palette[MAX_PALETTE_SIZE];
	uint16_t g_palette_size;
	
	uint8_t l_palette[MAX_PALETTE_SIZE];
	uint16_t l_palette_size;
	
	uint16_t scr_width;
	uint16_t scr_height;
	
	uint8_t bg_color_ind;
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
	
	.bg_color_ind = 0,
};

bool is_gif_file(file_t *file) {
	if (file == NULL)
		return false;
	
	storage_file_set_cursor(file, 0);
	storage_file_read(file, (uint32_t *)file_buf, 8);
	gif_header_t *f = (gif_header_t *)file_buf;
	
	if (memcmp(f->sig, "GIF", 3) != 0)
		return false;
	
	if (memcmp(f->ver, "89a", 3) != 0)
		return false;
	
	return true;
}


int parse_gif_header(const gif_header_t *hgif) {
	
	gif.scr_width = hgif->scr.width;
	gif.scr_height = hgif->scr.height;
	
	if (!hgif->scr.palette_flags.bits.have_global)
		return 0;
	
	gif.g_palette_size = 1 << (hgif->scr.palette_flags.bits.size + 1);
	
	gif.bg_color_ind = hgif->scr.bg_color_ind;
	
	return 1;
}


void parse_gpalette(const uint8_t *palette, uint8_t size) {
	for (uint16_t i = 0; i < size * DISP_LEDS_NUM; i += DISP_LEDS_NUM) {
		gif.g_palette[i] = palette[i];
		gif.g_palette[i + 1] = palette[i + 1];
		gif.g_palette[i + 2] = palette[i + 2];
		printf("[%02X%02X%02X]\n", gif.g_palette[i], gif.g_palette[i + 1], gif.g_palette[i + 2]);
	}
}


gif_error_t gif_execute(file_t *file) {
	gif_error_t ret = G_OK;
	if (file == NULL)
		return G_NOT_GIF_FILE;

	storage_file_set_cursor(file, 0);
	
	int header_size = (sizeof(gif_header_t) & 0xFFFFFFFC) + (sizeof(gif_header_t) & 0x03 ? 4 : 0);
	printf("header_size = %d\n", header_size);
	if (storage_file_read(file, (uint32_t *)file_buf, header_size) != header_size)
		return G_NOT_GIF_FILE;

	if (parse_gif_header((gif_header_t *)file_buf)) {
		printf("have global palette\n");
		if (storage_file_read(file, (uint32_t *)file_buf, gif.g_palette_size * DISP_LEDS_NUM) !=
			 gif.g_palette_size * DISP_LEDS_NUM)
			return G_NOT_GIF_FILE;
		parse_gpalette(file_buf, gif.g_palette_size);
	} else {
		printf("have no global palette\n");
	}

	printf("gif.scr_width = %d\n", gif.scr_width);
	printf("gif.scr_height = %d\n", gif.scr_height);
	printf("gif.g_palette_size = %d\n", gif.g_palette_size);
	printf("gif.bg_color_ind = %d\n", gif.bg_color_ind);
	return ret;
}

