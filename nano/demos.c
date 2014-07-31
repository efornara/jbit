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

static const uint8_t ciao_code[] PROGMEM = {
	169,67,141,53,2,169,73,141,54,2,169,65,141,55,2,169,79,141,56,2
};

static const uint8_t loop1_code[] PROGMEM = {
	238,40,2,141,18,2,234,76,0,3
};

static const uint8_t fill1_code[] PROGMEM = {
	162,0,169,88,157,40,2,232,224,40,208,248
};

static const uint8_t fill2_code[] PROGMEM = {
	162,40,169,88,157,39,2,202,208,250
};

static const uint8_t loop2_code[] PROGMEM = {
	169,4,141,17,2,169,65,141,40,2,141,18,2,169,25,170,238,40,2,141,
	18,2,202,208,247,170,206,40,2,141,18,2,202,208,247,76,15,3
};

static const uint8_t loop3_code[] PROGMEM = {
	160,65,162,0,169,32,157,40,2,232,152,157,40,2,141,18,2,224,9,208,
	239,169,32,157,40,2,202,152,157,40,2,141,18,2,224,0,208,239,200,192,
	71,208,215,169,32,157,40,2
};

static const uint8_t keypad_code[] PROGMEM = {
	169,0,141,18,2,173,24,2,240,248,201,42,240,30,162,0,157,40,2,168,
	169,0,141,18,2,169,32,157,40,2,152,232,224,10,208,236,169,1,141,24,
	2,76,0,3
};

static const uint8_t random_code[] PROGMEM = {
	169,5,141,23,2,173,23,2,24,105,49,141,53,2,173,23,2,24,105,49,
	141,56,2,141,18,2,173,24,2,240,248,162,1,142,24,2,201,42,208,221
};

static const uint8_t charset_code[] PROGMEM = {
	169,0,170,160,0,153,40,2,24,105,1,200,192,40,208,245,138,141,18,2,
	172,24,2,240,248,192,50,240,15,192,56,240,20,192,48,240,25,160,1,140,
	24,2,208,214,201,0,240,245,56,233,10,176,240,201,220,240,236,24,105,10,
	144,231
};

#define DEMO_PRG ciao_code

const uint8_t *const demo_code = DEMO_PRG;
const uint8_t demo_size = sizeof(DEMO_PRG);
