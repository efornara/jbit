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

// tty.cpp

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef __WIN32__
#include <windows.h>
#endif

#include "jbit.h"

#include "xv65.h"

#define IO_BASE 0x200
#define ERR XV65_EINVAL

namespace {

class TTYDevice : public Device {
private:
	int v_FRMFPS;
	void put_FRMDRAW() {
		fflush(stdout);
		if (v_FRMFPS) {
			double fps = v_FRMFPS / 4.0;
			int ms = (int)(1000 / fps);
#ifdef __WIN32__
			Sleep(ms);
#else
			struct timespec ts;
			ts.tv_sec = ms / 1000;
			ts.tv_nsec = 1000000 * (ms % 1000);
			nanosleep(&ts, NULL);
#endif
		}
	}
public:
	// IO
	void set_address_space(AddressSpace *dma) {
	}
	void reset() {
		v_FRMFPS = 0;
	}
	void put(int address, int value) {
		address += IO_BASE;
		value &= 0xff;
		switch (address) {
		case PUTCHAR:
			putchar(value);
			break;
		case FRMFPS:
			v_FRMFPS = value;
			break;
		case FRMDRAW:
			put_FRMDRAW();
			break;
		case PUTUINT8:
			printf("%d", value);
			break;
		}
	}
	int get(int address) {
		address += IO_BASE;
		switch (address) {
		case FRMFPS:
			return v_FRMFPS;
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

Device *new_Device() {
	return new TTYDevice();
}

DeviceEntry entry("tty", new_Device);

} // namespace
