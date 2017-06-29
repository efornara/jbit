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

// asm.cc

#include "asm.h"

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

const int n_of_mnemonics = sizeof(opcodes) / sizeof(OpcodeList);

const int am_size[] = {
	1, // -
	2, // r
	3, // n:n
	2, // #n
	2, // (n,X)
	2, // (n),Y
	2, // n"
	2, // n,X"
	2, // n,Y"
	3, // n:n,Y"
	3, // (n:n)"
	3, // n:n,X"
};

Segment::Segment(LineReader &r_) :
  r(r_),
  cursor(0),
  base(-1),
  size(-1) {
}

const ParseError *Segment::put(int value) {
	if (size != -1 && cursor >= size)
		return r.error("segment overflow");
	buf.append_char(value);
	cursor++;
	return 0;
}

const ParseError *Segment::put_word(int value) {
	if (put(value & 0xff))
		return &parse_error;
	return put(value >> 8);
}

void Segment::compute_size() {
	if (size != -1)
		return;
	size = (cursor + 0xff) & 0xff00;
}

void Segment::rewind() {
	buf.reset();
	cursor = 0;
}

void Segment::fill(Buffer *prg) {
	if (size == -1)
		return;
	const char *p = buf.get_data();
	int i, n = buf.get_length();
	for (i = 0; i < n; i++)
		prg->append_char(p[i]);
	for (; i < size; i++)
		prg->append_char(0);
}

Binary::Binary(LineReader &r_, Program *prg_) :
  prg(prg_),
  code(r_),
  data(r_),
  segment(&code) {
	code.set_base(0x300);
	for (int i = 0; i < HASHMAP_SLOTS; i++)
		symbols[i] = 0;
}

int Binary::get_slot(const char *s) {
	// djb2 - http://www.cse.yorku.ca/~oz/hash.html
	unsigned long n = 5381;
	char c;
	while ((c = *s++))
		n = n * 33 + c;
	return n & HASHMAP_MASK;
}

Symbol *Binary::find_symbol(const char *s, int slot) {
	if (slot < 0)
		slot = get_slot(s);
	Symbol *sym = symbols[slot];
	while (sym) {
		if (!strcmp(s, sym->name.get_s()))
			return sym;
		sym = sym->next;
	}
	return 0;
}

Symbol *Binary::get_symbol(const char *s) {
	int slot = get_slot(s);
	Symbol *sym = find_symbol(s, slot);
	if (!sym) {
		sym = new Symbol();
		sym->type = SYM_UNDEFINED;
		sym->name = String(&storage, s);
		sym->size = 2;
		sym->next = symbols[slot];
		symbols[slot] = sym;
	}
	return sym;
}

ParserEngine::ParserEngine(const Buffer *src, Program *prg) :
  r(src),
  bin(r, prg),
  tokenizer(r) {
}

const ParseError *ParserEngine::token_error(const Token &token) {
	return r.error(token.s_value);
}

Token ParserEngine::expect(TokenType expected) {
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

const ParseError *ParserEngine::parse_device() {
	Token device = expect(TK_STRING);
	if (device.type == TK_ERROR)
		return &parse_error;
	if (tokenizer.get().type != TK_EOL)
		return r.error("unexpected token");
	return pass->set_device(device.get_string().get_s());
}

const ParseError *ParserEngine::parse_size() {
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

const ParseError *ParserEngine::parse_segment(Token &directive) {
	if (tokenizer.get().type != TK_EOL)
		return r.error("unexpected token");
	return pass->switch_segment(directive.s_value);
}

const ParseError *ParserEngine::parse_define() {
	Token id = tokenizer.get();
	if (id.type == TK_ERROR)
		return &parse_error;
	if (id.type != TK_LABEL || id.arg != ARG_NONE)
		return r.error("expected name to define");
	const char *s = id.get_string().get_s();
	Token value = tokenizer.get();
	int size;
	switch (value.type) {
	case TK_ERROR:
		return &parse_error;
		break;
	case TK_BYTE:
		size = 1;
		break;
	case TK_WORD:
		size = 2;
		break;
	default:
		return r.error("expected value");
	}
	return pass->define_symbol(s, value.i_value, size);
}

const ParseError *ParserEngine::parse_req(bool start) {
	if (tokenizer.get().type != TK_EOL)
		return r.error("unexpected token");
	return pass->req(start);
}

const ParseError *ParserEngine::parse_bits(bool lookup) {
	Token arg = expect(TK_STRING);
	if (arg.type == TK_ERROR)
		return &parse_error;
	if (tokenizer.get().type != TK_EOL)
		return r.error("unexpected token");
	const char *s = arg.get_string().get_s();
	if (lookup)
		return pass->lookup(s);
	else
		return pass->bits(s);
}

const ParseError *ParserEngine::parse_directive(Token directive) {
	if (directive == "device")
		return parse_device();
	else if (directive == "size")
		return parse_size();
	else if (directive == "code" || directive == "data")
		return parse_segment(directive);
	else if (directive == "define")
		return parse_define();
	else if (directive == "req")
		return parse_req(true);
	else if (directive == "endreq")
		return parse_req(false);
	else if (directive == "lookup")
		return parse_bits(true);
	else if (directive == "bits")
		return parse_bits(false);
	else
		return r.internal_error();
}

const ParseError *ParserEngine::parse_value(Token *token, Operand *val, int *size) {
	// token: in (first) / out (delimiter)
	// val: out (3 cases: has_label, value, VALUE_GUARD)
	// size: out (-1: unknown)
	*size = -1;
	switch (token->type) {
	case TK_BYTE:
		val->value = token->i_value;
		*size = 1;
		break;
	case TK_WORD:
		val->value = token->i_value;
		*size = 2;
		break;
	case TK_LABEL:
		switch (token->arg) {
		case ARG_NONE:
			*size = pass->get_symbol_size(token->get_string().get_s());
			break;
		case ARG_HI:
		case ARG_LO:
		case ARG_REL:
			*size = 1;
			break;
		case ARG_DEF:
			return r.error("unexpected label definition");
		}
		val->has_label = true;
		val->name = token->get_string();
		val->arg = token->arg;
		break;
	default:
		return r.error("expected value");
	}
	*token = tokenizer.get();
	if (token->type == TK_ERROR)
		return &parse_error;
	return 0;
}

const ParseError *ParserEngine::parse_instruction(Token mnemonic) {
	const OpcodeList *op = &opcodes[mnemonic.i_value];
	Operand val;
	val.has_label = false;
	val.arg = ARG_NONE;
	val.value = VALUE_GUARD;
	AddressMode am = AM_IMP; // overwritten or error
	int size = -1;
	Token token = tokenizer.get();
	bool need_next = true;
	switch (token.type) {
	case TK_EOL:
		am = AM_IMP;
		break;
	case TK_IMMEDIATE:
		am = AM_IMM;
		token = tokenizer.get();
		if (parse_value(&token, &val, &size))
			return &parse_error;
		if (size != 1)
			return r.error("expected single byte");
		need_next = false;
		break;
	case TK_BYTE:
	case TK_WORD:
	case TK_LABEL:
		if (parse_value(&token, &val, &size))
			return &parse_error;
		switch (token.type) {
		case TK_EOL:
			if (op->opcode[AM_REL] != -1) {
				if (val.has_label) {
					switch (val.arg) {
					case ARG_NONE:
					case ARG_REL:
						val.arg = ARG_REL; // forced
						break;
					default:
						return r.error("not allowed on branches");
					}
				}
				am = AM_REL;
			} else {
				am = size == 1 ? AM_ZPG : AM_ABS;
			}
			need_next = false;
			break;
		case TK_COMMA_X:
			am = size == 1 ? AM_ZPG_X : AM_ABS_X;
			break;
		case TK_COMMA_Y:
			am = size == 1 ? AM_ZPG_Y : AM_ABS_Y;
			break;
		default:
			need_next = false;
			break;
		}
		break;
	case TK_OPEN_PAR:
		token = tokenizer.get();
		if (parse_value(&token, &val, &size))
			return &parse_error;
		switch (token.type) {
		case TK_CLOSE_PAR:
			token = tokenizer.get();
			switch (token.type) {
			case TK_COMMA_Y:
				if (size != 1)
					return r.error("expected single byte");
				am = AM_IND_Y;
				break;
			case TK_EOL:
				if (size != 2)
					return r.error("expected address");
				am = AM_IND;
				need_next = false;
				break;
			default:
				return r.error("expected EOL or ',Y'");
			}
			break;
		case TK_COMMA_X:
			if (size != 1)
				return r.error("expected single byte");
			token = tokenizer.get();
			if (token.type != TK_CLOSE_PAR)
				return r.error("expected ')'");
			am = AM_IND_X;
			break;
		default:
			return r.error("expected ')' or ',X'");
		}
		break;
	case TK_ERROR:
		need_next = false;
		break;
	default:
		return r.error("unexpected token");
	}
	if (need_next)
		token = tokenizer.get();
	if (token.type == TK_ERROR)
		return &parse_error;
	if (token.type != TK_EOL)
		return r.error("unexpected token");
	if (pass->instruction(op, am, val))
		return &parse_error;
	return 0;
}

const ParseError *ParserEngine::do_pass() {
	r.rewind();
	if (pass->begin())
		return &parse_error;
	Token token;
	do {
		if (r.get() == '#')
			continue;
		r.unget();
		tokenizer.new_line();
		do {
			token = tokenizer.get();
			switch (token.type) {
			case TK_DIRECTIVE:
				if (parse_directive(token))
					return &parse_error;
				token.type = TK_EOL;
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
			case TK_MNEMONIC:
				if (parse_instruction(token))
					return &parse_error;
				token.type = TK_EOL;
				break;
			case TK_IMMEDIATE:
			case TK_OPEN_PAR:
			case TK_CLOSE_PAR:
			case TK_COMMA_X:
			case TK_COMMA_Y:
				return r.error("unexpected delimiter");
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

const ParseError *ParserEngine::parse() {
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

const ParseError *parse_asm(const Buffer *src, Program *prg) {
	ParserEngine engine(src, prg);
	return engine.parse();
};
