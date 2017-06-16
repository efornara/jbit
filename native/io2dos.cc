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

// io2dos.cc

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include <dos.h>
#include <i86.h>
#include <conio.h>

#include "core.h"
#include "resource.h"
#include "blt08.h"

#include "libretro.h"

static void error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "E: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static void log(enum retro_log_level level, const char *fmt, ...) {
	va_list ap;
	if (level < RETRO_LOG_ERROR)
		return;
	va_start(ap, fmt);
	fprintf(stderr, "L: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

const uint8_t *blt08_buffer;

void mode13_update();

static void video_refresh(const void *data, unsigned width_, unsigned height_,
  size_t pitch) {
	blt08_buffer = (const uint8_t *)data;
	mode13_update();
}

bool quit = false;
bool esc = false;

static bool env(unsigned cmd, void *data) {
	if (cmd == RETRO_ENVIRONMENT_GET_LOG_INTERFACE) {
		retro_log_callback *l = (retro_log_callback *)data;
		l->log = log;
	} else if (cmd == RETRO_ENVIRONMENT_SET_MESSAGE) {
		quit = true;
	}
	return true;
}

static struct {
	char keyboard_id;
	bool pressed;
} key_state[] = {
	{ '0', false },
	{ '1', false },
	{ '2', false },
	{ '3', false },
	{ '4', false },
	{ '5', false },
	{ '6', false },
	{ '7', false },
	{ '8', false },
	{ '9', false },
	{ ',', false },
	{ '.', false },
	{ 0, false }
};

static void input_poll() {
	while (kbhit()) {
		int c = getch();
		if (c == 0x1b) {
			quit = true;
			esc = true;
		}
		for (int i = 0; key_state[i].keyboard_id; i++)
			if (c == key_state[i].keyboard_id)
				key_state[i].pressed = true;
	}
}

static int16_t input_state(unsigned port, unsigned device, unsigned index,
  unsigned id) {
	for (int i = 0; key_state[i].keyboard_id; i++) {
		if (device == RETRO_DEVICE_KEYBOARD && id == key_state[i].keyboard_id) {
			bool pressed = key_state[i].pressed;
			key_state[i].pressed = false;
			return pressed;
		}
	}
	return 0;
}

static Buffer jb;
static bool valid_program = false;

static void load_program(const char *s) {
	FILE *f = 0;
	if (!(f = fopen(s, "rb"))) {
		error("Failed opening file '%s'.", s);
		return;
	}
	jb.reset();
	fseek(f, 0, SEEK_END);
	long n = ftell(f);
	if (n > 1024L * 1024L) {
		error("Invalid file '%s' (size >1M).", s);
		fclose(f);
		return;
	}
	rewind(f);
	char *buf = jb.append_raw(n);
	int i = fread(buf, n, 1, f);
	fclose(f);
	if (i != 1) {
		error("Size check failed for '%s'.", s);
		return;
	}
	retro_game_info info;
	info.path = 0;
	info.data = jb.get_data();
	info.size = jb.get_length();
	info.meta = "";
	valid_program = retro_load_game(&info);
}

void mode13() {
    union REGS regs;
    regs.x.ax = 0x13;
    int86(0x10, &regs, &regs);
}

void mode03() {
    union REGS regs;
    regs.x.ax = 0x03;
    int86(0x10, &regs, &regs);
}

void vsync() {
	while (inp(0x03da) & 0x8)
		;
	while (!(inp(0x03da) & 0x8))
		;
}

void set_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
    outp(0x03c8, index);
    outp(0x03c9, red >> 2);
    outp(0x03c9, green >> 2);
    outp(0x03c9, blue >> 2);
}

void set_palette() {
    uint8_t index = blt08_palette.base_index;
    const uint8_t *rgb = blt08_palette.rgb;
    for (int i = 0; i < blt08_palette.n_of_entries; i++) {
		set_color(index++, rgb[0], rgb[1], rgb[2]);
		rgb += 3;
	}
}

void mode13_update() {
	_asm {
		push ds
		push es
		mov ax,0a000h
		mov es,ax
		mov di,320 * (200 - 128) / 2 + (320 - 128) / 2
		mov si,word ptr ss:blt08_buffer
		mov ds,word ptr ss:blt08_buffer + 2
		mov bl,128
update_line:
		mov cx,128
		rep movsb
		add di,320 - 128
		dec bl
		jnz update_line
		pop es
		pop ds
	}
}

int main(int argc, char **argv) {
	if (argc != 2) {
		error("Usage: io2sim.exe file");
		exit(1);
	}
	retro_set_environment(env);
	retro_init();
	retro_set_video_refresh(video_refresh);
	retro_set_input_poll(input_poll);
	retro_set_input_state(input_state);
	load_program(argv[1]);
	if (!valid_program)
		exit(1);
	mode13();
	atexit(mode03);
	vsync();
	set_palette();
	while (!quit) {
		retro_run();
		vsync();
	}
	if (!esc) {
		set_color(0, 0x30, 0x30, 0x30);
		getch();
		set_color(0, 0, 0, 0);
	}
	retro_deinit();
}
