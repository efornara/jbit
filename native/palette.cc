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

// palette.cc

#include "io2impl.h"

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

static const u32 microioPalette[] = {
	0x000000, // Foreground
	0x78c8b4, // Background
	0x90e0c0, // Border
	0x000000, // Unused
};

void Palette::reset(bool microio) {
	if (microio)
		set_microio();
	else
		set_standard();
}

void Palette::set_standard() {
	for (int i = 0; i < 16; i++)
		pal[i] = get_color_rgb(standardPalette[i]);
	mask = 0x0f;
}

void Palette::set_microio() {
	for (int i = 0; i < 4; i++)
		pal[i] = get_color_rgb(microioPalette[i]);
	mask = 0x03;
}

bool Palette::req_SETPAL(Request &req, u32f pos, u8 value) {
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
