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

// main.c

#include "defs.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int status = STATUS_RUNNING;

const char *c_update(const unsigned char *video)
{
	skin_update_video(video);
	return NULL;
}

const char *c_setstatus(const char *s)
{
	if (!strcmp(s, "OK"))
		status = STATUS_RUNNING;
	else if (!strcmp(s, "HALTED"))
		status = STATUS_HALTED;
	else if (!strcmp(s, "ERROR"))
		status = STATUS_FAILED;
	else
		return "status: expected OK, HALTED or ERROR";
	skin_update_status(status);
	return NULL;
}

void fatal(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

char *load_file(const char *file_name, int *size)
{
	FILE *f;
	int i = 0, j, n;
	char *buf;
	
	if ((f = fopen(file_name, "rb")) == NULL)
		fatal("cannot open file '%s'", file_name);
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	rewind(f);
	if ((buf = malloc(size == NULL ? n + 1 : n)) == NULL)
		fatal("malloc failed for '%s'", file_name);
	while (i < n) {
		if ((j = fread(&buf[i], 1, n - 1, f)) == 0)
			break;
		i += j;
	}
	if (i != n)
		fatal("size check failed for '%s'", file_name);
	if (size == NULL)
		buf[n] = '\0';
	else
		*size = n;
	fclose(f);
	return buf;
}

char *load_res_file(const char *res_file_name, int *size)
{
	static char buf[256];
	static int prefix = 0;

	if (prefix == 0) {
		char *env = getenv("JBIT_RES");
		if (env == NULL)
			prefix = sprintf(buf, "./res/");
		else if (strlen(env) > sizeof(buf) - 20)
			fatal("JBIT_RES too long");
		else
			prefix = sprintf(buf, "%s/", env);
	}
	if (strlen(res_file_name) + prefix + 1 >= sizeof(buf))
		fatal("JBIT_RES + file_name too long");
	strcpy(&buf[prefix], res_file_name);
	return load_file(buf, size);
}

char *load_jb(const char *file_name)
{
	const char *header = "\"Size: %d code page, %d data pages.\\n\\n\" +\n";
	const char *page = "\"%c %d:0\\n\" +\n";
	const char line0[] = "\"000: 000 000 000 000 000 000 000 000\\n\" +\n";
	char *jb, *p;
	int i_p, i_jb, i_pg, size, code_pages, data_pages;

	jb = load_file(file_name, &size);
	if (size < 12)
		fatal("invalid jb format (size < 12)");
	code_pages = jb[8] & 0xFF;
	data_pages = jb[9] & 0xFF;
	if (code_pages == 0 || code_pages + data_pages > 251)
		fatal("invalid jb format (pages)");
	if (size != 12 + (code_pages + data_pages) * 256)
		fatal("invalid jb format (size/pages mismatch)");
	size = 100 + sizeof(header) +
		(code_pages + data_pages) * (sizeof(page) + 32 * (sizeof(line0) - 1));
	if ((p = malloc(size)) == NULL)
		fatal("malloc failed for program");
	i_p = 0;
	i_p += sprintf(&p[i_p], "js__load(");
	i_p += sprintf(&p[i_p], header, code_pages, data_pages);
	i_jb = 12;
	for (i_pg = 3; i_pg < 3 + code_pages + data_pages; i_pg++) {
		int i, j;
		i_p += sprintf(&p[i_p], page, 'C', i_pg);
		for (i = 0; i < 32; i++) {
			for (j = 0; j < 8; j++)
				if (jb[i_jb + j])
					break;
			if (j == 8) {
				i_jb += 8;
				continue;
			}
			i_p += sprintf(&p[i_p], "\"%03d:", i * 8);
			for (j = 0; j < 8; j++)
				i_p += sprintf(&p[i_p], " %03d", jb[i_jb++] & 0xFF);
			i_p += sprintf(&p[i_p], "\\n\" +\n");
		}
	}
	i_p += sprintf(&p[i_p], "\"\");\n");
	if (i_p > size)
		fatal("internal error (program memory overflow)");
	free(jb);
	return p;
}

const char *sim_files[] = {
	"jbit-vm.js",
	"jbit-microio.js",
	"c-sim.js",
	NULL
};

static void load_sim_files()
{
	char *buf;
	int i;

	for (i = 0; sim_files[i] != NULL; i++) {
		buf = load_res_file(sim_files[i], NULL);
		jsengine_load(buf);
		free(buf);
	}
}

static void quit(void)
{
	skin_cleanup();
	jsengine_cleanup();
}

int main(int argc, char *argv[])
{
	static struct timespec ts;
	int c, ms;
	char *prog;

	if (argc != 2)
		fatal("usage: jbit file.jb");
	prog = load_jb(argv[1]);
	atexit(quit);
	jsengine_init();
	load_sim_files();
	jsengine_load(prog);
	free(prog);
	skin_init();
	while (1) {
		skin_flush();
		c = skin_poll_key();
		if (c == KEYCODE_BREAK)
			break;
		if (c != KEYCODE_NONE) {
			char do_putkey[32];
			sprintf(do_putkey, "js__putkey(%d);", c);
			jsengine_run(do_putkey);
		}
		ms = jsengine_run("js__advance();");
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
	return 0;
}
