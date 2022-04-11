#include "text.h"
#include "string.h"
#include "internal_palette.h"

static void text_draw_char(
    uint8_t *buf, 
    const uint8_t *bmp, 
    uint16_t x, 
    uint16_t y,
    uint16_t c_off_x,
    uint16_t c_off_y,
    uint8_t c_width,
    uint8_t c_height,
    uint8_t c_color[DISP_LEDS_NUM],
    uint8_t b_color[DISP_LEDS_NUM]
) {
    int max_row = y + c_height - c_off_y;
    int max_col = x + c_width - c_off_x;
    max_col = max_col > DISP_COLS_NUM ? DISP_COLS_NUM : max_col;
    max_row = max_row > DISP_ROWS_NUM ? DISP_ROWS_NUM : max_row;
    uint8_t cwb = c_width >> 3 + (c_width & 0x7 ? 1 : 0);
    uint8_t bit;
    for (int row = y; row < max_col; row++) {
        for (int col = x; col < max_row; col++) {
            bit = col - x;
            memcpy(
                &buf[(row * DISP_COLS_NUM + col) * DISP_LEDS_NUM], 
                (bmp[(row - y) * cwb + bit >> 3] >> (8 - bit & 0x7)) ? 
                    c_color : b_color,
                DISP_LEDS_NUM
            );
        }
    }
}

static int text_str_calc_pos(
    font_info_t *font, 
    char *str, int x, 
    uint8_t *ch_off, 
    int *char_num
) {
    int pos = 0;
    *char_num = -1;
    int len = strlen(str);
    while (x > pos) {
        *char_num++;
        if (*char_num >= len)
            return -1;
        pos += font->descriptors[str[*char_num] << 1] + CHAR_SEP_SIZE;
    }
    *ch_off = font->descriptors[str[*char_num] << 1] - (pos - x);
    return 0;
}

int text_draw_string(
    uint8_t *buf, 
    font_info_t *font,
    char *str, 
    int x,
    int y,
    int s_off_x,
    int s_off_y,
    uint8_t c_color[DISP_LEDS_NUM],
    uint8_t b_color[DISP_LEDS_NUM]
) {
    int first_char;
    uint8_t ch_offset;
    if (text_str_calc_pos(font, str, s_off_x, &ch_offset, &first_char) != 0)
        return -1;
    for (int i = first_char;) {

    }
}

int text_strlen_px(font_info_t *font, char *str) {
    int len = strlen(str);
    int sum = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] < font->start_character ||
            str[i] > font->end_character)
            return -1;
        sum += font->descriptors[str[i] << 1] + CHAR_SEP_SIZE;
    }
    return sum;
}
