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

// rom.h

struct RomEntry {
	const char *file_name;
	const unsigned char *data;
	unsigned compressed_size;
	unsigned original_size;
};

class RomResource {
private:
	const char *name;
	const unsigned char *data;
	unsigned size;
	bool own_data;
	bool load_embedded();
	bool load_external();
	RomResource(const char *name_) : name(name_), data(0), own_data(false) {}
	~RomResource() { delete[] name; if (own_data) delete[] data; }
public:
	const unsigned char *get_data() const { return data; };
	unsigned get_size() const { return size; }
	static RomResource *get(const char *name);
	static void release(RomResource *res);
};

enum ImageResourceFormat {
	IRF_IRAWRGBA,
	IRF_IPNGGEN
};

class ImageResource {
private:
	RomResource *rom;
	ImageResourceFormat format;
	const unsigned char *data;
	unsigned size;
	unsigned width;
	unsigned height;
	ImageResource(RomResource *rom_) : rom(rom_), data(0) {}
	~ImageResource();
public:
	ImageResourceFormat get_format() const { return format; }
	const unsigned char *get_data() const { return data; };
	unsigned get_size() const { return size; }
	unsigned get_width() const { return width; }
	unsigned get_height() const { return height; }
	static ImageResource *get(const char *name);
	static void release(ImageResource *res);
};
