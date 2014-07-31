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

#include <string.h>

#include "nano.h"

#define STATE_IDLE 0
#define STATE_MSG 1
#define STATE_MENU 2

uint8_t ui_state = STATE_IDLE;
uint8_t ui_result;

union {
	struct {
		const char *const *items;
		uint8_t current;
		uint8_t max;
	} menu;
} context;

static uint8_t cx;
static uint8_t cy;

static void goto_y(int y) {
	lcd_goto(0, y);
	cx = 0;
	cy = y;
}

static void goto_home() {
	goto_y(0);
}

static void put_x(uint8_t value) {
	lcd_write(LCD_DATA, value);
	cx++;
}

static void put_char(char c) {
	lcd_char(c);
	cx += LCD_FIXED_CHAR_WIDTH;
}

static void put_hline(int w) {
	int i;
	put_x(0);
	for (i = 0; i < w - 2; i++)
		put_x(0x08);
	put_x(0);
}

static void put_string(const char *s) {
	char c;
	while ((c = *s++))
		put_char(c);
}

static void put_endl() {
	for (; cx < LCD_WIDTH; cx++)
		lcd_write(LCD_DATA, 0);
	cx = 0;
	cy++;
}

static void draw_header(const char *title) {
	int i, w, n = strlen(title);

	goto_home();
	if (n > 13)
		return;
	w = (LCD_WIDTH - n * LCD_FIXED_CHAR_WIDTH) / 2;
	put_hline(w);
	for (i = 0; i < n; i++)
		put_char(title[i]);
	put_hline(w);
	put_endl();
}

static int menu_get_n_items() {
	int i;

	for (i = 0; context.menu.items[i]; i++)
		;
	return i;
}

static void draw_menu_items() {
	int i;

	goto_y(1);
	for (i = 0; context.menu.items[i] && i < 5; i++) {
		put_char(i == context.menu.current ? '>' : ' ');
		put_string(context.menu.items[i]);
		put_endl();
	}
	for (; i < 5; i++)
		put_endl();
}

static void process_events(uint8_t event, char c) {
	switch (ui_state) {
	case STATE_MSG:
		ui_result = 1;
		ui_state = STATE_IDLE;
		break;
	case STATE_MENU:
		switch (c) {
		case UP:
			if (context.menu.current > 0) {
				context.menu.current--;
				draw_menu_items();
			}
			break;
		case DOWN:
			if (context.menu.current < 5
			  && context.menu.current < menu_get_n_items() - 1) {
				context.menu.current++;
				draw_menu_items();
			}
			break;
		case '5':
			ui_result = context.menu.current;
			ui_state = STATE_IDLE;
			break;
		}
		break;
	}
}

static void start(uint8_t state) {
	keypad_handler = process_events;
	ui_state = state;
}

void ui_msg(const char *title, const char *msg) {
	lcd_clear();
	draw_header(title);
	put_string(msg);
	start(STATE_MSG);
}

void ui_menu(const char *title, const char *const items[]) {
	lcd_clear();
	draw_header(title);
	start(STATE_MENU);
	context.menu.items = items;
	context.menu.current = 0;
	draw_menu_items();
}
