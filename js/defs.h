/*
 * Copyright (C) 2012  Emanuele Fornara
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

// defs.h

#include <stdint.h>

#define STATUS_RUNNING 0
#define STATUS_HALTED 1
#define STATUS_FAILED 2

#define KEYCODE_NONE 0
#define KEYCODE_BREAK -1

void fatal(const char *format, ...);
char *load_res_file(const char *res_file_name, int *size);

#define FRAME_WIDTH 128
#define FRAME_HEIGHT 128

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 14

// f is WIDTH * HEIGHT * uint32_t (0xRRGGBBAA)
void frame_clear(uint32_t *f, uint32_t bg); 
void frame_putchar(uint32_t *f, int x, int y, int c, uint32_t fg, uint32_t bg);
void frame_screenshot(uint32_t *f, const char *file_name);

void skin_init(void);
void skin_cleanup(void);
void skin_update_video(const unsigned char *video);
void skin_update_status(int status);
void skin_flush(void);
int skin_poll_key(void);

int sdl_show(void);
int sdl_hide(void);
int sdl_poll_show(void);
void sdl_flush(uint32_t *f);

void jsengine_init(void);
void jsengine_cleanup(void);
void jsengine_load(const char *expr);
int jsengine_run(const char *expr);

const char *c_update(const unsigned char *video);
const char *c_setstatus(const char *status);
