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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#ifdef __I86__

bool RomResource::load_embedded() {
	return false;
}

#else

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "stb_image.h"

extern const RomEntry rom_entries[];
extern int n_of_roms;

bool RomResource::load_embedded() {
	const RomEntry *e = 0;
	for (int i = 0; i < n_of_roms; i++) {
		if (!strcmp(name, rom_entries[i].file_name)) {
			e = &rom_entries[i];
			break;
		}
	}
	if (!e)
		return false;
	const int n = e->original_size;
	data = new unsigned char[n];
	const int decoded_n = stbi_zlib_decode_buffer((char *)data, n,
	  (const char *)e->data, e->compressed_size);
	assert(n == decoded_n);
	size = n;
	return true;
}

#endif

#ifndef __I86__

bool RomResource::load_external() {
	return false;
}

#else

bool RomResource::load_external() {
	bool ret = false;
	FILE *f = 0;
	long n;
	if (!(f = fopen(name, "rb")))
		goto done;
	if (fseek(f, 0, SEEK_END))
		goto done;
	n = ftell(f);
	if (n <= 0)
		goto done;
	rewind(f);
	data = new unsigned char[n];
	if (fread(data, n, 1, f) != 1)
		goto done;
	size = (int)n;
	ret = true;
done:
	if (f)
		fclose(f);
	return ret;
}

#endif

RomResource *RomResource::get(const char *name) {
	RomResource *res;
	const int n = strlen(name) + 1;
	char *s = new char[n];
	memcpy(s, name , n);
	res = new RomResource(s);
	if (res->load_embedded())
		return res;
	if (res->load_external())
		return res;
	delete res;
	return 0;
}

void RomResource::release(RomResource *res) {
	delete res;
}
