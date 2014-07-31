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

#ifdef PLATFORM_PC
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#endif
#include <stdint.h>

#ifdef __AVR
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define pgm_read_byte(p) (*(p))
#define pgm_read_word(p) (*(p))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* CONFIG */

/* LCD: uncomment one of the following */
//#define LCD_NULL
#define LCD_HWSIM
//#define LCD_REAL

/* KEYPAD: uncomment one of the following */
//#define KEYPAD_NULL
#define KEYPAD_HWSIM
//#define KEYPAD_REAL

/* LCD */

#define LCD_COMMAND 0
#define LCD_DATA 1

#define LCD_WIDTH 84
#define LCD_ROWS 6
#define LCD_CHAR_HEIGHT 8
#define LCD_HEIGHT (LCD_ROWS * LCD_CHAR_HEIGHT)

#define LCD_FIXED_CHAR_WIDTH 6

#define LCD_BITMAP_SIZE (LCD_WIDTH * LCD_HEIGHT)

#define LCD_HWSIM_CMD_CLEAR 4

void lcd_init();
void lcd_write(unsigned char dc, unsigned char data);

void lcd_clear();
void lcd_goto(int col, int row);
void lcd_home();
void lcd_char(char c);

/* KEYPAD */

#define UP '2'
#define DOWN '8'
#define LEFT '4'
#define RIGHT '6'
#define SELECT '5'

#define KEYPAD_MASK_0 (1 << 0)
#define KEYPAD_MASK_1 (1 << 1)
#define KEYPAD_MASK_2 (1 << 2)
#define KEYPAD_MASK_3 (1 << 3)
#define KEYPAD_MASK_4 (1 << 4)
#define KEYPAD_MASK_5 (1 << 5)
#define KEYPAD_MASK_6 (1 << 6)
#define KEYPAD_MASK_7 (1 << 7)
#define KEYPAD_MASK_8 (1 << 8)
#define KEYPAD_MASK_9 (1 << 9)
#define KEYPAD_MASK_STAR (1 << 10)
#define KEYPAD_MASK_HASH (1 << 11)

#define KEYPAD_EVENT_RELEASE 0
#define KEYPAD_EVENT_LONGPRESS 1

extern const char *const keypad_labels;

extern uint16_t keypad_state;

void keypad_init();
void keypad_scan();

typedef void (*keypad_handler_t)(uint8_t event, char c);

extern keypad_handler_t keypad_handler;

void keypad_process();

/* SYS */

int sys_get_random_seed();

/* UI */

extern uint8_t ui_state;
extern uint8_t ui_result;

void ui_msg(const char *title, const char *msg);
void ui_menu(const char *title, const char *const items[]);

/* 6502 */

/*
  Fake6502 by Mike Chambers looks like a good CPU emulator.
  It adds 6K of flash and needs some fixes to make it use pgm, but it
  should be fairly easy to integrate.
  In the future, it would be nice to have an option to select another
  emulator with a trade-off that is a better fit for jbit nano
  (i.e. slower but smaller).
 */

void reset6502();
void step6502();

void trace6502(int enable);

/* MICRO IO */

#define MICROIO_CONVIDEO_SIZE 40
#define MICROIO_KEYBUF_SIZE 8

typedef struct {
	uint8_t convideo[MICROIO_CONVIDEO_SIZE];
	uint8_t keybuf[MICROIO_KEYBUF_SIZE];
	long long r_seed[2];
	long long r_divisor;
	uint8_t r_n_minus_1;
} microio_context_t;

void microio_init(microio_context_t *ctx, int random_seed);
void microio_put(microio_context_t *ctx, uint8_t addr, uint8_t data);
uint8_t microio_get(microio_context_t *ctx, uint8_t addr);

void microio_lcd(microio_context_t *ctx, uint8_t x, uint8_t y);
void microio_keypress(microio_context_t *ctx, uint8_t code);

/* MAIN */

#define MODULE_JBIT 0
#define MODULE_DEMOS 1
#define MODULE_SIM 2

void jbit_init();
void jbit_step();
void jbit_replace_with(int module);

#ifdef __cplusplus
};
#endif
