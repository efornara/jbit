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

// blt32.h

static inline bool blt32_is_color_opaque(uint32_t c) {
	return (c & 0xff000000) == 0xff000000;
}

static inline uint32_t blt32_mix_color(uint32_t dst, uint32_t src) {
	const uint8_t src_alpha = src >> 24;
	switch (src_alpha) {
	case 0:
		return dst;
	case 255:
		return src;
	default:
		break;
	}
	const uint8_t dst_alpha = (uint8_t)(256 - src_alpha);
	uint32_t out = 0xff000000;
#define C(x) (((x) >> 16) & 0xff)
	out |= ((C(dst) * dst_alpha + C(src) * src_alpha) & 0xff00) << (16 - 8);
#undef C
#define C(x) (((x) >> 8) & 0xff)
	out |= ((C(dst) * dst_alpha + C(src) * src_alpha) & 0xff00) << (8 - 8);
#undef C
#define C(x) ((x) & 0xff)
	out |= (C(dst) * dst_alpha + C(src) * src_alpha) >> 8; // (0 - 8)
#undef C
	return out;
}

extern uint32_t blt32_get_color_rgba(uint32_t c);
