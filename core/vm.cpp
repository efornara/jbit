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

// vm.cpp

#include "core.h"

#include <string.h>

namespace {

class VMImpl : public VM {
private:
	static const int mem_size = 256 * 256;
	IO *io;
	unsigned char m[mem_size];
public:
	VMImpl(IO *io_) : io(io_) {}
	void reset() {
		io->reset();
		memset(m, 0, mem_size);
	}
	void put(int address, int value) {
		if (address < 0 || address >= mem_size)
			return;
		if (value < 0 || value > 255)
			return;
		m[address] = static_cast<unsigned char>(value);
	}
	int step() {
		static const int io_addr = 0x200;
		static int i = 0;

		if (io->get(io_addr + 24) != 0)
			return 0;
		io->put(io_addr + 40, '0' + (i++ % 10));
		io->put(io_addr + 18, 0);
		return 1;
	}
};

} // namespace

VM *new_VM(IO *io) {
	return new VMImpl(io);
}
