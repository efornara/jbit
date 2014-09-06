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

#include "embd.h"


#if defined(ENABLE_SERIAL) || defined(LCD_HWSIM) || defined(KEYPAD_HWSIM)

void connect_to_hwsim() {
  static bool hwsim_started = false;
  if (!hwsim_started) {
    hwsim_started = true;
    Serial.begin(115200);
    while (true) {
      delayMicroseconds(50);
      while (Serial.available() > 0) {
        if (Serial.read() == '\r')
          return;
      }
    }
  }
}

#endif


#if defined(ENABLE_SERIAL)

static uint8_t *code;
static int state;

static void serial_loader_init() {
  connect_to_hwsim();
  jbit_prg_pgm = 0;
  code = NULL;
  state = -1;
}

static void get_line(char *buf, int len) {
  int i = 0;
  while (1) {
    while (Serial.available() > 0) {
      int value = Serial.read();
      switch (value) {
      case '\r':
        buf[i] = '\0';
        return;
      case '\n':
        continue; // skip
      default:
        buf[i++] = value;
      }
    }
  }
}

// TODO serial trace without serial loader
#if defined(ENABLE_VM_TRACE)

void vm_traces(const char *msg) {
  Serial.print("# ");
  Serial.print(msg);
  Serial.print("\n\r");
}

#endif

static int serial_loader_error(const char *msg) {
  Serial.print("# error: ");
  Serial.print(msg);
  Serial.print("\n\r");
  return -1;
}

static int serial_loader_step() {
  char line[64];
  char dirty;
  int nv;
  int ret = -1;
  get_line(line, sizeof(line));
  if (state == -1 || line[0] == 'P') {
    int n_code_pages;
    if ((nv = sscanf(line, "P %d %d %d%c",
     &n_code_pages, &jbit_prg_code_size, &jbit_prg_data_size,
     &dirty)) != 3)
      return serial_loader_error("header");
	jbit_prg_code_pages = n_code_pages;
    if (code)
      free(code);
    if ((code = (uint8_t *)malloc(jbit_prg_code_size + jbit_prg_data_size))
     == NULL)
      return serial_loader_error("malloc");
    state = 0;
  } else if (state < jbit_prg_code_size + jbit_prg_data_size) {
    int pos, data;
    if ((nv = sscanf(line, "B %d %d%c", &pos, &data, &dirty)) != 2)
      return serial_loader_error("byte");
    if (pos != state)
      return serial_loader_error("pos");
    if (data < 0 || data > 255)
      return serial_loader_error("data");
    code[pos] = data;
    state++;
    if (state == jbit_prg_code_size + jbit_prg_data_size)
      ret = 0;
  }
  Serial.print(line);
  Serial.print("\n\r");
  return ret;
}

extern "C" void serial_loader() {
  serial_loader_init();
  while (serial_loader_step())
    ;
  jbit_prg_code_ptr = code;
  jbit_prg_data_ptr = &code[jbit_prg_code_size];
}

#endif


#if defined(LCD_NULL)

#elif defined(LCD_HWSIM)

extern "C" void lcd_init() {
  connect_to_hwsim();
}

extern "C" void lcd_write(unsigned char dc, unsigned char data) {
  Serial.print("L ");
  Serial.print(dc);
  Serial.print(" ");
  Serial.print(data);
  Serial.print("\n\r");
}

#elif defined(LCD_REAL)

#define PIN_RESET 2
#define PIN_SCE   3
#define PIN_DC    4
#define PIN_SDIN  5
#define PIN_SCLK  6

#define LCD_C     LOW
#define LCD_D     HIGH

/* These parameters might need to be tuned (see datasheet) */
#define LCD_PRMS_VOP  0xB1
#define LCD_PRMS_TEMP 0x04
#define LCD_PRMS_BIAS 0x11

extern "C" void lcd_init() {
  pinMode(PIN_SCE, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_SDIN, OUTPUT);
  pinMode(PIN_SCLK, OUTPUT);
  digitalWrite(PIN_RESET, LOW);
  delay(1);
  digitalWrite(PIN_RESET, HIGH);
  lcd_write(LCD_C, 0x21); /* LCD Extended Commands. */
  lcd_write(LCD_C, LCD_PRMS_VOP);
  lcd_write(LCD_C, LCD_PRMS_TEMP);
  lcd_write(LCD_C, LCD_PRMS_BIAS);
  lcd_write(LCD_C, 0x0C); /* LCD in normal mode. */
  lcd_write(LCD_C, 0x20);
  lcd_write(LCD_C, 0x0C);
}

extern "C" void lcd_write(unsigned char dc, unsigned char data) {
  digitalWrite(PIN_DC, dc);
  digitalWrite(PIN_SCE, LOW);
  shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);
  digitalWrite(PIN_SCE, HIGH);
}

#else
#error "no lcd configured"
#endif


#if defined(KEYPAD_NULL)

#elif defined(KEYPAD_HWSIM)

uint16_t keypad_state;

static void keypad_update() {
  bool ready = false, done = false;
  int n = 0;
  Serial.print("K\n\r");
  keypad_state = 0;
  for (int spin = 0; spin < 100 && !done; spin++) {
    delayMicroseconds(50);
    while (Serial.available() > 0) {
      int value = Serial.read();
      switch (value) {
      case 'K':
        ready = true;
        break;
      case '0':
      case '1':
        if (ready) {
          keypad_state <<= 1;
          keypad_state |= value - '0';
          n++;
        }
        break;
      case '\r':
        done = true;
        break;
      }
    }
  }
  if (n != 12)
    keypad_state = 0;
}

extern "C" void keypad_init() {
  connect_to_hwsim();
  keypad_state = 0;
}

extern "C" void keypad_scan() {
  keypad_update();
}

#elif defined(KEYPAD_REAL)

/*

 COL0  COL1  COL2 |
 -----------------+--------
   1     2     3  |  ROW0
   4     5     6  |  ROW1
   7     8     9  |  ROW2
   *     0     #  |  ROW3

 */

#define PIN_ROW0_IN 7
#define PIN_ROW1_IN 8
#define PIN_ROW2_IN 9
#define PIN_ROW3_IN 10

#define PIN_COL0_OUT 17 /* A3 */
#define PIN_COL1_OUT 16 /* A2 */
#define PIN_COL2_OUT 15 /* A1 */

#define N_OF_COLS 3
#define N_OF_ROWS 4

uint8_t rows[] = {
  PIN_ROW0_IN,
  PIN_ROW1_IN,
  PIN_ROW2_IN,
  PIN_ROW3_IN,
};

uint8_t cols[] = {
  PIN_COL0_OUT,
  PIN_COL1_OUT,
  PIN_COL2_OUT,
};

uint16_t keypad_state;

extern "C" void keypad_init() {
  pinMode(PIN_ROW0_IN, INPUT_PULLUP);
  pinMode(PIN_ROW1_IN, INPUT_PULLUP);
  pinMode(PIN_ROW2_IN, INPUT_PULLUP);
  pinMode(PIN_ROW3_IN, INPUT_PULLUP);
  keypad_state = 0;
}

extern "C" void keypad_scan() {
  uint8_t c, r, n;

  keypad_state = 0;
  for (c = 0; c < N_OF_COLS; c++) {
    pinMode(cols[c], OUTPUT);
    digitalWrite(cols[c], LOW);
    for (r = 0, n = 0; r < N_OF_ROWS; r++, n += 3) {
      if (digitalRead(rows[r]) == LOW)
        keypad_state |= (1 << (c + n));
    }
    digitalWrite(cols[c], HIGH);
    pinMode(cols[c], INPUT);
  }
}

#else
#error "no keypad configured"
#endif

extern "C" uint32_t sys_get_millis() {
  return millis();
}

extern "C" int sys_get_random_seed() {
  return analogRead(0);
}

void setup() {
  jbit_init();
}

void loop() {
  uint16_t w = jbit_step();
  if (w)
    delay(w);
}
