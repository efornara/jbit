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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define H "14"
unsigned char bg[] = { 0x60, 0xe0, 0xc0 };
unsigned char fg[] = { 0x00, 0x00, 0x00 };

char buffer[16 * 1024];
int h;

void loadrom()
{
	FILE *f;
	int n;

	f = fopen("vga" H ".rom", "rb");
	assert(f);
	n = fread(buffer, 1, sizeof(buffer), f);
	assert(n == h * 256);
	fclose(f);
}

void saveppm()
{
	FILE *f;
	int width = 32 * 8, height = 8 * h;
	int x, y, c, cx, cy;
	unsigned char *color;

	f = fopen("vga" H ".ppm", "wb");
	assert(f);
	fprintf(f, "P6\n%d %d\n255\n", width, height);
	for (y = 0; y < height; y++) {
		cy = y % h;
		for (x = 0; x < width; x++) {
			cx = x % 8;
			c = (y / h) * 32 + (x / 8);
			if (buffer[c * h + cy] & (0x80 >> cx))
				color = fg;
			else
				color = bg;
			fwrite(color, 1, 3, f);
		}
	}
	fclose(f);
}

int main(int argc, char *argv[])
{
	h = atoi(H);
	loadrom();
	saveppm();
	return 0;
}
