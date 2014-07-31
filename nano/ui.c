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

uint8_t ui_result;

static uint8_t cx;
static uint8_t cy;

static void home() {
	lcd_goto(0, 0);
	cx = 0;
	cy = 0;
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

	home();
	if (n > 13)
		return;
	w = (LCD_WIDTH - n * LCD_FIXED_CHAR_WIDTH) / 2;
	put_hline(w);
	for (i = 0; i < n; i++)
		put_char(title[i]);
	put_hline(w);
	put_endl();
}

static void process_events(uint8_t event, uint8_t code) {
	ui_result = 1;
}

static void start() {
	keypad_handler = process_events;
	ui_result = 0;
}

void ui_msg(const char *title, const char *msg) {
	lcd_clear();
	draw_header(title);
	put_string(msg);
	start();
}
