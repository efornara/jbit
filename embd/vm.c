/*
 * Copyright (C) 2014  Emanuele Fornara
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "embd.h"

#ifdef ENABLE_VM

uint8_t read6502(uint16_t address);
void write6502(uint16_t address, uint8_t value);

#ifdef ENABLE_MICROIO
microio_context_t microio;
#endif

#ifdef ENABLE_PRIMO
primo_context_t primo;
#endif

uint8_t vm_vsync;
uint16_t vm_wait;

#ifdef ENABLE_VM_TRACE

void vm_tracef(const char *format, ...) {
	char msg[64];
	va_list ap;

	va_start(ap, format);
	vsnprintf(msg, sizeof(msg), format, ap);
	va_end(ap);
	msg[sizeof(msg) - 1] = '\0';
	vm_traces(msg);
}

static void vm_traceu(uint16_t address) {
	char msg[64];
	int i;
	uint8_t c;

	for (i = 0; i < sizeof(msg) - 1; i++) {
		if (!(c = read6502(address++)))
			break;
		msg[i] = c;
	}
	msg[i] = '\0';
	vm_traces(msg);
}

#endif

static void wait_for_key(uint8_t event, char c) {
	jbit_replace_with(MODULE_JBIT);
}

void vm_stop(const char *tag) {
#ifndef LCD_NULL
	int i;

	lcd_goto(0, 0);
	for (i = 0; tag[i]; i++)
		lcd_char(tag[i]);
#elif defined(ENABLE_VM_TRACE)
	vm_tracef("vm_stop: %s", tag);
#endif
#if defined(PLATFORM_PC) && !defined(PLATFORM_PC_SDL)
	exit(0);
#else
#ifndef KEYPAD_NULL
	keypad_handler = wait_for_key;
#endif
#endif
}

static const uint8_t irqvec[] PROGMEM = {
	// 255:240
	169,0,141,0,255,
	169,1,141,0,255,
	// 255:250
	245, 255, // NMI (exit 1)
	0, 3, // RESET
	240, 255, // BRK (exit 0)
};

// mpages; linear search for now

#define PAGE_SIZE 256

#define MPAGE_SIZE      0x0020
#define MPAGE_ADDR_MASK 0xffe0
#define MPAGE_DATA_MASK 0x001f

#define VM_WAIT_MS 50

#define VM_STATE_RUNNING 0
#define VM_STATE_HALTOK 1
#define VM_STATE_HALTFAIL 2
#define VM_STATE_INVOP 3
#define VM_STATE_OUTOFMEM 4
#define VM_STATE_BREAK 5

typedef struct {
	uint8_t page0[PAGE_SIZE];
	struct {
		uint16_t addr;
		uint8_t data[MPAGE_SIZE];
	} mpage[MAX_N_MPAGES];
	uint16_t n_mpages;
	uint16_t last_ro_addr;
	uint8_t vm_state;
	uint8_t vm_trchi;
} vm_context_t;

static vm_context_t ctx_;
static vm_context_t *ctx = &ctx_;

uint8_t get_prog_byte(int offset) {
	uint16_t data;
	// in pgm the full jb is available (fast)
	if (jbit_prg_pgm) {
		if (offset < jbit_prg_code_size)
			return pgm_read_byte(&(jbit_prg_code_ptr[offset]));
		else
			return 0; // demos
	}
	// in ram there might be holes (compact)
	if (offset < jbit_prg_code_size)
		return jbit_prg_code_ptr[offset];
	data = jbit_prg_code_pages << 8;
	if (offset < data)
		return 0;
	offset -= data;
	if (offset >= jbit_prg_data_size)
		return 0;
	return jbit_prg_data_ptr[offset];
}

#define REG(x) (x - 0xff00)

#ifdef ENABLE_VM_TRACE_MEM
uint8_t read6502_impl(uint16_t address) {
#else
uint8_t read6502(uint16_t address) {
#endif
	uint8_t page = address >> 8;
	uint8_t offset = address & 0xff;
	int i;
	switch (page) {
	case 0:
		return ctx->page0[offset];
	case 2:
#ifdef ENABLE_MICROIO
		if (offset > 16 && offset < 80)
			return microio_get(&microio, offset);
#endif
#ifdef ENABLE_PRIMO
		if (offset < 16 || offset > 80)
			return primo_get(&primo, offset);
#endif
		return 0;
	case 255:
		switch (offset) {
		case REG(VMTRCHI):
			return ctx->vm_trchi;
		default:
			if (offset >= 240)
				return pgm_read_byte(&(irqvec[offset - 240]));
		}
		return 0;
	}
	if (page == 1 || address > ctx->last_ro_addr)
		for (i = 0; i < ctx->n_mpages; i++)
			if ((address & MPAGE_ADDR_MASK) == ctx->mpage[i].addr)
				return ctx->mpage[i].data[offset & MPAGE_DATA_MASK];
	if (page != 1 && address < 0x300 + (jbit_prg_code_pages << 8) +  jbit_prg_data_size)
		return get_prog_byte(address - 0x300);
#ifdef ENABLE_ROM
	if (jbit_rom_ptr != NULL &&
	  address >= 0xf000 && address < 0xf000 + jbit_rom_size)
		return pgm_read_byte(&(jbit_rom_ptr[address - 0xf000]));
#endif
	return 0;
}

#ifdef ENABLE_VM_TRACE_MEM
uint8_t read6502(uint16_t address) {
	uint8_t page = address >> 8;
	uint8_t offset = address & 0xff;
	uint8_t value;

	value = read6502_impl(address);
	vm_tracef("read %d:%d %d", page, offset, value);
	return value;
}
#endif

void write6502(uint16_t address, uint8_t value) {
	uint8_t page = address >> 8;
	uint8_t offset = address & 0xff;
	uint16_t a;
	int i;
#ifdef ENABLE_VM_TRACE_MEM
	vm_tracef("write %d:%d %d", page, offset, value);
#endif
	switch (page) {
	case 0:
		ctx->page0[offset] = value;
		return;
	case 2:
#ifdef ENABLE_MICROIO
		if (offset > 16 && offset < 80)
			microio_put(&microio, offset, value);
#endif
#ifdef ENABLE_PRIMO
		if (offset < 16 || offset > 80)
			primo_put(&primo, offset, value);
#endif
		return;
	case 255:
		switch (offset) {
		case REG(VMEXIT):
			if (value)
				ctx->vm_state = VM_STATE_HALTFAIL;
			else
				ctx->vm_state = VM_STATE_HALTOK;
			break;
		case REG(VMTRCLO):
#ifdef ENABLE_VM_TRACE
			vm_traceu((ctx->vm_trchi << 8) | value);
#endif
			break;
		case REG(VMTRCHI):
			ctx->vm_trchi = value;
			break;
		default:
			break;
		}
		return;
	}
	for (i = 0; i < ctx->n_mpages; i++)
		if ((address & MPAGE_ADDR_MASK) == ctx->mpage[i].addr)
			goto write_value;
	if (ctx->n_mpages == MAX_N_MPAGES) {
		ctx->vm_state = VM_STATE_OUTOFMEM;
		return;
	}
	i = ctx->n_mpages++;
	a = address & MPAGE_ADDR_MASK;
	ctx->mpage[i].addr = a;
#ifdef ENABLE_VM_TRACE_MPAGE
	vm_tracef("vm: mpage %2d  %3d:%03d-%03d", i, a >> 8, a & 0xff, (a & 0xff) | 0x1f);
#endif
	if (page != 1 && a < ctx->last_ro_addr)
		ctx->last_ro_addr = a - 1;
	if (page == 1 || ctx->mpage[i].addr > 0x300 + (jbit_prg_code_pages << 8) +  jbit_prg_data_size) {
		memset(ctx->mpage[i].data, 0, MPAGE_SIZE);
	} else {
		int j;
		for (j = 0; j < MPAGE_SIZE; j++)
			ctx->mpage[i].data[j] = get_prog_byte(a + j - 0x300);
	}
write_value:
	ctx->mpage[i].data[offset & MPAGE_DATA_MASK] = value;
}

static void process_events(uint8_t event, char c) {
#ifdef ENABLE_MICROIO
	if (event == KEYPAD_EVENT_LONGPRESS) {
		if (ctx->vm_state == VM_STATE_RUNNING) {
			ctx->vm_state = VM_STATE_BREAK;
			vm_stop("BRK");
		}
		return;
	}
	microio_keypress(&microio, c);
#endif
}

void vm_init() {
	memset(&ctx_, 0, sizeof(ctx_));
	ctx->last_ro_addr = 0xffff;
	vm_wait = VM_WAIT_MS;
	trace6502(1);
	reset6502();
#ifndef LCD_NULL
	lcd_clear();
#endif
#ifdef ENABLE_MICROIO
	microio_init(&microio);
#endif
#ifdef ENABLE_PRIMO
	primo_init(&primo);
#endif
#ifndef KEYPAD_NULL
	keypad_handler = process_events;
#endif
}

uint16_t vm_step() {
	int i = 0;

	if (ctx->vm_state != VM_STATE_RUNNING)
		return VM_WAIT_MS;
	vm_vsync = 0;
	for (i = 0; i < 10000 && !vm_vsync; i++) {
		step6502();
		if (ctx->vm_state != VM_STATE_RUNNING) {
			switch (ctx->vm_state) {
			case VM_STATE_OUTOFMEM:
				vm_stop("MEM");
				break;
			case VM_STATE_HALTOK:
				vm_stop("HLT");
				break;
			default:
				vm_stop("???");
				break;
			}
			break;
		}
	}
#ifdef ENABLE_MICROIO
	microio_lcd(&microio, 12, 1);
#endif
	return vm_wait;
}

#endif
