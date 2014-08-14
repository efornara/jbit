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

#include <stdio.h>
#include <windows.h>

#include "../embd/embd.h"
#include "hwsim.h"

namespace {

hwsim_t hw;

const char *szClassName = "jbit";
LPBITMAPINFO pDIB;
HBRUSH body_brush;
HBRUSH key_brush;

void create() {
}

int key(int key_down, int code) {
	// "magic" mapping (same as javascript! should it be merged?)
	switch (code) {
	case 13: // RETURN
		code = 48 + 5;
		break;
	case 37: // LEFT
		code = 48 + 4;
		break;
	case 38: // UP
		code = 48 + 2;
		break;
	case 39: // RIGHT
		code = 48 + 6;
		break;
	case 40: // DOWN
		code = 48 + 8;
		break;
	case 170: // STAR
	case 188: // COMMA
		code = 42;
		break;
	case 163: // HASH
	case 190: // PERIOD
		code = 35;
		break;
	}
	if (code < ' ' || code > '~')
		return 0;
	return hwsim_key_update(&hw, key_down, code);
}

int mouse(int mouse_down, int x, int y) {
	return hwsim_mouse_update(&hw, mouse_down, x, y);
}

void paint(HDC dc) {
	hwsim_rect_t m;
	hwsim_get_metrics(&hw, HWSIM_M_DISPLAY, &m);
	StretchDIBits(dc,
	  m.x, m.y, m.w, m.h,
	  0, 0, LCD_WIDTH, LCD_HEIGHT,
	  hw.video, pDIB,
	  DIB_RGB_COLORS, SRCCOPY);
	hwsim_color_t c;
	hwsim_get_color(&hw, HWSIM_C_KEY_BG, &c);
	SetBkColor(dc, RGB(c.r, c.g, c.b));
	for (int i = 0; hwsim_keypad_labels[i]; i++) {
		char c = hwsim_keypad_labels[i];
		hwsim_get_metrics(&hw, c, &m);
		RECT rc;
		rc.left = m.x;
		rc.top = m.y;
		rc.right = m.x + m.w;
		rc.bottom = m.y + m.h;
		FillRect(dc, &rc, key_brush);
		SetTextAlign(dc, TA_LEFT | TA_TOP);
		TextOut(dc, rc.left + 2, rc.top + 2, &c, 1);
		SetTextAlign(dc, TA_RIGHT | TA_BOTTOM);
		const char *sub = hwsim_keypad_subs[i];
		TextOut(dc, rc.right - 2, rc.bottom - 2, sub, strlen(sub));
	}
}

void dib_create() {
	DWORD size = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2;
	pDIB = (LPBITMAPINFO)new BYTE[size];
	pDIB->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB->bmiHeader.biWidth = LCD_WIDTH;
	pDIB->bmiHeader.biHeight = -LCD_HEIGHT;
	pDIB->bmiHeader.biBitCount = 1;
	pDIB->bmiHeader.biPlanes = 1;
	pDIB->bmiHeader.biCompression = BI_RGB;
	pDIB->bmiHeader.biXPelsPerMeter = 1000;
	pDIB->bmiHeader.biYPelsPerMeter = 1000;
	pDIB->bmiHeader.biClrUsed = 0;
	pDIB->bmiHeader.biClrImportant = 0;
	RGBQUAD *pColors = (RGBQUAD *)&((BYTE *)pDIB)[sizeof(BITMAPINFOHEADER)];
	hwsim_color_t c;
	hwsim_get_color(&hw, HWSIM_C_DISPLAY_BG, &c);
	pColors[0].rgbRed = c.r;
	pColors[0].rgbGreen = c.g;
	pColors[0].rgbBlue = c.b;
	pColors[0].rgbReserved = 0;
	hwsim_get_color(&hw, HWSIM_C_DISPLAY_FG, &c);
	pColors[1].rgbRed = c.r;
	pColors[1].rgbGreen = c.g;
	pColors[1].rgbBlue = c.b;
	pColors[1].rgbReserved = 0;
}

void dib_destroy() {
	delete[] (BYTE *)pDIB;
}

} // namespace;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		dib_create();
		create();
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		paint(hdc);
		EndPaint(hWnd, &ps);
		} break;
	case WM_KEYDOWN:
		if (key(1, wParam))
			InvalidateRect(hWnd, NULL, TRUE);
		if (wParam == VK_ESCAPE) // TODO: remove (quick shortcut for testing)
			PostQuitMessage(0);
		break;
	case WM_KEYUP:
		if (key(0, wParam))
			InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_LBUTTONDOWN:
		if (mouse(1, LOWORD(lParam), HIWORD(lParam)))
			InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_LBUTTONUP:
	case WM_MOUSELEAVE:
		// non-standard handling, but good enough for me
		if (mouse(0, -1, -1))
			InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_DESTROY:
		dib_destroy();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow) {

	hwsim_init(&hw);

	hwsim_color_t c;
	hwsim_get_color(&hw, HWSIM_C_BODY, &c);
	body_brush = CreateSolidBrush(RGB(c.r, c.g, c.b));
	hwsim_get_color(&hw, HWSIM_C_KEY_BG, &c);
	key_brush = CreateSolidBrush(RGB(c.r, c.g, c.b));

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, (char *)IDI_APPLICATION);
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = body_brush;
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = szClassName;
	wcex.hIconSm        = LoadIcon(wcex.hInstance, (char *)IDI_APPLICATION);
	RegisterClassEx(&wcex);

	hwsim_rect_t m;
	hwsim_get_metrics(&hw, HWSIM_M_WINDOW, &m);
	HWND hWnd = CreateWindow(
		szClassName,
		"JBit " JBIT_VERSION,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		m.w, m.h,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteObject(key_brush);
	DeleteObject(body_brush);

	hwsim_cleanup(&hw);

	return msg.wParam;
}
