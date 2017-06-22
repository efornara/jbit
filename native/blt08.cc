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

// blt08.cc

#include <stdint.h>

#include "blt08.h"

static const uint8_t palette_rgb[] = {
	0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF,
	0xBE, 0x1A, 0x24,
	0x30, 0xE6, 0xC6,
	0xB4, 0x1A, 0xE2,
	0x1F, 0xD2, 0x1E,
	0x21, 0x1B, 0xAE,
	0xDF, 0xF6, 0x0A,
	0xB8, 0x41, 0x04,
	0x6A, 0x33, 0x04,
	0xFE, 0x4A, 0x57,
	0x42, 0x45, 0x40,
	0x70, 0x74, 0x6F,
	0x59, 0xFE, 0x59,
	0x5F, 0x53, 0xFE,
	0xA4, 0xA7, 0xA2,
	0x78, 0xC8, 0xB4,
	0x90, 0xE0, 0xC0,
	0x00, 0x00, 0x33,
	0x00, 0x00, 0x66,
	0x00, 0x00, 0x99,
	0x00, 0x00, 0xCC,
	0x00, 0x00, 0xFF,
	0x00, 0x33, 0x00,
	0x00, 0x33, 0x33,
	0x00, 0x33, 0x66,
	0x00, 0x33, 0x99,
	0x00, 0x33, 0xCC,
	0x00, 0x33, 0xFF,
	0x00, 0x66, 0x00,
	0x00, 0x66, 0x33,
	0x00, 0x66, 0x66,
	0x00, 0x66, 0x99,
	0x00, 0x66, 0xCC,
	0x00, 0x66, 0xFF,
	0x00, 0x99, 0x00,
	0x00, 0x99, 0x33,
	0x00, 0x99, 0x66,
	0x00, 0x99, 0x99,
	0x00, 0x99, 0xCC,
	0x00, 0x99, 0xFF,
	0x00, 0xCC, 0x00,
	0x00, 0xCC, 0x33,
	0x00, 0xCC, 0x66,
	0x00, 0xCC, 0x99,
	0x00, 0xCC, 0xCC,
	0x00, 0xCC, 0xFF,
	0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x33,
	0x00, 0xFF, 0x66,
	0x00, 0xFF, 0x99,
	0x00, 0xFF, 0xCC,
	0x00, 0xFF, 0xFF,
	0x33, 0x00, 0x00,
	0x33, 0x00, 0x33,
	0x33, 0x00, 0x66,
	0x33, 0x00, 0x99,
	0x33, 0x00, 0xCC,
	0x33, 0x00, 0xFF,
	0x33, 0x33, 0x00,
	0x33, 0x33, 0x33,
	0x33, 0x33, 0x66,
	0x33, 0x33, 0x99,
	0x33, 0x33, 0xCC,
	0x33, 0x33, 0xFF,
	0x33, 0x66, 0x00,
	0x33, 0x66, 0x33,
	0x33, 0x66, 0x66,
	0x33, 0x66, 0x99,
	0x33, 0x66, 0xCC,
	0x33, 0x66, 0xFF,
	0x33, 0x99, 0x00,
	0x33, 0x99, 0x33,
	0x33, 0x99, 0x66,
	0x33, 0x99, 0x99,
	0x33, 0x99, 0xCC,
	0x33, 0x99, 0xFF,
	0x33, 0xCC, 0x00,
	0x33, 0xCC, 0x33,
	0x33, 0xCC, 0x66,
	0x33, 0xCC, 0x99,
	0x33, 0xCC, 0xCC,
	0x33, 0xCC, 0xFF,
	0x33, 0xFF, 0x00,
	0x33, 0xFF, 0x33,
	0x33, 0xFF, 0x66,
	0x33, 0xFF, 0x99,
	0x33, 0xFF, 0xCC,
	0x33, 0xFF, 0xFF,
	0x66, 0x00, 0x00,
	0x66, 0x00, 0x33,
	0x66, 0x00, 0x66,
	0x66, 0x00, 0x99,
	0x66, 0x00, 0xCC,
	0x66, 0x00, 0xFF,
	0x66, 0x33, 0x00,
	0x66, 0x33, 0x33,
	0x66, 0x33, 0x66,
	0x66, 0x33, 0x99,
	0x66, 0x33, 0xCC,
	0x66, 0x33, 0xFF,
	0x66, 0x66, 0x00,
	0x66, 0x66, 0x33,
	0x66, 0x66, 0x66,
	0x66, 0x66, 0x99,
	0x66, 0x66, 0xCC,
	0x66, 0x66, 0xFF,
	0x66, 0x99, 0x00,
	0x66, 0x99, 0x33,
	0x66, 0x99, 0x66,
	0x66, 0x99, 0x99,
	0x66, 0x99, 0xCC,
	0x66, 0x99, 0xFF,
	0x66, 0xCC, 0x00,
	0x66, 0xCC, 0x33,
	0x66, 0xCC, 0x66,
	0x66, 0xCC, 0x99,
	0x66, 0xCC, 0xCC,
	0x66, 0xCC, 0xFF,
	0x66, 0xFF, 0x00,
	0x66, 0xFF, 0x33,
	0x66, 0xFF, 0x66,
	0x66, 0xFF, 0x99,
	0x66, 0xFF, 0xCC,
	0x66, 0xFF, 0xFF,
	0x99, 0x00, 0x00,
	0x99, 0x00, 0x33,
	0x99, 0x00, 0x66,
	0x99, 0x00, 0x99,
	0x99, 0x00, 0xCC,
	0x99, 0x00, 0xFF,
	0x99, 0x33, 0x00,
	0x99, 0x33, 0x33,
	0x99, 0x33, 0x66,
	0x99, 0x33, 0x99,
	0x99, 0x33, 0xCC,
	0x99, 0x33, 0xFF,
	0x99, 0x66, 0x00,
	0x99, 0x66, 0x33,
	0x99, 0x66, 0x66,
	0x99, 0x66, 0x99,
	0x99, 0x66, 0xCC,
	0x99, 0x66, 0xFF,
	0x99, 0x99, 0x00,
	0x99, 0x99, 0x33,
	0x99, 0x99, 0x66,
	0x99, 0x99, 0x99,
	0x99, 0x99, 0xCC,
	0x99, 0x99, 0xFF,
	0x99, 0xCC, 0x00,
	0x99, 0xCC, 0x33,
	0x99, 0xCC, 0x66,
	0x99, 0xCC, 0x99,
	0x99, 0xCC, 0xCC,
	0x99, 0xCC, 0xFF,
	0x99, 0xFF, 0x00,
	0x99, 0xFF, 0x33,
	0x99, 0xFF, 0x66,
	0x99, 0xFF, 0x99,
	0x99, 0xFF, 0xCC,
	0x99, 0xFF, 0xFF,
	0xCC, 0x00, 0x00,
	0xCC, 0x00, 0x33,
	0xCC, 0x00, 0x66,
	0xCC, 0x00, 0x99,
	0xCC, 0x00, 0xCC,
	0xCC, 0x00, 0xFF,
	0xCC, 0x33, 0x00,
	0xCC, 0x33, 0x33,
	0xCC, 0x33, 0x66,
	0xCC, 0x33, 0x99,
	0xCC, 0x33, 0xCC,
	0xCC, 0x33, 0xFF,
	0xCC, 0x66, 0x00,
	0xCC, 0x66, 0x33,
	0xCC, 0x66, 0x66,
	0xCC, 0x66, 0x99,
	0xCC, 0x66, 0xCC,
	0xCC, 0x66, 0xFF,
	0xCC, 0x99, 0x00,
	0xCC, 0x99, 0x33,
	0xCC, 0x99, 0x66,
	0xCC, 0x99, 0x99,
	0xCC, 0x99, 0xCC,
	0xCC, 0x99, 0xFF,
	0xCC, 0xCC, 0x00,
	0xCC, 0xCC, 0x33,
	0xCC, 0xCC, 0x66,
	0xCC, 0xCC, 0x99,
	0xCC, 0xCC, 0xCC,
	0xCC, 0xCC, 0xFF,
	0xCC, 0xFF, 0x00,
	0xCC, 0xFF, 0x33,
	0xCC, 0xFF, 0x66,
	0xCC, 0xFF, 0x99,
	0xCC, 0xFF, 0xCC,
	0xCC, 0xFF, 0xFF,
	0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x33,
	0xFF, 0x00, 0x66,
	0xFF, 0x00, 0x99,
	0xFF, 0x00, 0xCC,
	0xFF, 0x00, 0xFF,
	0xFF, 0x33, 0x00,
	0xFF, 0x33, 0x33,
	0xFF, 0x33, 0x66,
	0xFF, 0x33, 0x99,
	0xFF, 0x33, 0xCC,
	0xFF, 0x33, 0xFF,
	0xFF, 0x66, 0x00,
	0xFF, 0x66, 0x33,
	0xFF, 0x66, 0x66,
	0xFF, 0x66, 0x99,
	0xFF, 0x66, 0xCC,
	0xFF, 0x66, 0xFF,
	0xFF, 0x99, 0x00,
	0xFF, 0x99, 0x33,
	0xFF, 0x99, 0x66,
	0xFF, 0x99, 0x99,
	0xFF, 0x99, 0xCC,
	0xFF, 0x99, 0xFF,
	0xFF, 0xCC, 0x00,
	0xFF, 0xCC, 0x33,
	0xFF, 0xCC, 0x66,
	0xFF, 0xCC, 0x99,
	0xFF, 0xCC, 0xCC,
	0xFF, 0xCC, 0xFF,
	0xFF, 0xFF, 0x00,
	0xFF, 0xFF, 0x33,
	0xFF, 0xFF, 0x66,
	0xFF, 0xFF, 0x99,
	0xFF, 0xFF, 0xCC,
};

blt08_palette_t blt08_palette = {
	palette_rgb,
	sizeof(palette_rgb) / sizeof(uint8_t) / 3,
	1
};

uint8_t blt08_get_color_rgba(uint32_t c) {
	uint8_t r, g, b, a;
	const uint8_t *p;
	int i, min_d, best;
	a = (uint8_t)(c >> 24);
	if (!(a & 0x80))
		return 0;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c      );
	p = blt08_palette.rgb;
	min_d = 1000;
	best = 0;
	for (i = 0; i < blt08_palette.n_of_entries; i++) {
		int d = 0, d_;
		#define ADD_DISTANCE(c) d_ = c - *p++; d += d_ < 0 ? -d_ : d_;
		ADD_DISTANCE(r)
		ADD_DISTANCE(g)
		ADD_DISTANCE(b)
		#undef ADD_DISTANCE
		if (d < min_d) {
			best = i;
			if (d == 0)
				break;
			min_d = d;
		}
	}
	return (uint8_t)(blt08_palette.base_index + best);
}
