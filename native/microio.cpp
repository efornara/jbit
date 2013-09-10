/*
 * Copyright (C) 2012-2013  Emanuele Fornara
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

// microio.cpp

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <curses.h>

#include "jbit.h"

namespace {

void curses_init() {
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	timeout(0);
}

extern "C" void curses_cleanup() {
	endwin();
}

class CursesDevice : public Device {
private:
	static const int REG_FRMFPS = 17;
	static const int REG_FRMDRAW = 18;
	static const int REG_RANDOM = 23;
	static const int REG_KEYBUF = 24;
	static const int REG_CONVIDEO = 40;

	static const int KEYBUF_SIZE = 8;

	static const int COLS = 10;
	static const int ROWS = 4;
	static const int CONVIDEO_SIZE = COLS * ROWS;

	static const int KEYCODE_NONE = 0;
	static const int KEYCODE_BREAK = -1;

	char video_buf[40];
	int display_y;
	int display_x;
	int cursor_y;
	int cursor_x;
	const char *status_msg;

	void show_static() {
		clear();
		int y = 0;
		int x = 18;
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

	void flush() {
		mvaddstr(0, 0, status_msg);
		int i = 0;
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 10; x++) {
				int c = video_buf[i++];
				if (!isprint(c))
					c = ' ';
				mvaddch(display_y + y, display_x + x, c);
			}
		}
		move(cursor_y, cursor_x);
		refresh();
	}

	int filter_char(int c) {
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
		case ':':
			c = KEYCODE_BREAK;
			break;
		default:
			c = KEYCODE_NONE;
		}
		return c;
	}

	void wait(int ms) {
		struct timespec ts;
		if (ms < 10)
			ms = 10;
		if (ms < 1000) {
			ts.tv_sec = 0;
			ts.tv_nsec = 1000000 * ms;
		} else {
			ts.tv_sec = ms / 1000;
			ts.tv_nsec = 1000000 * (ms % 1000);
		}
		nanosleep(&ts, NULL);
	}

public:
	CursesDevice() {
		curses_init();
		atexit(curses_cleanup);
	}
	void reset() {
		memset(video_buf, ' ', sizeof(video_buf));
		status_msg = "";
		show_static();
		flush();
	}
	void put(int address, int value) {
		if (address == REG_FRMDRAW) {
			flush();
			wait(100);
		} else if (address >= REG_CONVIDEO && address < REG_CONVIDEO + CONVIDEO_SIZE) {
			video_buf[address - REG_CONVIDEO] = value;
		}
	}
	int get(int address) {
		if (address >= REG_CONVIDEO && address < REG_CONVIDEO + CONVIDEO_SIZE) {
			return video_buf[address - REG_CONVIDEO];
		}
		return 0;
	}
	void update(int status) {
		switch (status) {
		case 0: // OK
			status_msg = "RUNNING";
			break;
		case 1: // HALTED
			status_msg = "HALTED ";
			break;
		default:
			status_msg = "FAILED ";
			break;
		}
		int c = filter_char(getch());
		if (c == KEYCODE_BREAK)
			exit(0);
		return;
	}
};

} // namespace

Device *new_CursesDevice() {
	return new CursesDevice();
}
