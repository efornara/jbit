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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "jbit.h"

#include "xv65.h"

#define IO_BASE 0x200

namespace {

extern "C" void sig_handler(int) {
}

class Xv65Device : public Device {
private:
	AddressSpace *m;
	Buffer req;
	int v_REQPTRHI;
	int v_REQRES;
	unsigned char v_REQDAT[16];
	const char *r;
	int n;
	void put_value(unsigned char *buf, int size, int64_t value) {
		int pattern = 0x01020304;
		int patter_v0 = *(unsigned char *)&pattern;
		unsigned char *v = (unsigned char *)&value;
		if (patter_v0 == 0x01) {
			for (int i = 0; i < size; i++)
				buf[i] = v[sizeof(int64_t) - 1 - i];
		} else if (patter_v0 == 0x04) {
			for (int i = 0; i < size; i++)
				buf[i] = v[i];
		} else 
			abort();
	}
	void get_value(const unsigned char *buf, int size, int64_t *value) {
		int64_t v = 0;
		if (size > 0)
			v = buf[size - 1] & 0x80 ? -1 : 0;
		for (int i = 0; i < size; i++) {
			v <<= 8;
			v |= buf[size - i - 1];
		}
		*value = v;
	}
	int r_get_uint8(int i) {
		return r[i] & 0xff;
	}
	bool r_get_trailing_int(int i, int *value) {
		int len = n - i;
		if (len < 0 || len > 8)
			return false;
		int64_t v;
		get_value((const unsigned char *)&r[i], len, &v);
		*value = (int)v;
		return true;
	}
	int m_get_uint8(int addr) {
		int res = 0;
		if (addr >= 0 && addr <= 0xffff)
			res =  m->get(addr) & 0xff;
		return res;
	}
	int m_get_uint16(int addr) {
		int res = m_get_uint8(addr++);
		res |= m_get_uint8(addr) << 8;
		return res;
	}
	int req_DEBUG() {
		fprintf(stderr, "REQ_DEBUG %d\n", n);
		return 0;
	}
	int req_SYS_fork() {
		if (n != 1)
			return 1;
		pid_t pid = fork();
		if (pid == -1)
			return 1;
		put_value(v_REQDAT, 8, pid);
		return 0;
	}
	int req_SYS_exit() {
		int status;
		if (!r_get_trailing_int(1, &status))
			return 1;
		exit(status);
		return 0; // not reached
	}
	int req_SYS_wait() {
		if (n != 1)
			return 1;
		int status;
		pid_t pid = wait(&status);
		if (pid == -1)
			return 1;
		put_value(v_REQDAT, 8, pid);
		return 0;
	}
	int req_SYS_kill() {
		int sig;
		switch (n) {
		case 1:
			sig = SIGTERM;
			break;
		case 2:
			sig = r_get_uint8(1);
			break;
		default:
			return 1;
		}
		int64_t pid;
		get_value(v_REQDAT, sizeof(pid_t), &pid);
		int ret = kill((pid_t)pid, sig);
		return ret == -1 ? 1 : 0;
	}
	int req_SYS_getpid() {
		if (n != 1)
			return 1;
		pid_t pid = getpid();
		put_value(v_REQDAT, 8, pid);
		return 0;
	}
	int req_SYS_sleep() {
		int seconds;
		if (!r_get_trailing_int(1, &seconds))
			return 1;
		int ret = sleep(seconds);
		return ret != 0 ? 1 : 0;
	}
	void request() {
		int ret = 0xff;
		r = req.get_data();
		n = req.get_length();
		if (n == 0)
			return;
		switch (*r) {
		case REQ_DEBUG:
			ret = req_DEBUG();
			break;
		case REQ_SYS_fork:
			ret = req_SYS_fork();
			break;
		case REQ_SYS_exit:
			ret = req_SYS_exit();
			break;
		case REQ_SYS_wait:
			ret = req_SYS_wait();
			break;
		case REQ_SYS_kill:
			ret = req_SYS_kill();
			break;
		case REQ_SYS_getpid:
			ret = req_SYS_getpid();
			break;
		case REQ_SYS_sleep:
			ret = req_SYS_sleep();
			break;
		}
		v_REQRES = ret;
	}
	void put_REQPTRLO(int value) {
		int addr = ((v_REQPTRHI & 0xff) << 8) | (value & 0xff);
		int len = m_get_uint16(addr);
		addr += 2;
		req.reset();
		for (int i = 0; i < len; i++)
			req.append_char(m_get_uint8(addr++));
		request();
		req.reset();
	}
public:
	Xv65Device() {
		signal(SIGALRM, sig_handler);
	}
	void set_address_space(AddressSpace *dma) {
		m = dma;
	}
	void reset() {
		req.reset();
		v_REQRES = 0;
		v_REQPTRHI = 0;
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
			req.append_char(value);
			break;
		case REQEND:
			request();
			req.reset();
			break;
		case REQPTRHI:
			v_REQPTRHI = value;
			break;
		case REQPTRLO:
			put_REQPTRLO(value);
			break;
		default:
			if (address >= REQDAT && address < REQDAT + 16)
				v_REQDAT[address - REQDAT] = value;
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
		case PIDSIZE:
			return sizeof(pid_t);
		default:
			if (address >= REQDAT && address < REQDAT + 16)
				return v_REQDAT[address - REQDAT];
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
