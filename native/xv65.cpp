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

// xv65.cpp

#include <stdio.h>

#include "jbit.h"

#include "xv65.h"

#define IO_BASE 0x200

namespace {

class Xv65Device : public Device {
private:
	AddressSpace *m;
	Buffer req;
	int req_hi;
	const char *r;
	int n;
	int m_read_uint8(int addr) {
		int res = 0;
		if (addr >= 0 && addr <= 0xffff)
			res =  m->get(addr) & 0xff;
		return res;
	}
	int m_read_uint16(int addr) {
		int res = m_read_uint8(addr++);
		res |= m_read_uint8(addr) << 8;
		return res;
	}
	void req_DEBUG() {
		fprintf(stderr, "REQ_DEBUG %d\n", n);
	}
	void request() {
		r = req.get_data();
		n = req.get_length();
		if (n == 0)
			return;
		switch (*r) {
		case REQ_DEBUG:
			req_DEBUG();
			break;
		}
	}
	void put_REQPTRLO(int value) {
		int addr = ((req_hi & 0xff) << 8) | (value & 0xff);
		int len = m_read_uint16(addr);
		addr += 2;
		req.reset();
		for (int i = 0; i < len; i++)
			req.append_char(m_read_uint8(addr++));
		request();
		req.reset();
	}
public:
	void set_address_space(AddressSpace *dma) {
		m = dma;
	}
	void reset() {
		req.reset();
		req_hi = 0;
	}
	void put(int address, int value) {
		value &= 0xff;
		switch (address + IO_BASE) {
		case PUTCHAR:
			putchar(value);
			break;
		case REQPUT:
			req.append_char(value);
			break;
		case REQEND:
			request();
			req.reset();
			break;
		case REQPTRHI:
			req_hi = value;
			break;
		case REQPTRLO:
			put_REQPTRLO(value);
			break;
		}
	}
	int get(int address) {
		switch (address + IO_BASE) {
		case REQPTRHI:
			return req_hi;
		default:
			return 0;
		}
	}
	bool update(int status) {
		return false;
	}
};

Device *new_Device() {
	return new Xv65Device();
}

DeviceEntry entry("xv65", new_Device);

} // namespace

