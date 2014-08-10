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

#include "embd.h"

extern const uint8_t autorun_jb[] PROGMEM;

const uint8_t *jbit_prg_code_ptr;
const uint8_t *jbit_prg_data_ptr;
uint16_t jbit_prg_code_size;
uint16_t jbit_prg_data_size;
uint8_t jbit_prg_code_pages;
uint8_t jbit_prg_pgm;

const uint8_t *jbit_rom_data = NULL;

#ifndef ENABLE_UI
uint8_t ui_state = 0;
uint8_t ui_result = 0;
#endif

#ifdef ENABLE_VM
extern void vm_init();
extern uint16_t vm_step();
#endif

#ifdef ENABLE_DEMOS
extern void demos_init();
extern void demos_step();
#endif

static uint8_t module;

static const char *const modules[] = {
#ifdef ENABLE_DEMOS
	"Demos",
#endif
	0
};

#define SIZE_HEADER 12
#define OFFSET_CODEPAGES 8
#define OFFSET_DATAPAGES 9

void jbit_init() {
	lcd_init();
	lcd_clear();
	keypad_init();
#if defined(ENABLE_AUTORUN)
	jbit_prg_code_ptr = &autorun_jb[SIZE_HEADER];
	jbit_prg_code_pages = pgm_read_byte(&autorun_jb[OFFSET_CODEPAGES]);
	jbit_prg_code_size = jbit_prg_code_pages << 8;
	jbit_prg_data_ptr = &autorun_jb[SIZE_HEADER + jbit_prg_code_size];
	jbit_prg_data_size =  pgm_read_byte(&autorun_jb[OFFSET_DATAPAGES]) << 8;
	jbit_prg_pgm = 1;
	jbit_replace_with(MODULE_VM);
#elif defined(ENABLE_SERIAL)
	serial_loader();
	jbit_replace_with(MODULE_VM);
#elif defined(PLATFORM_PC)
	if (jbit_prg_code_ptr) {
		jbit_replace_with(MODULE_VM);
		return;
	}
#else
	jbit_replace_with(MODULE_JBIT);
#endif
}

void jbit_replace_with(int module_) {
	module = module_;
	keypad_handler = 0;
	switch (module) {
	case MODULE_JBIT:
#ifdef ENABLE_UI
		ui_menu("JBit", modules);
#endif
		break;
#ifdef ENABLE_VM
	case MODULE_VM:
		vm_init();
		break;
#endif
#ifdef ENABLE_DEMOS
	case MODULE_DEMOS:
		demos_init();
		break;
#endif
	default:
		jbit_replace_with(MODULE_JBIT);
	}
}

static int item_selected() {
	const char *name = modules[ui_result];
	if (!name)
		return MODULE_JBIT;
#ifdef ENABLE_DEMOS
	if (!strcmp(name, "Demos"))
		return MODULE_DEMOS;
#endif
	return MODULE_JBIT;
}

uint16_t jbit_step() {
	uint16_t w = 100;
	keypad_scan();
	keypad_process();
	if (ui_state)
		return w;
	switch (module) {
	case MODULE_JBIT:
		jbit_replace_with(item_selected());
		break;
#ifdef ENABLE_VM
	case MODULE_VM:
		w = vm_step();
		break;
#endif
#ifdef ENABLE_DEMOS
	case MODULE_DEMOS:
		demos_step();
		break;
#endif
	}
	return w;
}
