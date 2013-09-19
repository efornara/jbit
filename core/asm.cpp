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
	int get() {
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
		while (get() != EOL)
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
	const ParseError *internal_error() {
		return error("internal_error");
	}
};

struct OpcodeList {
	const char *mnemonic;
	int opcode[12];
};

const OpcodeList opcodes[] = {
//        -     r   n:n    #n (n,X) (n),Y     n   n,X   n,Y n:n,Y (n:n) n:n,X 
{"BRK",{  0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"BPL",{ -1,   16,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"JSR",{ -1,   -1,   32,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"BMI",{ -1,   48,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"RTI",{ 64,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"BVC",{ -1,   80,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"RTS",{ 96,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"BVS",{ -1,  112,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"BCC",{ -1,  144,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"LDY",{ -1,   -1,  172,  160,   -1,   -1,  164,  180,   -1,   -1,   -1,  188}},
{"BCS",{ -1,  176,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"CPY",{ -1,   -1,  204,  192,   -1,   -1,  196,   -1,   -1,   -1,   -1,   -1}},
{"BNE",{ -1,  208,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"CPX",{ -1,   -1,  236,  224,   -1,   -1,  228,   -1,   -1,   -1,   -1,   -1}},
{"BEQ",{ -1,  240,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"ORA",{ -1,   -1,   13,    9,    1,   17,    5,   21,   -1,   25,   -1,   29}},
{"AND",{ -1,   -1,   45,   41,   33,   49,   37,   53,   -1,   57,   -1,   61}},
{"EOR",{ -1,   -1,   77,   73,   65,   81,   69,   85,   -1,   89,   -1,   93}},
{"ADC",{ -1,   -1,  109,  105,   97,  113,  101,  117,   -1,  121,   -1,  125}},
{"STA",{ -1,   -1,  141,   -1,  129,  145,  133,  149,   -1,  153,   -1,  157}},
{"LDA",{ -1,   -1,  173,  169,  161,  177,  165,  181,   -1,  185,   -1,  189}},
{"CMP",{ -1,   -1,  205,  201,  193,  209,  197,  213,   -1,  217,   -1,  221}},
{"SBC",{ -1,   -1,  237,  233,  225,  241,  229,  245,   -1,  249,   -1,  253}},
{"LDX",{ -1,   -1,  174,  162,   -1,   -1,  166,   -1,  182,  190,   -1,   -1}},
{"BIT",{ -1,   -1,   44,   -1,   -1,   -1,   36,   -1,   -1,   -1,   -1,   -1}},
{"STY",{ -1,   -1,  140,   -1,   -1,   -1,  132,  148,   -1,   -1,   -1,   -1}},
{"ASL",{ 10,   -1,   14,   -1,   -1,   -1,    6,   22,   -1,   -1,   -1,   30}},
{"ROL",{ 42,   -1,   46,   -1,   -1,   -1,   38,   54,   -1,   -1,   -1,   62}},
{"LSR",{ 74,   -1,   78,   -1,   -1,   -1,   70,   86,   -1,   -1,   -1,   94}},
{"ROR",{106,   -1,  110,   -1,   -1,   -1,  102,  118,   -1,   -1,   -1,  126}},
{"STX",{ -1,   -1,  142,   -1,   -1,   -1,  134,   -1,  150,   -1,   -1,   -1}},
{"DEC",{ -1,   -1,  206,   -1,   -1,   -1,  198,  214,   -1,   -1,   -1,  222}},
{"INC",{ -1,   -1,  238,   -1,   -1,   -1,  230,  246,   -1,   -1,   -1,  254}},
{"PHP",{  8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"CLC",{ 24,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"PLP",{ 40,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"SEC",{ 56,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"PHA",{ 72,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"CLI",{ 88,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"PLA",{104,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"SEI",{120,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"DEY",{136,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"TYA",{152,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"TAY",{168,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"CLV",{184,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"INY",{200,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"CLD",{216,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"INX",{232,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"SED",{248,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"TXA",{138,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"TXS",{154,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"TAX",{170,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"TSX",{186,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"DEX",{202,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"NOP",{234,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1}},
{"JMP",{ -1,   -1,   76,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  108,   -1}},
};

enum TokenType {
	TK_ERROR,
	TK_BYTE,
	TK_WORD,
	TK_STRING,
	TK_DIRECTIVE,
	TK_LABEL,
	TK_MNEMONIC,
	TK_EOL
};

enum TokenArg {
	ARG_NONE = 0,
	ARG_HI = '>',
	ARG_LO = '<',
	ARG_REL = '?',
	ARG_DEF = ':',
};

class String {
private:
	Buffer *b;
	int i;
	friend class Token;
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

struct Token {
	TokenType type;
	TokenArg arg;
	Buffer *b_value;
	union {
		const char *s_value;
		int i_value;
	};
	void set_string(TokenType t, Buffer *b, const char *s) {
		type = t;
		b_value = b;
		String str(b, s);
		i_value = str.i;
	}
	String get_string() {
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
	const char *parse_char(int delim, int *ch) {
		int c = r.get();
		if (c == LineReader::EOL)
			return "unexpected EOL";
		if (c == delim) {
			*ch = -1;
			return 0;
		}
		if (c == '\\') {
			c = r.get();
			switch (c) {
			case 'a':
				*ch = '\a';
				return 0;
			case 'b':
				*ch = '\b';
				return 0;
			case 'e':
			case 'E':
				*ch = 0x1b;
				return 0;
			case 'f':
				*ch = '\f';
				return 0;
			case 'n':
				*ch = '\n';
				return 0;
			case 'r':
				*ch = '\r';
				return 0;
			case 't':
				*ch = '\t';
				return 0;
			case 'v':
				*ch = '\v';
				return 0;
			case '\\':
				*ch = '\\';
				return 0;
			case '\'':
				*ch = '\'';
				return 0;
			case '"':
				*ch = '"';
				return 0;
			default:
				return "invalid escape sequence";
			}
		}
		*ch = c;
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
			int c = r.get();
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
		const char *err;
		int c;
		if ((err = parse_char('\'', &c)))
			return error(err);
		if (c == -1)
			return error("empty character");
		int eoc;
		if ((err = parse_char('\'', &eoc)))
			return error(err);
		if (eoc != -1)
			return error("two many characters; use \" for strings");
		Token token;
		token.type = TK_BYTE;
		token.i_value = c & 0xff;
		return token;
	}
	Token get_hex() {
		int value = 0, n = 0;
		while (1) {
			int c = r.get();
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
			int c = r.get();
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
			int c;
			const char *err = parse_char('"', &c);
			if (err)
				return error(err);
			if (c == -1)
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
			int c = r.get();
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
	Token get_identifier(TokenArg arg) {
		while (1) {
			int c = r.get();
			if (c == LineReader::EOL)
				break;
			if (isspace(c)) {
				r.unget();
				break;
			}
			if (c == ':') {
				if (arg != ARG_NONE)
					return error("unexpected ':'");
				arg = ARG_DEF;
				break;
			}
			if (!isalpha(c) && !isdigit(c) && c != '_')
				return error("invalid character in identifier");
			buf.append_char(c);
		}
		if (buf.get_length() == 0)
			return error("empty identifier");
		buf.append_char(0);
		Token token;
		token.set_string(TK_LABEL, &line_buf, buf.get_data());
		token.arg = arg;
		return token;
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
		int c = r.get();
		if (c == LineReader::EOL || iscomment(c))
			return eol;
		while (c != LineReader::EOL) {
			while (c != LineReader::EOL && isspace(c))
				c = r.get();
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
			switch (c) {
			case '<':
			case '>':
			case '?':
				return get_identifier((TokenArg)c);
			}
			if (isalpha(c) || c == '_') {
				r.unget();
				return get_identifier((TokenArg)0);
			}
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
		Segment *s;
		int offset;
		Address() : s(0), offset(0) {}
		Address(Segment *s_, int o) : s(s_), offset(o) {}
		int get() { return s->get_base() + offset; }
	};
	Address get_address() { return Address(this, cursor); }
	Segment(LineReader &r_) : r(r_),  cursor(0), base(-1), size(-1) {}
	const ParseError *put(int value) {
		if (size != -1 && cursor >= size)
			return r.error("segment overflow");
		buf.append_char(value);
		cursor++;
		return 0;
	}
	void set_base(int base_) { base = base_; }
	int get_base() { return base; }
	bool is_size_set() { return size != -1; }
	void set_n_of_pages(int n) { size = n << 8; }
	int get_n_of_pages() { return size >> 8; }
	int get_cursor() { return base + cursor; }
	void compute_size() {
		if (size != -1)
			return;
		size = (cursor + 0xff) & 0xff00;
	}
	void rewind() {
		buf.reset();
		cursor = 0;
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

// just a list for now (poor perf., but quick and easy to impl.)
struct Label {
	String name;
	Segment::Address addr;
	struct Label *next;
};

struct Binary {
	LineReader &r;
	Buffer storage;
	Label *labels;
	Program *prg;
	String device;
	Segment code;
	Segment data;
	Segment *segment;
	int n_of_labels;
	Binary(LineReader &r_, Program *prg_) : r(r_), prg(prg_), code(r_), data(r_), segment(&code), n_of_labels(0) {
		code.set_base(0x300);
		labels = new Label();
		labels->name = String(&storage, "__start__");
		labels->addr = code.get_address();
		labels->next = 0;
	}
	Label *find_label(const char *s) {
		Label *l = labels;
		while (l) {
			if (!strcmp(s, l->name.get_s()))
				return l;
			l = l->next;
		}
		return 0;
	}
	Label *add_label(Segment *seg, const char *s) {
		Label *l = new Label();
		l->name = String(&storage, s);
		l->addr = seg->get_address();
		l->next = labels;
		labels = l;
		return l;
	}
	void dump_labels() {
		Label *l = labels;
		while (l) {
			fprintf(stderr, "%s %d\n", l->name.get_s(), l->addr.offset);
			l = l->next;
		}
	}
};

class Pass {
protected:
	LineReader &r;
	Binary &bin;
public:
	Pass(LineReader &r_, Binary &bin_) : r(r_), bin(bin_) {}
	const ParseError *switch_segment(const char *s) {
		if (!strcmp(s, "code"))
			bin.segment = &bin.code;
		else
			bin.segment = &bin.data;
		return 0;
	}
	virtual const ParseError *begin() { return 0; }
	virtual const ParseError *set_device(const char *s) { return 0; }
	virtual const ParseError *set_size(int code, int data) { return 0; }
	virtual const ParseError *put(int value) { return 0; }
	virtual const ParseError *label(const char *s, TokenArg a) { return 0; }
	virtual const ParseError *end() { return 0; }
	virtual ~Pass() {}
};

class Pass1 : public Pass {
public:
	Pass1(LineReader &r_, Binary &bin_) : Pass(r_, bin_) {}
	const ParseError *set_device(const char *s) {
		Token t;
		t.set_string(TK_STRING, &bin.prg->metadata_storage, s);
		bin.device = t.get_string();
		return 0;
	}
	const ParseError *set_size(int code_, int data_) {
		if (bin.code.is_size_set())
			return r.error("program size already set");
		if (code_ + data_ > 256 - 3)
			return r.error("program doesn't fit in memory");
		bin.code.set_n_of_pages(code_);
		bin.data.set_n_of_pages(data_);
		bin.data.set_base(0x300 + (code_ << 8));
		return 0;
	}
	const ParseError *put(int value) {
		return bin.segment->put(value);
	}
	const ParseError *label(const char *s, TokenArg a) {
		switch (a) {
		case ARG_DEF:
			if (bin.find_label(s))
				return r.error("duplicate label");
			bin.add_label(bin.segment, s);
			return 0;
		case ARG_HI:
		case ARG_LO:
		case ARG_REL:
			return bin.segment->put(0);
		case ARG_NONE:
			if (bin.segment->put(0))
				return &parse_error;
			return bin.segment->put(0);
		default:
			return r.internal_error();
		}
	}
	const ParseError *end() {
		bin.code.compute_size();
		bin.data.compute_size();
		bin.data.set_base(0x300 + (bin.code.get_n_of_pages() << 8));
		return 0;
	}
};

class Pass2 : public Pass {
public:
	Pass2(LineReader &r_, Binary &bin_) : Pass(r_, bin_) {}
	const ParseError *begin() {
		bin.code.rewind();
		bin.data.rewind();
		bin.segment = &bin.code;
		return 0;
	}
	const ParseError *put(int value) {
		return bin.segment->put(value);
	}
	const ParseError *label(const char *s, TokenArg a) {
		if (a == ARG_DEF)
			return 0;
		Label *l = bin.find_label(s);
		if (!l)
			return r.error("undefined label");
		int addr = l->addr.get();
		switch (a) {
		case ARG_HI:
			return bin.segment->put(addr >> 8);
		case ARG_LO:
			return bin.segment->put(addr & 0xff);
		case ARG_NONE:
			if (bin.segment->put(addr & 0xff))
				return &parse_error;
			return bin.segment->put(addr >> 8);
		case ARG_REL: {
			int offset = addr - (bin.segment->get_cursor() + 1);
			if (offset > 127 || offset < -128)
				return r.error("address is too far");
			return bin.segment->put(offset);
		}
		default:
			return r.internal_error();
		}
	}
	const ParseError *end() {
		bin.prg->device_tag = Tag(bin.device.get_s());
		bin.prg->n_of_code_pages = bin.code.get_n_of_pages();
		bin.prg->n_of_data_pages = bin.data.get_n_of_pages();
		bin.code.fill(bin.prg);
		bin.data.fill(bin.prg);
		return 0;
	}
};

class ParserEngine {
private:
	LineReader r;
	Binary bin;
	Tokenizer tokenizer;
	Pass *pass;
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
		return pass->set_device(device.get_string().get_s());
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
		return pass->set_size(code_pages.i_value, data_pages.i_value);
	}
	const ParseError *parse_codedata(Token &directive) {
		if (tokenizer.get().type != TK_EOL)
			return r.error("unexpected token");
		return pass->switch_segment(directive.s_value);
	}
	const ParseError *parse_directive(Token directive) {
		if (directive == "device")
			return parse_device();
		else if (directive == "size")
			return parse_size();
		else if (directive == "code" || directive == "data")
			return parse_codedata(directive);
		else
			return r.internal_error();
	}
public:
	ParserEngine(const Buffer *src, Program *prg) : r(src), bin(r, prg), tokenizer(r) {}
	const ParseError *do_pass() {
		r.rewind();
		if (pass->begin())
			return &parse_error;
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
					if (pass->put(token.i_value))
						return &parse_error;
					break;
				case TK_WORD:
					if (pass->put(token.i_value & 0xff))
						return &parse_error;
					if (pass->put(token.i_value >> 8))
						return &parse_error;
					break;
				case TK_STRING: {
					const char *s = token.get_string().get_s
();
					for (int i = 0; s[i]; i++)
						if (pass->put(s[i] & 0xff))
							return &parse_error;
					break;
				}
				case TK_LABEL:
					if (pass->label(token.get_string().get_s(), token.arg))
						return &parse_error;
					break;
				case TK_ERROR:
					return token_error(token);
				case TK_EOL:
					break;
				default:
					return r.internal_error();
				}
			} while (token.type != TK_EOL);
		} while (r.nextline());
		return pass->end();
	}
	const ParseError *parse() {
		Pass1 pass1(r, bin);
		pass = &pass1;
		if (do_pass())
			return &parse_error;
		Pass2 pass2(r, bin);
		pass = &pass2;
		if (do_pass())
			return &parse_error;
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

const char *get_jbit_version() {
	return "1.1";
}
