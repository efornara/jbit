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
	{ HWSIM_C_KEY_FG,       { 0x00,0x00,0x00 } },
	{ HWSIM_C_KEY_P_BG,     { 0x00,0x00,0x00 } },
	{ HWSIM_C_KEY_P_FG,     { 0xcb,0xcf,0x88 } },
};

#define N_OF_COLORS (sizeof(colors) / sizeof(struct color_t))

const char *hwsim_keypad_labels = "0123456789*#";

const char *hwsim_keypad_subs[] = {
	"",
	"",
	"abc",
	"def",
	"ghi",
	"jkl",
	"mno",
	"pqrs",
	"tuv",
	"wxyz",
	",",
	".",
};

// max 15 keys per label
const char *hwsim_keypad_keys[] = {
	"0",
	"1",
	"2abcABC",
	"3defDEF",
	"4ghiGHI",
	"5jklJKL",
	"6mnoMNO",
	"7pqrsPQRS",
	"8tuvTUV",
	"9wxyzWXYZ",
	"*,",
	"#.",
};

void hwsim_init(hwsim_t *hw) {
	memset(hw, 0, sizeof(hwsim_t));
	hw->mouse_state = -1;
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
	i = p - hwsim_keypad_labels - 1;
	// swap '0' and '*', if needed
	if (i == -1)
		i = 10;
	else if (i == 10)
		i = 11;
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

static int keypad_update(hwsim_t *hw, int i, int down, uint16_t key_mask) {
	uint16_t keypad_mask, old_state;

	if (down)
		hw->key_pressed[i] |= key_mask;
	else
		hw->key_pressed[i] &= ~key_mask;
	keypad_mask = (1 << i);
	old_state = hw->keypad_state;
	if (hw->key_pressed[i])
		hw->keypad_state |= keypad_mask;
	else
		hw->keypad_state &= ~keypad_mask;
	return hw->keypad_state != old_state;
}

int hwsim_key_update(hwsim_t *hw, int key_down, int value) {
	int i;
	const char *p;
	uint16_t key_mask = 0;

	for (i = 0; i < 12; i++) {
		if ((p = strchr(hwsim_keypad_keys[i], value)) != NULL) {
			key_mask = (1 << (p - hwsim_keypad_keys[i]));
			break;
		}
	}
	if (i == 12)
		return 0;
	return keypad_update(hw, i, key_down, key_mask);
}

int hwsim_mouse_update(hwsim_t *hw, int mouse_down, int x, int y) {
	const uint16_t key_mask = 0x8000;
	int i, force_update = 0;

	if (mouse_down) {
		if (hw->mouse_state != -1) {
			/*
			 It happens, for example, when the user leaves the window holding
			 the mouse button pressed (no WM_MOUSELEAVE is generated in win32)
			*/
			keypad_update(hw, hw->mouse_state, 0, key_mask);
			hw->mouse_state = -1;
			force_update = 1;
		}
		for (i = 0; i < 12; i++) {
			hwsim_rect_t m;
			hwsim_get_metrics(hw, hwsim_keypad_labels[i], &m);
			if (x >= m.x && x < m.x + m.w && y >= m.y && y < m.y + m.h)
				break;
		}
		if (i == 12)
			return force_update;
		hw->mouse_state = i;
	} else {
		if (hw->mouse_state == -1)
			return 0;
		i = hw->mouse_state;
		hw->mouse_state = -1;
	}
	return keypad_update(hw, i, mouse_down, key_mask) || force_update;
}
