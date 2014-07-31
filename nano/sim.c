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

#include "nano.h"

microio_context_t microio;

const char *const keys = "0123456789*#";

void test_keypad() {
	int i, mask;

	lcd_goto(4, 5);
	keypad_scan();
	for (i = 0, mask = 1; i < 12; i++, mask <<= 1)
		lcd_char((keypad_state & mask) ? keys[i] : ' ');
}

static char vsync;

static const uint8_t code[] PROGMEM = {
	// loop1
	238, 40, 2, 141, 18, 2, 234, 76, 0, 3
};

static const uint8_t irqvec[] PROGMEM = {
	0, 4, // NMI
	0, 3, // RESET
	0, 4, // BRK
};

uint8_t read6502(uint16_t address) {
	int page = address >> 8;
	int offset = address & 0xff;
	if (page == 2) {
		if (offset >= 40 && offset < 80)
			return microio_get(&microio, offset);
	}
	if (page == 3) {
		if (offset < sizeof(code))
			return pgm_read_byte(&(code[offset]));
	}
	if (page == 255) {
		if (offset >= 250)
			return pgm_read_byte(&(irqvec[offset - 250]));
	}
#ifdef PLATFORM_PC
	printf("read6502: %d:%d\n", address >> 8, address & 0xff);
#endif
	return 0;
}

void write6502(uint16_t address, uint8_t value) {
	int page = address >> 8;
	int offset = address & 0xff;
	if (page != 2)
		goto error;
	if (offset >= 40 && offset < 80)
		microio_put(&microio, offset, value);
	if (offset == 18)
		vsync = 1;
	return;
error:
#ifdef PLATFORM_PC
	printf("write6502: %d:%d %d\n", address >> 8, address & 0xff, value);
#endif
	return;
}

void process_events(uint8_t event, uint8_t code) {
#ifdef PLATFORM_PC
	printf("event: %d %d\n", event, code);
#endif
}

void sim_init() {
	lcd_init();
	lcd_clear();
	keypad_init();
	trace6502(0);
	reset6502();
	microio_init(&microio);
	keypad_handler = process_events;
}

void sim_step() {
	int i = 0;

	vsync = 0;
	for (i = 0; i < 100 && !vsync; i++)
		step6502();
	microio_lcd(&microio, 12, 1);
	keypad_process();
	test_keypad();
}
