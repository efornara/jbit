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

#include <Arduino.h>

#include "nano.h"

#ifdef ENABLE_PRIMO

#include "_primo.h"

#define REG(x) (x - 0x200)

#define UNASSIGNED 0xff

extern "C" void primo_init(primo_context_t *ctx) {
	ctx->map = UNASSIGNED;
	ctx->io = UNASSIGNED;
	ctx->digital = UNASSIGNED;
	ctx->analog = 0;
}

extern "C" void primo_put(primo_context_t *ctx, uint8_t addr, uint8_t data) {
#ifdef ENABLE_SERIAL_TRACE
	serial_trace("primo put %d %d", addr, data);
#endif
	switch (addr) {
	case REG(IOID):
		ctx->io = data;
		break;
	case REG(DIGID):
		ctx->digital = data;
		break;
	case REG(DIGCFG):
		pinMode(ctx->digital, data);
		break;
	case REG(DIGVAL):
		digitalWrite(ctx->digital, data);
		break;
	}
}

extern "C" uint8_t primo_get(primo_context_t *ctx, uint8_t addr) {
#ifdef ENABLE_SERIAL_TRACE
	serial_trace("primo get %d", addr);
#endif
	switch (addr) {
	case REG(IOID):
		return ctx->io;
	case REG(DIGID):
		return ctx->digital;
	case REG(DIGCFG):
		return 255;
	case REG(DIGVAL):
		return digitalRead(ctx->digital);
	}
	return 0;
}

#endif
