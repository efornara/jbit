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

// core.h

struct Tag {
	const char *s;
	Tag(const char *s_ = 0) : s(s_) {}
	bool is_valid() const { return s != 0; }
	bool operator==(const Tag &o) const;
	bool operator==(const char *o) const;
};

class Buffer {
private:
	char *data;
	int size;
	int length;
	Buffer(const Buffer &); // copy forbidden
	Buffer& operator=(const Buffer&); // assignment forbidden
public:
	Buffer(int initial_size = 256);
	~Buffer() { delete[] data; }
	void reset() { length = 0; }
	char *append_raw(int len);
	void append_char(char c);
	void append_data(const char *p, int len);
	void append_string(const char *s);
	void append_line(const char *line);
	const char *get_data() const { return data; }
	int get_length() const { return length; }
};

class String {
private:
	Buffer *b;
	int i;
	friend struct Token;
public:
	String() : b(0), i(-1) {}
	String(Buffer *b_, const char *s) : b(b_) {
		if (s) {
			i = b->get_length();
			b->append_string(s);
		} else {
			i = -1;
		}
	}
	String(Buffer *b_, int i_) : b(b_), i(i_) {}
	const char *get_s() {
		if (i == - 1)
			return 0;
		else
			return &b->get_data()[i];
	}
};

class Program : public Buffer {
public:
	Buffer metadata_storage;
	Tag device_tag;
	int n_of_code_pages;
	int n_of_data_pages;
	Program() : n_of_code_pages(-1) {}
};

struct ParseError {
	const char *msg;
	int lineno;
	int colno;
};

const ParseError *parse_asm(const Buffer *src, Program *prg);

extern const char *get_jbit_version();

struct SymDef {
	const char *name; // last name is 0
	int value;
};

extern const char *asm_devices[]; // 0 terminated

extern const SymDef *get_device_symdefs(const char *device);
