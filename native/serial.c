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

#include "serial.h"

#include <string.h>
#include <time.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int jbit_serial_open(jbit_serial_t *ctx, const char *filename, int speed) {
	struct termios config;

	ctx->fd = open(filename, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (ctx->fd < 0)
		goto error;
	if (!isatty(ctx->fd))
		goto error;
	memset(&config, 0, sizeof(config));
	config.c_iflag = 0;
	config.c_oflag = 0;
	config.c_cflag = CS8 | CREAD | CLOCAL;
	config.c_lflag = 0;
	config.c_cc[VMIN] = 1;
	config.c_cc[VTIME] = 0;
	cfsetispeed(&config, B115200);
	cfsetospeed(&config, B115200);
	if (tcsetattr(ctx->fd, TCSAFLUSH, &config) != 0)
		goto error;
	ctx->n = 0;
	return 0;
error:
	jbit_serial_close(ctx);
	return -1;
}

int jbit_serial_close(jbit_serial_t *ctx) {
	if (ctx->fd >= 0)
		close(ctx->fd);
	ctx->fd = -1;
	return 0;
}

int jbit_serial_write(jbit_serial_t *ctx) {
	struct timespec req, rem;
	int w, n, rc;
	
	w = 0;
	if ((n = strlen(ctx->out)) > JBIT_SERIAL_BUFFER_SIZE)
		goto error;
	while (w < n) {
		rc = write(ctx->fd, &ctx->out[w], n - w);
		if (rc < 0) {
			if (errno != EAGAIN && errno != EINTR)
				goto error;
		} else if (rc == 0) {
			goto error;
		} else {
			w += rc;
		}
		req.tv_sec = 0;
		req.tv_nsec = 50000;
		nanosleep(&req, &rem);
	}
	return 0;
error:
	jbit_serial_close(ctx);
	return -1;
}

int jbit_serial_poll(jbit_serial_t *ctx, jbit_serial_read_t callback) {
	unsigned char buf[1024];
	int rc, i;

	while (1) {
		rc = read(ctx->fd, buf, sizeof(buf));
		if (rc < 0) {
			if (errno == EAGAIN || errno == EINTR)
				return 0;
			break;
		} else if (rc > 0) {
			for (i = 0; i < rc; i++) {
				char c = buf[i];
				ctx->in[ctx->n] = c;
				if (c == '\n') {
					/* skip */
				} else if (c == '\r') {
					ctx->in[ctx->n] = '\0';
					callback(ctx);
					ctx->n = 0;
				} else {
					ctx->n++;
					if (ctx->n == JBIT_SERIAL_BUFFER_SIZE)
						goto error;
				}
			}
		} else { /* rc == 0 */
			break;
		}
	}
error:
	jbit_serial_close(ctx);
	return -1;
}
