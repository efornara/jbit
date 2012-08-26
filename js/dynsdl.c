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

// dynsdl.c

#include "defs.h"

#include <stdio.h>
#include <string.h>

#include <dlfcn.h>

#define ZOOM_X2

#define INIT_FLAGS 0x20 // SDL_INIT_VIDEO
#define EVENT_QUIT 12 // SDL_QUIT

typedef struct SDL_Surface {
	uint32_t flags;
	void *format;
	int w, h;
	uint16_t pitch;
	uint32_t *pixels;
} Surface;

typedef struct {
	int16_t x, y;
	uint16_t w, h;
} Rect;

typedef struct {
	uint8_t type;
	int pad[64];
} Event;

static void *h = NULL;
static int (*Init)(uint32_t flags);
static void (*Quit)(void);
static Surface *(*SetVideoMode)(int width, int height, int bpp, uint32_t flags);
static void (*WM_SetCaption)(const char *title, const char *icon);
static void (*Flip)(void *surface);
static Surface *(*CreateRGBSurface)(uint32_t flags, int width, int height, int depth, uint32_t redmask, uint32_t greenmask, uint32_t bluemask, uint32_t alphamask);
static void (*FreeSurface)(Surface *surface);
static void (*UpperBlit)(Surface *src, Rect *srcrect, Surface *dst, Rect *dstrect);
static int (*PollEvent)(Event *event);

static int load(void)
{
	static int tried = 0;

	if (tried)
		return h != NULL;
	tried = 1;
	if ((h = dlopen("libSDL.so", RTLD_LAZY)) == NULL)
		goto error;
	if ((Init = dlsym(h, "SDL_Init")) == NULL)
		goto error;
	if ((Quit = dlsym(h, "SDL_Quit")) == NULL)
		goto error;
	if ((SetVideoMode = dlsym(h, "SDL_SetVideoMode")) == NULL)
		goto error;
	if ((WM_SetCaption = dlsym(h, "SDL_WM_SetCaption")) == NULL)
		goto error;
	if ((Flip = dlsym(h, "SDL_Flip")) == NULL)
		goto error;
	if ((CreateRGBSurface = dlsym(h, "SDL_CreateRGBSurface")) == NULL)
		goto error;
	if ((FreeSurface = dlsym(h, "SDL_FreeSurface")) == NULL)
		goto error;
	if ((UpperBlit = dlsym(h, "SDL_UpperBlit")) == NULL)
		goto error;
	if ((PollEvent = dlsym(h, "SDL_PollEvent")) == NULL)
		goto error;
	return 1;
error:
	if (h != NULL) {
		dlclose(h);
		h = NULL;
	}
	return 0;
}

#ifdef ZOOM_X2
#define WIDTH (FRAME_WIDTH * 2)
#define HEIGHT (FRAME_HEIGHT * 2)
#else
#define WIDTH FRAME_WIDTH
#define HEIGHT FRAME_HEIGHT
#endif

static int initialized = 0;
static Rect rect = { 0, 0, WIDTH, HEIGHT };
static Surface *screen = NULL;
static Surface *display = NULL;

int sdl_show(void)
{
	if (h == NULL && !load())
		return 0;
	if (Init(INIT_FLAGS))
		return 0;
	if ((screen = SetVideoMode(WIDTH, HEIGHT, 24, 0)) == NULL) {
		Quit();
		return 0;
	}
	if ((display = CreateRGBSurface(0, WIDTH, HEIGHT, 32,
	  0xff000000, 0x00ff0000, 0x0000ff00, 0x00000000)) == NULL) {
		screen = NULL;
		Quit();
		return 0;
	}
	initialized = 1;
	WM_SetCaption("JBit", "JBit");
	return 1;
}

int sdl_hide(void)
{
	if (h == NULL)
		return 0;
	if (!initialized)
		return 0;
	FreeSurface(display);
	display = NULL;
	screen = NULL;
	Quit();
	initialized = 0;
	return 0;
}

int sdl_poll_show(void)
{
	Event event;

	if (h == NULL || !initialized)
		return 0;
	while (1) {
		if (!PollEvent(&event))
			return 1;
		if (event.type == EVENT_QUIT) {
			sdl_hide();
			return 0;
		}
	}
}

#ifdef ZOOM_X2
static void copy_x2(uint32_t *f)
{
	int i, j, x, y;

	i = j = 0;
	for (y = 0; y < FRAME_HEIGHT; y++) {
		for (x = 0; x < FRAME_WIDTH; x++) {
			int c = f[j++];
			display->pixels[i] = c;
			display->pixels[i + 1] = c;
			display->pixels[i + WIDTH] = c;
			display->pixels[i + 1 + WIDTH] = c;
			i += 2;
		}
		i += WIDTH;
	}
}
#endif

void sdl_flush(uint32_t *f)
{
	if (h == NULL || !initialized)
		return;
#ifdef ZOOM_X2
	copy_x2(f);
#else
	memcpy(display->pixels, f, sizeof(uint32_t) * WIDTH * HEIGHT);
#endif
	UpperBlit(display, &rect, screen, &rect);
	Flip(screen);
}
