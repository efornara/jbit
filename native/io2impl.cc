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

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "jbit.h"
#include "devimpl.h"

#include "_io2.h"

#define IO_BASE 0x200

static uint32_t *buffer;
static const int stride = 128;

#include "rom.h"

static RomResource *font = 0;
static const int font_width = 8;
static const int font_height = 14;

static const uint32_t bg_color = 0x0090e0c0;
static const uint32_t console_bg_color = 0x0078c8b4;
static const uint32_t console_fg_color = 0x00000000;

bool io2_opengl = false;

// TODO: check endianness
static inline uint32_t get_color(uint32_t c) {
	if (io2_opengl) {
		uint32_t out = c & 0xff00ff00;
		out |= ((c & 0x000000ff) << 16);
		out |= ((c & 0x00ff0000) >> 16);
		return out;
	}
	return c;
}

static void font_draw(int x, int y, uint32_t bg, uint32_t fg, uint8_t c) {
	const uint8_t *r = &font->get_data()[c * font_height];
	uint32_t *b = &buffer[y * stride + x];
	for (int y = 0; y < font_height; y++) {
		int mask = 0x80;
		for (int x = 0; x < font_width; x++) {
			uint32_t color;
			if (*r & mask)
				color = fg;
			else
				color = bg;
			*b++ = color;
			mask >>= 1;
		}
		b += (stride - font_width);
		r++;
	}
}

class Console {
private:
	uint8_t *buf;
	int cols, rows;
	int ox, oy;
	uint32_t fg, bg;
public:
	Console() : buf(0), cols(0), rows(0)  {}
	~Console() { delete[] buf; }
	void reset(int cols, int rows, int width, int height) {
		this->cols = cols;
		this->rows = rows;
		delete[] buf;
		buf = new uint8_t[cols * rows];
		ox = (width - cols * font_width)  / 2;
		oy = (height - rows * font_height)  / 2;
		memset(buf, ' ', rows * cols);
		fg = get_color(console_fg_color);
		bg = get_color(console_bg_color);
	}
	void put(int address, uint8_t value) {
		buf[address] = value;
	}
	uint8_t get(int address) {
		return buf[address];
	}
	void render() {
		int i = 0, y = oy;
		for (int r = 0; r < rows; r++) {
			int x = ox;
			for (int c = 0; c < cols; c++) {
				font_draw(x, y, bg, fg, buf[i++]);
				x += font_width;
			}
			y += font_height;
		}
	}
};

class IO2Impl : public IO2 {
public:
	static const int FRAME_MIN_WAIT = 10000;
private:
	AddressSpace *m;
	int frameno;
	Random random;
	MicroIOKeybuf keybuf;
	Console console;
	int v_FRMFPS;
	int wait_us;
	int rel_time;
	void render_background() {
		uint32_t bg = get_color(bg_color);
		for (int i = 0; i < width * height; i++)
			buffer[i] = bg;
	}
	void render() {
		render_background();
		console.render();
	}
	void put_FRMDRAW() {
		int fps4 = v_FRMFPS;
		wait_us = (int)(1000000.0 / (fps4 / 4.0));
		if (wait_us < FRAME_MIN_WAIT)
			wait_us = FRAME_MIN_WAIT;
		rel_time = 0;
	}
public:
	// AddressSpace
	int get(int address) {
		address += IO_BASE;
		switch (address) {
		case FRMFPS:
			return v_FRMFPS;
		case RANDOM:
			return random.get();
		default:
			if (address >= KEYBUF && address < KEYBUF + MicroIOKeybuf::KEYBUF_SIZE)
				return keybuf.get(address - KEYBUF);
			else if (address >= CONVIDEO && address < CONVIDEO + MicroIODisplay::CONVIDEO_SIZE)
				return console.get(address - CONVIDEO);
		}
		return 0;
	}
	void put(int address, int value) {
		address += IO_BASE;
		value &= 0xff;
		switch (address) {
		case FRMFPS:
			v_FRMFPS = value;
			break;
		case FRMDRAW:
			put_FRMDRAW();
			break;
		case RANDOM:
			random.put(value);
			break;
		case KEYBUF:
			keybuf.put(address - KEYBUF, value);
			break;
		default:
			if (address >= CONVIDEO && address < CONVIDEO + MicroIODisplay::CONVIDEO_SIZE)
				console.put(address - CONVIDEO, (uint8_t)value);
		}
	}
	// IO
	void set_address_space(AddressSpace *dma) {
		m = dma;
	}
	void reset() {
		random.reset();
		keybuf.reset();
		console.reset(10, 4, width, height);
		v_FRMFPS = 40;
		wait_us = 0;
	}
	// IO2
	const void *get_framebuffer() { return buffer; }
	void keypress(int key) {
		keybuf.enque(key);
	}
	bool update(int dt_us) {
		if (dt_us)
			frameno++;
		if (wait_us) {
			rel_time += dt_us;
			if (rel_time < wait_us)
				return true;
			wait_us = 0;
			rel_time = 0;
		}
		if (dt_us)
			render();
		return false;
	}
	// IO2Impl
	IO2Impl() : frameno(0) {
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
