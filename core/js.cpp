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

// js.cpp

#include <string.h>

#include <emscripten.h>

namespace {

const int IO = 0x200;

int a;
char c;

inline void io_put(int addr, int value) {
	asm("JBIT.io_put(%0, %1);"
	    : /* output */
	    : "n"(addr), "n"(value) /* input */
	    : /* clobbered registers */
	    );
}

inline int io_get(int addr) {
	int value;
	asm("%1 = JBIT.io_get(%0);"
	    : "=r"(value) /* output */
	    : "n"(addr) /* input */
	    : /* clobbered registers */
	    );
	return value;
}

};

extern "C" {

void vm_load(const char *code) {
	a = strlen(code);
	c = 'A';
}

int vm_step() {
	if (c > 'Z' || io_get(IO + 24) != 0)
		return 0;
	io_put(IO + 40, c++);
	io_put(IO + 18, a);
	return 1;
}

} // extern "C"
