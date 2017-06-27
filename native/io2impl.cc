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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint_fast8_t u8f;
typedef uint_fast16_t u16f;
typedef uint_fast32_t u32f;
typedef uint_fast64_t u64f;

#include "jbit.h"
#include "devimpl.h"

#include "_io2.h"

#define IO_BASE 0x200

static const int stride = 128;

#include "rom.h"

static RomResource *font = 0;
static const int font_width = 8;
static const int font_height = 14;

#ifdef __DOS__
#include "blt08.h"
typedef uint8_t color_t;
#define get_color_rgba blt08_get_color_rgba
#define is_color_opaque blt08_is_color_opaque
#define mix_color blt08_mix_color
#define COLORDEPTH 7
#define ALPHADEPTH 1
#else
#include "blt32.h"
typedef uint32_t color_t;
#define get_color_rgba blt32_get_color_rgba
#define is_color_opaque blt32_is_color_opaque
#define mix_color blt32_mix_color
#define COLORDEPTH 24
#define ALPHADEPTH 8
#endif

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
	static const u32f END = 0xffffffff;
	static const unsigned SHORT_REQ_SIZE = 255;
	static const unsigned REQDAT_SIZE = 32;
private:
	u8 m[SHORT_REQ_SIZE];
	u8 v_REQDAT[REQDAT_SIZE];
	u32f length;
public:
	Buffer buf;
	Request() : buf(16 * 4) { reset(); }
	void rewind() {
		length = 0;
	}
	void reset() {
		rewind();
		memset(v_REQDAT, 0, sizeof(v_REQDAT));
	}
	u8 get(u16 address) {
		if (address >= REQDAT && address < REQDAT + REQDAT_SIZE)
			return v_REQDAT[address - REQDAT];
		return 0;
	}
	void put(u16 address, u8 value) {
		if (address >= REQDAT && address < REQDAT + REQDAT_SIZE)
			v_REQDAT[address - REQDAT] = value;
	}
	void append(u8 value) {
		if (length >= SHORT_REQ_SIZE)
			length++;
		else
			m[length++] = value;
	}
	void append_uint16(u16 value) {
		append(value & 0xff);
		append(value >> 8);
	}
	int n() const {
		return (int)length;
	}
	u32f n_streaming() {
		return length;
	}
	u8 id() const {
		return length ? m[0] : 0;
	}
	u8 get_uint8(int pos) const {
		return (u8)m[pos];
	}
	u16 get_uint16(int pos) const {
		return
		  ((u16)m[pos + 1] << 8) |
		  ((u16)m[pos]);
	}
	const char *get_string0(int pos, int *next_pos) const {
		for (unsigned i = pos; i < length; i++) {
			if (!m[i]) {
				if (next_pos)
					*next_pos = i + 1;
				return (const char *)&m[pos];
			}
		}
		return 0;
	}
	void put_uint8(int pos, u8 value) {
		v_REQDAT[pos] = value;
	}
	void put_uint16(int pos, u16 value) {
		v_REQDAT[pos] = value & 0xff;
		v_REQDAT[pos + 1] = value >> 8;
	}
	void put_uint64(int pos, u64 value) {
		for (int i = 0; i < 8; i++) {
			v_REQDAT[pos++] = value & 0xff;
			value >>= 8;
		}
	}
};

enum ObjType {
	OT_Image,
};

class Obj {
private:
	unsigned count;
public:
	Obj() : count(0) {}
	virtual ~Obj() {}
	friend class Ref;
};

class Ref {
private:
	unsigned id;
	Obj *get() {
		if (!id)
			return 0;
		return objs[id - 1];
	}
	void del() {
		if (!id)
			return;
		delete objs[id - 1];
		objs[id - 1] = 0;
	}
	static Obj **objs;
	static unsigned size;
public:
	Ref() : id(0) {}
	Ref(unsigned id_) : id(id_) {
		Obj *obj = ptr();
		if (obj)
			obj->count++;
	}
	Ref(const Ref &ref) {
		id = ref.id;
		Obj *obj = ptr();
		if (obj)
			obj->count++;
	}
	~Ref() {
		Obj *obj = ptr();
		if (obj && !--obj->count)
			del();
	}
	Ref& operator=(const Ref &src) {
		if (&src == this)
			return *this;
		Obj *obj = ptr();
		if (obj && !--obj->count)
			del();
		id = src.id;
		obj = ptr();
		if (obj)
			obj->count++;
		return *this;
	}
	bool is_null() { return id != 0; }
	Obj *ptr() { return get(); }
	template<typename T> T *as() { return (T *)ptr(); }
	static void init(unsigned size_) {
		objs = new Obj*[size_];
		size = size_;
		for (unsigned i = 0; i < size; i++)
			objs[i] = 0;
	}
	static void cleanup() {
		for (unsigned i = 0; i < size; i++)
			delete objs[i];
		delete[] objs;
	}
	static unsigned create(ObjType type);
};

Obj **Ref::objs = 0;
unsigned Ref::size = 0;

class Array {
private:
	Ref *refs;
	u8 maxn;
public:
	Array(u8 maxn_) {
		refs = new Ref[maxn_ + 1];
		maxn = maxn_;
	}
	~Array() {
		delete[] refs;
	}
	void dim(u8 maxn_) {
		if (maxn == maxn_)
			return;
		Ref *old = refs;
		refs = new Ref[maxn_ + 1];
		u8 n = maxn < maxn_ ? maxn : maxn_;
		for (u8 i = 0; i <= n; i++)
			refs[i] = old[i];
		maxn = maxn_;
		delete[] old;
	}
	bool is_valid(u8 n) {
		return n > 0 && n <= maxn;
	}
	Ref& operator[](int n) {
		return refs[n];
	}
};

// adapted from Vice (changes: White has been made pure)
static const u32 standardPalette[] = {
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

#define COLOR_MICROIO_BORDER 2

static const u32 microioPalette[] = {
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
	Palette() : mask(0) {}
	void reset(bool microio) {
		if (microio)
			set_microio();
		else
			set_standard();
	}
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
	color_t operator[](u8 id) const {
		return pal[id & mask];
	}
	bool req_SETPAL(Request &req, u32f pos, u8 value) {
		if (pos != Request::END) {
			if (pos == 0)
				req.buf.reset();
			else if (pos > 0 && pos < 1 + 256 * 3)
				req.buf.append_char(value);
			return true;
		}
		int n = req.n() - 1;
		if (n == 0) {
			reset(false);
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
		const u8 *data = (const u8 *)req.buf.get_data();
		for (int i = 0, j = 0; i < n; i++) {
			pal[i] = get_color_rgb(
				((u32)data[j] << 16) |
				((u32)data[j + 1] << 8) |
				((u32)data[j + 2])
			);
			j += 3;
		}
		return true;
	}
};

class Console {
private:
	struct Cell {
		u8 chr;
		u8 fg;
		u8 bg;
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
	u8 v_CONCOLS, v_CONROWS, v_CONCX, v_CONCY;
	u8 rw(RWOp op, u16 reg, u8 value, int cx, int cy) {
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
	void put(u16 address, u8 value) {
		switch (address) {
		case CONCOLS: {
			int max = width / font_width;
			if (!value || value > max)
				value = (u8)max;
			if (value != v_CONCOLS) {
				v_CONCOLS = value;
				resize();
			}
			} break;
		case CONROWS: {
			int max = height / font_height;
			if (!value || value > max)
				value = (u8)max;
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
	u8 get(u16 address) {
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
	void render(bool microio) {
		int i = 0, y = oy;
		for (int r = 0; r < v_CONROWS; r++) {
			int x = ox;
			for (int c = 0; c < v_CONCOLS; c++) {
				const Cell *cell = &buf[i++];
				if (cell->chr || microio)
					font_draw(x, y, pal[cell->bg], pal[cell->fg], cell->chr);
				x += font_width;
			}
			y += font_height;
		}
	}
};

enum ImgType {
	IT_Alpha0,
	IT_Alpha1,
	IT_Alpha8,
};

class Image : public Obj {
private:
	u16 width;
	u16 height;
	color_t *data;
	ImgType type;
	void set_size(u16 width_, u16 height_) {
		delete[] data;
		width = width_;
		height = height_;
		data = new color_t[width * height];
	}
public:
	Image() : width(0), height(0), data(0), type(IT_Alpha0) {}
	~Image() { delete[] data; }
	friend class Images;
};

class Images {
private:
	static const unsigned IPNGGEN_HEADER_SIZE = 9;
	static const unsigned IRAWRGBA_HEADER_SIZE = 7;
	static const unsigned INITIAL_DIM = 3;
	const Palette &pal;
	const int width, height;
	Array v;
	Ref bgimg;
	Ref tmp;
	enum PngStatus {
		Invalid,
		PalSize,
		PalData,
		ImgData
	};
	union Ctx {
		struct {
			PngStatus st;
			u32f n;
			u32f i;
			u32f bitbuf;
			u16f src_width;
			u16f x;
			u8f zoom;
		} IPNGGEN;
		struct {
			u32f n;
			u32 c;
			int i;
		} IRAWRGBA;
	} ctx;
	void png_begin(Request &req, int pal_size) {
		const u8 id = req.get_uint8(1);
		u16 width = req.get_uint16(2);
		u16 height = req.get_uint16(4);
		const u8 depth = req.get_uint8(6);
		const u8 color = req.get_uint8(7);
		const u8 flags = req.get_uint8(8);
		u8 bits;
		Image *img;
		if (!v.is_valid(id))
			goto error;
		switch (depth) {
		case 1:
		case 2:
		case 4:
			if (color != IPNGGEN_CT_GRAYSCALE
			  && color != IPNGGEN_CT_INDEXED_COLOR)
				goto error;
			break;
		case 8:
			break;
		case 16:
			if (color == IPNGGEN_CT_INDEXED_COLOR)
				goto error;
			break;
		default:
			goto error;
		}
		switch (color) {
		case IPNGGEN_CT_GRAYSCALE:
		case IPNGGEN_CT_INDEXED_COLOR:
			bits = depth;
			break;
		case IPNGGEN_CT_GRAYSCALE_ALPHA:
			bits = depth * 2;
			break;
		case IPNGGEN_CT_TRUECOLOR:
			bits = depth * 3;
			break;
		case IPNGGEN_CT_TRUECOLOR_ALPHA:
			bits = depth * 4;
			break;
		default:
			goto error;
		}
		ctx.IPNGGEN.x = 0;
		ctx.IPNGGEN.i = 0;
		ctx.IPNGGEN.n = IPNGGEN_HEADER_SIZE
		  + (((width * bits) + 7) >> 3) * height;
		if (pal_size) {
			if (flags & IPNGGEN_FLAGS_PALREF)
				ctx.IPNGGEN.n += 1 + pal_size;
			else
				ctx.IPNGGEN.n += 1 + pal_size * 3;
		}
		if (color == IPNGGEN_CT_INDEXED_COLOR && (width & 0x3) == 0)
			ctx.IPNGGEN.zoom = 1 + ((flags >> 2) & 0x7);
		else
			ctx.IPNGGEN.zoom = 1;
		ctx.IPNGGEN.src_width = width;
		width *= ctx.IPNGGEN.zoom;
		height *= ctx.IPNGGEN.zoom;
		tmp = Ref::create(OT_Image);
		img = tmp.as<Image>();
		img->set_size(width, height);
		img->type = IT_Alpha0;
		if (pal_size)
			ctx.IPNGGEN.st = PalData;
		else
			ctx.IPNGGEN.st = ImgData;
		return;
error:
		ctx.IPNGGEN.st = Invalid;
	}
	void png_put_pixel(Image *img, color_t c) {
		const u8f zoom = ctx.IPNGGEN.zoom;
		if (zoom == 1) {
			img->data[ctx.IPNGGEN.i++] = c;
		} else {
			u32f offset = ctx.IPNGGEN.i;
			for (u8f y = 0; y < ctx.IPNGGEN.zoom; y++) {
				for (u8f x = 0; x < ctx.IPNGGEN.zoom; x++)
					img->data[offset + x] = c;
				offset += img->width;
			}
			ctx.IPNGGEN.i += zoom;
		}
#if ALPHADEPTH == 8
		const u8 alpha = c >> 24;
		switch (alpha) {
		case 0:
			break;
		case 255:
			if (img->type == IT_Alpha0)
				img->type = IT_Alpha1;
			break;
		default:
			img->type = IT_Alpha8;
			break;
		}
#else
		if (!c)
			img->type = IT_Alpha1;
#endif
		ctx.IPNGGEN.x++;
		if (ctx.IPNGGEN.x == ctx.IPNGGEN.src_width) {
			ctx.IPNGGEN.x = 0;
			if (zoom > 1)
				ctx.IPNGGEN.i += img->width * (zoom - 1);
		}
	}
	u8 png_get_bits(u8 *value, u8f bits) {
		u8 v = 0;
		u8 mask = 0x80;
		for (u8f i = 0; i < bits; i++) {
			v <<= 1;
			if (*value & mask)
				v |= 0x01;
			*value <<= 1;
		}
		return v;
	}
	void png_index_data(Request &req, u8 value) {
		const u8 depth = req.get_uint8(6);
		Image *img = tmp.as<Image>();
		const u8 *p = (const u8 *)req.buf.get_data();
		const u8 last_entry = req.get_uint8(9);
		const u16f src_width = ctx.IPNGGEN.src_width;
		u8f j = 0;
		while (j < 8 && ctx.IPNGGEN.x < src_width) {
			u8 v = png_get_bits(&value, depth);
			j += depth;
			if (v > last_entry)
				v = 0;
			color_t c;
			memcpy(&c, &p[v * sizeof(color_t)], sizeof(color_t));
			if (!v) {
				const u8 flags = req.get_uint8(8);
				if (flags & IPNGGEN_FLAGS_IDX0TRANSP) {
					png_put_pixel(img, 0);
					continue;
				}
			}
			png_put_pixel(img, c);
		}
	}
	void png_value_data(Request &req, u32f pos, u8 value) {
		const u8 depth = req.get_uint8(6);
		if (depth == 16) {
			if ((pos & 0x1) == 1)
				return;
			pos >>= 1;
		}
		Image *img = tmp.as<Image>();
		const u8 color = req.get_uint8(7);
		switch (color) {
		case IPNGGEN_CT_GRAYSCALE:
			if (depth >= 8) {
				png_put_pixel(img, get_color_rgb(((u32)value << 16)
				  | ((u32)value << 8) | value));
			} else {
				u8f j = 0;
				int top = (1 << depth) - 1;
				while (j < 8 && ctx.IPNGGEN.x < ctx.IPNGGEN.src_width) {
					u8 v = png_get_bits(&value, depth);
					v = (u8)(v * 255 / top);
					png_put_pixel(img, get_color_rgb(((u32)v << 16)
					  | ((u32)v << 8) | v));
					j += depth;
				}
			}
			break;
		case IPNGGEN_CT_GRAYSCALE_ALPHA:
			if ((pos & 0x1) == 0)
				ctx.IPNGGEN.bitbuf = ((u32)value << 16) | ((u32)value << 8)
				  | value;
			else
				png_put_pixel(img, get_color_rgba(((u32)value << 24)
				  | ctx.IPNGGEN.bitbuf));
			break;
		case IPNGGEN_CT_TRUECOLOR:
			switch (pos % 3) {
			case 0:
				ctx.IPNGGEN.bitbuf = (u32)value << 16;
				return;
			case 1:
				ctx.IPNGGEN.bitbuf |= (u32)value << 8;
				return;
			}
			png_put_pixel(img, get_color_rgb(ctx.IPNGGEN.bitbuf | value));
			break;
		case IPNGGEN_CT_TRUECOLOR_ALPHA:
			switch (pos & 0x3) {
			case 0:
				ctx.IPNGGEN.bitbuf = (u32)value << 16;
				return;
			case 1:
				ctx.IPNGGEN.bitbuf |= (u32)value << 8;
				return;
			case 2:
				ctx.IPNGGEN.bitbuf |= value;
				return;
			}
			png_put_pixel(img, get_color_rgba(((u32)value << 24)
			  | ctx.IPNGGEN.bitbuf));
			break;
		}
	}
public:
	Images(const Palette &pal_, int width_, int height_)
	  : pal(pal_), width(width_), height(height_), v(INITIAL_DIM) {}
	void reset() {
		v.dim(INITIAL_DIM);
		for (unsigned id = 0; id <= INITIAL_DIM; id++)
			v[id] = 0;
		bgimg = 0;
		tmp = 0;
	}
	bool req_SETBGIMG(Request &req) {
		if (req.n() != 2)
			return false;
		const u8 id = req.get_uint8(1);
		bgimg = v.is_valid(id) ? v[id] : 0;
		return true;
	}
	bool req_IDESTROY(Request &req) {
		if (req.n() != 2)
			return false;
		const u8 id = req.get_uint8(1);
		if (!v.is_valid(id))
			return false;
		v[id] = 0;
		return true;
	}
	bool req_IDIM(Request &req) {
		if (req.n() != 2)
			return false;
		const u8 last_id = req.get_uint8(1);
		v.dim(last_id);
		return true;
	}
	bool req_IINFO(Request &req) {
		if (req.n() != 2)
			return false;
		const u8 id = req.get_uint8(1);
		if (!v.is_valid(id))
			return false;
		Image *img = tmp.as<Image>();
		if (!img)
			return false;
		req.put_uint16(IINFO_WIDTH, img->width);
		req.put_uint16(IINFO_HEIGHT, img->height);
		req.put_uint8(IINFO_FLAGS, 0);
		return true;
	}
	bool req_ILOAD(Request &req) {
		int n = req.n();
		if (n < 4)
			return false;
		const u8 id = req.get_uint8(1);
		int pos;
		const char *s = req.get_string0(2, &pos);
		if (!s || pos != n)
			return false;
		ImageResource *image = ImageResource::get(s);
		if (!image)
			return false;
		req.rewind();
		bool res = false;
		const ImageResourceFormat format = image->get_format();
		if (format == IRF_IRAWRGBA) {
			req.append(REQ_IRAWRGBA);
			req.append(id);
			req.append_uint16((u16)image->get_width());
			req.append_uint16((u16)image->get_height());
			req.append(IRAWRGBA_FLAGS_ALPHA);
			unsigned i;
			for (i = 0; i < IRAWRGBA_HEADER_SIZE; i++)
				req_IRAWRGBA(req, i, req.get_uint8(i));
			const u8 *data = image->get_data();
			for (unsigned j = 0; j < image->get_size(); j++) {
				req.append(data[j]);
				req_IRAWRGBA(req, i++, data[j]);
			}
			res = req_IRAWRGBA(req, Request::END, 0);
		} else if (format == IRF_IPNGGEN) {
			const u8 *data = image->get_data();
			for (unsigned i = 0; i < image->get_size(); i++) {
				u8 value;
				if (i == 1)
					value = id;
				else
					value = data[i];
				req.append(value);
				req_IPNGGEN(req, i, value);
			}
			res = req_IPNGGEN(req, Request::END, 0);
		}
		ImageResource::release(image);
		return res;
	}
	bool req_IPNGGEN(Request &req, u32f pos, u8 value) {
		if (pos == 0) {
			ctx.IPNGGEN.st = Invalid;
			req.buf.reset();
		} else if (pos == IPNGGEN_HEADER_SIZE - 1) {
			const u8 color = req.get_uint8(7);
			if (color == IPNGGEN_CT_INDEXED_COLOR)
				ctx.IPNGGEN.st = PalSize;
			else
				png_begin(req, 0);
		} else if (ctx.IPNGGEN.st == PalSize) {
			png_begin(req, value + 1);
		} else if (ctx.IPNGGEN.st == PalData) {
			const u8 flags = req.get_uint8(8);
			int n = (int)pos - IPNGGEN_HEADER_SIZE - 1;
			color_t c;
			if (flags & IPNGGEN_FLAGS_PALREF) {
				c = pal[value];
			} else {
				switch (n % 3) {
				case 0:
					ctx.IPNGGEN.bitbuf = (u32)value << 16;
					return true;
				case 1:
					ctx.IPNGGEN.bitbuf |= (u32)value << 8;
					return true;
				}
				c = get_color_rgb(ctx.IPNGGEN.bitbuf | value);
				n /= 3;
			}
			char *b = req.buf.append_raw(sizeof(color_t));
			memcpy(b, &c, sizeof(color_t));
			const u8 last_entry = req.get_uint8(9);
			if (n == (int)last_entry)
				ctx.IPNGGEN.st = ImgData;
		} else if (pos == Request::END) {
			if (ctx.IPNGGEN.st != ImgData
			  || req.n_streaming() != ctx.IPNGGEN.n) {
				tmp = 0;
				return false;
			}
			const u8 id = req.get_uint8(1);
			v[id] = tmp;
			tmp = 0;
		} else if (ctx.IPNGGEN.st == ImgData) {
			if (pos == ctx.IPNGGEN.n) {
				ctx.IPNGGEN.st = Invalid;
				return true;
			}
			const u8 color = req.get_uint8(7);
			if (color == IPNGGEN_CT_INDEXED_COLOR)
				png_index_data(req, value);
			else
				png_value_data(req, pos - IPNGGEN_HEADER_SIZE, value);
		}
		return true;
	}
	bool req_IRAWRGBA(Request &req, u32f pos, u8 value) {
		if (pos == 0) {
			ctx.IRAWRGBA.n = 0;
		} else if (pos == IRAWRGBA_HEADER_SIZE - 1) {
			const u8 id = req.get_uint8(1);
			if (!v.is_valid(id))
				return true;
			const u16 width = req.get_uint16(2);
			const u16 height = req.get_uint16(4);
			tmp = Ref::create(OT_Image);
			Image *img = tmp.as<Image>();
			img->set_size(width, height);
			img->type = IT_Alpha0;
			ctx.IRAWRGBA.i = 0;
			ctx.IRAWRGBA.n = IRAWRGBA_HEADER_SIZE
			  + img->width * img->height * 4;
		} else if (pos >= IRAWRGBA_HEADER_SIZE && pos <= ctx.IRAWRGBA.n) {
			pos -= IRAWRGBA_HEADER_SIZE;
			switch (pos & 0x3) {
			case 0:
				ctx.IRAWRGBA.c = (u32)value << 16;
				break;
			case 1:
				ctx.IRAWRGBA.c |= (u32)value << 8;
				break;
			case 2:
				ctx.IRAWRGBA.c |= value;
				break;
			case 3: {
				Image *img = tmp.as<Image>();
				const u8 flags = req.get_uint8(6);
				if (flags & IRAWRGBA_FLAGS_ALPHA) {
					switch (value) {
					case 0:
						break;
					case 255:
						if (img->type == IT_Alpha0)
							img->type = IT_Alpha1;
						break;
					default:
						img->type = IT_Alpha8;
						break;
					}
					img->data[ctx.IRAWRGBA.i] = get_color_rgba(ctx.IRAWRGBA.c
					  | ((u32)value << 24));
				} else {
					img->data[ctx.IRAWRGBA.i] = get_color_rgb(ctx.IRAWRGBA.c);
				}
				ctx.IRAWRGBA.i++;
				} break;
			}
		} else if (pos == Request::END) {
			if (req.n_streaming() != ctx.IRAWRGBA.n) {
				tmp = 0;
				return false;
			}
			const u8 id = req.get_uint8(1);
			v[id] = tmp;
			tmp = 0;
		}
		return true;
	}
	void render() {
		Image *img = bgimg.as<Image>();
		if (!img || !img->data)
			return;
		const unsigned dst_stride = width;
		const unsigned src_stride = img->width;
		int dst_ox = (width - img->width) / 2;
		int dst_oy = (height - img->height) / 2;
		unsigned src_ox = 0;
		unsigned src_oy = 0;
		unsigned src_width = img->width;
		unsigned src_height = img->height;
		if (dst_ox < 0) {
			src_ox += -dst_ox;
			dst_ox = 0;
			src_width = width;
		}
		if (dst_oy < 0) {
			src_oy += -dst_oy;
			dst_oy = 0;
			src_height = height;
		}
		u16f i = src_oy * src_stride + src_ox;
		u16f j = dst_oy * dst_stride + dst_ox;
		const ImgType type = img->type;
		for (u16f y = 0; y < src_height; y++) {
			for (u16f x = 0; x < src_width; x++) {
				const color_t src_c = img->data[i + x];
				switch (type) {
				case IT_Alpha0:
					buffer[j + x] = src_c;
					break;
				case IT_Alpha1:
					if (is_color_opaque(src_c))
						buffer[j + x] = src_c;
					break;
				case IT_Alpha8: {
					const color_t dst_c = buffer[j + x];
					buffer[j + x] = mix_color(dst_c, src_c);
					} break;
				}
			}
			i += src_stride;
			j += dst_stride;
		}
	}
};

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
	  console(palette, width, height), images(palette, width, height) {
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
		Ref::cleanup();
	}
};

IO2 *new_IO2() {
	return new IO2Impl();
}
