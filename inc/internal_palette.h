#ifndef INTERNAL_PALETTE_H
#define INTERNAL_PALETTE_H

#define DISP_LEDS_NUM	3
#define DISP_COLS_NUM	30
#define DISP_ROWS_NUM	10

#define GIF_INTERNAL_PALETTE { \
		{0x00, 0x00, 0x00}, {0x00, 0x00, 0x33}, {0x00, 0x00, 0x66}, {0x00, 0x00, 0x99}, {0x00, 0x00, 0xCC}, {0x00, 0x00, 0xFF}, \
		{0x00, 0x33, 0x00}, {0x00, 0x33, 0x33}, {0x00, 0x33, 0x66}, {0x00, 0x33, 0x99}, {0x00, 0x33, 0xCC}, {0x00, 0x33, 0xFF}, \
		{0x00, 0X66, 0x00}, {0x00, 0X66, 0x33}, {0x00, 0X66, 0x66}, {0x00, 0X66, 0x99}, {0x00, 0X66, 0xCC}, {0x00, 0X66, 0xFF}, \
		{0x00, 0x99, 0x00}, {0x00, 0x99, 0x33}, {0x00, 0x99, 0x66}, {0x00, 0x99, 0x99}, {0x00, 0x99, 0xCC}, {0x00, 0x99, 0xFF}, \
		{0x00, 0xCC, 0x00}, {0x00, 0xCC, 0x33}, {0x00, 0xCC, 0x66}, {0x00, 0xCC, 0x99}, {0x00, 0xCC, 0xCC}, {0x00, 0xCC, 0xFF}, \
		{0x00, 0xFF, 0x00}, {0x00, 0xFF, 0x33}, {0x00, 0xFF, 0x66}, {0x00, 0xFF, 0x99}, {0x00, 0xFF, 0xCC}, {0x00, 0xFF, 0xFF}, \
		\
		{0x33, 0x00, 0x00}, {0x33, 0x00, 0x33}, {0x33, 0x00, 0x66}, {0x33, 0x00, 0x99}, {0x33, 0x00, 0xCC}, {0x33, 0x00, 0xFF}, \
		{0x33, 0x33, 0x00}, {0x33, 0x33, 0x33}, {0x33, 0x33, 0x66}, {0x33, 0x33, 0x99}, {0x33, 0x33, 0xCC}, {0x33, 0x33, 0xFF}, \
		{0x33, 0X66, 0x00}, {0x33, 0X66, 0x33}, {0x33, 0X66, 0x66}, {0x33, 0X66, 0x99}, {0x33, 0X66, 0xCC}, {0x33, 0X66, 0xFF}, \
		{0x33, 0x99, 0x00}, {0x33, 0x99, 0x33}, {0x33, 0x99, 0x66}, {0x33, 0x99, 0x99}, {0x33, 0x99, 0xCC}, {0x33, 0x99, 0xFF}, \
		{0x33, 0xCC, 0x00}, {0x33, 0xCC, 0x33}, {0x33, 0xCC, 0x66}, {0x33, 0xCC, 0x99}, {0x33, 0xCC, 0xCC}, {0x33, 0xCC, 0xFF}, \
		{0x33, 0xFF, 0x00}, {0x33, 0xFF, 0x33}, {0x33, 0xFF, 0x66}, {0x33, 0xFF, 0x99}, {0x33, 0xFF, 0xCC}, {0x33, 0xFF, 0xFF}, \
		\
		{0x66, 0x00, 0x00}, {0x66, 0x00, 0x33}, {0x66, 0x00, 0x66}, {0x66, 0x00, 0x99}, {0x66, 0x00, 0xCC}, {0x66, 0x00, 0xFF}, \
		{0x66, 0x33, 0x00}, {0x66, 0x33, 0x33}, {0x66, 0x33, 0x66}, {0x66, 0x33, 0x99}, {0x66, 0x33, 0xCC}, {0x66, 0x33, 0xFF}, \
		{0x66, 0X66, 0x00}, {0x66, 0X66, 0x33}, {0x66, 0X66, 0x66}, {0x66, 0X66, 0x99}, {0x66, 0X66, 0xCC}, {0x66, 0X66, 0xFF}, \
		{0x66, 0x99, 0x00}, {0x66, 0x99, 0x33}, {0x66, 0x99, 0x66}, {0x66, 0x99, 0x99}, {0x66, 0x99, 0xCC}, {0x66, 0x99, 0xFF}, \
		{0x66, 0xCC, 0x00}, {0x66, 0xCC, 0x33}, {0x66, 0xCC, 0x66}, {0x66, 0xCC, 0x99}, {0x66, 0xCC, 0xCC}, {0x66, 0xCC, 0xFF}, \
		{0x66, 0xFF, 0x00}, {0x66, 0xFF, 0x33}, {0x66, 0xFF, 0x66}, {0x66, 0xFF, 0x99}, {0x66, 0xFF, 0xCC}, {0x66, 0xFF, 0xFF}, \
		\
		{0x99, 0x00, 0x00}, {0x99, 0x00, 0x33}, {0x99, 0x00, 0x66}, {0x99, 0x00, 0x99}, {0x99, 0x00, 0xCC}, {0x99, 0x00, 0xFF}, \
		{0x99, 0x33, 0x00}, {0x99, 0x33, 0x33}, {0x99, 0x33, 0x66}, {0x99, 0x33, 0x99}, {0x99, 0x33, 0xCC}, {0x99, 0x33, 0xFF}, \
		{0x99, 0X66, 0x00}, {0x99, 0X66, 0x33}, {0x99, 0X66, 0x66}, {0x99, 0X66, 0x99}, {0x99, 0X66, 0xCC}, {0x99, 0X66, 0xFF}, \
		{0x99, 0x99, 0x00}, {0x99, 0x99, 0x33}, {0x99, 0x99, 0x66}, {0x99, 0x99, 0x99}, {0x99, 0x99, 0xCC}, {0x99, 0x99, 0xFF}, \
		{0x99, 0xCC, 0x00}, {0x99, 0xCC, 0x33}, {0x99, 0xCC, 0x66}, {0x99, 0xCC, 0x99}, {0x99, 0xCC, 0xCC}, {0x99, 0xCC, 0xFF}, \
		{0x99, 0xFF, 0x00}, {0x99, 0xFF, 0x33}, {0x99, 0xFF, 0x66}, {0x99, 0xFF, 0x99}, {0x99, 0xFF, 0xCC}, {0x99, 0xFF, 0xFF}, \
		\
		{0xCC, 0x00, 0x00}, {0xCC, 0x00, 0x33}, {0xCC, 0x00, 0x66}, {0xCC, 0x00, 0x99}, {0xCC, 0x00, 0xCC}, {0xCC, 0x00, 0xFF}, \
		{0xCC, 0x33, 0x00}, {0xCC, 0x33, 0x33}, {0xCC, 0x33, 0x66}, {0xCC, 0x33, 0x99}, {0xCC, 0x33, 0xCC}, {0xCC, 0x33, 0xFF}, \
		{0xCC, 0X66, 0x00}, {0xCC, 0X66, 0x33}, {0xCC, 0X66, 0x66}, {0xCC, 0X66, 0x99}, {0xCC, 0X66, 0xCC}, {0xCC, 0X66, 0xFF}, \
		{0xCC, 0x99, 0x00}, {0xCC, 0x99, 0x33}, {0xCC, 0x99, 0x66}, {0xCC, 0x99, 0x99}, {0xCC, 0x99, 0xCC}, {0xCC, 0x99, 0xFF}, \
		{0xCC, 0xCC, 0x00}, {0xCC, 0xCC, 0x33}, {0xCC, 0xCC, 0x66}, {0xCC, 0xCC, 0x99}, {0xCC, 0xCC, 0xCC}, {0xCC, 0xCC, 0xFF}, \
		{0xCC, 0xFF, 0x00}, {0xCC, 0xFF, 0x33}, {0xCC, 0xFF, 0x66}, {0xCC, 0xFF, 0x99}, {0xCC, 0xFF, 0xCC}, {0xCC, 0xFF, 0xFF}, \
		\
		{0xFF, 0x00, 0x00}, {0xFF, 0x00, 0x33}, {0xFF, 0x00, 0x66}, {0xFF, 0x00, 0x99}, {0xFF, 0x00, 0xCC}, {0xFF, 0x00, 0xFF}, \
		{0xFF, 0x33, 0x00}, {0xFF, 0x33, 0x33}, {0xFF, 0x33, 0x66}, {0xFF, 0x33, 0x99}, {0xFF, 0x33, 0xCC}, {0xFF, 0x33, 0xFF}, \
		{0xFF, 0X66, 0x00}, {0xFF, 0X66, 0x33}, {0xFF, 0X66, 0x66}, {0xFF, 0X66, 0x99}, {0xFF, 0X66, 0xCC}, {0xFF, 0X66, 0xFF}, \
		{0xFF, 0x99, 0x00}, {0xFF, 0x99, 0x33}, {0xFF, 0x99, 0x66}, {0xFF, 0x99, 0x99}, {0xFF, 0x99, 0xCC}, {0xFF, 0x99, 0xFF}, \
		{0xFF, 0xCC, 0x00}, {0xFF, 0xCC, 0x33}, {0xFF, 0xCC, 0x66}, {0xFF, 0xCC, 0x99}, {0xFF, 0xCC, 0xCC}, {0xFF, 0xCC, 0xFF}, \
		{0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0x33}, {0xFF, 0xFF, 0x66}, {0xFF, 0xFF, 0x99}, {0xFF, 0xFF, 0xCC}, {0xFF, 0xFF, 0xFF}, \
	}

#endif // INTERNAL_PALETTE_H
