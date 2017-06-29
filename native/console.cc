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

// console.cc

#include "io2impl.h"

u8 Console::rw(RWOp op, u16 reg, u8 value, int cx, int cy) {
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

void Console::reset() {
	v_CONCOLS = 10;
	v_CONROWS = 4;
	v_CONCX = 0;
	v_CONCY = 0;
	resize();
}

void Console::resize() {
	delete[] buf;
	const int size = v_CONCOLS * v_CONROWS;
	buf = new Cell[size];
	ox = (width - v_CONCOLS * font_width)  / 2;
	oy = (height - v_CONROWS * font_height)  / 2;
}

void Console::put(u16 address, u8 value) {
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

u8 Console::get(u16 address) {
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

void Console::render(bool microio) {
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
