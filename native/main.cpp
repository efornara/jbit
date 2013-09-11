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

// main.cpp

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jbit.h"

extern Device *new_CursesDevice();
extern Device *new_Xv65Device();

namespace {

void fatal(const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "jbit: ");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

void usage(int code = 1) {
	printf("\n"
	 "usage: jbit [options] file\n"
	 "\n"
	 "options:\n"
	 "  -d device\n"
	 "\n");
	exit(code);
}

Buffer *load_file(const char *file_name) {
	FILE *f;
	int i = 0, j, n;
	Buffer *buffer;
	
	if ((f = fopen(file_name, "rb")) == NULL)
		fatal("cannot open file '%s'", file_name);
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	rewind(f);
	buffer = new Buffer(n);
	if (!buffer)
		fatal("new Buffer failed for '%s'", file_name);
	char *buf = buffer->append_raw(n);
	while (i < n) {
		if ((j = fread(&buf[i], 1, n - 1, f)) == 0)
			break;
		i += j;
	}
	if (i != n)
		fatal("size check failed for '%s'", file_name);
	fclose(f);
	return buffer;
}

enum DeviceTag {
	DEV_UNKNOWN,
	DEV_MICROIO,
	DEV_XV65
};

class JBParser {
private:
	const Buffer *buffer;
	const char *jb;
	int size;
public:
	JBParser(const Buffer *buffer_) : buffer(buffer_) {
		jb = buffer->get_data();
		size = buffer->get_length();
		if (size < 12)
			fatal("invalid jb format (size < 12)");
	}
	DeviceTag parse(Buffer *program) {
		int code_pages = jb[8] & 0xFF;
		int data_pages = jb[9] & 0xFF;
		if (code_pages == 0 || code_pages + data_pages > 251)
			fatal("invalid jb format (pages)");
		int program_size = (code_pages + data_pages) * 256;
		if (size != 12 + program_size)
			fatal("invalid jb format (size/pages mismatch)");
		program->reset();
		char *raw = program->append_raw(program_size);
		memcpy(raw, &jb[12], program_size);
		if (memcmp(jb, "JBit", 4) == 0)
			return DEV_MICROIO;
		else if (memcmp(jb, "xv65", 4) == 0)
			return DEV_XV65;
		return DEV_UNKNOWN;
	}
};

void startup(const char *file_name, DeviceTag dev_tag, VM **vm, Device **dev) {
	Buffer *file = load_file(file_name);
	Parser parser(file);
	Buffer program;
	DeviceTag tag;
	if (parser.has_signature()) {
		const ParseError *e = parser.parse(&program);
		if (e)
			fatal("line %d: %s", e->lineno, e->msg.get_data());
		tag = DEV_XV65;
	} else {
		JBParser jb_parser(file);
		tag = jb_parser.parse(&program);
	}
	delete file;
	if (dev_tag != DEV_UNKNOWN)
		tag = dev_tag;
	switch (tag) {
	case DEV_MICROIO:
		(*dev) = new_CursesDevice();
		break;
	case DEV_XV65:
		(*dev) = new_Xv65Device();
		break;
	default:
		fatal("invalid jb format (signature)");
	}
	(*vm) = new_VM(*dev);
	(*vm)->reset();
	(*vm)->load(&program);
}

void run(const char *file_name, DeviceTag dev_tag) {
	VM *vm;
	Device *dev;
	startup(file_name, dev_tag, &vm, &dev);
	int vm_status = 0;
	bool dev_keepalive = true;
	while (vm_status == 0 || dev_keepalive) {
		vm_status = vm->step();
		dev_keepalive = dev->update(vm_status);
	}
}

} // namespace

int main(int argc, char *argv[])
{
	DeviceTag dev_tag = DEV_UNKNOWN;
	const char *file_name = 0;
	for (int i = 1; i < argc; i++) {
		const char *s = argv[i];
		if (!strcmp(s, "-d")) {
			if (++i == argc)
				usage();
			const char *d = argv[i];
			if (!strcmp(d, "microio"))
				dev_tag = DEV_MICROIO;
			else if (!strcmp(d, "xv65"))
				dev_tag = DEV_XV65;
			else
				fatal("unknown device '%s'", d);
		} else if (!file_name) {
			file_name = s;
		} else {
			usage();
		}
	}
	if (!file_name)
		usage();
	run(file_name, dev_tag);
}
