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

extern const uint8_t *jbit_prg_code;
extern uint16_t jbit_prg_size;
extern uint8_t jbit_prg_pgm;

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
#if defined(ENABLE_TRACE)

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
    if ((nv = sscanf(line, "P %d%c", &jbit_prg_size, &dirty)) != 1)
      return serial_loader_error("header");
    if (code)
      free(code);
    if ((code = (uint8_t *)malloc(jbit_prg_size)) == NULL)
      return serial_loader_error("malloc");
    state = 0;
  } else if (state < jbit_prg_size) {
    int pos, data;
    if ((nv = sscanf(line, "B %d %d%c", &pos, &data, &dirty)) != 2)
      return serial_loader_error("byte");
    if (pos != state)
      return serial_loader_error("pos");
    if (data < 0 || data > 255)
      return serial_loader_error("data");
    code[pos] = data;
    state++;
    if (state == jbit_prg_size) {
      Serial.print("# finished\n\r");
      ret = 0;
    }
  }
  Serial.print(line);
  Serial.print("\n\r");
  return ret;
}

extern "C" void serial_loader() {
  serial_loader_init();
  while (serial_loader_step())
    ;
  jbit_prg_code = code;
}

#endif


#if defined(LCD_NULL)

extern "C" void lcd_init() {
}

extern "C" void lcd_write(unsigned char dc, unsigned char data) {
}

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

#error "not implemented"

#else
#error "no lcd configured"
#endif


#if defined(KEYPAD_NULL)

uint16_t keypad_state;

extern "C" void keypad_init() {
  keypad_state = 0;
}

extern "C" void keypad_scan() {
}

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

#error "not implemented"

#else
#error "no keypad configured"
#endif

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
