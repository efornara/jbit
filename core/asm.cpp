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

// asm.cpp

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "core.h"

namespace {

class LineReader {
private:
	const char *d;
	int len;
	int i, start;
	int lineno;
public:
	static const int EOL = INT_MAX;
	void rewind() {
		i = 0;
		start = 0;
		lineno = 1;
	}
	int get_lineno() { return lineno; }
	LineReader(const Buffer *b) : d(b->get_data()), len(b->get_length()) {
		rewind();
	}
	int getc() {
		if (i >= len)
			return EOL;
		char c = d[i];
		if (c == 0 || c == '\n' || c == '\r')
			return EOL;
		i++;
		return c;
	}
	void unget() {
		if (i != start)
			i--;
	}
	bool nextline() {
		while (getc() != EOL)
			;
		i++;
		while (i < len && d[i] == '\r')
			i++;
		if (i >= len)
			return false;
		start = i;
		lineno++;
		return true;
	}
};

} // namespace

bool Tag::operator==(const Tag &o) const {
	if (!s)
		return !o.s;
	if (!o.s)
		return false;
	return !strcmp(s, o.s);
}

bool Tag::operator==(const char *o) const {
	if (!s)
		return !o;
	if (!o)
		return false;
	return !strcmp(s, o);
}

Buffer::Buffer(int initial_size) {
	if (initial_size <= 0)
		initial_size = 1;
	data = new char[initial_size]; // OK to crash if low memory
	size = initial_size;
	length = 0;
}

char *Buffer::append_raw(int len) {
	int new_length = length + len;
	if (new_length > size) {
		int new_size = size * 2;
		if (new_size < new_length)
			new_size = new_length;
		char *new_data = new char[new_size];
		memcpy(new_data, data, length);
		delete[] data;
		data = new_data;
		size = new_size;
	}
	char *raw = &data[length];
	length += len;
	return raw;
}

void Buffer::append_char(char c) {
	char *raw = append_raw(1);
	*raw = c;
}

void Buffer::append_data(const char *p, int len) {
	char *raw = append_raw(len);
	memcpy(raw, p, len);
}

void Buffer::append_string(const char *s) {
	int len = strlen(s) + 1;
	char *raw = append_raw(len);
	memcpy(raw, s, len);
}

void Buffer::append_line(const char *line) {
	int len = strlen(line);
	char *raw = append_raw(len + 1);
	memcpy(raw, line, len);
	raw[len] = '\n';
}

Parser::Parser(const Buffer *src_) : src(src_) {
}

const ParseError *Parser::parse(Program *prg) {
	static ParseError e;
	prg->reset();
	Buffer token(32);
	LineReader r(src);
	do {
		int c = r.getc();
		if (c == LineReader::EOL || c == '#')
			continue;
		while (c != LineReader::EOL) {
			while (isspace(c))
				c = r.getc();
			if (c == LineReader::EOL)
				break;
			token.reset();
			while (c != LineReader::EOL && !isspace(c)) {
				token.append_char(c);
				c = r.getc();
			}
			token.append_char(0);
			const char *t = token.get_data();
			char ch;
			for (int i = 0; (ch = t[i]); i++)
				if (!isdigit((int)ch))
					goto error;
			int n = atoi(t);
			if (n > 255)
				goto error;
			prg->append_char(n);
		}
	} while (r.nextline());
	return 0;
error:
	e.msg.reset();
	e.msg.append_string("parse error");
	e.lineno = r.get_lineno();
	return &e;
};

