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
#include <errno.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <SDL/SDL.h>

#include "nano.h"

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
	const char *title = "JBit Nano";

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

static void usage() {
	printf("usage: jbnano [port]\n");
	exit(1);
}

static int fd;

static void remote_send_keypad_state() {
	char buf[32];
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
	write(fd, buf, 16);
}

static void remote_handle_char(char c) {
	static const int profile_error_rate = 0;
	static int n = 0;
	static long long tot = 0, err = 0;
	static char line[124];

	line[n] = c;
	if (c == '\n') {
		/* skip */
	} else if (c == '\r') {
		int rc, data, nv, done = 0;
		char dirty;
		line[n] = '\0';
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
		n = 0;
	} else {
		n++;
		assert(n < sizeof(line));
	}
}

static void remote(const char *port) {
	struct termios config;
	int rc, i;
	char buf[1024];
	Uint32 t;
	
	printf("remote\n");
	fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	assert(fd >= 0);
	assert(isatty(fd));
	memset(&config, 0, sizeof(config));
	config.c_iflag = 0;
	config.c_oflag = 0;
	config.c_cflag = CS8 | CREAD | CLOCAL;
	config.c_lflag = 0;
	config.c_cc[VMIN] = 1;
	config.c_cc[VTIME] = 0;
	cfsetispeed(&config, B115200);
	cfsetospeed(&config, B115200);
	rc = tcsetattr(fd, TCSAFLUSH, &config);
	assert(rc == 0);
	printf("waiting for arduino to reset...\n");
	t = SDL_GetTicks();
	SDL_Delay(3000);
	sdl_init();
	write(fd, "C\n\r", 3);
	while (1) {
		while (1) {
			rc = read(fd, buf, sizeof(buf));
			assert(rc != 0);
			if (rc < 0) {
				if (errno == EAGAIN)
					break;
				assert(errno == EINTR);
			} else {
				for (i = 0; i < rc; i++)
					remote_handle_char(buf[i]);
			}
		}
		if (SDL_GetTicks() - t > 30) {
			t = SDL_GetTicks();
			sdl_sync();
		}
	}
	close(fd);
}

static void local() {
	printf("local\n");
	sdl_init();
	sim_init();
	while (1) {
		sim_step();
		sdl_sync();
		SDL_Delay(80);
	}
}

int main(int argc, char *argv[]) {
	switch (argc) {
	case 1:
		local();
		break;
	case 2:
		remote(argv[1]);
		break;
	default:
		usage();
	}
	return 0;
}

#endif
