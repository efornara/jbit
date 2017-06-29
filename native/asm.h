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

// asm.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "core.h"

extern ParseError parse_error;

class LineReader {
private:
	const char *d;
	int len;
	int i, start;
	int lineno;
public:
	static const int EOL = INT_MAX;
	void rewind();
	LineReader(const Buffer *b);
	int get();
	void unget() {
		if (i != start)
			i--;
	}
	bool nextline();
	const ParseError *error(const char *msg);
	const ParseError *internal_error();
	int get_lineno() { return lineno; }
	int get_colno() { return i - start; }
};

struct OpcodeList {
	const char *mnemonic;
	int opcode[12];
};

extern const OpcodeList opcodes[];
extern const int n_of_mnemonics;

enum AddressMode {
	AM_IMP = 0,
	AM_REL,
	AM_ABS,
	AM_IMM,
	AM_IND_X,
	AM_IND_Y,
	AM_ZPG,
	AM_ZPG_X,
	AM_ZPG_Y,
	AM_ABS_Y,
	AM_IND,
	AM_ABS_X,
};

extern const int am_size[];

enum TokenType {
	TK_ERROR,
	TK_BYTE,
	TK_WORD,
	TK_STRING,
	TK_DIRECTIVE,
	TK_LABEL,
	TK_MNEMONIC,
	TK_IMMEDIATE,
	TK_OPEN_PAR,
	TK_CLOSE_PAR,
	TK_COMMA_X,
	TK_COMMA_Y,
	TK_EOL
};

enum TokenArg {
	ARG_NONE = 0,
	ARG_HI = '>',
	ARG_LO = '<',
	ARG_REL = '?',
	ARG_DEF = ':',
};

struct Token {
	TokenType type;
	TokenArg arg;
	Buffer *b_value;
	union {
		const char *s_value;
		int i_value;
	};
	Token() {}
	Token(TokenType type_) : type(type_) {}
	void set_string(TokenType t, Buffer *b, const char *s);
	String get_string();
	bool operator==(const char *o);
};

extern const char *directives[];

class Tokenizer {
private:
	LineReader &r;
	Buffer line_buf;
	Buffer buf;
	Token eol;
	Token error(const char *msg);
	bool isseparator(int c);
	const char *parse_dec(int *value);
	const char *parse_char(int delim, int *ch);
	Token get_dec();
	Token get_chr();
	Token get_hex();
	Token get_bin();
	Token get_string();
	Token get_directive();
	Token get_identifier(TokenArg arg);
	Token get_comma();
public:
	Tokenizer(LineReader &r_);
	void new_line();
	const Token get();
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
	Segment(LineReader &r_);
	const ParseError *put(int value);
	const ParseError *put_word(int value);
	void compute_size();
	void rewind();
	void fill(Buffer *prg);
	Address get_address() { return Address(this, cursor); }
	void set_base(int base_) { base = base_; }
	int get_base() { return base; }
	bool is_size_set() { return size != -1; }
	void set_n_of_pages(int n) { size = n << 8; }
	int get_n_of_pages() { return size >> 8; }
	int get_cursor() { return cursor; }
	int get_current_address() { return base + cursor; }
};

static const int HASHMAP_SLOTS = 256; // must be a power of 2
static const int HASHMAP_MASK = HASHMAP_SLOTS - 1;

enum SymbolType {
	SYM_UNDEFINED,
	SYM_LABEL,
	SYM_DEFINE,
};

struct Symbol {
	struct Symbol *next;
	SymbolType type;
	String name;
	Segment::Address addr;
	int value;
	int size;
};

struct Binary {
private:
	Buffer storage;
	Symbol *symbols[HASHMAP_SLOTS];
	int get_slot(const char *s);
public:
	Buffer reqs;
	Program *prg;
	String device;
	Segment code;
	Segment data;
	Segment *segment;
	Binary(LineReader &r_, Program *prg_);
	Symbol *find_symbol(const char *s, int slot = -1);
	Symbol *get_symbol(const char *s);
};

struct Operand {
	bool has_label;
	String name;
	TokenArg arg;
	long value;
};

class Pass {
private:
	char lookup_table[16 + 1];
protected:
	LineReader &r;
	Binary &bin;
public:
	Pass(LineReader &r_, Binary &bin_);
	int get_symbol_size(const char *s);
	const ParseError *switch_segment(const char *s);
	const ParseError *lookup(const char *s);
	const ParseError *bits(const char *s);
	virtual const ParseError *begin() { return 0; }
	virtual const ParseError *set_device(const char *s) { return 0; }
	virtual const ParseError *set_size(int code, int data) { return 0; }
	virtual const ParseError *define_symbol(const char *s, int value, int size) { return 0; }
	virtual const ParseError *put(int value) { return 0; }
	virtual const ParseError *label(const char *s, TokenArg a) { return 0; }
	virtual const ParseError *instruction(const OpcodeList *op, int am, Operand operand) { return 0; }
	virtual const ParseError *req(bool start) { return 0; }
	virtual const ParseError *end() { return 0; }
	virtual ~Pass() {}
};

class Pass1 : public Pass {
private:
	Segment *req_segment;
	int req_start_cursor;
public:
	Pass1(LineReader &r_, Binary &bin_);
	const ParseError *set_device(const char *s);
	const ParseError *set_size(int code_, int data_);
	const ParseError *define_symbol(const char *s, int value, int size);
	const ParseError *put(int value);
	const ParseError *label(const char *s, TokenArg a);
	const ParseError *instruction(const OpcodeList *op, int am, Operand operand);
	const ParseError *req(bool start);
	const ParseError *end();
};

#define VALUE_GUARD 100000L

class Pass2 : public Pass {
private:
	int req_id;
public:
	Pass2(LineReader &r_, Binary &bin_);
	const ParseError *begin();
	const ParseError *put(int value);
	const ParseError *do_label(const char *s, TokenArg a, int size);
	const ParseError *label(const char *s, TokenArg a);
	const ParseError *instruction(const OpcodeList *op, int am, Operand operand);
	const ParseError *req(bool start);
	const ParseError *end();
};

class ParserEngine {
private:
	LineReader r;
	Binary bin;
	Tokenizer tokenizer;
	Pass *pass;
	const ParseError *token_error(const Token &token);
	Token expect(TokenType expected);
	const ParseError *parse_device();
	const ParseError *parse_size();
	const ParseError *parse_segment(Token &directive);
	const ParseError *parse_define();
	const ParseError *parse_req(bool start);
	const ParseError *parse_bits(bool lookup);
	const ParseError *parse_directive(Token directive);
	const ParseError *parse_value(Token *token, Operand *val, int *size);
	const ParseError *parse_instruction(Token mnemonic);
public:
	ParserEngine(const Buffer *src, Program *prg);
	const ParseError *do_pass();
	const ParseError *parse();
};
