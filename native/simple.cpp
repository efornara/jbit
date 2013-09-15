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

// simple.cpp

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

class SimpleDevice : public Device {
private:
	const static int top_memory = 0xffff;
	AddressSpace *m;
	int argc;
	char **argv;
	Buffer req_buf;
	Random random;
	int v_REQPTRHI;
	int v_REQRES;
	int v_REQERRNO;
	int v_FRMFPS;
	unsigned char v_REQDAT[16];
	const char *r;
	int n;
	void put_value(unsigned char *buf, int size, uint64_t value) {
		int pattern = 0x01020304;
		int patter_v0 = *(unsigned char *)&pattern;
		unsigned char *v = (unsigned char *)&value;
		if (patter_v0 == 0x01) {
			for (int i = 0; i < size; i++)
				buf[i] = v[sizeof(uint64_t) - 1 - i];
		} else if (patter_v0 == 0x04) {
			for (int i = 0; i < size; i++)
				buf[i] = v[i];
		} else 
			abort();
	}
	void get_value(const unsigned char *buf, int size, uint64_t *value) {
		uint64_t v = 0;
		for (int i = 0; i < size; i++) {
			v <<= 8;
			v |= buf[size - i - 1];
		}
		*value = v;
	}
	int r_get_uint8(int i) {
		return r[i] & 0xff;
	}
	int r_get_uint16(int i) {
		return (r_get_uint8(i + 1) << 8) | r_get_uint8(i);
	}
	bool r_get_trailing_int(int i, int *value) {
		int len = n - i;
		if (len < 0 || len > 8)
			return false;
		uint64_t v;
		get_value((const unsigned char *)&r[i], len, &v);
		*value = (int)v;
		return true;
	}
	int r_parse_string(int i) {
		for (; i < n; i++)
			if (r[i] == 0)
				return i + 1; // pos after string
		return -1;
	}
	int m_get_uint8(int addr) {
		int res = 0;
		if (addr >= 0 && addr <= top_memory)
			res =  m->get(addr) & 0xff;
		return res;
	}
	int m_get_uint16(int addr) {
		return (m_get_uint8(addr + 1) << 8) | m_get_uint8(addr);
	}
	int req_ARGC() {
		if (n != 1)
			return ERR;
		put_value(v_REQDAT, 8, argc);
		return 0;
	}
	int put_string(int addr, int len, const char *s) {
		if (addr + len > top_memory)
			return ERR;
		int str_len = strlen(s) + 1;
		put_value(v_REQDAT, 8, str_len);
		if (len >= str_len) {
			for (int i = 0; s[i]; i++)
				m->put(addr + i, s[i]);
			m->put(addr + str_len, 0);
		}
		return 0;
	}
	int req_ARGV() {
		if (n < 5)
			return ERR;
		int addr = r_get_uint16(1);
		int len = r_get_uint16(3);
		int i;
		if (!r_get_trailing_int(5, &i))
			return ERR;
		if (i < 0 || i >= argc)
			return XV65_EDOM;
		return put_string(addr, len, argv[i]);
	}
	int req_ENV() {
		if (n <= 5)
			return ERR;
		int addr = r_get_uint16(1);
		int len = r_get_uint16(3);
		int i = r_parse_string(5);
		if (i == -1)
			return ERR;
		const char *name = &r[5];
		if (n - i != 0)
			return ERR;
		const char *value = getenv(name);
		if (!value)
			value = "";
		return put_string(addr, len, value);
	}
	void request() {
		int ret = ERR;
		r = req_buf.get_data();
		n = req_buf.get_length();
		int id = -1;
		if (n > 0)
			id = *r;
		switch (id) {
		case REQ_ARGC:
			ret = req_ARGC();
			break;
		case REQ_ARGV:
			ret = req_ARGV();
			break;
		case REQ_ENV:
			ret = req_ENV();
			break;
		}
		v_REQRES = ret;
		if (ret != 0)
			v_REQERRNO = ret;
		req_buf.reset();
	}
	void put_REQPTRLO(int value) {
		int addr = (v_REQPTRHI << 8) | value;
		int len = m_get_uint16(addr);
		addr += 2;
		req_buf.reset();
		for (int i = 0; i < len; i++)
			req_buf.append_char(m_get_uint8(addr++));
		request();
	}
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
		m = dma;
	}
	void reset() {
		req_buf.reset();
		random.reset();
		v_REQRES = 0;
		v_REQPTRHI = 0;
		v_REQERRNO = 0;
		v_FRMFPS = 0;
		memset(v_REQDAT, 0, sizeof(v_REQDAT));
	}
	void put(int address, int value) {
		address += IO_BASE;
		value &= 0xff;
		switch (address) {
		case PUTCHAR:
			putchar(value);
			break;
		case REQPUT:
			req_buf.append_char(value);
			break;
		case REQEND:
			request();
			break;
		case REQPTRHI:
			v_REQPTRHI = value;
			break;
		case REQPTRLO:
			put_REQPTRLO(value);
			break;
		case REQERRNO:
			v_REQERRNO = value;
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
		case RANDOM:
			random.put(value);
			break;
		default:
			if (address >= REQDAT && address < REQDAT + 16) {
				v_REQDAT[address - REQDAT] = value;
			}
			break;
		}
	}
	int get(int address) {
		address += IO_BASE;
		switch (address) {
		case REQRES:
			return v_REQRES;
		case REQPTRHI:
			return v_REQPTRHI;
		case REQERRNO:
			return v_REQERRNO;
		case FRMFPS:
			return v_FRMFPS;
		case RANDOM:
			return random.get();
			break;
		default:
			if (address >= REQDAT && address < REQDAT + 16) {
				return v_REQDAT[address - REQDAT];
			}
			return 0;
		}
	}
	// Device
	void set_args(int argc_, char **argv_) {
		argc = argc_;
		argv = argv_;
	}
	bool update(int status) {
		return false;
	}
};

Device *new_Device() {
	return new SimpleDevice();
}

DeviceEntry entry("simple", new_Device);

} // namespace
