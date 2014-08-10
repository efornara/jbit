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

#include "embd.h"

#ifdef ENABLE_PRIMO

#include "_primo.h"

#define REG(x) (x - 0x200)

#define UNASSIGNED 0xff

#define FLAGS_ANALOG_SHIFTED 0x01

void primo_init(primo_context_t *ctx) {
	ctx->analog = 0;
	ctx->map = UNASSIGNED;
	ctx->io = UNASSIGNED;
	ctx->digital = UNASSIGNED;
	ctx->flags = 0;
	vm_wait = 0;
}

static void request(primo_context_t *ctx) {
	int n = ctx->reqn;
	ctx->reqn = 0;
	if (n == 0)
		goto error;
	switch (ctx->reqdat[0]) {
	case REQ_DELAY: {
		uint16_t ms;
		if (n == 2)
			ms = ctx->reqdat[1];
		else if (n == 3)
			ms = ctx->reqdat[1] | (ctx->reqdat[2] << 8);
		else
			goto error;
#ifdef ENABLE_PRIMO_TRACEREQ
		vm_tracef("primo req delay %d", ms);
#endif
		delay(ms);
		} break;
	case REQ_MILLIS: {
		uint32_t ms;
		if (n != 1)
			goto error;
		ms = millis();
#ifdef ENABLE_PRIMO_TRACEREQ
		vm_tracef("primo req millis %ld", ms);
#endif
		memcpy(&ctx->reqdat[0], &ms, sizeof(ms));
		} break;
	}
	ctx->reqres = 0;
	return;
error:
#ifdef ENABLE_PRIMO_TRACEREQ
	vm_tracef("primo req error");
#endif
	ctx->reqres = 0xff;
}

static void copy_request(primo_context_t *ctx, uint8_t lo) {
//	uint16_t address = (ctx->reqhi << 8) | lo;
	// TODO
	ctx->reqn = 0;
}

void primo_put(primo_context_t *ctx, uint8_t addr, uint8_t data) {
#ifdef ENABLE_PRIMO_TRACEIO
	vm_tracef("primo put %d %d", addr, data);
#endif
	switch (addr) {
	case REG(REQPUT):
		if (ctx->reqn < 4)
			ctx->reqdat[ctx->reqn++] = data;
		break;
	case REG(REQEND):
		request(ctx);
		break;
	case REG(REQPTRLO):
		copy_request(ctx, data);
		request(ctx);
		break;
	case REG(REQPTRHI):
		ctx->reqhi = data;
		break;
	case REG(IOID):
		ctx->io = data;
		break;
	case REG(DIGID):
		ctx->digital = data;
		break;
	case REG(DIGWCFG):
		pinMode(ctx->digital, data);
		break;
	case REG(DIGVAL):
		digitalWrite(ctx->digital, data);
		break;
	case REG(DIGPWM):
		analogWrite(ctx->digital, data);
		break;
	case REG(ANLGLO):
		ctx->analog = analogRead(data);
		ctx->flags &= ~FLAGS_ANALOG_SHIFTED;
		break;
	case REG(ANLGHI):
		ctx->analog = analogRead(data);
		ctx->flags |= FLAGS_ANALOG_SHIFTED;
		break;
	}
}

#ifdef ENABLE_PRIMO_TRACEIO
uint8_t primo_get_impl(primo_context_t *ctx, uint8_t addr) {
#else
uint8_t primo_get(primo_context_t *ctx, uint8_t addr) {
#endif
	switch (addr) {
	case REG(IOID):
		return ctx->io;
	case REG(DIGID):
		return ctx->digital;
	case REG(DIGWCFG):
		return 0xff;
	case REG(DIGVAL):
		return digitalRead(ctx->digital);
	case REG(ANLGLO):
		return ctx->analog & 0xff;
	case REG(ANLGHI):
		if (ctx->flags & FLAGS_ANALOG_SHIFTED)
			return ctx->analog >> 2;
		else
			return ctx->analog >> 8;
	default:
		if (addr >= REG(REQDAT) && addr < REG(REQDAT) + 4)
			return ctx->reqdat[addr - REG(REQDAT)];
	}
	return 0;
}

#ifdef ENABLE_PRIMO_TRACEIO
uint8_t primo_get(primo_context_t *ctx, uint8_t addr) {
	uint8_t value;

	value = primo_get_impl(ctx, addr);
	vm_tracef("primo get %d %d", addr, value);
	return value;
}
#endif

#endif
