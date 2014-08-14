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

#define HWSIM_M_DISPLAY 'd'
#define HWSIM_M_WINDOW 'w'

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
} hwsim_rect_t;

#define HWSIM_C_BODY 1
#define HWSIM_C_DISPLAY_BG 2
#define HWSIM_C_DISPLAY_FG 3
#define HWSIM_C_KEY_BG 4

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} hwsim_color_t;

typedef struct {
	uint16_t key_pressed[12];
	uint16_t keypad_state;
	uint8_t video[10000]; // TODO
} hwsim_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const char *hwsim_keypad_labels;
extern const char *hwsim_keypad_subs[];

void hwsim_init(hwsim_t *hw);
void hwsim_cleanup(hwsim_t *hw);
int hwsim_get_metrics(hwsim_t *hw, int element, hwsim_rect_t *m);
int hwsim_get_color(hwsim_t *hw, int element, hwsim_color_t *c);
int hwsim_keypad_update(hwsim_t *hw, int key_down, int value);

#ifdef __cplusplus
};
#endif
