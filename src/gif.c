#include "string.h"
#include "math.h"
#include "ansiGraphic2.1.h"
#include "gif.h"

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

typedef struct {
    uint16_t length;
    uint16_t prefix;
    uint8_t  suffix;
} entry_t;

typedef struct {
    int bulk;
    int nentries;
    entry_t *entries;
} table_t;

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


gif_palette_t i_palette = {
	.colors = GIF_INTERNAL_PALETTE,
	.size = INTERNAL_PALETTE_SIZE
};


static uint16_t read_num(file_t *file)
{
    uint8_t bytes[2];

    storage_file_read(file, bytes, 2);
    return bytes[0] + (((uint16_t) bytes[1]) << 8);
}


bool is_gif_file(file_t *file) {
	const uint8_t sigver_size = 8;
	uint8_t __attribute__((aligned((4)))) buf[sigver_size];

	if (file == NULL)
		return false;
	
	storage_file_set_cursor(file, 0, S_SET);
	storage_file_read(file, buf, sigver_size);
	gif_header_t *f = (gif_header_t *)buf;
    storage_file_set_cursor(file, 0, S_SET);
	
	if (memcmp(f->sig, "GIF", 3) != 0)
		return false;
	
	if (memcmp(f->ver, "89a", 3) != 0)
		return false;
	
	return true;
}


gif_err_t gif_open(file_t *file, gif_t *gif) {
    LOGI("Gif open");
    uint8_t *bgcolor;
	gif_err_t ret = G_OK;
	gif_header_t hgif;

	if (file == NULL)
		return G_NOT_GIF_FILE;

	gif->fd = file;
	storage_file_set_cursor(file, 0, S_SET);
	storage_file_read(file, (uint8_t *)&hgif, sizeof(gif_header_t));
	
	gif->width = hgif.scr.width;
	gif->height = hgif.scr.height;

    LOGI("screen width = %d", gif->width);
    LOGI("screen height = %d", gif->height);
	
    if (hgif.scr.palette_flags.bits.have_global) {
		gif->gct.size = 1 << (hgif.scr.palette_flags.bits.size + 1);
		gif->depth = hgif.scr.palette_flags.bits.color_resolution;
		gif->bgindex = hgif.scr.bg_color_ind;
		storage_file_read(file, gif->gct.colors, gif->gct.size * DISP_LEDS_NUM);
		gif->palette = &gif->gct;
		// if (gif->bgindex)
        // 	memset(gif->frame, gif->bgindex, gif->width * gif->height);
		bgcolor = &gif->palette->colors[gif->bgindex * DISP_LEDS_NUM];
        LOGI("have global table");
        LOGI("global table size = %d", gif->gct.size);
        LOGI("color resolution = %d", gif->depth);
        LOGI("bg color index = %d", gif->bgindex);
	} else {
        LOGI("have no global table");
		gif->palette = &i_palette;
		gif->gct.size = 0;
		bgcolor = &gif->palette->colors[0];
	}

	if (bgcolor[0] || bgcolor[1] || bgcolor [2])
		for (int i = 0; i < gif->width * gif->height; i++)
			memcpy(&gif->frame[i * DISP_LEDS_NUM], bgcolor, DISP_LEDS_NUM);

    LOGI("bg color = [%02X%02X%02X]", bgcolor[0], bgcolor[1], bgcolor[2]);
    
    gif->anim_start = storage_file_set_cursor(file, 0, S_CUR);
    LOGI("animation start addr = 0x%08X(%d)", gif->anim_start, gif->anim_start);
    
    return ret;
}


static void discard_sub_blocks(gif_t *gif) {
    uint8_t size;
    LOGI("Discard subblocks");
    do {
		storage_file_read(gif->fd, &size, 1);
        storage_file_set_cursor(gif->fd, size, S_CUR);
    } while (size);
}


static void read_plain_text_ext(gif_t *gif) {
    LOGI("Read plain text");
    if (gif->plain_text) {
        uint16_t tx, ty, tw, th;
        uint8_t cw, ch, fg, bg;
        uint32_t sub_block;
        storage_file_set_cursor(gif->fd, 1, S_CUR);
        tx = read_num(gif->fd);
        ty = read_num(gif->fd);
        tw = read_num(gif->fd);
        th = read_num(gif->fd);
        storage_file_read(gif->fd, &cw, 1);
        storage_file_read(gif->fd, &ch, 1);
        storage_file_read(gif->fd, &fg, 1);
        storage_file_read(gif->fd, &bg, 1);
        sub_block = storage_file_set_cursor(gif->fd, 0, S_CUR);
        gif->plain_text(gif, tx, ty, tw, th, cw, ch, fg, bg);
        storage_file_set_cursor(gif->fd, sub_block, S_SET);
    } else {
        storage_file_set_cursor(gif->fd, 13, S_CUR);
    }
    discard_sub_blocks(gif);
}


static void read_graphic_control_ext(gif_t *gif) {
    uint8_t rdit;
    LOGI("Read graphic extention");
    storage_file_set_cursor(gif->fd, 1, S_CUR);
    storage_file_read(gif->fd, &rdit, 1);
    gif->gce.disposal = (rdit >> 2) & 3;
    gif->gce.input = rdit & 2;
    gif->gce.transparency = rdit & 1;
    gif->gce.delay = read_num(gif->fd);
    storage_file_read(gif->fd, &gif->gce.tindex, 1);
    storage_file_set_cursor(gif->fd, 1, S_CUR);
    LOGI("disposal = %d", gif->gce.disposal);
    LOGI("input = %d", gif->gce.input);
    LOGI("transparency = %d", gif->gce.transparency);
    LOGI("delay = %d ms", gif->gce.delay * 10);
}


static void read_comment_ext(gif_t *gif) {
    LOGI("Read comment");
    if (gif->comment) {
        LOGI("have comment cb");
        uint32_t sub_block = storage_file_set_cursor(gif->fd, 0, S_CUR);
        gif->comment(gif);
        storage_file_set_cursor(gif->fd, sub_block, S_SET);
    }
    discard_sub_blocks(gif);
}


static void read_application_ext(gif_t *gif)
{
    char app_id[8];
    char app_auth_code[3];
    LOGI("Read application");
    storage_file_set_cursor(gif->fd, 1, S_CUR);
    storage_file_read(gif->fd, app_id, 8);
    storage_file_read(gif->fd, app_auth_code, 3);
    if (!strncmp(app_id, "NETSCAPE", sizeof(app_id))) {
        LOGI("NETSCAPE");
        storage_file_set_cursor(gif->fd, 2, S_CUR);
        gif->loop_count = read_num(gif->fd);
        LOGI("loop count = %d", gif->loop_count);
        storage_file_set_cursor(gif->fd, 1, S_CUR);
    } else if (gif->application) {
        LOGI("have application cb");
        uint32_t sub_block = storage_file_set_cursor(gif->fd, 0, S_CUR);
        gif->application(gif, app_id, app_auth_code);
        storage_file_set_cursor(gif->fd, sub_block, S_SET);
        discard_sub_blocks(gif);
    } else {
        LOGI("have no application cb");
        discard_sub_blocks(gif);
    }
}


static void read_ext(gif_t *gif) {
    uint8_t label;
    LOGI("Read extention");
    storage_file_read(gif->fd, &label, 1);
    switch (label) {
    case 0x01:
        read_plain_text_ext(gif);
        break;
    case 0xF9:
        read_graphic_control_ext(gif);
        break;
    case 0xFE:
        read_comment_ext(gif);
        break;
    case 0xFF:
        read_application_ext(gif);
        break;
    default:
        break;
    }
}


static int read_image(gif_t *gif) {
    image_descriptor_t img_d;
    LOGI("Read image");
	storage_file_read(gif->fd, (uint8_t *)&img_d, sizeof(image_descriptor_t));

    gif->fx = img_d.x_offset;
    gif->fy = img_d.y_offset;

    LOGI("offset x = %d", gif->fx);
    LOGI("offset y = %d", gif->fy);
    
    if (gif->fx >= gif->width || gif->fy >= gif->height)
        return -1;
    
    gif->fw = img_d.width;
    gif->fh = img_d.heigth;
    
    gif->fw = MIN(gif->fw, gif->width - gif->fx);
    gif->fh = MIN(gif->fh, gif->height - gif->fy);

    LOGI("width = %d", gif->fw);
    LOGI("height = %d", gif->fh);

    if (img_d.palette_flags.bits.have_local) {
        LOGI("have local table");
        gif->lct.size = 1 << (img_d.palette_flags.bits.size + 1);
        LOGI("local table size = %d", gif->lct.size);
        storage_file_read(gif->fd, gif->lct.colors, DISP_LEDS_NUM * gif->lct.size);
        gif->palette = &gif->lct;
    } else if (gif->gct.size != 0) {
        LOGI("use global palettte");
        gif->palette = &gif->gct;
	} else {
        LOGI("use internal palette");
		gif->palette = &i_palette;
	}
	
    return 0;
}


static uint16_t get_key(
    gif_t *gif, 
    int key_size, 
    uint8_t *sub_len, 
    uint8_t *shift, 
    uint8_t *byte
    ) {
    int bits_read;
    int rpad;
    int frag_size;
    uint16_t key;

    key = 0;
    for (bits_read = 0; bits_read < key_size; bits_read += frag_size) {
        rpad = (*shift + bits_read) % 8;
        if (rpad == 0) {
            if (*sub_len == 0) {
                storage_file_read(gif->fd, sub_len, 1);
                if (*sub_len == 0)
                    return 0x1000;
            }
            storage_file_read(gif->fd, byte, 1);
            (*sub_len)--;
        }
        frag_size = MIN(key_size - bits_read, 8 - rpad);
        key |= ((uint16_t) ((*byte) << rpad)) >> bits_read;
    }
    key >>= 8 - key_size;
    key &= (1 << key_size) - 1;
    *shift = (*shift + key_size) % 8;
    return key;
}


void gif_decode_n_render(gif_t *gif, uint8_t *buffer) {
    LOGI("Decode and render");
    uint8_t lzw, byte;
    uint8_t shift = 0;
    uint8_t sub_len = 0;
    uint16_t key, clear, stop;
    uint8_t *color;
    uint16_t x, y;
    storage_file_read(gif->fd, &lzw, 1);
    LOGI("LZW = %d", lzw);
    int start = storage_file_set_cursor(gif->fd, 0, S_CUR);
    discard_sub_blocks(gif);
    int end = storage_file_set_cursor(gif->fd, 0, S_CUR);
    LOGI("frame start addr = %08X(%d)\nframe end addr = %08X(%d)", 
        start, start,
        end, end
    );
    storage_file_set_cursor(gif->fd, start, S_SET);
    clear = 1 << lzw;
    stop = clear + 1;
    lzw++;
    uint32_t frm_off = 0;
    uint32_t frm_size = gif->fw * gif->fh;
    while (frm_off < frm_size) {
        key = get_key(gif, lzw, &sub_len, &shift, &byte);
        if (key == clear) continue;
        if (key == stop || key == 0x1000) break;
        x = frm_off % gif->fw;
        y = frm_off / gif->fw;
        LOGI("key = 0x%02X", key);
        color = &gif->palette->colors[key * DISP_LEDS_NUM];
        if (!gif->gce.transparency || key != gif->gce.tindex)
            memcpy(
                &buffer[(y * gif->fw + x) * DISP_LEDS_NUM], 
                color, 
                DISP_LEDS_NUM
            );
        frm_off++;
    }
    if (key == stop)
        storage_file_read(gif->fd, &sub_len, 1);
    LOGI("set cursor to the end of the frame");
    storage_file_set_cursor(gif->fd, end, S_SET);
}


static void decode_frame(gif_t *gif) {
    LOGI("Decode frame");
    uint8_t lzw, byte;
    uint8_t shift = 0;
    uint8_t sub_len = 0;
    uint16_t key, clear, stop;
    storage_file_read(gif->fd, &lzw, 1);
    LOGI("LZW = %d", lzw);
    int start = storage_file_set_cursor(gif->fd, 0, S_CUR);
    discard_sub_blocks(gif);
    int end = storage_file_set_cursor(gif->fd, 0, S_CUR);
    storage_file_set_cursor(gif->fd, start, S_SET);
    clear = 1 << lzw;
    stop = clear + 1;
    lzw++;
    for (int y = gif->fy; y < gif->fy + gif->fh; y++) {
        for (int x = gif->fx; x < gif->fx + gif->fw; x++) {
            key = get_key(gif, lzw, &sub_len, &shift, &byte);
            if (key == clear) continue;
            if (key == stop || key == 0x1000) break;
            LOGI("y = %d\tx = %d\tkey = %d", y, x, key);
            gif->frame[y][x] = key;
        }
    }
    storage_file_set_cursor(gif->fd, end, S_SET);
}


static void render_frame_rect(gif_t *gif, uint8_t *buffer) {
    LOGI("Render frame rect");
    int i, j, k;
    uint8_t index, *color;
    i = gif->fy * gif->width + gif->fx;
    for (j = 0; j < gif->fh; j++) {
        for (k = 0; k < gif->fw; k++) {
            index = gif->frame[gif->fy + j][gif->fx + k];
            color = &gif->palette->colors[index * DISP_LEDS_NUM];
            if (!gif->gce.transparency || index != gif->gce.tindex)
                memcpy(&buffer[(i + k) * DISP_LEDS_NUM], color, DISP_LEDS_NUM);
        }
        i += gif->width;
    }
}


static void dispose(gif_t *gif) {
    LOGI("Dispose");
    int i, j, k;
    uint8_t *bgcolor;
    switch (gif->gce.disposal) {
    case 2: 
        LOGI("fill with bg color");
        i = gif->fy * gif->width + gif->fx;
        for (j = 0; j < gif->fh; j++) {
            for (k = 0; k < gif->fw; k++)
                gif->frame[i][k] = gif->bgindex;
            i += gif->width;
        }
        break;
    case 3: 
        LOGI("nothing");
        break;
    default:
        LOGI("render frame");
        // render_frame_rect(gif, gif->frame);
    }
}


int gif_get_frame(gif_t *gif) {
    char sep;
    LOGI("Get frame");
    dispose(gif);
    storage_file_read(gif->fd, &sep, 1);
    LOGI("next byte = 0x%02X('%c')", sep, sep);
    while (sep != ',') {
        if (sep == ';')
            return 0;
        if (sep == '!')
            read_ext(gif);
        else return -1;
        storage_file_read(gif->fd, &sep, 1);
        LOGI("next byte = 0x%02X('%c')", sep, sep);
    }
    if (read_image(gif) == -1)
        return -1;
    return 1;
}


int gif_is_bgcolor(gif_t *gif, uint8_t color[DISP_LEDS_NUM]) {
    return !memcmp(
		&gif->palette->colors[gif->bgindex * DISP_LEDS_NUM], 
		color, 
		DISP_LEDS_NUM
	);
}


void gif_rewind(gif_t *gif) {
    LOGI("Rewind");
    storage_file_set_cursor(gif->fd, gif->anim_start, S_SET);
}


void *console_create_display(int width, int height) {
#if (LOGLEVEL == LOGLEVEL_OFF)
	ansigraphic_image_RGB_t *screen = 
	ansigraphic_newImage_RGB(width * 2, height);
	return screen;
#else
    return NULL;
#endif
}

void console_delete_display(void *screen) {
#if (LOGLEVEL == LOGLEVEL_OFF)
    ansigraphic_deleteImage_RGB(screen);
#endif
}

void console_display_image(void *display, uint8_t *raw_image) {
#if (LOGLEVEL == LOGLEVEL_OFF)
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
#endif
}
