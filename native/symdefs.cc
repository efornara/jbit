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

// symdefs.cc

#include <string.h>

#include "core.h"

static const SymDef stdout_symdefs[] = {
{"PUTCHAR",0x0200},
{"PUTUINT8",0x0216},
{0,0},
}; 

static const SymDef xv65_symdefs[] = {
#include "d_xv65.h"
{ 0, 0 },
}; 

static const SymDef microio_symdefs[] = {
#include "d_mio.h"
{ 0, 0 },
}; 

static const SymDef io2_symdefs[] = {
#include "d_io2.h"
{ 0, 0 },
}; 

const char *asm_devices[] = {
	"stdout",
	"xv65",
	"microio",
	"io2",
	0
};

const SymDef *get_device_symdefs(const char *device) {
	if (!strcmp(device, "stdout"))
		return stdout_symdefs;
	if (!strcmp(device, "xv65"))
		return xv65_symdefs;
	if (!strcmp(device, "microio"))
		return microio_symdefs;
	if (!strcmp(device, "io2"))
		return io2_symdefs;
	return 0;
}
