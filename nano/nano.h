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
#define LCD_HEIGHT (LCD_ROWS * 8)

#define LCD_BITMAP_SIZE (LCD_WIDTH * LCD_HEIGHT)

#define LCD_HWSIM_CMD_CLEAR 4

void lcd_init();
void lcd_write(unsigned char dc, unsigned char data);

void lcd_clear();
void lcd_goto(int col, int row);
void lcd_home();
void lcd_char(char c);

/* KEYPAD */

#define KEYPAD_CODE_0 0
#define KEYPAD_CODE_1 1
#define KEYPAD_CODE_2 2
#define KEYPAD_CODE_3 3
#define KEYPAD_CODE_4 4
#define KEYPAD_CODE_5 5
#define KEYPAD_CODE_6 6
#define KEYPAD_CODE_7 7
#define KEYPAD_CODE_8 8
#define KEYPAD_CODE_9 9
#define KEYPAD_CODE_STAR 10
#define KEYPAD_CODE_HASH 11

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

extern uint16_t keypad_state;

void keypad_init();
void keypad_scan();

typedef void (*keypad_handler_t)(uint8_t event, uint8_t code);

extern keypad_handler_t keypad_handler;

void keypad_process();

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

typedef struct {
	uint8_t convideo[MICROIO_CONVIDEO_SIZE];
} microio_context_t;

void microio_init(microio_context_t *ctx);
void microio_put(microio_context_t *ctx, uint8_t addr, uint8_t data);
uint8_t microio_get(microio_context_t *ctx, uint8_t addr);

void microio_lcd(microio_context_t *ctx, int x, int y);

/* JBIT SIM */

void sim_init();
void sim_step();

#ifdef __cplusplus
};
#endif
