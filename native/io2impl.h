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

// io2impl.h

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

static inline color_t get_color_rgb(uint32_t c) {
	return get_color_rgba(c | 0xff000000);
}

#include "rom.h"

static const int stride = 128;

extern color_t *buffer;

static const int font_width = 8;
static const int font_height = 14;

void font_draw(int x, int y, color_t bg, color_t fg, uint8_t c);

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
		return n <= maxn;
	}
	Ref& operator[](int n) {
		return refs[n];
	}
};

#define COLOR_MICROIO_BORDER 2

class Palette {
private:
	color_t pal[256];
	int mask;
public:
	Palette() : mask(0) {}
	void reset(bool microio);
	void set_standard();
	void set_microio();
	color_t operator[](u8 id) const {
		return pal[id & mask];
	}
	bool req_SETPAL(Request &req, u32f pos, u8 value);
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
	u8 rw(RWOp op, u16 reg, u8 value, int cx, int cy);
public:
	Console(const Palette &pal_, int width_, int height_)
	  : pal(pal_), width(width_), height(height_), buf(0) {}
	~Console() {
		delete[] buf;
	}
	void reset();
	void resize();
	void put(u16 address, u8 value);
	u8 get(u16 address);
	void render(bool microio);
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
	void render_pixel(u32f buf_offset, u32f img_offset) {
		const color_t src_c = data[img_offset];
		switch (type) {
		case IT_Alpha0:
			buffer[buf_offset] = src_c;
			break;
		case IT_Alpha1:
			if (is_color_opaque(src_c))
				buffer[buf_offset] = src_c;
			break;
		case IT_Alpha8: {
			const color_t dst_c = buffer[buf_offset];
			buffer[buf_offset] = mix_color(dst_c, src_c);
			} break;
		}
	}
	friend class Images;
	friend class Layers;
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
	bool load_external;
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
	void png_begin(Request &req, int pal_size);
	void png_put_pixel(Image *img, color_t c);
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
	void png_index_data(Request &req, u8 value);
	void png_value_data(Request &req, u32f pos, u8 value);
public:
	Images(const Palette &pal_, int width_, int height_) :
	  pal(pal_), width(width_), height(height_), v(INITIAL_DIM),
	  load_external(false) {}
	void reset();
	Ref get(u8 id);
	bool req_SETBGIMG(Request &req);
	bool req_IDESTROY(Request &req);
	bool req_IDIM(Request &req);
	bool req_IINFO(Request &req);
	bool req_ILOAD(Request &req);
	void load_begin_external();
	Ref load_end_external();
	bool load(Request &req, u8 id, const char *s);
	bool req_IPNGGEN(Request &req, u32f pos, u8 value);
	bool req_IRAWRGBA(Request &req, u32f pos, u8 value);
	void render();
};

class Layers {
private:
	const Palette &pal;
	Images &images;
	const int width, height;
	struct Layer {
		Ref img;
	};
	struct TiledLayer : Layer {
		u8 twidth;
		u8 theight;
		u16 cols;
		u16 rows;
		u16 cx;
		u16 cy;
		u8 *cells;
	} gameset;
public:
	Layers(const Palette &pal_, Images &images_, int width_, int height_)
	  : pal(pal_), images(images_), width(width_), height(height_) {
		gameset.cells = 0;
	}
	void reset();
	void put(u16 address, u8 value);
	u8 get(u16 address);
	bool req_GAMESET(Request &req);
	void render();
};
