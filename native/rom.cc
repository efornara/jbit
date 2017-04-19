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

// rom.cc

#include "rom.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "stb_image.h"

extern const RomEntry rom_entries[];
extern RomResource rom_resources[];
extern int n_of_roms;

void RomResource::release() {
	delete[] data;
	data = 0;
}

struct Rom {
	const RomEntry *e;
	RomResource *r;
};

static Rom find(const char *name) {
	Rom rom = { 0, 0 };
	for (int i = 0; i < n_of_roms; i++) {
		if (!strcmp(name, rom_entries[i].file_name)) {
			rom.e = &rom_entries[i];
			rom.r = &rom_resources[i];
			break;
		}
	}
	return rom;
}

RomResource *RomResource::load(const char *name) {
	Rom rom = find(name);
	if (!rom.e)
		return 0;
	if (rom.r->data)
		return rom.r;
	const int n = rom.e->original_size;
	rom.r->data = new uint8_t[n];
	const int decoded_n = stbi_zlib_decode_buffer((char *)rom.r->data, n,
	  (const char *)rom.e->data, rom.e->compressed_size);
	assert(n == decoded_n);
	rom.r->size = n;
	return rom.r;
}

void RomResource::cleanup() {
	for (int i = 0; i < n_of_roms; i++)
		rom_resources[i].release();
}
