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

#include "jbit.h"

namespace {

void fatal(const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

char *load_file(const char *file_name, int *size) {
	FILE *f;
	int i = 0, j, n;
	char *buf;
	
	if ((f = fopen(file_name, "rb")) == NULL)
		fatal("cannot open file '%s'", file_name);
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	rewind(f);
	buf = new char[n];
	if (!buf)
		fatal("new failed for '%s'", file_name);
	while (i < n) {
		if ((j = fread(&buf[i], 1, n - 1, f)) == 0)
			break;
		i += j;
	}
	if (i != n)
		fatal("size check failed for '%s'", file_name);
	*size = n;
	fclose(f);
	return buf;
}

class JBFile {
private:
	char *jb;
	int size;
public:
	JBFile(const char *file_name) : jb(0) {
		jb = load_file(file_name, &size);
	}
	~JBFile() {
		delete[] jb;
	}
	void load_into_vm(VM *vm) {
		if (size < 12)
			fatal("invalid jb format (size < 12)");
		int code_pages = jb[8] & 0xFF;
		int data_pages = jb[9] & 0xFF;
		if (code_pages == 0 || code_pages + data_pages > 251)
			fatal("invalid jb format (pages)");
		if (size != 12 + (code_pages + data_pages) * 256)
			fatal("invalid jb format (size/pages mismatch)");
		int p = 0x300;
		for (int i = 12; i < size; i++)
			vm->put(p++, jb[i]);
	}
};

void load_jb_into_vm(const char *file_name, VM *vm) {
	JBFile jb(file_name);
	jb.load_into_vm(vm);
}

} // namespace

extern Device *new_CursesDevice();

int main(int argc, char *argv[])
{
	if (argc != 2)
		fatal("usage: jbit file.jb");
	Device *dev = new_CursesDevice();
	VM *vm = new_VM(dev);
	vm->reset();
	load_jb_into_vm(argv[1], vm);
	int vm_status = 0;
	while (1) {
		vm_status = vm->step();
		dev->update(vm_status);
	}
}
