/*
 * Copyright (C) 2012-2017  Emanuele Fornara
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

// std.cc

#include <stdio.h>

#include "jbit.h"
#include "devparts.h"

#include "_std.h"

#define IO_BASE 0x200

class StdDevice : public Device {
private:
	Random random;
	int std_c;
public:
	// IO
	void set_address_space(AddressSpace *dma) {
	}
	void reset() {
		random.reset();
		std_c = EOF;
	}
	void put(int address, int value) {
		address += IO_BASE;
		value &= 0xff;
		switch (address) {
		case PUTCHAR:
			putchar(value);
			break;
		case GETCHAR:
			std_c = getchar();
			break;
		case PUTUINT8:
			printf("%d", value);
			break;
		case RANDOM:
			random.put(value);
			break;
		}
	}
	int get(int address) {
		address += IO_BASE;
		switch (address) {
		case GETCHAR:
			return std_c & 0xff;
		case GETEOF:
			return (feof(stdin) || ferror(stdin)) ? 255 : 0;
		case RANDOM:
			return random.get();
		default:
			return 0;
		}
	}
	// Device
	void set_args(int argc_, char **argv_) {
	}
	bool update(int status) {
		return false;
	}
};

static Device *new_Device(Tag tag) {
	return new StdDevice();
}

static DeviceEntry entry("std", new_Device);
