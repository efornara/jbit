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

ParseError parse_error;

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
	const ParseError *error(const char *msg) {
		parse_error.lineno = get_lineno();
		parse_error.colno = get_colno();
		parse_error.msg = msg;
		return &parse_error;
	}
};

enum TokenType {
	TK_ERROR,
	TK_BYTE,
	TK_WORD,
	TK_STRING,
	TK_DIRECTIVE,
	TK_EOL
};

class String {
private:
	const Buffer *b;
	int i;
public:
	String() : b(0), i(-1) {}
	String(const Buffer *b_, int i_) : b(b_), i(i_) {}
	const char *get_s() const {
		if (i == - 1)
			return 0;
		else
			return &b->get_data()[i];
	}
};

struct Token {
	TokenType type;
	const Buffer *b_value;
	union {
		const char *s_value;
		int i_value;
	};
	void set_string(TokenType t, Buffer *b, const char *s) {
		type = t;
		b_value = b;
		if (s) {
			i_value = b->get_length();
			b->append_string(s);
		} else {
			i_value = -1;
		}
	}
	const String get_string() {
		return String(b_value, i_value);
	}
	bool operator==(const char *o) {
		if (type != TK_DIRECTIVE)
			return false;
		return !strcmp(s_value, o);
	}
};

const char *directives[] = {
	"device",
	"size",
	"code",
	"data",
	0
};


class Tokenizer {
private:
	LineReader &r;
	Buffer line_buf;
	Buffer buf;
	Token eol;
	Token error(const char *msg) {
		Token err;
		err.type = TK_ERROR;
		err.s_value = msg;
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
			return error("unexpected EOL");
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
	Token get_string() {
		while (1) {
			int c = r.getc();
			if (c == LineReader::EOL)
				return error("unexpected EOL");
			if (c == '"')
				break;
			buf.append_char(c);
		}
		buf.append_char(0);
		Token token;
		token.set_string(TK_STRING, &line_buf, buf.get_data());
		return token;
	}
	Token get_directive() {
		while (1) {
			int c = r.getc();
			if (c == LineReader::EOL)
				break;
			if (isspace(c)) {
				r.unget();
				break;
			}
			if (!isalpha(c))
				return error("expected alphabetic letter");
			buf.append_char(c);
		}
		buf.append_char(0);
		const char *t = buf.get_data();
		for (int i = 0; directives[i]; i++) {
			if (!strcmp(directives[i], t)) {
				Token token;
				token.type = TK_DIRECTIVE;
				token.s_value = directives[i];
				return token;
			}
		}
		return error("unknown directive");
	}
public:
	Tokenizer(LineReader &r_) : r(r_), buf(64) {
		eol.type = TK_EOL;
	}
	void new_line() {
		line_buf.reset();
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
			if (c == '"')
				return get_string();
			if (c == '.')
				return get_directive();
			return error("unexpected character");
		}
		return eol;
	}
};

class Segment {
private:
	LineReader &r;
	Buffer buf;
	int cursor;
	int base;
	int size;
public:
	struct Address {
		Segment *buf;
		int offset;
		Address(Segment *b, int o) : buf(b), offset(o) {}
	};
	Address get_address() { return Address(this, cursor); }
	int solve_address(Address &addr) { return base + cursor; }
	Segment(LineReader &r_) : r(r_),  cursor(0), base(-1), size(-1) {}
	const ParseError *put(int value) {
		if (size != -1 && cursor >= size)
			return r.error("segment overflow");
		buf.append_char(value);
		return 0;
	}
	void set_base(int base_) { base = base_; }
	bool is_size_set() { return size != -1; }
	void set_n_of_pages(int n) { size = n << 8; }
	int get_n_of_pages() { return size >> 8; }
	void compute_size() {
		if (size != -1)
			return;
		size = (cursor + 0xff) & 0xff00;
	}
	void fill(Buffer *prg) {
		if (size == -1)
			return;
		const char *p = buf.get_data();
		int i, n = buf.get_length();
		for (i = 0; i < n; i++)
			prg->append_char(p[i]);
		for (; i < size; i++)
			prg->append_char(0);
	}
};

struct Asm {
	LineReader &r;
	Program *prg;
	String device;
	Segment code;
	Segment data;
	Segment *segment;
	Asm(LineReader &r_, Program *prg_) : r(r_), prg(prg_), code(r_), data(r_), segment(&code) {
		code.set_base(0x300);
	}
	void end() {
		prg->device_tag = Tag(device.get_s());
		code.compute_size();
		data.compute_size();
		code.fill(prg);
		data.fill(prg);
	}
	const ParseError *set_device(const String &s) {
		Token t;
		t.set_string(TK_STRING, &prg->metadata_storage, s.get_s());
		device = t.get_string();
		return 0;
	}
	const ParseError *set_size(int code_, int data_) {
		if (code.is_size_set())
			return r.error("program size already set");
		if (code_ + data_ > 256 - 3)
			return r.error("program doesn't fit in memory");
		code.set_n_of_pages(code_);
		data.set_n_of_pages(data_);
		data.set_base(0x300 + (code_ << 8));
		return 0;
	}
	const ParseError *switch_segment(const char *s) {
		if (!strcmp(s, "code")) {
			segment = &code;
			return 0;
		}
		if (!data.is_size_set())
			return r.error("program size not set");
		segment = &data;
		return 0;
	}
	const ParseError *put(int value) {
		return segment->put(value);
	}
};

class ParserEngine {
private:
	LineReader r;
	Asm asm_prg;
	Tokenizer tokenizer;
	const ParseError *token_error(const Token &token) {
		return r.error(token.s_value);
	}
	Token expect(TokenType expected) {
		Token token = tokenizer.get();
		if (token.type == TK_ERROR) {
			token_error(token);
		} else if (token.type != expected) {
			switch (expected) {
			case TK_STRING:
				r.error("string expected");
				return token;
			default:
				r.error("unexpected token");
				token.type = TK_ERROR;
				token.s_value = parse_error.msg;
			}
		}
		return token;
	}
	const ParseError *parse_device() {
		Token device = expect(TK_STRING);
		if (device.type == TK_ERROR)
			return &parse_error;
		if (tokenizer.get().type != TK_EOL)
			return r.error("unexpected token");
		return asm_prg.set_device(device.get_string());
	}
	const ParseError *parse_size() {
		Token code_pages = expect(TK_BYTE);
		if (code_pages.type == TK_ERROR)
			return &parse_error;
		Token data_pages = expect(TK_BYTE);
		if (data_pages.type == TK_ERROR)
			return &parse_error;
		if (tokenizer.get().type != TK_EOL)
			return r.error("unexpected token");
		return asm_prg.set_size(code_pages.i_value, data_pages.i_value);
	}
	const ParseError *parse_codedata(Token &directive) {
		if (tokenizer.get().type != TK_EOL)
			return r.error("unexpected token");
		return asm_prg.switch_segment(directive.s_value);
	}
	const ParseError *parse_directive(Token directive) {
		if (directive == "device")
			return parse_device();
		else if (directive == "size")
			return parse_size();
		else if (directive == "code" || directive == "data")
			return parse_codedata(directive);
		else
			return r.error("internal error");
	}
public:
	ParserEngine(const Buffer *src, Program *prg) : r(src), asm_prg(r, prg), tokenizer(r) {}
	const ParseError *parse() {
		Token token;
		do {
			tokenizer.new_line();
			do {
				token = tokenizer.get();
				switch (token.type) {
				case TK_DIRECTIVE:
					if (parse_directive(token))
						return &parse_error;
					break;
				case TK_BYTE:
					if (asm_prg.put(token.i_value))
						return &parse_error;
					break;
				case TK_WORD:
					if (asm_prg.put(token.i_value & 0xff))
						return &parse_error;
					if (asm_prg.put(token.i_value >> 8))
						return &parse_error;
					break;
				case TK_ERROR:
					return token_error(token);
				case TK_EOL:
					break;
				default:
					return r.error("internal error");
				}
			} while (token.type != TK_EOL);
		} while (r.nextline());
		asm_prg.end();
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
	ParserEngine engine(src, prg);
	return engine.parse();
};
