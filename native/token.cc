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

// token.cc

#include "asm.h"

ParseError parse_error;

LineReader::LineReader(const Buffer *b) :
  d(b->get_data()),
  len(b->get_length()) {
	rewind();
}

void LineReader::rewind() {
	i = 0;
	start = 0;
	lineno = 1;
}

int LineReader::get() {
	if (i >= len)
		return EOL;
	char c = d[i];
	while (i < len && c == '\r')
		c = d[++i];
	if (c == 0 || c == '\n' || c == '\r')
		return EOL;
	i++;
	return c;
}

bool LineReader::nextline() {
	while (get() != EOL)
		;
	i++;
	if (i >= len)
		return false;
	start = i;
	lineno++;
	return true;
}

const ParseError *LineReader::error(const char *msg) {
	parse_error.lineno = get_lineno();
	parse_error.colno = get_colno();
	parse_error.msg = msg;
	return &parse_error;
}

const ParseError *LineReader::internal_error() {
	return error("internal_error");
}

void Token::set_string(TokenType t, Buffer *b, const char *s) {
	type = t;
	b_value = b;
	String str(b, s);
	i_value = str.i;
}

String Token::get_string() {
	return String(b_value, i_value);
}

bool Token::operator==(const char *o) {
	if (type != TK_DIRECTIVE)
		return false;
	return !strcmp(s_value, o);
}

const char *directives[] = {
	"device",
	"size",
	"code",
	"data",
	"define",
	"req",
	"endreq",
	"lookup",
	"bits",
	0
};

Tokenizer::Tokenizer(LineReader &r_) : r(r_), buf(64) {
	eol.type = TK_EOL;
}

Token Tokenizer::error(const char *msg) {
	Token err;
	err.type = TK_ERROR;
	err.s_value = msg;
	return err;
}

bool Tokenizer::isseparator(int c) {
	switch (c) {
	case '#':
	case ',':
	case ')':
		return true;
	default:
		return isspace(c);
	}
}

const char *Tokenizer::parse_dec(int *value) {
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

const char *Tokenizer::parse_char(int delim, int *ch) {
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

Token Tokenizer::get_dec() {
	const char *err;
	int hi = -1;
	r.unget();
	while (1) {
		int c = r.get();
		if (c == LineReader::EOL)
			break;
		if (isseparator(c)) {
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

Token Tokenizer::get_chr() {
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

Token Tokenizer::get_hex() {
	int value = 0, n = 0;
	while (1) {
		int c = r.get();
		if (c == LineReader::EOL)
			break;
		if (isseparator(c)) {
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

Token Tokenizer::get_bin() {
	int value = 0, n = 0;
	while (1) {
		int c = r.get();
		if (c == LineReader::EOL)
			break;
		if (isseparator(c)) {
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

Token Tokenizer::get_string() {
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

Token Tokenizer::get_directive() {
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

Token Tokenizer::get_identifier(TokenArg arg) {
	while (1) {
		int c = r.get();
		if (c == LineReader::EOL)
			break;
		if (isseparator(c)) {
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
	const char *s = buf.get_data();
	for (int i = 0; i < n_of_mnemonics; i++) {
		if (!strcasecmp(s, opcodes[i].mnemonic)) {
			if (arg != ARG_NONE)
				return error("label qualifier incompatible with mnemonic");
			Token token;
			token.type = TK_MNEMONIC;
			token.i_value = i;
			return token;
		}
	}
	Token token;
	token.set_string(TK_LABEL, &line_buf, s);
	token.arg = arg;
	return token;
}

Token Tokenizer::get_comma() {
	int c = r.get();
	switch (c) {
	case 'x':
	case 'X':
		return Token(TK_COMMA_X);
	case 'y':
	case 'Y':
		return Token(TK_COMMA_Y);
	default:
		return error("expected X or Y after ','");
	}
}

void Tokenizer::new_line() {
	line_buf.reset();
}

const Token Tokenizer::get() {
	buf.reset();
	int c = r.get();
	while (c != LineReader::EOL) {
		while (c != LineReader::EOL && isspace(c))
			c = r.get();
		switch (c) {
		case LineReader::EOL:
		case ';':
			return eol;
		case '\'':
			return get_chr();
		case '$':
			return get_hex();
		case '%':
			return get_bin();
		case '"':
			return get_string();
		case '.':
			return get_directive();
		case '<':
		case '>':
		case '?':
			return get_identifier((TokenArg)c);
		case '#':
			return Token(TK_IMMEDIATE);
		case '(':
			return Token(TK_OPEN_PAR);
		case ')':
			return Token(TK_CLOSE_PAR);
		case ',':
			return get_comma();
		}
		if (isdigit(c))
			return get_dec();
		if (isalpha(c) || c == '_') {
			r.unget();
			return get_identifier((TokenArg)0);
		}
		return error("unexpected character");
	}
	return eol;
}
