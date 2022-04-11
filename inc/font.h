#ifndef FONT_H
#define FONT_H

#include "main.h"

typedef struct font_info
{
   uint8_t character_height;
   uint8_t start_character;
   uint8_t end_character;
   const uint16_t *descriptors;
   const uint8_t *bitmaps;
} font_info_t;

#endif // FONT_H
