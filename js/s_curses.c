/*
 * Copyright (C) 2012  Emanuele Fornara
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

// s_curses.c

#include "defs.h"

#include <string.h>
#include <ctype.h>
#include <curses.h>

static int initialized = 0;
static int sdl = 0;
static int display_y;
static int display_x;
static int cursor_y;
static int cursor_x;
static char video_buf[40];
static const char *status_msg;

static void show_static()
{
	int y, x;

	clear();
	y = 0;
	x = 18;
	mvaddstr(y++, x, "[:] break");
	y++;
	x = 7;
	display_y = y + 1;
	display_x = x + 1;
	mvaddstr(y++, x, "+----------+");
	mvaddstr(y++, x, "|          |");
	mvaddstr(y++, x, "|          |");
	mvaddstr(y++, x, "|          |");
	mvaddstr(y++, x, "|          |");
	mvaddstr(y++, x, "+----------+");
	x = 0;
	y++;
	mvaddstr(y++, x, "[1]      [2] abc  [3] def");
	y++;
	mvaddstr(y++, x, "[4] ghi  [5] jkl  [6] mno");
	y++;
	mvaddstr(y++, x, "[7] pqrs [8] tuv  [9] wxyz");
	y++;
	mvaddstr(y++, x, "[*]      [0]  +   [#]    ");
	y++;
	cursor_y = y;
	cursor_x = 0;
}

void skin_init(void)
{
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	timeout(0);
	memset(video_buf, ' ', sizeof(video_buf));
	status_msg = "";
	show_static();
	initialized = 1;
}

void skin_cleanup(void)
{
	if (sdl) {
		sdl_hide();
		sdl = 0;
	}
	if (initialized)
		endwin();
	initialized = 0;
}

#define OX ((FRAME_WIDTH - (10 * CHAR_WIDTH)) / 2)
#define OY ((FRAME_HEIGHT - (4 * CHAR_HEIGHT)) / 2)

static void draw_frame(uint32_t *f)
{
	int i, y, x;

	frame_clear(f, 0x80ffe0ff);
	for (y = 0, i = 0; y < 4; y++)
		for (x = 0; x < 10; x++, i++)
			frame_putchar(f, OX + x * CHAR_WIDTH, OY + y * CHAR_HEIGHT,
			  video_buf[i], 0x000000ff, 0x60e0c0ff);
}

void skin_flush(void)
{
	int i, y, x, c;

	mvaddstr(0, 0, status_msg);
	i = 0;
	for (y = 0; y < 4; y++) {
		for (x = 0; x < 10; x++) {
			c = video_buf[i++];
			if (!isprint(c))
				c = ' ';
			mvaddch(display_y + y, display_x + x, c);
		}
	}
	move(cursor_y, cursor_x);
	refresh();
	if (sdl) {
		uint32_t f[FRAME_WIDTH * FRAME_HEIGHT];
		draw_frame(f);
		sdl_flush(f);
	}
}

void skin_update_video(const unsigned char *video)
{
	memcpy(video_buf, video, sizeof(video_buf));
}

static int filter_char(int c)
{
	switch (c) {
	case 'a':
	case 'b':
	case 'c':
		c = '2';
		break;
	case 'd':
	case 'e':
	case 'f':
		c = '3';
		break;
	case 'g':
	case 'h':
	case 'i':
		c = '4';
		break;
	case 'j':
	case 'k':
	case 'l':
		c = '5';
		break;
	case 'm':
	case 'n':
	case 'o':
		c = '6';
		break;
	case 'p':
	case 'q':
	case 'r':
	case 's':
		c = '7';
		break;
	case 't':
	case 'u':
	case 'v':
		c = '8';
		break;
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		c = '9';
		break;
	case '+':
		c = '0';
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '*':
	case '0':
	case '#':
		break;
	case 'D': // display
		if (sdl)
			sdl = sdl_hide();
		else
			sdl = sdl_show();
		break;
	case 'S': { // screenshot
		uint32_t f[FRAME_WIDTH * FRAME_HEIGHT];
		draw_frame(f);
		frame_screenshot(f, "out.ppm");
		} break;
	case ':':
		c = KEYCODE_BREAK;
		break;
	default:
		c = KEYCODE_NONE;
	}
	return c;
}

int skin_poll_key(void)
{
	if (sdl)
		sdl = sdl_poll_show();
	return filter_char(getch());
}

void skin_update_status(int status)
{
	switch (status) {
	case STATUS_RUNNING:
		status_msg = "RUNNING";
		break;
	case STATUS_HALTED:
		status_msg = "HALTED ";
		break;
	case STATUS_FAILED:
		status_msg = "FAILED ";
		break;
	}
}
