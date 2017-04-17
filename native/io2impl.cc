/*
 * Copyright (C) 2012-2017  Emanuele Fornara
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

// io2impl.cc

#include <string.h>
#include <stdint.h>

#include "jbit.h"

static uint32_t *buffer;
static const int stride = 128;

#include "rom.h"

static RomResource *font = 0;
static const int font_width = 8;
static const int font_height = 14;

static const uint32_t bg_color = 0x00aaff88;
static const uint32_t fg_color = 0x00000000;

static void font_draw(int x, int y, uint8_t c) {
	const uint8_t *r = &font->get_data()[c * font_height];
	uint32_t *b = &buffer[y * stride + x];
	for (int y = 0; y < font_height; y++) {
		int mask = 0x80;
		for (int x = 0; x < font_width; x++) {
			uint32_t color;
			if (*r & mask)
				color = fg_color;
			else
				color = bg_color;
			*b++ = color;
			mask >>= 1;
		}
		b += (stride - font_width);
		r++;
	}
}

static void render() {
	static int n = 0;
	static uint8_t c = ' ';
	if (++n == 10) {
		font_draw(10, 30, c++);
		if (c > 'z')
			c = ' ';
		n = 0;
	}
}

class IO2Impl : public IO2 {
public:
	// AddressSpace
	void put(int address, int value) {}
	int get(int address) { return 0; }
	// IO
	void set_address_space(AddressSpace *dma) {}
	void reset() {}
	// IO2
	const void *get_framebuffer() { return buffer; }
	void keypress(int key) {}
	void frame() { render(); }
	// IO2Impl
	IO2Impl() {
		buffer = new uint32_t[width * height];
		memset(buffer, 0, width * height * sizeof(uint32_t));
		font = RomResource::load("vga14.rom");
	}
	~IO2Impl() {
		RomResource::cleanup();
		delete[] buffer;
		buffer = 0;
	}
};

IO2 *new_IO2() {
	return new IO2Impl();
}
