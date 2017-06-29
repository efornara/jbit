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

#include "io2impl.h"

static RomResource *font = 0;

color_t *buffer;

void font_draw(int x, int y, color_t bg, color_t fg, uint8_t c) {
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

Obj **Ref::objs = 0;
unsigned Ref::size = 0;


unsigned Ref::create(ObjType type) {
	unsigned id;
	for (id = 0; id < Ref::size; id++)
		if (!Ref::objs[id])
			break;
	if (id == Ref::size)
		return 0;
	Obj *obj = 0;
	switch (type) {
	case OT_Image:
		obj = new Image();
		break;
	}
	if (!obj)
		return 0;
	Ref::objs[id] = obj;
	return id + 1;
}

class IO2Impl : public IO2 {
public:
	static const unsigned FRAME_MIN_WAIT = 10000;
private:
	bool microio;
	AddressSpace *m;
	int frameno;
	uint64_t systime;
	Request req;
	u8 v_REQRES;
	u8 v_REQPTRHI;
	u8 v_ENABLE;
	Random random;
	MicroIOKeybuf keybuf;
	Palette palette;
	Console console;
	color_t bgcol;
	Images images;
	Layers layers;
	int v_FRMFPS;
	u32f wait_us;
	u32f rel_time;
	void render_background() {
		for (int i = 0; i < width * height; i++)
			buffer[i] = bgcol;
	}
	void render() {
		if (v_ENABLE & ENABLE_BGCOL)
			render_background();
		if (v_ENABLE & ENABLE_BGIMG)
			images.render();
		if (v_ENABLE & ENABLE_CONSOLE)
			console.render(microio);
		if (v_ENABLE & ENABLE_LAYERS)
			layers.render();
	}
	u8 m_get_uint8(u16 addr) {
		return (u8)m->get(addr);
	}
	u16 m_get_uint16(u16 addr) {
		return (u16)((m_get_uint8(addr + 1) << 8) | m_get_uint8(addr));
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
		u64 t = 0;
		if (is_abs)
			t = time(0) * (u64)1000;
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
	bool req_LOADROM() {
		int n = req.n();
		if (n < 4)
			return false;
		const u16f addr = req.get_uint16(1);
		if (addr < 0x300)
			return false;
		int pos;
		const char *s = req.get_string0(3, &pos);
		if (!s)
			return false;
		u16f offset = 0;
		u16f size = 0;
		if (pos <= n - 2) {
			offset = req.get_uint16(pos);
			pos += 2;
		}
		if (pos <= n - 2) {
			size = req.get_uint16(pos);
			pos += 2;
		}
		if (pos != n)
			return false;
		RomResource *rom = RomResource::get(s);
		if (!rom)
			return false;
		const u8 *data = (const u8 *)rom->get_data();
		const u16f rom_size = rom->get_size();
		if (!size || rom_size < (u32f)(offset + size))
			size = rom_size - offset;
		for (u16f i = addr, j = offset, k = 0; i < 0xff00 && k < size; k++)
			m->put(i++, data[j++]);
		RomResource::release(rom);
		return true;
	}
	bool req_DPYINFO() {
		if (req.n() != 1)
			return false;
		req.put_uint16(DPYINFO_WIDTH, width);
		req.put_uint16(DPYINFO_HEIGHT, height);
		req.put_uint8(DPYINFO_COLORDEPTH, COLORDEPTH);
		req.put_uint8(DPYINFO_ALPHADEPTH, ALPHADEPTH);
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
			  ((u32)req.get_uint8(1) << 16) |
			  ((u32)req.get_uint8(2) << 8) |
			  ((u32)req.get_uint8(3))
			);
			return true;
		default:
			return false;
		}
	}
	bool request_is_streaming() {
		switch (req.id()) {
		case REQ_SETPAL:
		case REQ_IPNGGEN:
		case REQ_IRAWRGBA:
		case REQ_LTLPUT:
			return true;
		default:
			return false;
		}
	}
	bool request_stream(u32f pos, u8 value) {
		switch (req.id()) {
		case REQ_SETPAL:
			return palette.req_SETPAL(req, pos, value);
		case REQ_IPNGGEN:
			return images.req_IPNGGEN(req, pos, value);
		case REQ_IRAWRGBA:
			return images.req_IRAWRGBA(req, pos, value);
		}
		return false;
	}
	void request_put(u8 value) {
		req.append(value);
		if (request_is_streaming())
			request_stream(req.n_streaming() - 1, value);
	}
	void request_end() {
		bool res = false;
		const bool is_streaming = request_is_streaming();
		if (req.n_streaming() > Request::SHORT_REQ_SIZE && !is_streaming)
			goto done;
		switch (req.id()) {
		case REQ_TIME:
			res = req_TIME();
			break;
		case REQ_LOADROM:
			res = req_LOADROM();
			break;
		case REQ_DPYINFO:
			res = req_DPYINFO();
			break;
		case REQ_SETBGCOL:
			res = req_SETBGCOL();
			break;
		case REQ_SETBGIMG:
			res = images.req_SETBGIMG(req);
			break;
		case REQ_IDESTROY:
			res = images.req_IDESTROY(req);
			break;
		case REQ_IDIM:
			res = images.req_IDIM(req);
			break;
		case REQ_IINFO:
			res = images.req_IINFO(req);
			break;
		case REQ_ILOAD:
			res = images.req_ILOAD(req);
			break;
		case REQ_GAMESET:
			res = layers.req_GAMESET(req);
			if (res)
				v_ENABLE = ENABLE_BGCOL | ENABLE_LAYERS;
			break;
		default:
			if (is_streaming)
				res = request_stream(Request::END, 0);
			break;
		}
done:
		v_REQRES = res ? 0 : 255;
		req.rewind();
	}
	void put_REQPTRLO(u8 value) {
		u16 addr = (v_REQPTRHI << 8) | value;
		u16 len = m_get_uint16(addr);
		addr += 2;
		req.rewind();
		for (int i = 0; i < len; i++)
			request_put(m_get_uint8(addr++));
		if (!len)
			request_put(0);
		request_end();
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
		u16 address = (u16)(address_ + IO_BASE);
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
		case TCOLLO:
		case TCOLHI:
		case TROWLO:
		case TROWHI:
		case TCELL:
			return layers.get(address);
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
		u16 address = (u16)(address_ + IO_BASE);
		u8 value = (u8)value_;
		switch (address) {
		case REQEND:
			request_end();
			break;
		case REQPTRLO:
			put_REQPTRLO(value);
			break;
		case REQPTRHI:
			v_REQPTRHI = value;
			break;
		case REQPUT:
			request_put(value);
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
		case TCOLLO:
		case TCOLHI:
		case TROWLO:
		case TROWHI:
		case TCELL:
			layers.put(address, value);
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
		palette.reset(microio);
		console.reset();
		bgcol = palette[microio ? COLOR_MICROIO_BORDER : COLOR_WHITE];
		images.reset();
		layers.reset();
		v_FRMFPS = 40;
		wait_us = 0;
	}
	// IO2
	void set_microio(bool microio_) { microio = microio_; }
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
	IO2Impl() : microio(false), frameno(0),
	  console(palette, width, height), images(palette, width, height),
	  layers(palette, images, width, height) {
		Ref::init(100); // TODO: profile
		buffer = new color_t[width * height];
		memset(buffer, 0, width * height * sizeof(color_t));
		font = RomResource::get("vga14.rom");
	}
	~IO2Impl() {
		RomResource::release(font);
		delete[] buffer;
		buffer = 0;
		images.reset();
		layers.reset();
		Ref::cleanup();
	}
};

IO2 *new_IO2() {
	return new IO2Impl();
}
