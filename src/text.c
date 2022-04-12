#include "text.h"
#include "string.h"

static void text_draw_char(
    uint8_t *buf, 
    const uint8_t *bmp, 
    int x0, 
    int y0,
    int x1,
    int y1,
    int char_offset_x,
    int char_offset_y,
    uint8_t char_width,
    uint8_t char_height,
    uint8_t char_color[DISP_LEDS_NUM],
    uint8_t background_color[DISP_LEDS_NUM]
) {
    if (x1 > DISP_COLS_NUM)
        x1 = DISP_COLS_NUM;
    if (y1 > DISP_ROWS_NUM)
        y1 = DISP_ROWS_NUM;
    int max_row = y0 + char_height - char_offset_y;
    int max_col = x0 + char_width - char_offset_x;
    max_col = max_col > x1 ? x1 : max_col;
    max_row = max_row > y1 ? y1 : max_row;
    uint8_t cwb = (char_width >> 3) + (char_width & 0x7 ? 1 : 0);
    uint8_t bit;
    for (int row = y0; row < max_row; row++) {
        for (int col = x0; col < max_col; col++) {
            bit = col - x0 + char_offset_x;
            memcpy(
                &buf[(row * DISP_COLS_NUM + col) * DISP_LEDS_NUM], 
                (bmp[(row - y0 + char_offset_y) * cwb + (bit >> 3)] & (0x80 >> (bit & 0x7))) ? 
                    char_color : background_color,
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
    *char_num = 0;
    int position = font->descriptors[(str[*char_num] - font->start_character) << 1];
    int length = strlen(str);
    while (x > position) {
        (*char_num)++;
        if (*char_num >= length)
            return -1;
        position += font->descriptors[(str[*char_num] - font->start_character) << 1] + 
            CHAR_SEP_SIZE;
    }
    *ch_off = font->descriptors[(str[*char_num] - font->start_character) << 1] - 
        (position - x);
    return 0;
}

int text_draw_string(
    uint8_t *buf, 
    font_info_t *font,
    char *str, 
    int x0,
    int y0,
    int x1,
    int y1,
    int string_offset_x,
    int string_offset_y,
    uint8_t char_color[DISP_LEDS_NUM],
    uint8_t background_color[DISP_LEDS_NUM]
) {
    int char_counter;
    uint8_t char_offset;
    if (text_str_calc_pos(font, str, string_offset_x, &char_offset, &char_counter) != 0)
        return -1;
    uint16_t bitmap_index;
    uint8_t char_height = font->character_height;
    uint8_t char_width;
    uint16_t desc_index;
    int str_len = strlen(str);
    while (x0 < x1 && char_counter < str_len) {
        desc_index = (str[char_counter] - font->start_character) << 1;
        bitmap_index = font->descriptors[desc_index + 1];
        char_width = font->descriptors[desc_index];
        text_draw_char(
            buf, 
            &font->bitmaps[bitmap_index], 
            x0, 
            y0,
            x1,
            y1,
            char_offset, 
            string_offset_y, 
            char_width,
            char_height,
            char_color,
            background_color
        );
        x0 += char_width + CHAR_SEP_SIZE - char_offset;
        char_offset = 0;
        char_counter++;
    }
    return 0;
}

int text_strlen_px(font_info_t *font, char *str) {
    int len = strlen(str);
    int sum = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] < font->start_character ||
            str[i] > font->end_character)
            return -1;
        sum += font->descriptors[(str[i] - font->start_character) << 1] + 
            CHAR_SEP_SIZE;
    }
    return sum;
}
