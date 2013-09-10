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

#include "core.h"

#include <emscripten.h>

namespace {

const int io_addr = 0x200;
IO *io = 0;
VM *vm = 0;

class IOImpl : public IO {
	void reset() {
		asm("JBIT.io_reset();"
		    : /* output */
		    : /* input */
		    : /* clobbered registers */
		    );
	}
	void put(int address, int value) {
		asm("JBIT.io_put(%0, %1);"
		    : /* output */
		    : "n"(address), "n"(value) /* input */
		    : /* clobbered registers */
		    );
	}
	int get(int address) {
		int value;
		asm("%1 = JBIT.io_get(%0);"
		    : "=r"(value) /* output */
		    : "n"(address) /* input */
		    : /* clobbered registers */
		    );
		return value;
	}
};

} // namespace

extern "C" {

void vm_reset() {
	if (!io)
		io = new IOImpl();
	if (!vm)
		vm = new_VM(io);
	vm->reset();
}

void vm_put(int address, int value) {
	asm("console.log('vm_put(' + %0 + ', ' + %1 + ')');"
	    : /* output */
	    : "n"(address), "n"(value) /* input */
	    : /* clobbered registers */
	    );
	if (vm)
		vm->put(address, value);
}

int vm_step() {
	if (vm)
		return vm->step();
	return 0;
}

} // extern "C"
