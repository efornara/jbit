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

// images.cc

#include "io2impl.h"

void Images::png_begin(Request &req, int pal_size) {
	const u8 id = req.get_uint8(1);
	u16 width = req.get_uint16(2);
	u16 height = req.get_uint16(4);
	const u8 depth = req.get_uint8(6);
	const u8 color = req.get_uint8(7);
	const u8 flags = req.get_uint8(8);
	u8 bits;
	Image *img;
	if (!load_external && !v.is_valid(id))
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

void Images::png_put_pixel(Image *img, color_t c) {
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

void Images::png_index_data(Request &req, u8 value) {
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

void Images::png_value_data(Request &req, u32f pos, u8 value) {
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

void Images::reset() {
	v.dim(INITIAL_DIM);
	for (unsigned id = 0; id <= INITIAL_DIM; id++)
		v[id] = 0;
	bgimg = 0;
	tmp = 0;
}

Ref Images::get(u8 id) {
	if (!v.is_valid(id))
		return Ref();
	return v[id];
}

bool Images::req_SETBGIMG(Request &req) {
	if (req.n() != 2)
		return false;
	const u8 id = req.get_uint8(1);
	bgimg = v.is_valid(id) ? v[id] : 0;
	return true;
}

bool Images::req_IDESTROY(Request &req) {
	if (req.n() != 2)
		return false;
	const u8 id = req.get_uint8(1);
	if (!v.is_valid(id))
		return false;
	v[id] = 0;
	return true;
}

bool Images::req_IDIM(Request &req) {
	if (req.n() != 2)
		return false;
	const u8 last_id = req.get_uint8(1);
	v.dim(last_id);
	return true;
}

bool Images::req_IINFO(Request &req) {
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

bool Images::req_ILOAD(Request &req) {
	int n = req.n();
	if (n < 4)
		return false;
	const u8 id = req.get_uint8(1);
	int pos;
	const char *s = req.get_string0(2, &pos);
	if (!s || pos != n)
		return false;
	return load(req, id, s);
}

void Images::load_begin_external() {
	load_external = true;
}

Ref Images::load_end_external() {
	load_external = false;
	Ref res = tmp;
	tmp = 0;
	return res;
}

bool Images::load(Request &req, u8 id, const char *s) {
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

bool Images::req_IPNGGEN(Request &req, u32f pos, u8 value) {
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
		if (!load_external) {
			const u8 id = req.get_uint8(1);
			v[id] = tmp;
			tmp = 0;
		}
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

bool Images::req_IRAWRGBA(Request &req, u32f pos, u8 value) {
	if (pos == 0) {
		ctx.IRAWRGBA.n = 0;
	} else if (pos == IRAWRGBA_HEADER_SIZE - 1) {
		const u8 id = req.get_uint8(1);
		if (!load_external && !v.is_valid(id))
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
		if (!load_external) {
			const u8 id = req.get_uint8(1);
			v[id] = tmp;
			tmp = 0;
		}
	}
	return true;
}

void Images::render() {
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
	for (u16f y = 0; y < src_height; y++) {
		for (u16f x = 0; x < src_width; x++)
			img->render_pixel(j + x, i + x);
		i += src_stride;
		j += dst_stride;
	}
}
