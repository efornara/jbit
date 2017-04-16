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

// mio.cc

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "jbit.h"
#include "devimpl.h"

#include "_xv65.h"

#define IO_BASE 0x200

class MicroIODevice : public Device {
public:
	static const int FRAME_MIN_WAIT = 10;
	static const int KEYCODE_NONE = 0;
	static const int KEYCODE_BREAK = -1;
private:
	MicroIODeviceDriver *drv;
	Random random;
	MicroIOKeybuf keybuf;
	MicroIODisplay display;
	int v_FRMFPS;
	const char *status_msg;
	void put_FRMDRAW() {
		int fps4 = v_FRMFPS;
		int ms = (int)(1000.0 / (fps4 / 4.0));
		if (ms < FRAME_MIN_WAIT)
			ms = FRAME_MIN_WAIT;
		drv->flush(status_msg, ms);
	}
	int filter_char(int c) {
		if (c == ':')
			return KEYCODE_BREAK;
		c = MicroIOKeybuf::map_keypad(c);
		return c ? c : KEYCODE_NONE;
	}
public:
	MicroIODevice() {
		status_msg = "";
		drv = new_MicroIODeviceDriver();
	}
	~MicroIODevice() {
		delete drv;
	}
	// IO
	void set_address_space(AddressSpace *dma) {
	}
	void reset() {
		random.reset();
		keybuf.reset();
		display.reset();
		v_FRMFPS = 40;
		drv->set_display(&display);
		drv->reset();
	}
	int get(int address) {
		address += IO_BASE;
		switch (address) {
		case FRMFPS:
			return v_FRMFPS;
		case RANDOM:
			return random.get();
		default:
			if (address >= KEYBUF && address < KEYBUF + MicroIOKeybuf::KEYBUF_SIZE)
				return keybuf.get(address - KEYBUF);
			else if (address >= CONVIDEO && address < CONVIDEO + MicroIODisplay::CONVIDEO_SIZE)
				return display.get(address - CONVIDEO);
		}			
		return 0;
	}
	void put(int address, int value) {
		address += IO_BASE;
		value &= 0xff;
		switch (address) {
		case FRMFPS:
			v_FRMFPS = value;
			break;
		case FRMDRAW:
			put_FRMDRAW();
			break;
		case RANDOM:
			random.put(value);
			break;
		case KEYBUF:
			keybuf.put(address - KEYBUF, value);
			break;
		default:
			if (address >= CONVIDEO && address < CONVIDEO + MicroIODisplay::CONVIDEO_SIZE)
				display.put(address - CONVIDEO, value);
		}			
	}
	// Device
	void set_args(int argc_, char **argv_) {
	}
	bool update(int status) {
		switch (status) {
		case 0: // OK
			status_msg = "RUNNING";
			break;
		case 1: // HALTED
			status_msg = "HALTED ";
			break;
		default:
			status_msg = "FAILED ";
			break;
		}
		int c = filter_char(drv->get_key());
		if (c == KEYCODE_BREAK)
			exit(0);
		if (c != KEYCODE_NONE)
			keybuf.enque(c);
		if (status)
			drv->flush(status_msg, 100);
		return true;
	}
};

static Device *new_Device(Tag tag) {
	return new MicroIODevice();
}

static DeviceEntry entry("microio", new_Device);
