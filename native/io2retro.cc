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

// io2retro.cc

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "core.h"

#include "libretro.h"

static const int width = 128;
static const int height = 128;

static retro_environment_t env;
static retro_video_refresh_t video_refresh;
static retro_input_poll_t input_poll;
static retro_input_state_t input_state;

static retro_log_printf_t l;

static uint32_t *buffer;

static void fallback_log(enum retro_log_level level, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
}

static void check_variables() {
}

#define KEYPAD_MASK_KEYBOARD 0x01
#define KEYPAD_MASK_JOYPAD 0x02

static struct {
	char label;
	int keyboard_id;
	int joypad_id;
	uint8_t current;
	uint8_t last;
} keypad[] = {
	{ '1', '1', RETRO_DEVICE_ID_JOYPAD_Y, 0, 0 },
	{ '2', '2', RETRO_DEVICE_ID_JOYPAD_UP, 0, 0 },
	{ '3', '3', RETRO_DEVICE_ID_JOYPAD_X, 0, 0 },
	{ '4', '4', RETRO_DEVICE_ID_JOYPAD_LEFT, 0, 0 },
	{ '5', '5', RETRO_DEVICE_ID_JOYPAD_A, 0, 0 },
	{ '6', '6', RETRO_DEVICE_ID_JOYPAD_RIGHT, 0, 0 },
	{ '7', '7', RETRO_DEVICE_ID_JOYPAD_L, 0, 0 },
	{ '8', '8', RETRO_DEVICE_ID_JOYPAD_DOWN, 0, 0 },
	{ '9', '9', RETRO_DEVICE_ID_JOYPAD_R, 0, 0 },
	{ '*', ',', RETRO_DEVICE_ID_JOYPAD_SELECT, 0, 0 },
	{ '0', '0', RETRO_DEVICE_ID_JOYPAD_B, 0, 0 },
	{ '#', '.', RETRO_DEVICE_ID_JOYPAD_START, 0, 0 },
	{ 0, 0, 0, 0, 0 }
};

static void fetch_input() {
	input_poll();
	for (int i = 0; keypad[i].label; i++) {
		keypad[i].current = 0;
		if (input_state(0, RETRO_DEVICE_KEYBOARD, 0, keypad[i].keyboard_id))
			keypad[i].current |= KEYPAD_MASK_KEYBOARD;
		if (input_state(0, RETRO_DEVICE_JOYPAD, 0, keypad[i].joypad_id))
			keypad[i].current |= KEYPAD_MASK_JOYPAD;
	}
}

static void dispatch_keypress() {
	for (int i = 0; keypad[i].label; i++)
		if (keypad[i].current && !keypad[i].last)
			buffer[width * (height - 2) + i] ^= 0x000000ff;
}

static void commit_input() {
	for (int i = 0; keypad[i].label; i++)
		keypad[i].last = keypad[i].current;
}

static void render() {
	static int bi = 0;
	buffer[bi] ^= 0x00ffffff;
	if (++bi == width * 32)
		bi = 0;
	for (int i = 0; keypad[i].label; i++) {
		uint32_t color;
		if (keypad[i].current & KEYPAD_MASK_KEYBOARD)
			color = 0x00ff0000;
		else
			color = 0x00000000;
		buffer[width * (height - 1) + i] = color;
		if (keypad[i].current & KEYPAD_MASK_JOYPAD)
			color = 0x0000ff00;
		else
			color = 0x00000000;
		buffer[width * (height - 1) + i + 20] = color;
	}
}

extern "C"
void retro_set_video_refresh(retro_video_refresh_t cb) {
	video_refresh = cb;
}

extern "C"
void retro_set_audio_sample(retro_audio_sample_t cb) {
}

extern "C"
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
}

extern "C"
void retro_set_input_poll(retro_input_poll_t cb) {
	input_poll = cb;
}

extern "C"
void retro_set_input_state(retro_input_state_t cb) {
	input_state = cb;
}

extern "C"
void retro_init() {
	buffer = new uint32_t[width * height];
	memset(buffer, 0, width * height * sizeof(uint32_t));
}

extern "C"
void retro_deinit() {
	delete[] buffer;
	buffer = 0;
}

extern "C"
unsigned retro_api_version() {
	return RETRO_API_VERSION;
}

extern "C"
void retro_get_system_info(struct retro_system_info *info) {
	memset(info, 0, sizeof(*info));
	info->library_name = "JBit";
	info->library_version = get_jbit_version();
	info->need_fullpath = false;
	info->valid_extensions = "jb";
}

extern "C"
void retro_get_system_av_info(struct retro_system_av_info *info) {
	info->timing.fps = 60.0f;
	info->timing.sample_rate = 44100.0f;
	info->geometry.base_width = width;
	info->geometry.base_height = height;
	info->geometry.max_width = width;
	info->geometry.max_height = height;
	info->geometry.aspect_ratio = 1.0f;
}

extern "C"
void retro_set_controller_port_device(unsigned port, unsigned device) {
}

extern "C"
void retro_reset() {
}

extern "C"
void retro_run() {
	fetch_input();
	dispatch_keypress();
	render();
	commit_input();
	video_refresh(buffer, width, height, width * sizeof(uint32_t));
	bool updated = false;
	if (!env(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated))
		return;
	if (updated)
		check_variables();
}

extern "C"
size_t retro_serialize_size() {
	return 0;
}

extern "C"
bool retro_serialize(void *data_, size_t size) {
	return false;
}

extern "C"
bool retro_unserialize(const void *data_, size_t size) {
	return false;
}

extern "C"
void retro_cheat_reset() {
}

extern "C"
void retro_cheat_set(unsigned index, bool enabled, const char *code) {
}

extern "C"
bool retro_load_game(const struct retro_game_info *info) {
	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
	if (!env(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
		l(RETRO_LOG_ERROR, "Failed setting pixel format (XRGB8888).\n");
		return false;
	}
	check_variables();
	return true;
}

extern "C"
bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num) {
	return false;
}

extern "C"
void retro_unload_game() {
}

extern "C"
unsigned retro_get_region() {
	return RETRO_REGION_NTSC;
}

extern "C"
void *retro_get_memory_data(unsigned id) {
	return 0;
}

extern "C"
size_t retro_get_memory_size(unsigned id) {
	return 0;
}

extern "C"
void retro_set_environment(retro_environment_t cb) {
	env = cb;
	bool support_no_game = true; // TODO
	env(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &support_no_game);
	struct retro_log_callback log_callback;
	if (env(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log_callback))
		l = log_callback.log;
	else
		l = fallback_log;
}
