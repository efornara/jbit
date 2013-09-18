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
	int get_colno() { return i - start; }
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

enum TokenType {
	TK_ERROR,
	TK_BYTE,
	TK_EOL
};

struct Token {
	TokenType type;
	union {
		const char *e_value;
		int i_value;
	};
};

class Tokenizer {
private:
	LineReader &r;
	Buffer buf;
	Token eol;
	Token error(const char *msg) {
		Token token;
		token.type = TK_ERROR;
		token.e_value = msg;
		return token;
	}
public:
	Tokenizer(LineReader &r_) : r(r_), buf(32) {
		eol.type = TK_EOL;
	}
	void reset() {
	}
	const Token get() {
		Token token;
		buf.reset();
		int c = r.getc();
		if (c == LineReader::EOL || c == '#')
			return eol;
		while (c != LineReader::EOL) {
			while (isspace(c))
				c = r.getc();
			if (c == LineReader::EOL)
				break;
			while (c != LineReader::EOL && !isspace(c)) {
				buf.append_char(c);
				c = r.getc();
			}
			buf.append_char(0);
			const char *t = buf.get_data();
			char ch;
			for (int i = 0; (ch = t[i]); i++)
				if (!isdigit((int)ch))
					return error("not an integer");
			int n = atoi(t);
			if (n > 255)
				return error("byte out of range");
			token.type = TK_BYTE;
			token.i_value = n;
			return token;
		}
		return eol;
	}
};

ParseError parse_error;

class ParserEngine {
private:
	LineReader reader;
	Tokenizer tokenizer;
	void fill_base_error() {
		parse_error.lineno = reader.get_lineno();
		parse_error.colno = reader.get_colno();
	}
	const ParseError *internal_error() {
		fill_base_error();
		parse_error.msg = "internal error";
		return &parse_error;
	}
	const ParseError *token_error(const Token &token) {
		fill_base_error();
		parse_error.msg = token.e_value;
		return &parse_error;
	}
public:
	ParserEngine(const Buffer *src) : reader(src), tokenizer(reader) {}
	const ParseError *parse(Program *prg) {
		Token token;
		prg->reset();
		do {
			tokenizer.reset();
			do {
				token = tokenizer.get();
				switch (token.type) {
				case TK_BYTE:
					prg->append_char(token.i_value);
					break;
				case TK_ERROR:
					return token_error(token);
				case TK_EOL:
					break;
				default:
					return internal_error();
				}
			} while (token.type != TK_EOL);
		} while (reader.nextline());
		return 0;
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

const ParseError *parse_asm(const Buffer *src, Program *prg) {
	ParserEngine engine(src);
	return engine.parse(prg);
};
