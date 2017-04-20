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

// jbit.h

#include "core.h"

class AddressSpace {
public:
	virtual void put(int address, int value) = 0;
	virtual int get(int address) = 0;
	virtual ~AddressSpace() {}
};

class IO : public AddressSpace {
public:
	virtual void set_address_space(AddressSpace *dma) = 0;
	virtual void reset() = 0;
	virtual ~IO() {}
};

class VM {
public:
	virtual void reset() = 0;
	virtual void load(const Program *prg) = 0;
	virtual int step() = 0;
	virtual ~VM() {}
};

extern VM *new_VM(IO *io);

class Device : public IO {
public:
	virtual void set_args(int argc, char **argv) = 0;
	virtual bool update(int status) = 0;
	virtual ~Device() {}
};

struct DeviceEntry;

class DeviceRegistry {
private:
	static const int max_n_of_entries = 8;
	const DeviceEntry *devices[max_n_of_entries];
	int n;
	DeviceRegistry() : n(0) {}
public:
	static DeviceRegistry *get_instance();
	void add(const DeviceEntry *entry);
	int get_n() { return n; }
	const DeviceEntry *get(int i) { return devices[i]; }
	const DeviceEntry *get(Tag tag);
};

typedef Device *(*NewDeviceFn)(Tag tag);

struct DeviceEntry {
	Tag tag;
	NewDeviceFn new_Device;
	DeviceEntry(const char *t, NewDeviceFn f) : tag(t), new_Device(f)  {
		DeviceRegistry::get_instance()->add(this);
	}
};

// IO2 might become a Device later on. For now, get a chance to review arch.
class IO2 : public IO {
protected:
	// fixed for easy development, but a TV-friendly 320x180 is also likely
	static const int width = 128;
	static const int height = 128;
public:
	int get_width() const { return width; }
	int get_height() const { return height; }
	virtual const void *get_framebuffer() = 0;
	virtual void keypress(int key) = 0;
	virtual bool update(int dt_us) = 0;
	virtual ~IO2() {}
};

extern IO2 *new_IO2();
