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

// miodos.cc

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "jbit.h"
#include "devimpl.h"

void text();
void clear();
void waitms(int ms);
void move(int y, int x);
void mvaddstr(int y, int x, const char *msg);
int getch();

#include <dpmi.h>

void text() {
	__dpmi_regs r;
	memset(&r, 0, sizeof r);
	r.x.ax = 3;
	__dpmi_int(0x10, &r);
}

void clear() {
	__dpmi_regs r;
	memset(&r, 0, sizeof r);
	r.x.ax = 3;
	__dpmi_int(0x10, &r);
	memset(&r, 0, sizeof r);
	r.x.ax = 0x0100;
	r.x.cx = 0x2607;
	__dpmi_int(0x10, &r);
}

void waitms(int ms) {
	__dpmi_regs r;
	memset(&r, 0, sizeof r);
	int usec = ms * 1000;
	r.x.ax = 0x8600;
	r.x.cx = usec >> 16;
	r.x.dx = usec & 0xffff;
	__dpmi_int(0x15, &r);
}

void move(int y, int x) {
	__dpmi_regs r;
	memset(&r, 0, sizeof r);
	r.x.ax = 0x0200;
	r.x.dx = (y << 8) + x;
	__dpmi_int(0x10, &r);
}

void mvaddstr(int y, int x, const char *msg) {
	__dpmi_regs r;
	memset(&r, 0, sizeof r);
	move(y, x);
	for (int i = 0; msg[i]; i++) {
		r.x.ax = 0x0200;
		r.x.dx = msg[i];
		__dpmi_int(0x21, &r);
	}
}

int getch() {
	__dpmi_regs r;
	memset(&r, 0, sizeof r);
	r.x.ax = 0x0100;
	__dpmi_int(0x16, &r);
	if (!(r.x.flags & 0x40)) {
		memset(&r, 0, sizeof r);
		r.x.ax = 0x0000;
		__dpmi_int(0x16, &r);
		return r.x.ax & 0xff;
	}
	return 0;
}

class DOSMicroIODeviceDriver : public MicroIODeviceDriver {
private:
	const MicroIODisplay *display;
	int display_y;
	int display_x;
	int cursor_y;
	int cursor_x;
	void show_static() {
		clear();
		int y = 0;
		int x = 18;
		mvaddstr(y++, x, "[:] break");
		y++;
		x = 7;
		display_y = y;
		display_x = x;
		for (int i = 0; i < MicroIODisplay::N_OF_LINES; i++)
			mvaddstr(y++, x, display->get_line(i));
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
	void flush(const char *status) {
		mvaddstr(0, 0, status);
		for (int i = 0; i < MicroIODisplay::N_OF_LINES; i++)
			mvaddstr(display_y + i, display_x, display->get_line(i));
		move(cursor_y, cursor_x);
	}
	void wait(int ms) {
		waitms(ms);
	}
public:
	DOSMicroIODeviceDriver() {
		clear();
		atexit(text);
	}
	// MicroIODeviceDriver
	void set_display(const MicroIODisplay *display_) {
		display = display_;
	}
	void reset() {
		show_static();
		flush("");
	}
	void flush(const char *status, int ms) {
		flush(status);
		wait(ms);
	}
	int get_key() {
		return getch();
	}
};

MicroIODeviceDriver *new_MicroIODeviceDriver() {
	return new DOSMicroIODeviceDriver();
}
