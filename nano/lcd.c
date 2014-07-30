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

#include "nano.h"

/*
	Hard to trace the original source of this font to give proper attribution.
	The internet is full of copies that are pretty much the same.
	INAL, but it is believed that font bitmaps are not copyrightable anyway.
	If you know the history of this font, please leave a comment on github.

	Emanuele
*/
static const unsigned char fixed_font[] PROGMEM = {
	/* ASCII */
	0x00, 0x00, 0x00, 0x00, 0x00, /* (space) */
	0x00, 0x00, 0x5F, 0x00, 0x00, /* ! */
	0x00, 0x07, 0x00, 0x07, 0x00, /* " */
	0x14, 0x7F, 0x14, 0x7F, 0x14, /* # */
	0x24, 0x2A, 0x7F, 0x2A, 0x12, /* $ */
	0x23, 0x13, 0x08, 0x64, 0x62, /* % */
	0x36, 0x49, 0x55, 0x22, 0x50, /* & */
	0x00, 0x05, 0x03, 0x00, 0x00, /* ' */
	0x00, 0x1C, 0x22, 0x41, 0x00, /* ( */
	0x00, 0x41, 0x22, 0x1C, 0x00, /* ) */
	0x08, 0x2A, 0x1C, 0x2A, 0x08, /* * */
	0x08, 0x08, 0x3E, 0x08, 0x08, /* + */
	0x00, 0x50, 0x30, 0x00, 0x00, /* , */
	0x08, 0x08, 0x08, 0x08, 0x08, /* - */
	0x00, 0x60, 0x60, 0x00, 0x00, /* . */
	0x20, 0x10, 0x08, 0x04, 0x02, /* / */
	0x3E, 0x51, 0x49, 0x45, 0x3E, /* 0 */
	0x00, 0x42, 0x7F, 0x40, 0x00, /* 1 */
	0x42, 0x61, 0x51, 0x49, 0x46, /* 2 */
	0x21, 0x41, 0x45, 0x4B, 0x31, /* 3 */
	0x18, 0x14, 0x12, 0x7F, 0x10, /* 4 */
	0x27, 0x45, 0x45, 0x45, 0x39, /* 5 */
	0x3C, 0x4A, 0x49, 0x49, 0x30, /* 6 */
	0x01, 0x71, 0x09, 0x05, 0x03, /* 7 */
	0x36, 0x49, 0x49, 0x49, 0x36, /* 8 */
	0x06, 0x49, 0x49, 0x29, 0x1E, /* 9 */
	0x00, 0x36, 0x36, 0x00, 0x00, /* : */
	0x00, 0x56, 0x36, 0x00, 0x00, /* ; */
	0x00, 0x08, 0x14, 0x22, 0x41, /* < */
	0x14, 0x14, 0x14, 0x14, 0x14, /* = */
	0x41, 0x22, 0x14, 0x08, 0x00, /* > */
	0x02, 0x01, 0x51, 0x09, 0x06, /* ? */
	0x32, 0x49, 0x79, 0x41, 0x3E, /* @ */
	0x7E, 0x11, 0x11, 0x11, 0x7E, /* A */
	0x7F, 0x49, 0x49, 0x49, 0x36, /* B */
	0x3E, 0x41, 0x41, 0x41, 0x22, /* C */
	0x7F, 0x41, 0x41, 0x22, 0x1C, /* D */
	0x7F, 0x49, 0x49, 0x49, 0x41, /* E */
	0x7F, 0x09, 0x09, 0x01, 0x01, /* F */
	0x3E, 0x41, 0x41, 0x51, 0x32, /* G */
	0x7F, 0x08, 0x08, 0x08, 0x7F, /* H */
	0x00, 0x41, 0x7F, 0x41, 0x00, /* I */
	0x20, 0x40, 0x41, 0x3F, 0x01, /* J */
	0x7F, 0x08, 0x14, 0x22, 0x41, /* K */
	0x7F, 0x40, 0x40, 0x40, 0x40, /* L */
	0x7F, 0x02, 0x04, 0x02, 0x7F, /* M */
	0x7F, 0x04, 0x08, 0x10, 0x7F, /* N */
	0x3E, 0x41, 0x41, 0x41, 0x3E, /* O */
	0x7F, 0x09, 0x09, 0x09, 0x06, /* P */
	0x3E, 0x41, 0x51, 0x21, 0x5E, /* Q */
	0x7F, 0x09, 0x19, 0x29, 0x46, /* R */
	0x46, 0x49, 0x49, 0x49, 0x31, /* S */
	0x01, 0x01, 0x7F, 0x01, 0x01, /* T */
	0x3F, 0x40, 0x40, 0x40, 0x3F, /* U */
	0x1F, 0x20, 0x40, 0x20, 0x1F, /* V */
	0x7F, 0x20, 0x18, 0x20, 0x7F, /* W */
	0x63, 0x14, 0x08, 0x14, 0x63, /* X */
	0x03, 0x04, 0x78, 0x04, 0x03, /* Y */
	0x61, 0x51, 0x49, 0x45, 0x43, /* Z */
	0x00, 0x00, 0x7F, 0x41, 0x41, /* [ */
	0x02, 0x04, 0x08, 0x10, 0x20, /* "\" */
	0x41, 0x41, 0x7F, 0x00, 0x00, /* ] */
	0x04, 0x02, 0x01, 0x02, 0x04, /* ^ */
	0x40, 0x40, 0x40, 0x40, 0x40, /* _ */
	0x00, 0x01, 0x02, 0x04, 0x00, /* ` */
	0x20, 0x54, 0x54, 0x54, 0x78, /* a */
	0x7F, 0x48, 0x44, 0x44, 0x38, /* b */
	0x38, 0x44, 0x44, 0x44, 0x20, /* c */
	0x38, 0x44, 0x44, 0x48, 0x7F, /* d */
	0x38, 0x54, 0x54, 0x54, 0x18, /* e */
	0x08, 0x7E, 0x09, 0x01, 0x02, /* f */
	0x08, 0x14, 0x54, 0x54, 0x3C, /* g */
	0x7F, 0x08, 0x04, 0x04, 0x78, /* h */
	0x00, 0x44, 0x7D, 0x40, 0x00, /* i */
	0x20, 0x40, 0x44, 0x3D, 0x00, /* j */
	0x00, 0x7F, 0x10, 0x28, 0x44, /* k */
	0x00, 0x41, 0x7F, 0x40, 0x00, /* l */
	0x7C, 0x04, 0x18, 0x04, 0x78, /* m */
	0x7C, 0x08, 0x04, 0x04, 0x78, /* n */
	0x38, 0x44, 0x44, 0x44, 0x38, /* o */
	0x7C, 0x14, 0x14, 0x14, 0x08, /* p */
	0x08, 0x14, 0x14, 0x18, 0x7C, /* q */
	0x7C, 0x08, 0x04, 0x04, 0x08, /* r */
	0x48, 0x54, 0x54, 0x54, 0x20, /* s */
	0x04, 0x3F, 0x44, 0x40, 0x20, /* t */
	0x3C, 0x40, 0x40, 0x20, 0x7C, /* u */
	0x1C, 0x20, 0x40, 0x20, 0x1C, /* v */
	0x3C, 0x40, 0x30, 0x40, 0x3C, /* w */
	0x44, 0x28, 0x10, 0x28, 0x44, /* x */
	0x0C, 0x50, 0x50, 0x50, 0x3C, /* y */
	0x44, 0x64, 0x54, 0x4C, 0x44, /* z */
	0x00, 0x08, 0x36, 0x41, 0x00, /* { */
	0x00, 0x00, 0x7F, 0x00, 0x00, /* | */
	0x00, 0x41, 0x36, 0x08, 0x00, /* } */
	0x08, 0x08, 0x2A, 0x1C, 0x08, /* -> */
	0x08, 0x1C, 0x2A, 0x08, 0x08, /* <- */

	/* JBIT CUSTOM */
	0xFF, 0xA3, 0xA3, 0xA3, 0xFF, /* undefined */
};

void lcd_clear() {
#ifdef LCD_HWSIM
	lcd_write(LCD_COMMAND, LCD_HWSIM_CMD_CLEAR);
#else
	int i;
	for (i = 0; i < LCD_WIDTH * LCD_HEIGHT / 8; i++)
		lcd_write(LCD_DATA, 0);
#endif
}

void lcd_goto(int col, int row) {
	lcd_write(LCD_COMMAND, 0x80 | col);
	lcd_write(LCD_COMMAND, 0x40 | row);
}

void lcd_home() {
	lcd_goto(0, 0);
}

void lcd_char(char c) {
	int i, p = c & 0xff;
	if (p < 32 && p > 127)
		p = 128;
	p -= 32;
	p *= 5;
	for (i = 0; i < 5; i++)
		lcd_write(LCD_DATA, pgm_read_byte(&(fixed_font[p++])));
	lcd_write(LCD_DATA, 0);
}
