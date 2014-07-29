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

#ifdef PLATFORM_DESKTOP

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <SDL/SDL.h>

#include "nano.h"

void lcd_init() {
}

void lcd_write(unsigned char dc, unsigned char data) {
	static int n = 0;
	printf("%-6d %s %02X\n", n++, dc ? "DAT" : "CMD", data);
}

static void usage() {
	printf("usage: jbnano [port]\n");
	exit(1);
}

static void remote(const char *port) {
	struct termios config;
	int fd, rc, n;
	char line[64];
	
	printf("remote...\n");
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
	cfsetispeed(&config, B9600);
	cfsetospeed(&config, B9600);
	rc = tcsetattr(fd, TCSAFLUSH, &config);
	assert(rc == 0);
	n = 0;
	while (1) {
		while (1) {
			rc = read(fd, &line[n], 1);
			assert(rc != 0);
			if (rc < 0) {
				if (errno == EAGAIN)
					break;
				assert(errno == EINTR);
			} else {
				if (line[n] == '\n')
					; /* skip */
				if (line[n] == '\r') {
					int rc, data, nv;
					line[n] = '\0';
					nv = sscanf(line, "L %d %d", &rc, &data);
					if (nv == 2)
						lcd_write(rc, data);
					n = 0;
				} else {
					n++;
					assert(n < sizeof(line));
				}
			}
		}
		usleep(100);
	}
	close(fd);
}

static void local() {
	printf("local...\n");
	sim_init();
	while (1) {
		sim_step();
		usleep(1000000);
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
