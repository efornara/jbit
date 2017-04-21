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

// io2win32.cc

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#define WINVER 0x500
#define _WIN32_IE 0x0500
#include <windows.h>
#include <commctrl.h>

// TODO: better
#ifndef VK_OEM_COMMA
#define VK_OEM_COMMA 0xBC
#endif
#ifndef VK_OEM_PERIOD
#define VK_OEM_PERIOD 0xBE
#endif

#include "core.h"
#include "resource.h"

#include "libretro.h"

extern bool io2_opengl;

static const int width = 128;
static const int height = 128;

static const char *szClassName = "jbit";

#include "io2gl12.h"
typedef GL12Renderer GLRenderer;
static GLRenderer gl;

static void show_error(const char *fmt, va_list arg) {
	char s[1024];
	vsnprintf(s, sizeof(s), fmt, arg);
	s[sizeof(s) - 1] = '\0';
	MessageBox(0, s, "Error", MB_OK | MB_ICONERROR);
}

static void error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	show_error(fmt, ap);
	va_end(ap);
}

static void log(enum retro_log_level level, const char *fmt, ...) {
	va_list ap;
	if (level < RETRO_LOG_ERROR)
		return;
	va_start(ap, fmt);
	show_error(fmt, ap);
	va_end(ap);
}

static Buffer file_name;
static Buffer jb;
static bool valid_program = false;
static bool running = false;

static void load_program() {
	FILE *f = 0;

	if (valid_program) {
		retro_unload_game();
		valid_program = false;
		running = false;
	}
	if (file_name.get_length() == 0) {
		valid_program = retro_load_game(0);
		running = valid_program;
		return;
	}
	const char *s = file_name.get_data();
	if (!(f = fopen(s, "rb"))) {
		error("Failed opening file '%s'.", s);
		return;
	}
	jb.reset();
	fseek(f, 0, SEEK_END);
	int n = ftell(f);
	if (n > 1024 * 1024) {
		error("Invalid file '%s' (size >1M).", s);
		fclose(f);
		return;
	}
	rewind(f);
	char *buf = jb.append_raw(n);
	int i = 0, j;
	while (i < n) {
		if ((j = fread(&buf[i], 1, n - 1, f)) == 0)
			break;
		i += j;
	}
	fclose(f);
	if (i != n) {
		error("Size check failed for '%s'.", s);
		return;
	}
	retro_game_info info;
	info.path = 0;
	info.data = jb.get_data();
	info.size = jb.get_length();
	info.meta = "";
	valid_program = retro_load_game(&info);
	running = valid_program;
}

static void parse_arg(const char *arg) {
	bool inside = false;
	bool quoted = false;
	for (int i = 0; arg[i]; i++) {
		char c = arg[i];
		if (inside) {
			if (quoted && c == '"')
				break;
			if (!quoted && isspace(c))
				break;
			file_name.append_char(c);
		} else {
			if (c == '"') {
				inside = true;
				quoted = true;
			} else if (!isspace(c)) {
				file_name.append_char(c);
				inside = true;
			}
		}
	}
	if (inside)
		file_name.append_char(0);
}

#define JOYPAD_NULL 1000

static struct {
	int code;
	unsigned keyboard_id;
	unsigned joypad_id;
	bool pressed;
} key_state[] = {
	{ '0', '0', JOYPAD_NULL, false },
	{ '1', '1', JOYPAD_NULL, false },
	{ '2', '2', JOYPAD_NULL, false },
	{ '3', '3', JOYPAD_NULL, false },
	{ '4', '4', JOYPAD_NULL, false },
	{ '5', '5', JOYPAD_NULL, false },
	{ '6', '6', JOYPAD_NULL, false },
	{ '7', '7', JOYPAD_NULL, false },
	{ '8', '8', JOYPAD_NULL, false },
	{ '9', '9', JOYPAD_NULL, false },
	{ VK_OEM_COMMA, ',', JOYPAD_NULL, false },
	{ VK_OEM_PERIOD, '.', JOYPAD_NULL, false },
	{ 'A', 0, RETRO_DEVICE_ID_JOYPAD_Y, false },
	{ VK_UP, 0, RETRO_DEVICE_ID_JOYPAD_UP, false },
	{ 'S', 0, RETRO_DEVICE_ID_JOYPAD_X, false },
	{ VK_LEFT, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, false },
	{ 'X', 0, RETRO_DEVICE_ID_JOYPAD_A, false },
	{ VK_RIGHT, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, false },
	{ 'Q', 0, RETRO_DEVICE_ID_JOYPAD_L, false },
	{ VK_DOWN, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, false },
	{ 'W', 0, RETRO_DEVICE_ID_JOYPAD_R, false },
	{ VK_SHIFT, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, false },
	{ 'Z', 0, RETRO_DEVICE_ID_JOYPAD_B, false },
	{ VK_RETURN, 0, RETRO_DEVICE_ID_JOYPAD_START, false },
	{ 0, false }
};

static void key(int code, bool pressed) {
	for (int i = 0; key_state[i].code; i++)
		if (code == key_state[i].code)
			key_state[i].pressed = pressed;
}

static void video_refresh(const void *data, unsigned width_, unsigned height_,
  size_t pitch) {
	gl.draw((const unsigned char *)data);
}

static bool env(unsigned cmd, void *data) {
	if (cmd == RETRO_ENVIRONMENT_GET_LOG_INTERFACE) {
		retro_log_callback *l = (retro_log_callback *)data;
		l->log = log;
	} else if (cmd == RETRO_ENVIRONMENT_SET_MESSAGE) {
		running = false;
	}
	return true;
}

static void input_poll() {
}

static int16_t input_state(unsigned port, unsigned device, unsigned index,
  unsigned id) {
	for (int i = 0; key_state[i].code; i++) {
		if (device == RETRO_DEVICE_KEYBOARD && id == key_state[i].keyboard_id)
			return key_state[i].pressed;
		if (device == RETRO_DEVICE_JOYPAD && id == key_state[i].joypad_id)
			return key_state[i].pressed;
	}
	return 0;
}

typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
static PFNWGLSWAPINTERVALFARPROC wglSwapInterval;

static HINSTANCE hInst;
static HDC hDC;

static void load_file_dialog(HWND hWnd) {
	OPENFILENAME ofn;
	char szFileName[1024] = "";
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "JBit Files (*.jb)\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "jb";
	if (GetOpenFileName(&ofn)) {
		file_name.reset();
		file_name.append_string(szFileName);
		load_program();
	}
}

static WPARAM main_loop() {
	HACCEL hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(ID_ACCEL));
	MSG msg;
	while (1) {
		while (1) {
			bool busy_loop = valid_program && running;
			if (busy_loop && !PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
				break;
			if (!GetMessage(&msg, 0, 0, 0))
				return msg.wParam;
			if (!TranslateAccelerator(msg.hwnd, hAccel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (valid_program)
			retro_run();
		if (!wglSwapInterval)
			Sleep(10); // TODO: better
		SwapBuffers(hDC);
	}
}

INT_PTR CALLBACK AboutDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hWndDlg, (INT_PTR)LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_INITDIALOG: {
		HWND hWnd;
		hWnd = GetDlgItem(hWndDlg, ID_VERSION);
		SetWindowText(hWnd, get_jbit_version());
		char license[2048], c, *p = license;
		for (int i = 0; (c = jbit_license[i]); i++) {
			if (c == '\n')
				*p++ = '\r';
			*p++ = c;
		}
		*p++ = '\0';
		hWnd = GetDlgItem(hWndDlg, ID_LICENSE);
		SetWindowText(hWnd, license);
		} return (INT_PTR)TRUE;
	}
	return (INT_PTR) FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		DragAcceptFiles(hWnd, TRUE);
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		SwapBuffers(hDC);
		} break;
	case WM_KEYDOWN:
		key(wParam, true);
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);
		break;
	case WM_KEYUP:
		key(wParam, false);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_FILE_LOAD:
			load_file_dialog(hWnd);
			break;
		case ID_FILE_RELOAD:
			load_program();
			break;
		case ID_FILE_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case ID_HELP_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(ID_ABOUTDLG), hWnd, AboutDlgProc);
			break;
		}
		break;
	case WM_DROPFILES: {
		char szFileName[1024];
		HDROP hDrop = (HDROP)wParam;
		DragQueryFile(hDrop, 0, szFileName, sizeof(szFileName));
		DragFinish(hDrop);
		file_name.reset();
		file_name.append_string(szFileName);
		load_program();
		} break;
	case WM_DESTROY:
		DragAcceptFiles(hWnd, FALSE);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow) {
	parse_arg(lpCmdLine);
	hInst = hInstance;
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icc);
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ID_APPICON));
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground = 0;
	wcex.lpszMenuName = MAKEINTRESOURCE(ID_APPMENU);
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(ID_APPICON));
	RegisterClassEx(&wcex);
	const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
	  | WS_MINIMIZEBOX;
	RECT rect = { 0, 0, width * 3, height * 3 };
	AdjustWindowRect(&rect, style, TRUE);
	HWND hWnd = CreateWindow(szClassName, APP_NAME,
	  style, CW_USEDEFAULT, CW_USEDEFAULT,
	  rect.right - rect.left, rect.bottom - rect.top,
	  0, 0, hInstance, 0);
	hDC = GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	int pf = ChoosePixelFormat(hDC, &pfd);
	if (pf == 0)
		return 0;
	if (!SetPixelFormat(hDC, pf, &pfd))
		return 0;
	HGLRC hRC;
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);
	io2_opengl = true;
	retro_set_environment(env);
	retro_init();
	retro_set_video_refresh(video_refresh);
	retro_set_input_poll(input_poll);
	retro_set_input_state(input_state);
	load_program();
	gl.init();
	wglSwapInterval = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress(
	  "wglSwapIntervalEXT");
	if (wglSwapInterval)
		wglSwapInterval(1);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	WPARAM ret = main_loop();
	wglMakeCurrent(0, 0);
	ReleaseDC(hWnd, hDC);
	wglDeleteContext(hRC);
	DestroyWindow(hWnd);
	retro_deinit();
	return ret;
}
