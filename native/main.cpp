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

#include <unistd.h>

#include "jbit.h"

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
	 "       jbit [options] -a file arg...\n"
	 "\n"
	 "options:\n"
	 "  -d device     override device selection\n"
	 "  -c jb|asm     convert file (warning: output to stdout)\n"
	 "\n"
	 "devices:\n");
	DeviceRegistry *reg = DeviceRegistry::get_instance();
	int n = reg->get_n();
	if (n == 0)
	printf("  - none -\n");
	for (int i = 0; i < n; i++)
		printf("  %s\n", reg->get(i)->tag.s);
	printf("\n");
	exit(code);
}

const char *resolve_file_name(const char *name, Buffer *buffer) {
	if (name[0] == '/')
		return name;
	const char *c_env = getenv("JBIT_PATH");
	if (!c_env)
		return name;
	int size = strlen(c_env) + 1;
	Buffer env_buf(size);
	char *env = env_buf.append_raw(size);
	memcpy(env, c_env, size);
	const char *dir = strtok(env, ":");
	do {
		int n = strlen(dir);
		if (n == 0)
			continue;
		buffer->reset();
		buffer->append_data(dir, n);
		if (dir[n - 1] != '/')
			buffer->append_char('/');
		buffer->append_string(name);
		const char *file_name = buffer->get_data();
		if (access(file_name, F_OK) == 0) // less surprises than R_OK
			return file_name;
	} while ((dir = strtok(NULL, ":")));
	return name;
}

Buffer *load_file(const char *name) {
	FILE *f;
	int i = 0, j, n;
	Buffer *buffer = new Buffer();
	
	const char *file_name = resolve_file_name(name, buffer);
	if ((f = fopen(file_name, "rb")) == NULL)
		fatal("cannot open file '%s'", file_name);
	buffer->reset();
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	if (n > 1024 * 1024)
		fatal("invalid file '%s' (size >1M)", file_name);
	rewind(f);
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

Tag AsmFmtTag("asm");
Tag JBFmtTag("jb");

Tag MicroIODevTag("microio");
Tag Xv65DevTag("xv65");

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
	void parse(Program *prg) {
		int code_pages = jb[8] & 0xFF;
		int data_pages = jb[9] & 0xFF;
		if (code_pages == 0 || code_pages + data_pages > 251)
			fatal("invalid jb format (pages)");
		int program_size = (code_pages + data_pages) * 256;
		if (size != 12 + program_size)
			fatal("invalid jb format (size/pages mismatch)");
		prg->reset();
		char *raw = prg->append_raw(program_size);
		memcpy(raw, &jb[12], program_size);
		if (memcmp(jb, "JBit", 4) == 0)
			prg->device_tag = MicroIODevTag;
		else if (memcmp(jb, "xv65", 4) == 0)
			prg->device_tag = Xv65DevTag;
	}
};

void parse(const char *file_name, Tag dev_tag, Program *prg) {
	Buffer *file = load_file(file_name);
	Parser parser(file);
	if (parser.has_signature()) {
		const ParseError *e = parser.parse(prg);
		if (e)
			fatal("line %d: %s", e->lineno, e->msg.get_data());
	} else {
		JBParser jb_parser(file);
		jb_parser.parse(prg);
	}
	delete file;
	if (dev_tag.is_valid())
		prg->device_tag = dev_tag;
}

void startup(const char *file_name, Tag dev_tag, VM **vm, Device **dev) {
	Program prg;
	parse(file_name, dev_tag, &prg);
	if (!prg.device_tag.is_valid()) {
#ifdef __WIN32__
		const char *def_device = "simple";
#else
		const char *def_device = "xv65";
#endif
		prg.device_tag = Tag(def_device);
	}
	const DeviceEntry *dev_entry = DeviceRegistry::get_instance()->get(prg.device_tag);
	if (!dev_entry)
		fatal("device '%s' not available", prg.device_tag.s);
	(*dev) = dev_entry->new_Device();
	(*vm) = new_VM(*dev);
	(*vm)->reset();
	(*vm)->load(&prg);
}

void run(int argc, char *argv[], Tag dev_tag) {
	VM *vm;
	Device *dev;
	startup(argv[0], dev_tag, &vm, &dev);
	dev->set_args(argc, argv);
	int vm_status = 0;
	bool dev_keepalive = true;
	while (vm_status == 0 || dev_keepalive) {
		vm_status = vm->step();
		dev_keepalive = dev->update(vm_status);
	}
}

void convert_to_asm(const Program *prg) {
	static const int bytes_per_line = 16;
	printf("#! /usr/bin/env jbit");
	if (prg->device_tag.is_valid())
		printf(" -d %s", prg->device_tag.s);
	printf(" -a\n\n");
	const char *sep = 0, *space = " ";
	for (int i = 0; i < prg->get_length(); i++) {
		sep = (i % bytes_per_line == (bytes_per_line - 1)) ? "\n" : space;
		printf("%d%s", prg->get_data()[i] & 0xff, sep);
	}
	if (sep == space)
		printf("\n");
}

void convert_to_jb(const Program *prg) {
	if (prg->device_tag == MicroIODevTag)
		printf("JBit");
	else if (prg->device_tag == Xv65DevTag)
		printf("xv65");
	else
		fatal("convert: unknown device");
	int n_of_pages = (prg->get_length() + 255) >> 8;
	putchar(0); // header size
	putchar(12);
	putchar(1); // version
	putchar(0);
	putchar(n_of_pages); // code pages
	putchar(0); // data pages
	putchar(0); // reserved
	putchar(0);
	int i;
	for (i = 0; i < prg->get_length(); i++)
		putchar(prg->get_data()[i]);
	for (; i & 0xff; i++)
		putchar(0);
}

void convert(const char *file_name, Tag dev_tag, Tag fmt_tag) {
	Program prg;
	parse(file_name, dev_tag, &prg);
	if (fmt_tag == AsmFmtTag)
		convert_to_asm(&prg);
	else if (fmt_tag == JBFmtTag)
		convert_to_jb(&prg);
	else
		fatal("internal error (convert)");
}

} // namespace

DeviceRegistry *DeviceRegistry::get_instance() {
	static DeviceRegistry *registry = 0;
	if (!registry)
		registry = new DeviceRegistry();
	return registry;
}

void DeviceRegistry::add(const DeviceEntry *entry) {
	if (n == max_n_of_entries)
		fatal("too many devices");
	devices[n++] = entry;
}

const DeviceEntry *DeviceRegistry::get(Tag tag) {
	for (int i = 0; i < n; i++)
		if (devices[i]->tag == tag)
			return devices[i];
	return 0;
}

int main(int argc, char *argv[])
{
	Tag fmt_tag;
	Tag dev_tag;
	int filename_i = -1;
	for (int i = 1; i < argc; i++) {
		const char *s = argv[i];
		if (!strcmp(s, "-d")) {
			if (++i == argc)
				usage();
			const char *d = argv[i];
			if (DeviceRegistry::get_instance()->get(d))
				dev_tag = Tag(d);
			else
				fatal("unknown device '%s'", d);
		} else if (!strcmp(s, "-c")) {
			if (++i == argc)
				usage();
			const char *f = argv[i];
			if (!strcmp(f, "asm") || !strcmp(f, "jb"))
				fmt_tag = Tag(f);
			else
				fatal("unknown conversion format '%s'", f);
		} else if (!strcmp(s, "-a") && filename_i == -1) {
			if (++i == argc)
				usage();
			filename_i = i;
			break;
		} else if (filename_i == -1) {
			filename_i = i;
		} else {
			usage();
		}
	}
	if (filename_i == -1)
		usage();
	if (fmt_tag.is_valid())
		convert(argv[filename_i], dev_tag, fmt_tag);
	else
		run(argc - filename_i, &argv[filename_i], dev_tag);
}
