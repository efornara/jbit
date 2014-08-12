/*
 * Copyright (C) 2014  Emanuele Fornara
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

#include <stdint.h>
#include <string.h>

#include "hwsim.h"

static struct metric_t {
	char key;
	hwsim_rect_t value;
} metrics[] = {
	{ HWSIM_M_WINDOW,       {   0,   0, 200, 300 } },
	{ HWSIM_M_DISPLAY,      {  10,  10, 168,  96 } },
};

#define N_OF_METRICS (sizeof(metrics) / sizeof(struct metric_t))

static struct color_t {
	char key;
	hwsim_color_t value;
} colors[] = {
	{ HWSIM_C_BODY,         { 0x30,0x30,0x30 } },
	{ HWSIM_C_DISPLAY_BG,   { 0x96,0xbb,0xa4 } },
	{ HWSIM_C_DISPLAY_FG,   { 0x00,0x00,0x00 } },
	{ HWSIM_C_KEY_BG,       { 0xcb,0xcf,0x88 } },
};

#define N_OF_COLORS (sizeof(colors) / sizeof(struct color_t))

const char *hwsim_keypad_labels = "0123456789*#";

void hwsim_init(hwsim_t *hw) {
	memset(hw, 0, sizeof(hw->video));
	hw->video[0] = 0xff;
}

void hwsim_cleanup(hwsim_t *hw) {
}

int hwsim_get_metrics(hwsim_t *hw, int element, hwsim_rect_t *m) {
	const char *p;
	int i, col, row;

	for (i = 0; i < N_OF_METRICS; i++) {
		if (metrics[i].key == element) {
			*m = metrics[i].value;
			return 1;
		}
	}
	if ((p = strchr(hwsim_keypad_labels, (char)element)) == NULL)
		return 0;
	i = p - hwsim_keypad_labels;
	// swap '9' and '*', if needed
	if (i == 9)
		i = 10;
	else if (i == 10)
		i = 9;
	col = i % 3;
	row = i / 3;
	m->x = 10 + col * 60;
	m->y = 118 + row * 38;
	m->w = 48;
	m->h = 30;
	return 1;
}

int hwsim_get_color(hwsim_t *hw, int element, hwsim_color_t *c) {
	int i;

	for (i = 0; i < N_OF_COLORS; i++) {
		if (colors[i].key == element) {
			*c = colors[i].value;
			return 1;
		}
	}
	return 0;
}
