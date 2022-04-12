#ifndef TEXT_H
#define TEXT_H

#include "main.h"
#include "font.h"
#include "internal_palette.h"

#define CHAR_SEP_SIZE   1

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
);

int text_strlen_px(font_info_t *font, char *str);

#endif // TEXT_H
