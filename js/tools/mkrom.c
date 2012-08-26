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

/* search for kbd_1.15.1.orig.tar.gz , then look in data/partialfonts */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define H "14"

char buffer[16 * 1024];
FILE *rom;
int h;

void dump(int chars)
{
	int i, j, k, z = 0;
	for (i = 0; i < chars; i++) {
		for (j = 0; j < h; j++, z++) {
			int mask = 0x80;
			for (k = 0; k < 8; k++) {
				printf("%c", mask & buffer[z] ? 'X' : '.');
				mask >>= 1;
			}
			printf("\n");
		}
		printf("\n");
	}
}

void copy(const char *file_name, int chars)
{
	FILE *f;
	int n;

	f = fopen(file_name, "rb");
	assert(f);
	n = fread(buffer, 1, 32, f);
	assert(n == 32);
	n = fread(buffer, 1, sizeof(buffer), f);
	fwrite(buffer, 1, chars * h, rom);
	fclose(f);
}

void zero(int chars)
{
	memset(buffer, 0, chars * h);
	fwrite(buffer, 1, chars * h, rom);
}

void charart()
{
	int c, y, z = 0;

	for (c = 0; c < 16; c++) {
		for (y = 0; y < (h - 2) / 2; y++)
			buffer[z++] = c & 0x01 ? 0x18 : 0x00;
		for (; y < 2 + (h - 2) / 2; y++)
			buffer[z++] = 0x18
			  | (c & 0x02 ? 0xe0 : 0x00)
			  | (c & 0x04 ? 0x07 : 0x00);
		for (; y < h; y++)
			buffer[z++] = c & 0x08 ? 0x18 : 0x00;
	}
	fwrite(buffer, 1, 16 * h, rom);
}

int main(int argc, char *argv[])
{
	h = atoi(H);
	rom = fopen("vga" H ".rom", "wb");
	assert(rom);
	zero(32);
	copy("ascii.20-7f." H, 96);
	charart();
	zero(16);
	copy("8859-1.a0-ff." H, 96);
	return 0;
}
