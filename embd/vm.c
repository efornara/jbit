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

#include <string.h>
#include <stdarg.h>

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

void test_keypad() {
	int i, mask;

	lcd_goto(4, 5);
	keypad_scan();
	for (i = 0, mask = 1; i < 12; i++, mask <<= 1)
		lcd_char((keypad_state & mask) ? keypad_labels[i] : ' ');
}

uint8_t vm_vsync;
uint16_t vm_wait;

#ifdef ENABLE_TRACE

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

void vm_fatal(const char *msg) {
#ifdef ENABLE_TRACE
	vm_traces(msg);
#endif
#ifdef PLATFORM_PC
	abort();
#else
	// TODO
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

#define VM_STATE_RUNNING 0
#define VM_STATE_HALTOK 1
#define VM_STATE_HALTFAIL 2
#define VM_STATE_INVOP 3
#define VM_STATE_OUTOFMEM 4

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
	if (jbit_prg_pgm)
		return pgm_read_byte(&(jbit_prg_code[offset]));
	else
		return jbit_prg_code[offset];
}

#define REG(x) (x - 0xff00)

uint8_t read6502(uint16_t address) {
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
	if (page != 1 && address < 0x300 + jbit_prg_size)
		return get_prog_byte(address - 0x300);
	return 0;
}

void write6502(uint16_t address, uint8_t value) {
	uint8_t page = address >> 8;
	uint8_t offset = address & 0xff;
	int i, a;
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
#ifdef ENABLE_TRACE
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
#ifdef ENABLE_TRACE
	vm_tracef("vm: mpage %2d  %3d:%03d-%03d", i, a >> 8, a & 0xff, (a & 0xff) | 0x1f);
#endif
	if (page != 1 && a < ctx->last_ro_addr)
		ctx->last_ro_addr = a - 1;
	if (page == 1 || ctx->mpage[i].addr > 0x300 + jbit_prg_size) {
		memset(ctx->mpage[i].data, 0, MPAGE_SIZE);
	} else {
		int j;
		for (j = 0; j < MPAGE_SIZE; j++)
			if (a + j < 0x300 + jbit_prg_size)
				ctx->mpage[i].data[j] = get_prog_byte(a + j - 0x300);
			else
				ctx->mpage[i].data[j] = 0;
	}
write_value:
	ctx->mpage[i].data[offset & MPAGE_DATA_MASK] = value;
}

static void process_events(uint8_t event, char c) {
#ifdef ENABLE_MICROIO
	microio_keypress(&microio, c);
#endif
}

void vm_init() {
	memset(&ctx_, 0, sizeof(ctx_));
	ctx->last_ro_addr = 0xffff;
	vm_wait = 100;
	trace6502(0);
	reset6502();
	lcd_clear();
#ifdef ENABLE_MICROIO
	microio_init(&microio);
#endif
#ifdef ENABLE_PRIMO
	primo_init(&primo);
#endif
	keypad_handler = process_events;
}

uint16_t vm_step() {
	int i = 0;

	vm_vsync = 0;
	for (i = 0; i < 10000 && !vm_vsync; i++) {
		step6502();
		switch (ctx->vm_state) {
			case VM_STATE_OUTOFMEM:
				vm_fatal("vm: out of memory");
			case VM_STATE_RUNNING:
				break;
			case VM_STATE_HALTOK:
#if defined(PLATFORM_PC) && !defined(PLATFORM_PC_SDL)
				exit(0);
#else
				jbit_replace_with(MODULE_JBIT);
#endif
				return 0;
			default:
				vm_fatal("vm: unknown state");
				break;
		}
	}
#ifdef ENABLE_MICROIO
	microio_lcd(&microio, 12, 1);
#endif
	return vm_wait;
}

#endif
