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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <unistd.h>

#include "jbit.h"
#include "serial.h"

namespace {

const char *p;
int len;

char cmd[128];

int rc;

void get_program(const Program *prg) {
	p = prg->get_data();
	int i, n = (prg->n_of_code_pages + prg->n_of_data_pages) * 256;
	for (i = n ; i > 0; i--)
		if (p[i - 1])
			break;
	len = i;
}

jbit_serial_t serial;

void serial_open(const char *port) {
	rc = jbit_serial_open(&serial, port, 115200);
	assert(rc == 0);
}

void serial_write() {
	rc = jbit_serial_write(&serial);
	assert(rc == 0);
//	printf("> %s", serial.out);
}

int state = -1;

void set_cmd(const char *format...) {
	va_list ap;

	va_start(ap, format);
	vsnprintf(cmd, sizeof(cmd), format, ap);
	va_end(ap);
	strcpy(serial.out, cmd);
	strcat(serial.out, "\n\r");
}

void set_cmd_from_state() {
	if (state == -1) {
		set_cmd("P %d", len);
	} else if (state >= 0 && state < len) {
		set_cmd("B %d %d", state, p[state] & 0xff);
	} else {
		printf(" ...done.\n");
		exit(0);
	}
}

void handle_line(jbit_serial_t *ctx) {
//	printf("< %s", serial.in);
	if (serial.in[0] == '#') {
		printf("%s\n", serial.in);
		return;
	}
	if (!strcmp(cmd, serial.in)) {
		if (state == -1)
			printf("Sending %d bytes...\n", len);
		state++;
	}
}

} // namespace

void send_file(const Program *prg, const char *port) {
	get_program(prg);
	serial_open(port);
	printf("waiting for arduino to reset...\n");
	usleep(3000000);
	set_cmd("C");
	serial_write();
	while (1) {
		int old_state = state;
		set_cmd_from_state();
		serial_write();
		for (int spin = 0; spin < 100; spin++) {
			rc = jbit_serial_poll(&serial, handle_line);
			assert(rc == 0);
			if (state > old_state)
				break;
			usleep(50);
		}
	}
}
