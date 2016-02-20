/*
 * Copyright (C) 2012-2016  Emanuele Fornara
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

// devimpl.h

class Random {
private:
	static const long long MAXRAND = 0xFFFFFFFFFFFFLL;
	long long seed[2];
	int n;
	long long divisor;
	long long next();
public:
	Random() { reset(); }
	void reset();
	int get();
	void put(int max);
};

class MicroIODisplay {
public:
	static const int COLS = 10;
	static const int ROWS = 4;
	static const int CONVIDEO_SIZE = COLS * ROWS;
	static const int N_OF_LINES = ROWS + 2;
private:
	char video_buf[CONVIDEO_SIZE];
	mutable char line_buf[COLS + 3];
public:
	void reset();
	void put(int address, int value);
	int get(int address) const;
	const char *get_line(int i) const;
};

class MicroIOKeybuf {
public:
	static const int KEYBUF_SIZE = 8;
private:
	char key_buf[KEYBUF_SIZE];
public:
	static int map_keypad(int c);
	void reset();
	void put(int address, int value);
	int get(int address) const;
	void enque(int value);
};
