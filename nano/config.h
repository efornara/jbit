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

// Facilities
#define ENABLE_UI

// Modules
#define ENABLE_VM
#define ENABLE_DEMOS

// Devices
#define ENABLE_MICROIO
#define ENABLE_MICROIO_RANDOM

// Loaders
//#define ENABLE_AUTORUN

// LCD (one of)
//#define LCD_NULL
#define LCD_HWSIM
//#define LCD_REAL

// KEYPAD (one of)
//#define KEYPAD_NULL
#define KEYPAD_HWSIM
//#define KEYPAD_REAL


// internal configuration checks

#ifndef ENABLE_UI
#if defined(ENABLE_DEMOS)
#error "Interactive modules only available if UI is enabled"
#endif
#endif

#ifndef ENABLE_VM
#if defined(ENABLE_MICROIO)
#error "Devices only available if VM is enabled"
#endif
#endif
