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

#include <stdio.h>
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
	TK_WORD,
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
		Token err;
		err.type = TK_ERROR;
		err.e_value = msg;
		return err;
	}
	bool iscomment(int c) {
		return c == '#' || c == ';';
	}
	const char *parse_dec(int *value) {
		if (buf.get_length() == 0)
			return "expected byte";
		buf.append_char(0);
		const char *t = buf.get_data();
		int n = atoi(t);
		if (n > 255)
			return "byte out of range (expected: 0..255)";
		*value = n;
		return 0;
	}
	// the get_* functions:
	// - expect buf to be clean
	// - unget the delimitating character (unless it's error or EOL)
	Token get_dec() {
		const char *err;
		int hi = -1;
		r.unget();
		while (1) {
			int c = r.getc();
			if (c == LineReader::EOL)
				break;
			if (isspace(c)) {
				r.unget();
				break;
			}
			if (c == ':') {
				if (hi != -1)
					return error("unexpected ':'; got one already");
				if ((err = parse_dec(&hi)))
					return error(err);
				buf.reset();
				continue;
			}
			if (!isdigit(c))
				return error("expected decimal digit");
			buf.append_char(c);
		}
		int lo;
		if ((err = parse_dec(&lo)))
			return error(err);
		Token token;
		if (hi != -1) {
			token.type = TK_WORD;
			token.i_value = (hi << 8) | lo;
		} else {
			token.type = TK_BYTE;
			token.i_value = lo;
		}
		return token;
	}
	Token get_chr() {
		int c = r.getc();
		if (c == LineReader::EOL)
			return error("expecting a character; got EOL");
		if (c == '\'')
			return error("\"'\" not supported yet");
		if (r.getc() != '\'')
			return error("expecting \"'\" to terminate character");
		Token token;
		token.type = TK_BYTE;
		token.i_value = c & 0xff;
		return token;
	}
	Token get_hex() {
		int value = 0, n = 0;
		while (1) {
			int c = r.getc();
			if (c == LineReader::EOL)
				break;
			if (isspace(c)) {
				r.unget();
				break;
			}
			int digit;
			switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				digit = c - '0';
				break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				digit = c - 'a' + 10;
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				digit = c - 'A' + 10;
				break;
			default:
				return error("expected hexdecimal digit");
			}
			value = (value << 4) | digit;
			n++;
		}
		Token token;
		if (n <= 2)
			token.type = TK_BYTE;
		else if (n <= 4)
			token.type = TK_WORD;
		else
			return error("hex. literal too long (max 4 digits)");
		token.i_value = value;
		return token;
	}
	Token get_bin() {
		int value = 0, n = 0;
		while (1) {
			int c = r.getc();
			if (c == LineReader::EOL)
				break;
			if (isspace(c)) {
				r.unget();
				break;
			}
			int digit;
			switch (c) {
			case '0':
			case '1':
				digit = c - '0';
				break;
			default:
				return error("expected binary digit");
			}
			value = (value << 1) | digit;
			n++;
		}
		if (n > 8)
			return error("bin. literal too long (max 8 digits)");
		Token token;
		token.type = TK_BYTE;
		token.i_value = value;
		return token;
	}
public:
	Tokenizer(LineReader &r_) : r(r_), buf(32) {
		eol.type = TK_EOL;
	}
	void reset() {
	}
	const Token get() {
		buf.reset();
		int c = r.getc();
		if (c == LineReader::EOL || iscomment(c))
			return eol;
		while (c != LineReader::EOL) {
			while (isspace(c))
				c = r.getc();
			if (c == LineReader::EOL || iscomment(c))
				return eol;
			if (isdigit(c))
				return get_dec();
			if (c == '\'')
				return get_chr();
			if (c == '$')
				return get_hex();
			if (c == '%')
				return get_bin();
			return error("unexpected character");
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
				case TK_WORD:
					prg->append_char(token.i_value & 0xff);
					prg->append_char(token.i_value >> 8);
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
