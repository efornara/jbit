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

// layers.cc

#include "io2impl.h"

void Layers::reset() {
	gameset.img = 0;
	delete[] gameset.cells;
	gameset.cells = 0;
}

void Layers::put(u16 address, u8 value) {
	if (!gameset.cells)
		return;
	switch (address) {
	case TCOLLO:
		if (value < gameset.cols)
			gameset.cx = value;
		break;
	case TROWLO:
		if (value < gameset.rows)
			gameset.cy = value;
		break;
	case TCELL:
		gameset.cells[gameset.cy * gameset.cols + gameset.cx] = value;
		break;
	}
}

u8 Layers::get(u16 address) {
	if (!gameset.cells)
		return 0;
	switch (address) {
	case TCOLLO:
		return gameset.cx;
	case TROWLO:
		return gameset.cy;
	case TCELL:
		return gameset.cells[gameset.cy * gameset.cols + gameset.cx];
	}
	return 0;
}

bool Layers::req_GAMESET(Request &req) {
	int n = req.n();
	u8 image_id = TILESET_SILK; // layer_id = 0
	u8 cols = 0, rows = 0, twidth = 0, theight = 0;
	u8 default_twidth = 8, default_theight = 8;
	const char *tileset = 0;
	Ref tmp;
	int pos = 1;
	if (n > pos)
		image_id = req.get_uint8(pos++);
	if (n > pos) {
		if (n - pos < 2)
			return false;
		cols = req.get_uint8(pos++);
		rows = req.get_uint8(pos++);
	}
	if (n > pos)
		req.get_uint8(pos++); // layer_id (not used yet)
	if (n > pos) {
		if (n - pos < 2)
			return false;
		twidth = req.get_uint8(pos++);
		theight = req.get_uint8(pos++);
	}
	if (n != pos)
		return false;
	switch (image_id) {
	case TILESET_SILK:
		tileset = "/silk.png";
		default_twidth = 16;
		default_theight = 16;
		image_id = 0;
		break;
	case TILESET_FONT:
		tileset = "/font.png";
		image_id = 0;
		break;
	case TILESET_MICRO:
		tileset = "/micro.png";
		image_id = 0;
		break;
	}
	if (!twidth)
		twidth = default_twidth;
	if (!theight)
		theight = default_theight;
	if (!cols)
		cols = width / twidth;
	if (!rows)
		rows = height / theight;
	if (!rows || !cols)
		return false;
	if ((rows * twidth > width) || (cols * theight > height))
		return false;
	if (tileset) {
		images.load_begin_external();
		bool res = images.load(req, 0, tileset);
		tmp = images.load_end_external();
		if (!res)
			return false;
	} else {
		tmp = images.get(image_id);
	}
	Image *img = tmp.as<Image>();
	if (!img || !img->data)
		return false;
	if (img->width % twidth || img->height % theight)
		return false;
	delete[] gameset.cells;
	TiledLayer *tl = &gameset;
	tl->img = tmp;
	tl->twidth = twidth;
	tl->theight = theight;
	tl->cols = cols;
	tl->rows = rows;
	tl->cx = 0;
	tl->cy = 0;
	const int size = rows * cols;
	tl->cells = new u8[size];
	memset(tl->cells, 0, size);
	req.put_uint16(GAMESET_COLS, cols);
	req.put_uint16(GAMESET_ROWS, rows);
	return true;
}

void Layers::render() {
	TiledLayer *tl = &gameset;
	Image *img = tl->img.as<Image>();
	if (!img || !img->data)
		return;
	const unsigned dst_stride = width;
	const unsigned src_stride = img->width;
	const unsigned dst_ox = (width - (tl->rows * tl->twidth)) / 2;
	const unsigned dst_oy = (height - (tl->cols * tl->theight)) / 2;
	const unsigned src_cols = img->width / tl->twidth;
	u16f j = dst_oy * dst_stride + dst_ox;
	u16f cell_id = 0;
	for (u16f r = 0; r < tl->rows; r++) {
		for (u16f c = 0; c < tl->cols; c++, j += tl->twidth) {
			u16f cell = tl->cells[cell_id++];
			if (!cell--)
				continue;
			const u16f src_x = (cell % src_cols) * tl->twidth;
			const u16f src_y = (cell / src_cols) * tl->theight;
			u16f i = src_y * src_stride + src_x;
			u16f jj = j;
			for (u8f y = 0; y < tl->theight; y++) {
				for (u8f x = 0; x < tl->twidth; x++)
					img->render_pixel(jj + x, i + x);
				i += src_stride;
				jj += dst_stride;
			}
		}
		j += dst_stride * (tl->theight - 1);
	}
}
