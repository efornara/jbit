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

#ifdef PLATFORM_PC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "embd.h"

#ifdef PLATFORM_PC_SDL

#include <SDL/SDL.h>

#define SCALE 5
#define BORDER 10
#define BORDER2 (BORDER * 2)

#define BDCOLOR 0xa4ceb5
#define BGCOLOR 0x96bba4
#define FGCOLOR 0x000000

#define DISPLAY_WIDTH (LCD_WIDTH * SCALE)
#define DISPLAY_HEIGHT (LCD_HEIGHT * SCALE)

#define SCREEN_WIDTH (DISPLAY_WIDTH + BORDER2)
#define SCREEN_HEIGHT (DISPLAY_HEIGHT + BORDER2)

extern uint8_t lcd_bitmap[LCD_BITMAP_SIZE];

static SDL_Surface *screen;

static void sdl_init() {
	int rc;
	const char *title = "JBit Embd";

	rc = SDL_Init(SDL_INIT_VIDEO);
	assert(rc != -1);
	atexit(SDL_Quit);
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 24, SDL_DOUBLEBUF);
	assert(screen);
	SDL_WM_SetCaption(title, title);
}

extern void keypad_update(int key_down, int value);

static void sdl_events() {
	SDL_Event ev;
	int key;

	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			key = ev.key.keysym.sym;
			switch (key) {
			case SDLK_UP:
				key = '2';
				break;
			case SDLK_DOWN:
				key = '8';
				break;
			case SDLK_LEFT:
				key = '4';
				break;
			case SDLK_RIGHT:
				key = '6';
				break;
			case SDLK_RETURN:
				key = '5';
				break;
			case ',':
				key = '*';
				break;
			case '.':
				key = '#';
				break;
			}
			switch (key) {
			case SDLK_ESCAPE:
				exit(0);
				break;
			default:
				keypad_update(ev.type == SDL_KEYDOWN, key);
				break;
			}
			break;
		case SDL_QUIT:
			exit(0);
		}
	}
}

static void sdl_update_screen() {
	SDL_Rect rect;
	int x, y, r, i, j;

	rect.x = 0;
	rect.y = 0;
	rect.w = SCREEN_WIDTH;
	rect.h = SCREEN_HEIGHT;
	SDL_FillRect(screen, &rect, BDCOLOR);
	rect.x = BORDER;
	rect.y = BORDER;
	rect.w = DISPLAY_WIDTH;
	rect.h = DISPLAY_HEIGHT;
	SDL_FillRect(screen, &rect, BGCOLOR);
	rect.w = SCALE;
	rect.h = SCALE;
	for (i = 0, r = 0; r < LCD_ROWS; r++) {
		y = BORDER + r * 8 * SCALE;
		for (x = 0; x < LCD_WIDTH; x++) {
			rect.x = BORDER + x * SCALE;
			for (j = 0; j < 8; j++, i++) {
				rect.y = y + j * SCALE;
				if (lcd_bitmap[i])
					SDL_FillRect(screen, &rect, FGCOLOR);
			}
		}
	}
	SDL_Flip(screen);
}

static void sdl_sync() {
	sdl_events();
	sdl_update_screen();
}

#include "../native/serial.h"
#include "../native/serial.c"

static jbit_serial_t serial;

static void remote_send_keypad_state() {
	char *buf = serial.out;
	unsigned short mask = 1;
	int i;

	buf[0] = 'K';
	buf[1] = ' ';
	for (i = 13; i >= 2; i--) {
		buf[i] = (keypad_state & mask) ? '1' : '0';
		mask <<= 1;
	}
	buf[14] = '\n';
	buf[15] = '\r';
	buf[16] = '\0';
	jbit_serial_write(&serial);
}

static void remote_handle_line(jbit_serial_t *ctx) {
	static const int profile_error_rate = 0;
	static long long tot = 0, err = 0;

	const char *line = serial.in;
	int rc, data, nv, done = 0;
	char dirty;

	if (line[0] == 'L') {
		nv = sscanf(line, "L %d %d%c", &rc, &data, &dirty);
		if (nv == 2) {
			lcd_write(rc, data);
			done = 1;
		}
	} else if (line[0] == 'K') {
		if (line[1] == '\0') {
			remote_send_keypad_state();
			done = 1;
		}
	}
	if (!done)
		err++;
	tot++;
	if (profile_error_rate && tot % 1000 == 0)
		fprintf(stderr, "serial: err:%lld, tot:%lld, error rate: %.3f%%\n",
		  err, tot, ((double)err / tot) * 100.0);
}

static void remote(const char *port) {
	int rc;
	Uint32 t;
	
	fprintf(stderr, "remote\n");
	rc = jbit_serial_open(&serial, port, 115200);
	assert(rc == 0);
	fprintf(stderr, "waiting for arduino to reset...\n");
	t = SDL_GetTicks();
	SDL_Delay(3000);
	sdl_init();
	strcpy(serial.out, "C\n\r");
	rc = jbit_serial_write(&serial);
	assert(rc == 0);
	while (1) {
		rc = jbit_serial_poll(&serial, remote_handle_line);
		assert(rc == 0);
		if (SDL_GetTicks() - t > 30) {
			t = SDL_GetTicks();
			sdl_sync();
		}
	}
	rc = jbit_serial_close(&serial);
	assert(rc == 0);
}

#endif // PLATFORM_PC_SDL

#ifdef ENABLE_VM_TRACE

void vm_traces(const char *msg) {
	fprintf(stderr, "# %s\n", msg);
}

#endif

static void load_jb_file(const char *file_name) {
	uint8_t header[12];
	uint8_t *jb;
	int n, rc;
	FILE *f;
	int n_code_pages;
	int n_data_pages;

	f = fopen(file_name, "r");
	assert(f);
	rc = fread(header, 12, 1, f);
	assert(rc == 1);
	fclose(f);
	n_code_pages = header[8];
	n_data_pages = header[9];
	n = ((n_code_pages + n_data_pages) << 8);
	jb = (uint8_t *)malloc(n);
	assert(jb);
	f = fopen(file_name, "r");
	assert(f);
	rc = fread(header, 12, 1, f);
	assert(rc == 1);
	rc = fread(jb, n, 1, f);
	assert(rc == 1);
	fclose(f);
	jbit_prg_code_ptr = &jb[0];
	jbit_prg_code_size = n_code_pages << 8;
	jbit_prg_code_pages = n_code_pages;
	jbit_prg_data_ptr = &jb[jbit_prg_code_size];
	jbit_prg_data_size = n_data_pages << 8;
}

static void load_rom(const char *file_name) {
	uint8_t *rom;
	int rc;
	FILE *f;

	f = fopen(file_name, "r");
	assert(f);
	rom = (uint8_t *)malloc(0x800 + 1);
	assert(rom);
	rc = fread(rom, 1, 0x800 + 1, f);
	assert(rc > 0 && rc <= 0x800);
	fclose(f);
	jbit_rom_ptr = rom;
	jbit_rom_size = rc;
}

static void local() {
#ifdef PLATFORM_PC_SDL
	sdl_init();
#endif
	jbit_init();
	while (1) {
		jbit_step();
#ifdef PLATFORM_PC_SDL
		sdl_sync();
		SDL_Delay(80);
#endif
	}
}

static void usage() {
	printf("usage: jbembd [-r rom] [file]\n");
#ifdef PLATFORM_PC_SDL
	printf("       jbembd -S port\n");
#endif
	exit(1);
}

int main(int argc, char *argv[]) {
	int filename_i = -1;
	int i;

	jbit_rom_ptr = NULL;
	for (i = 1; i < argc; i++) {
		const char *s = argv[i];
		if (!strcmp(s, "-r")) {
			if (++i == argc)
				usage();
			load_rom(argv[i]);
#ifdef PLATFORM_PC_SDL
		} else if (!strcmp(s, "-S")) {
			if (i != 1 || argc != 3)
				usage();
			remote(argv[i + 1]);
			exit(0);
#endif
		} else if (filename_i == -1) {
			filename_i = i;
		} else {
			usage();
		}
	}
	jbit_prg_code_ptr = NULL;
	if (filename_i != -1)
		load_jb_file(argv[filename_i]);
	local();
	return 0;
}

#endif
