/*
 * Copyright (C) 2014  Emanuele Fornara
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_JS)

#include <string.h>
#include <stdint.h>

#include "nano.h"

uint8_t lcd_bitmap[LCD_BITMAP_SIZE];

uint8_t *lcd_get_bitmap() {
	return lcd_bitmap;
}

static int col;
static int row;

static void clear() {
	memset(lcd_bitmap, 0, sizeof(lcd_bitmap));
}

void lcd_init() {
	clear();
	col = 0;
	row = 0;
}

void lcd_write(unsigned char dc, unsigned char data) {
	int i;

	switch (dc) {
	case LCD_COMMAND:
		switch (data) {
		case LCD_HWSIM_CMD_CLEAR:
			clear();
			break;
		default:
			if (data & 0x80) {
				int c = data & 0x7f;
				if (c < LCD_WIDTH)
					col = c;
			} else if (data & 0x40) {
				int r = data & 0x0f;
				if (r < LCD_ROWS)
					row = r;
			}
		}
		break; 
	case LCD_DATA:
		for (i = 0; i < 8; i++) {
			int pixel = data & 0x1;
			lcd_bitmap[(col * 8) + (LCD_WIDTH * 8 * row) + i] = pixel;
			data >>= 1;
		}
		col++;
		if (col == LCD_WIDTH) {
			col = 0;
			row++;
			if (row == LCD_ROWS)
				row = 0;
		}
		break; 
	}
}

unsigned short keypad_state;

void keypad_init() {
	keypad_state = 0;
}

void keypad_scan() {
}

void keypad_update(int key_down, int value) {
	unsigned short mask = 0;
	switch (value) {
	case 'a': case 'b': case 'c':
	case 'A': case 'B': case 'C':
		value = '2';
		break;
	case 'd': case 'e': case 'f':
	case 'D': case 'E': case 'F':
		value = '3';
		break;
	case 'g': case 'h': case 'i':
	case 'G': case 'H': case 'I':
		value = '4';
		break;
	case 'j': case 'k': case 'l':
	case 'J': case 'K': case 'L':
		value = '5';
		break;
	case 'm': case 'n': case 'o':
	case 'M': case 'N': case 'O':
		value = '6';
		break;
	case 'p': case 'q': case 'r': case 's':
	case 'P': case 'Q': case 'R': case 'S':
		value = '7';
		break;
	case 't': case 'u': case 'v':
	case 'T': case 'U': case 'V':
		value = '8';
		break;
	case 'w': case 'x': case 'y': case 'z':
	case 'W': case 'X': case 'Y': case 'Z':
		value = '9';
		break;
	}
	switch (value) {
	case '*':
		mask = KEYPAD_MASK_STAR;
		break;
	case '#':
		mask = KEYPAD_MASK_HASH;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		mask = (1 << (value - '0'));
		break;
	default:
		return;
	}
	if (key_down)
		keypad_state |= mask;
	else
		keypad_state &= ~mask;
}

#endif
