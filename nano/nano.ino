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

#include "nano.h"


#if defined(LCD_HWSIM) || defined(KEYPAD_HWSIM)

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
  jbit_step();
  delay(100);
}
