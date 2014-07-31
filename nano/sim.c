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

uint8_t read6502(uint16_t address) {
	return 0;
}

void write6502(uint16_t address, uint8_t value) {
}

void sim_init() {
	lcd_init();
	lcd_clear();
	keypad_init();
	reset6502();
	microio_init(&microio);
}

void sim_step() {
	static uint8_t c = 0;
	int i;

	step6502();
	for (i = 40; i < 80; i++)
		microio_put(&microio, i, c);
	c++;
	microio_lcd(&microio, 12, 1);
	test_keypad();
}
