/*
 * Copyright (C) 2012  Emanuele Fornara
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

// frame.c

#include "defs.h"

#include <stdio.h>

void frame_clear(uint32_t *f, uint32_t bg) 
{
	int i;

	for (i = 0; i < FRAME_WIDTH * FRAME_HEIGHT; i++)
		f[i] = bg;
}

#if (CHAR_WIDTH != 8 || CHAR_HEIGHT != 14)
#error "unsupported CHAR size"
#endif

void frame_putchar(uint32_t *f, int x, int y, int c, uint32_t fg, uint32_t bg)
{
	static char *font = NULL;
	int i, j, mask, i_font, i_f;

	if (font == NULL) {
		int size;
		font = load_res_file("vga14.rom", &size);
		if (size != 14 * 256)
			fatal("invalid font size");
	}
	i_font = (c & 0xFF) * 14;
	i_f = x + y * FRAME_WIDTH;
	for (i = 0; i < 14; i++) {
		mask = 0x80;
		for (j = 0; j < 8; j++) {
			if (font[i_font] & mask)
				f[i_f] = fg;
			else
				f[i_f] = bg;
			mask >>= 1;
			i_f++;
		}
		i_f += FRAME_WIDTH - 8;
		i_font++;
	}
}

void frame_screenshot(uint32_t *f, const char *file_name)
{
	FILE *file;
	int i, x, y;

	if ((file = fopen(file_name, "wb")) == NULL)
		fatal("frame_screenshot failed for '%s'", file_name);
	fprintf(file, "P6\n%d %d\n255\n", FRAME_WIDTH, FRAME_HEIGHT);
	for (y = 0, i = 0; y < FRAME_HEIGHT; y++) {
		for (x = 0; x < FRAME_WIDTH; x++, i++) {
			uint32_t c = f[i];
			fputc((c >> 24) & 0xFF, file);
			fputc((c >> 16) & 0xFF, file);
			fputc((c >>  8) & 0xFF, file);
		}
	}
	fclose(file);
}
