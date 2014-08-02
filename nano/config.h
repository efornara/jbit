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


#include "local.h"


#ifndef LOCAL_CONFIG

#ifdef __AVR

#define ENABLE_VM
#define ENABLE_PRIMO
#define ENABLE_SERIAL
#define ENABLE_SERIAL_TRACE
#define LCD_NULL
#define KEYPAD_NULL

#else

#define ENABLE_UI
#define ENABLE_VM
#define ENABLE_DEMOS
#define ENABLE_MICROIO
#define ENABLE_MICROIO_RANDOM
#define LCD_NULL
#define KEYPAD_NULL

#endif

#endif


// derived values

#if !defined(LCD_HWSIM) && !defined(LCD_REAL)
#define LCD_NULL
#endif

#if !defined(KEYPAD_HWSIM) && !defined(KEYPAD_REAL)
#define KEYPAD_NULL
#endif

// internal configuration checks

#ifndef ENABLE_UI
#if defined(ENABLE_DEMOS)
#error "Interactive modules only available if UI is enabled"
#endif
#endif

#ifndef ENABLE_VM
#if defined(ENABLE_MICROIO) || defined(ENABLE_PRIMO)
#error "Devices only available if VM is enabled"
#endif
#endif

#ifndef __AVR
#if defined(ENABLE_SERIAL)
#error "Serial loader not available for this target"
#endif
#endif
