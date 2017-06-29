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

// passes.cc

#include "asm.h"

Pass::Pass(LineReader &r_, Binary &bin_) : r(r_), bin(bin_) {
	memcpy(lookup_table, "01", 2 + 1);
}

int Pass::get_symbol_size(const char *s) {
	return bin.get_symbol(s)->size;
}

const ParseError *Pass::switch_segment(const char *s) {
	if (!strcmp(s, "code"))
		bin.segment = &bin.code;
	else
		bin.segment = &bin.data;
	return 0;
}

const ParseError *Pass::lookup(const char *s) {
	int n = strlen(s);
	if (n < 2 || n > 16)
		return r.error("lookup length must be 2..16");
	memcpy(lookup_table, s, n + 1);
	return 0;
}

const ParseError *Pass::bits(const char *s) {
	int lookup_n = strlen(lookup_table);
	int n_of_bits;
	if (lookup_n == 2)
		n_of_bits = 1;	
	else if (lookup_n <= 4)
		n_of_bits = 2;	
	else
		n_of_bits = 4;	
	int buffer = 0;
	int accumulated = 0;
	for (int i = 0; s[i]; i++) {
		int c;
		for (c = 0; c < lookup_n; c++)
			if (lookup_table[c] == s[i])
				break;
		if (c == lookup_n)
			return r.error("couldn't find character in lookup");
		int mask = 1 << (n_of_bits - 1);
		while (mask) {
			buffer <<= 1;
			if (c & mask)
				buffer |= 0x01;
			mask >>= 1;
			accumulated++;
		}
		if (accumulated == 8) {
			if (bin.segment->put(buffer))
				return &parse_error;
			buffer = 0;
			accumulated = 0;
		}
	}
	if (accumulated) {
		while (accumulated < 8) {
			buffer <<= 1;
			accumulated++;
		}
		if (bin.segment->put(buffer))
			return &parse_error;
	}
	return 0;
}

Pass1::Pass1(LineReader &r_, Binary &bin_) :
  Pass(r_, bin_),
  req_segment(0) {
}

const ParseError *Pass1::set_device(const char *s) {
	Token t;
	t.set_string(TK_STRING, &bin.prg->metadata_storage, s);
	bin.device = t.get_string();
	const SymDef *syms = get_device_symdefs(bin.device.get_s());
	if (syms) {
		for (int i = 0; syms[i].name; i++) {
			int value = syms[i].value;
			if (define_symbol(syms[i].name, value, value > 255 ? 2 : 1))
				return &parse_error;
		}
	}
	return 0;
}

const ParseError *Pass1::set_size(int code_, int data_) {
	if (bin.code.is_size_set())
		return r.error("program size already set");
	if (code_ + data_ > 256 - 3)
		return r.error("program doesn't fit in memory");
	bin.code.set_n_of_pages(code_);
	bin.data.set_n_of_pages(data_);
	bin.data.set_base(0x300 + (code_ << 8));
	return 0;
}

const ParseError *Pass1::define_symbol(const char *s, int value, int size) {
	Symbol *sym = bin.find_symbol(s);
	if (!sym) {
		sym = bin.get_symbol(s);
		sym->type = SYM_DEFINE;
		sym->value = value;
		sym->size = size;
		return 0;
	}
	if (sym->type == SYM_UNDEFINED)
		return r.error("symbol used before being defined");
	else
		return r.error("duplicate symbol");
}

const ParseError *Pass1::put(int value) {
	return bin.segment->put(value);
}

const ParseError *Pass1::label(const char *s, TokenArg a) {
	Symbol *sym = bin.get_symbol(s);
	switch (a) {
	case ARG_DEF:
		if (sym->type != SYM_UNDEFINED)
			return r.error("duplicate symbol");
		sym->type = SYM_LABEL;
		sym->addr = bin.segment->get_address();
		return 0;
	case ARG_HI:
	case ARG_LO:
	case ARG_REL:
		return bin.segment->put(0);
	case ARG_NONE:
		if (sym->size == 2)
			return bin.segment->put_word(0);
		else
			return bin.segment->put(0);
	default:
		return r.internal_error();
	}
}

const ParseError *Pass1::instruction(const OpcodeList *op, int am, Operand operand) {
	if (op->opcode[am] == -1)
		return r.error("invalid address mode");
	int n = am_size[am];
	if (operand.has_label)
		bin.get_symbol(operand.name.get_s());
	for (int i = 0; i < n; i++)
		if (bin.segment->put(0))
			return &parse_error;
	return 0;
}

const ParseError *Pass1::req(bool start) {
	if (start) {
		if (req_segment)
			return r.error("previuos req not ended");
		req_segment = bin.segment;
		req_start_cursor = req_segment->get_cursor();
		req_segment->put_word(0);
	} else {
		if (!req_segment)
			return r.error("req not started");
		if (req_segment != bin.segment)
			return r.error("req cannot cross segments");
		int *size = (int *)bin.reqs.append_raw(sizeof(int));
		*size = bin.segment->get_cursor() - req_start_cursor - 2;
		req_segment = 0;
	}
	return 0;
}

const ParseError *Pass1::end() {
	bin.code.compute_size();
	bin.data.compute_size();
	bin.data.set_base(0x300 + (bin.code.get_n_of_pages() << 8));
	return 0;
}

Pass2::Pass2(LineReader &r_, Binary &bin_) :
  Pass(r_, bin_),
  req_id(0) {
}

const ParseError *Pass2::begin() {
	bin.code.rewind();
	bin.data.rewind();
	bin.segment = &bin.code;
	return 0;
}

const ParseError *Pass2::put(int value) {
	return bin.segment->put(value);
}

const ParseError *Pass2::do_label(const char *s, TokenArg a, int size) {
	if (a == ARG_DEF)
		return 0;
	Symbol *sym = bin.get_symbol(s);
	if (sym->type == SYM_UNDEFINED)
		return r.error("undefined symbol");
	int value;
	switch (a) {
	case ARG_HI:
	case ARG_LO:
	case ARG_REL:
		if (sym->size == 1)
			return r.error("symbol is a byte");
		break;
	case ARG_NONE:
		if (size != -1 && sym->size != size)
			return r.error("symbol/operand size mismatch");
		break;
	default:
		;
	}
	if (sym->type == SYM_LABEL)
		value = sym->addr.get();
	else
		value = sym->value;
	if (size == -1)
		size = sym->size;
	switch (a) {
	case ARG_HI:
		return bin.segment->put(value >> 8);
	case ARG_LO:
		return bin.segment->put(value & 0xff);
	case ARG_NONE:
		if (size == 1)
			return bin.segment->put(value);
		else
			return bin.segment->put_word(value);
	case ARG_REL: {
		int offset = value - (bin.segment->get_current_address() + 1);
		if (offset > 127 || offset < -128)
			return r.error("address is too far");
		return bin.segment->put(offset);
	}
	default:
		return r.internal_error();
	}
}

const ParseError *Pass2::label(const char *s, TokenArg a) {
	return do_label(s, a, -1);
}

const ParseError *Pass2::instruction(const OpcodeList *op, int am, Operand operand) {
	if (bin.segment->put(op->opcode[am]))
		return &parse_error;
	int n = am_size[am];
	if (operand.has_label)
		return do_label(operand.name.get_s(), operand.arg, n - 1);
	if (n != 1 && operand.value == VALUE_GUARD)
		return r.error("internal error: VALUE_GUARD");
	if (n == 2) {
		if (bin.segment->put(operand.value))
			return &parse_error;
	} else if (n == 3) {
		if (bin.segment->put_word(operand.value))
			return &parse_error;
	}
	return 0;
}

const ParseError *Pass2::req(bool start) {
	if (start) {
		int *size = (int *)bin.reqs.get_data();
		bin.segment->put_word(size[req_id++]);
	}
	return 0;
}

const ParseError *Pass2::end() {
	bin.prg->device_tag = Tag(bin.device.get_s());
	bin.prg->n_of_code_pages = bin.code.get_n_of_pages();
	bin.prg->n_of_data_pages = bin.data.get_n_of_pages();
	bin.code.fill(bin.prg);
	bin.data.fill(bin.prg);
	return 0;
}
