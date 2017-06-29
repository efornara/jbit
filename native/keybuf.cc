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

// keybuf.cc

#include <stdio.h>
#include <string.h>

#include "jbit.h"
#include "devparts.h"

int MicroIOKeybuf::map_keypad(int c) {
	switch (c) {
	case 'a':
	case 'b':
	case 'c':
		c = '2';
		break;
	case 'd':
	case 'e':
	case 'f':
		c = '3';
		break;
	case 'g':
	case 'h':
	case 'i':
		c = '4';
		break;
	case 'j':
	case 'k':
	case 'l':
		c = '5';
		break;
	case 'm':
	case 'n':
	case 'o':
		c = '6';
		break;
	case 'p':
	case 'q':
	case 'r':
	case 's':
		c = '7';
		break;
	case 't':
	case 'u':
	case 'v':
		c = '8';
		break;
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		c = '9';
		break;
	case '+':
		c = '0';
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '*':
	case '0':
	case '#':
		break;
	default:
		c = 0;
	}
	return c;
}

void MicroIOKeybuf::reset() {
	memset(key_buf, 0, sizeof(key_buf));
}

void MicroIOKeybuf::put(int address, int value) {
	for (int i = 0; i < KEYBUF_SIZE - 1; i++)
		key_buf[i] = key_buf[i + 1];
	key_buf[KEYBUF_SIZE - 1] = 0;
}

int MicroIOKeybuf::get(int address) const {
	return key_buf[address] & 0xFF;
}

void MicroIOKeybuf::enque(int value) {
	for (int i = 0; i < KEYBUF_SIZE; i++)
		if (key_buf[i] == 0) {
			key_buf[i] = (char)value;
			return;
		}
}
