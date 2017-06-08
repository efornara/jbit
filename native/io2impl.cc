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
#include <time.h>

#include "jbit.h"
#include "devimpl.h"

#include "_io2.h"

#define IO_BASE 0x200

static const int stride = 128;

#include "rom.h"

static RomResource *font = 0;
static const int font_width = 8;
static const int font_height = 14;

#include "blt32.h"
typedef uint32_t color_t;
#define get_color_rgba blt32_get_color_rgba

static color_t *buffer;

color_t get_color_rgb(uint32_t c) {
	return get_color_rgba(c | 0xff000000);
}

static void font_draw(int x, int y, color_t bg, color_t fg, uint8_t c) {
	const uint8_t *r = &font->get_data()[c * font_height];
	color_t *b = &buffer[y * stride + x];
	for (int y = 0; y < font_height; y++) {
		int mask = 0x80;
		for (int x = 0; x < font_width; x++) {
			color_t color;
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

class Request {
public:
	static const int REQDAT_SIZE = 32;
private:
	Buffer buf;
	uint8_t v_REQDAT[16];
public:
	Request() { reset(); }
	void rewind() {
		buf.reset();
	}
	void reset() {
		rewind();
		memset(v_REQDAT, 0, sizeof(v_REQDAT));
	}
	uint8_t get(uint16_t address) {
		if (address >= REQDAT && address < REQDAT + REQDAT_SIZE)
			return v_REQDAT[address - REQDAT];
		return 0;
	}
	void put(uint16_t address, uint8_t value) {
		if (address == REQPUT)
			buf.append_char(value);
		else if (address >= REQDAT && address < REQDAT + REQDAT_SIZE)
			v_REQDAT[address - REQDAT] = value;
	}
	int n() const {
		return buf.get_length();
	}
	uint8_t id() const {
		return buf.get_length() ? (uint8_t)buf.get_data()[0] : 0;
	}
	uint8_t get_uint8(int pos) const {
		return (uint8_t)buf.get_data()[pos];
	}
	void put_uint8(int pos, uint8_t value) {
		v_REQDAT[pos] = value;
	}
	void put_uint16(int pos, uint16_t value) {
		v_REQDAT[pos] = value & 0xff;
		v_REQDAT[pos + 1] = value >> 8;
	}
	void put_uint64(int pos, uint64_t value) {
		for (int i = 0; i < 8; i++) {
			v_REQDAT[pos++] = value & 0xff;
			value >>= 8;
		}
	}
};

// adapted from Vice (changes: White has been made pure)
static const uint32_t standardPalette[] = {
	0x000000, // Black
	0xFFFFFF, // White
	0xBE1A24, // Red
	0x30E6C6, // Cyan
	0xB41AE2, // Purple
	0x1FD21E, // Green
	0x211BAE, // Blue
	0xDFF60A, // Yellow
	0xB84104, // Orange
	0x6A3304, // Brown
	0xFE4A57, // Light Red
	0x424540, // Dark Gray
	0x70746F, // Medium Gray
	0x59FE59, // Light Green
	0x5F53FE, // Light Blue
	0xA4A7A2, // Light Gray
};

static const uint32_t microioPalette[] = {
	0x000000, // Foreground
	0x78c8b4, // Background
	0x90e0c0, // Border
	0x000000, // Unused
};

class Palette {
private:
	color_t pal[256];
	int mask;
public:
	Palette() { reset(); }
	void reset() { set_standard(); } // TODO: extend JB format
	void set_standard() {
		for (int i = 0; i < 16; i++)
			pal[i] = get_color_rgb(standardPalette[i]);
		mask = 0x0f;
	}
	void set_microio() {
		for (int i = 0; i < 4; i++)
			pal[i] = get_color_rgb(microioPalette[i]);
		mask = 0x03;
	}
	color_t operator[](uint8_t id) const {
		return pal[id & mask];
	}
	bool req_SETPAL(Request &req) {
		int n = req.n() - 1;
		if (n == 0) {
			reset();
			return true;
		}
		if (n % 3 != 0)
			return false;
		n /= 3;
		if (n > 256)
			return false;
		for (mask = 0x80; mask > 1 && mask >= n; mask >>= 1)
			;
		mask <<= 1;
		mask--;
		for (int i = 0, j = 1; i < n; i++) {
			pal[i] = get_color_rgb(
				((uint32_t)req.get_uint8(j) << 16) |
				((uint32_t)req.get_uint8(j + 1) << 8) |
				((uint32_t)req.get_uint8(j + 2))
			);
			j += 3;
		}
		return true;
	}
};

class Console {
private:
	struct Cell {
		uint8_t chr;
		uint8_t fg;
		uint8_t bg;
		Cell() : chr(' '), fg(COLOR_BLACK), bg(COLOR_WHITE) {}
	};
	enum RWOp {
		Read,
		Write
	};
	const Palette &pal;
	const int width, height;
	int ox, oy;
	Cell *buf;
	uint8_t v_CONCOLS, v_CONROWS, v_CONCX, v_CONCY;
	uint8_t rw(RWOp op, uint16_t reg, uint8_t value, int cx, int cy) {
		if (cx >= v_CONCOLS || cy >= v_CONROWS)
			return 0;
		Cell *cell = &buf[cx + cy * v_CONCOLS];
		if (op == Write) {
			switch (reg) {
			case CONCCHR:
				cell->chr = value;
				break;
			case CONCFG:
				cell->fg = value;
				break;
			case CONCBG:
				cell->bg = value;
				break;
			}
		} else {
			switch (reg) {
			case CONCCHR:
				return cell->chr;
			case CONCFG:
				return cell->fg;
			case CONCBG:
				return cell->bg;
			}
		}
		return 0;
	}
public:
	Console(const Palette &pal_, int width_, int height_)
	  : pal(pal_), width(width_), height(height_), buf(0) {}
	~Console() {
		delete[] buf;
	}
	void reset() {
		v_CONCOLS = 10;
		v_CONROWS = 4;
		v_CONCX = 0;
		v_CONCY = 0;
		resize();
	}
	void resize() {
		delete[] buf;
		const int size = v_CONCOLS * v_CONROWS;
		buf = new Cell[size];
		ox = (width - v_CONCOLS * font_width)  / 2;
		oy = (height - v_CONROWS * font_height)  / 2;
	}
	void put(uint16_t address, uint8_t value) {
		switch (address) {
		case CONCOLS: {
			int max = width / font_width;
			if (!value || value > max)
				value = (uint8_t)max;
			if (value != v_CONCOLS) {
				v_CONCOLS = value;
				resize();
			}
			} break;
		case CONROWS: {
			int max = height / font_height;
			if (!value || value > max)
				value = (uint8_t)max;
			if (value != v_CONROWS) {
				v_CONROWS = value;
				resize();
			}
			} break;
		case CONCX:
			v_CONCX = value;
			break;
		case CONCY:
			v_CONCY = value;
			break;
		case CONCCHR:
		case CONCFG:
		case CONCBG:
			rw(Write, address, value, v_CONCX, v_CONCY);
			break;
		default: {
			const int offset = address - CONVIDEO;
			rw(Write, CONCCHR, value, offset % 10, offset / 10);
			} break;
		}
	}
	uint8_t get(uint16_t address) {
		switch (address) {
		case CONCOLS:
			return v_CONCOLS;
		case CONROWS:
			return v_CONROWS;
		case CONCX:
			return v_CONCX;
		case CONCY:
			return v_CONCY;
		case CONCCHR:
		case CONCFG:
		case CONCBG:
			return rw(Read, address, 0, v_CONCX, v_CONCY);
		default: {
			const int offset = address - CONVIDEO;
			return rw(Read, CONCCHR, 0, offset % 10, offset / 10);
			} break;
		}
	}
	void render() {
		int i = 0, y = oy;
		for (int r = 0; r < v_CONROWS; r++) {
			int x = ox;
			for (int c = 0; c < v_CONCOLS; c++) {
				const Cell *cell = &buf[i++];
				if (cell->chr)
					font_draw(x, y, pal[cell->bg], pal[cell->fg], cell->chr);
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
	uint64_t systime;
	Request req;
	uint8_t v_REQRES;
	uint8_t v_REQPTRHI;
	uint8_t v_ENABLE;
	Random random;
	MicroIOKeybuf keybuf;
	Palette palette;
	Console console;
	color_t bgcol;
	int v_FRMFPS;
	int wait_us;
	int rel_time;
	void render_background() {
		for (int i = 0; i < width * height; i++)
			buffer[i] = bgcol;
	}
	void render() {
		if (v_ENABLE & ENABLE_BGCOL)
			render_background();
		if (v_ENABLE & ENABLE_CONSOLE)
			console.render();
	}
	uint8_t m_get_uint8(uint16_t addr) {
		return (uint8_t)m->get(addr);
	}
	uint16_t m_get_uint16(uint16_t addr) {
		return (uint16_t)((m_get_uint8(addr + 1) << 8) | m_get_uint8(addr));
	}
	bool req_TIME() {
		bool is_abs = false;
		int fract = TIME_1000;
		switch (req.n()) {
		case 1:
			break;
		case 3:
			fract = req.get_uint8(2);
		case 2:
			is_abs = req.get_uint8(1) == TIME_ABS;
			break;
		default:
			return false;
		}
		uint64_t t = 0;
		if (is_abs)
			t = time(0) * (uint64_t)1000;
		else
			t = systime / 1000;
		switch (fract) {
		case TIME_1:
			t /= 1000;
			break;
		case TIME_10:
			t /= 100;
			break;
		case TIME_100:
			t /= 10;
			break;
		}
		req.put_uint64(0, t);
		return true;
	}
	bool req_DPYINFO() {
		if (req.n() != 1)
			return false;
		req.put_uint16(DPYINFO_WIDTH, width);
		req.put_uint16(DPYINFO_HEIGHT, height);
		req.put_uint8(DPYINFO_COLORDEPTH, 24);
		req.put_uint8(DPYINFO_ALPHADEPTH, 8);
		req.put_uint8(DPYINFO_FLAGS, DPYINFO_FLAGS_ISCOLOR | DPYINFO_FLAGS_ISMIDP2);
		return true;
	}
	bool req_SETBGCOL() {
		switch (req.n()) {
		case 2:
			bgcol = palette[req.get_uint8(1)];
			return true;
		case 4:
			bgcol = get_color_rgb(
			  ((uint32_t)req.get_uint8(1) << 16) |
			  ((uint32_t)req.get_uint8(2) << 8) |
			  ((uint32_t)req.get_uint8(3))
			);
			return true;
		default:
			return false;
		}
	}
	void request() {
		bool res = false;
		switch (req.id()) {
		case REQ_TIME:
			res = req_TIME();
			break;
		case REQ_DPYINFO:
			res = req_DPYINFO();
			break;
		case REQ_SETBGCOL:
			res = req_SETBGCOL();
			break;
		case REQ_SETPAL:
			res = palette.req_SETPAL(req);
			break;
		}
		v_REQRES = res ? 0 : 255;
		req.rewind();
	}
	void put_REQPTRLO(uint8_t value) {
		uint16_t addr = (v_REQPTRHI << 8) | value;
		uint16_t len = m_get_uint16(addr);
		addr += 2;
		req.rewind();
		for (int i = 0; i < len; i++)
			req.put(REQPUT, m_get_uint8(addr++));
		if (!len)
			req.put(REQPUT, 0);
		request();
	}
	void put_FRMDRAW() {
		if (!v_FRMFPS)
			return;
		int fps4 = v_FRMFPS;
		wait_us = 4000000 / fps4;
		if (wait_us < FRAME_MIN_WAIT)
			wait_us = FRAME_MIN_WAIT;
		rel_time = 0;
	}
public:
	// AddressSpace
	int get(int address_) {
		uint16_t address = (uint16_t)(address_ + IO_BASE);
		switch (address) {
		case REQRES:
			return v_REQRES;
		case REQPTRHI:
			return v_REQPTRHI;
		case ENABLE:
			return v_ENABLE;
		case FRMFPS:
			return v_FRMFPS;
		case RANDOM:
			return random.get();
		case CONCOLS:
		case CONROWS:
		case CONCX:
		case CONCY:
		case CONCCHR:
		case CONCFG:
		case CONCBG:
			return console.get(address);
		default:
			if (address >= KEYBUF && address < KEYBUF + MicroIOKeybuf::KEYBUF_SIZE)
				return keybuf.get(address - KEYBUF);
			else if (address >= CONVIDEO && address < CONVIDEO + MicroIODisplay::CONVIDEO_SIZE)
				return console.get(address);
			else if (address >= REQDAT && address < REQDAT + Request::REQDAT_SIZE)
				return req.get(address);
		}
		return 0;
	}
	void put(int address_, int value_) {
		uint16_t address = (uint16_t)(address_ + IO_BASE);
		uint8_t value = (uint8_t)value_;
		switch (address) {
		case REQEND:
			request();
			break;
		case REQPTRLO:
			put_REQPTRLO(value);
			break;
		case REQPTRHI:
			v_REQPTRHI = value;
			break;
		case REQPUT:
			req.put(address, value);
			break;
		case ENABLE:
			v_ENABLE = value;
			break;
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
		case CONCOLS:
		case CONROWS:
		case CONCX:
		case CONCY:
		case CONCCHR:
		case CONCFG:
		case CONCBG:
			console.put(address, value);
			return;
		default:
			if (address >= CONVIDEO && address < CONVIDEO + MicroIODisplay::CONVIDEO_SIZE)
				console.put(address, value);
			else if (address >= REQDAT && address < REQDAT + Request::REQDAT_SIZE)
				return req.put(address, value);
		}
	}
	// IO
	void set_address_space(AddressSpace *dma) {
		m = dma;
	}
	void reset() {
		systime = 0;
		req.reset();
		v_REQRES = 0;
		v_REQPTRHI = 0;
		v_ENABLE = ENABLE_BGCOL | ENABLE_CONSOLE;
		random.reset();
		keybuf.reset();
		palette.reset();
		console.reset();
		bgcol = get_color_rgb(palette[COLOR_WHITE]);
		v_FRMFPS = 40;
		wait_us = 0;
	}
	// IO2
	const void *get_framebuffer() { return buffer; }
	void keypress(int key) {
		keybuf.enque(key);
	}
	bool update(int dt_us) {
		if (dt_us) {
			systime += dt_us;
			frameno++;
		}
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
	IO2Impl() : frameno(0), console(palette, width, height) {
		buffer = new color_t[width * height];
		memset(buffer, 0, width * height * sizeof(color_t));
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
