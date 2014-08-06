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

/* jshint eqeqeq: true, forin: true, immed: true, latedef: true, newcap: true,
          undef: true, unused: true, strict: true */
/* global document, window, Module, getValue */

var JBEMBD = {};

(function() {

	"use strict";

	var scale = 1,
		LCD_WIDTH = 84,
		LCD_HEIGHT = 48,
		LCD_ROWS = 6,
		width = 0,
		height = 0,
		jbit_step,
		keypad_update,
		lcd_bitmap,
		buttons,
		body,
		display,
		keys,
		pixel;

	jbit_step = Module.cwrap('jbit_step', 'number', []);
	keypad_update = Module.cwrap('keypad_update', 'number', ['number', 'number']);

	buttons = [
		{ keys:'1', label:'1' },
		{ keys:'2abc', label:'2', sub:'abc' },
		{ keys:'3def', label:'3', sub:'def' },
		{ keys:'4ghi', label:'4', sub:'ghi' },
		{ keys:'5jkl', label:'5', sub:'jkl' },
		{ keys:'6mno', label:'6', sub:'mno' },
		{ keys:'7pqrs', label:'7', sub:'pqrs' },
		{ keys:'8tuv', label:'8', sub:'tuv' },
		{ keys:'9wxyz', label:'9', sub:'wxyz' },
		{ keys:'*', id:'STAR', label:'*' },
		{ keys:'1', label:'0' },
		{ keys:'#', id:'SHARP', label:'#' }
	];

	function createDisplay(parent_) {
		var e;

		e = document.createElement("canvas");
		e.id = "jb_display";
		e.className = "jbit_embd_display";
		parent_.appendChild(e);
		return e;
	}

	function createKeys(parent_) {
		var i, b, id, e;

		keys = [];
		for (i = 0; i < buttons.length; i++) {
			b = buttons[i];
			id = b.hasOwnProperty('id') ? b.id : b.label;
			e = document.createElement("div");
			e.id = "jb_key_" + id;
			e.className = "jbit_embd_key";
			e.innerHTML = b.label;
			parent_.appendChild(e);
			keys[i] = e;
		}
	}

	function createSim(parent_) {
		var e;

		e = document.createElement("div");
		e.id = "jb_sim";
		e.className = "jbit_embd_sim";
		display = createDisplay(e);
		createKeys(e);
		parent_.appendChild(e);
		return e;
	}

	// TODO: find out best practices; this works on iceweasel (deb. wheezy)
	function onKey(is_down, e) {
		var code = e.keyCode;

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
			return;
		keypad_update(is_down, code);
		return false;
	}

	function onMouse(is_down, e) {
		var id,
			key;

		id = e.target.id;
		if (id === "jb_key_STAR")
			key = "*";
		else if (id === "jb_key_SHARP")
			key = "#";
		else
			key = id.substring(7);
		keypad_update(is_down, key.charCodeAt(0));
		return false;
	}

	function addMouseEvents() {
		var i, b, id, e;

		function mouseup(ev) { onMouse(false, ev); }
		function mousedown(ev) { onMouse(true, ev); }

		for (i = 0; i < buttons.length; i++) {
			b = buttons[i];
			id = b.hasOwnProperty('id') ? b.id : b.label;
			e = document.getElementById("jb_key_" + id);
			e.addEventListener('mouseup', mouseup, false);
			e.addEventListener('mousedown', mousedown, false);
			e.addEventListener('touchend', mouseup, false);
			e.addEventListener('touchstart', mousedown, false);
		}
	}

	function reflowDisplay(x0, y0, s) {
		var w, h;

		w = LCD_WIDTH * s;
		h = LCD_HEIGHT * s;
		display.width = w + "";
		display.height = h + "";
		display.style.left = x0 + "px";
		display.style.top = y0 + "px";
		display.style.width = w + "px";
		display.style.height = h + "px";
	}

	function reflowKeys(x0, y0, w, h, mx, my) {
		var column,
			x, y, i, e;

		column = 0;
		x = x0;
		y = y0;
		for (i = 0; i < keys.length; i++) {
			e = keys[i];
			e.style.left = x + "px";
			e.style.top = y + "px";
			e.style.width = w + "px";
			e.style.height = h + "px";
			x += w + mx;
			column++;
			if (column === 3) {
				column = 0;
				x = x0;
				y += h + my;
			}
		}
	}

	function reflow() {
		var MIN_WIDTH = 120,
			MIN_HEIGHT = 160,
			SCALE_MARGIN_X = (1 / 30),
			SCALE_MARGIN_Y = (1 / 30),
			margin_x,
			margin_y,
			portrait,
			ctx,
			column_width,
			keypad_width,
			keypad_height,
			sc, i, w, h, x0, y0;

		if (width < MIN_WIDTH || height < MIN_HEIGHT) {
			width = MIN_WIDTH;
			height = MIN_HEIGHT;
		}
		body.style.width = width + "px";
		body.style.height = height + "px";
		margin_x = Math.floor(width * SCALE_MARGIN_X);
		margin_y = Math.floor(width * SCALE_MARGIN_Y);

		if (height > width) {
			portrait = true;
			column_width = width;
		} else {
			portrait = false;
			column_width = width / 2;
		}

		for (sc = 1; sc < 8; sc++) {
			if (LCD_WIDTH * (sc + 1) > column_width - 2 * margin_x)
				break;
		}
		x0 = (column_width - LCD_WIDTH * sc) >> 1;
		if (portrait)
			y0 = margin_y;
		else
			y0 = (height - LCD_HEIGHT * sc) >> 1;
		reflowDisplay(x0, y0, sc);

		if (pixel === undefined || scale !== sc) {
			ctx = display.getContext('2d');
			pixel = ctx.createImageData(sc, sc);
			for (i = 0; i < sc * sc; i++) {
				pixel.data[i*4 + 0] = 0;
				pixel.data[i*4 + 1] = 0;
				pixel.data[i*4 + 2] = 0;
				pixel.data[i*4 + 3] = 255;
			}
			scale = sc;
		}

		if (portrait) {
			keypad_width = width;
			keypad_height = height - (LCD_HEIGHT * scale + margin_y * 2);
			x0 = 0;
			y0 = LCD_HEIGHT * scale + margin_y;
		} else {
			keypad_width = width / 2;
			keypad_height = height;
			x0 = width >> 1;
			y0 = 0;
		}
		x0 += margin_x;
		y0 += margin_y;
		w = Math.floor((keypad_width - margin_x * 4) / 3);
		h = Math.floor((keypad_height - margin_y * 5) / 4);
		reflowKeys(x0, y0, w, h, margin_x, margin_y);
	}

	function resize() {
		var w,
			h;

		if (typeof(window.innerWidth) === 'number') {
			w = window.innerWidth;
			h = window.innerHeight;
		} else {
			w = document.documentElement.clientWidth;
			h = document.documentElement.clientHeight;
		}
		if (width !== w || height !== h) {
			width = w;
			height = h;
			reflow();
		}
	}

	JBEMBD.init = function(parent_) {
		var e = parent_;

		body = createSim(e);
		resize();
		Module.ccall('jbit_init', 'number', [], []);
		lcd_bitmap = Module.ccall('lcd_get_bitmap', 'number', [], []);
		window.addEventListener('keyup', function(e) { onKey(false, e); }, false);
		window.addEventListener('keydown', function(e) { onKey(true, e); }, false);
		addMouseEvents();
		window.setInterval(JBEMBD.update, 100);
		window.onresize = resize;
	};

	JBEMBD.update = function() {
		var ctx,
			rect_x, rect_y,
			x, y, r, i, j;

		jbit_step();
		if (pixel === undefined)
			return;
		ctx = display.getContext('2d');
		ctx.clearRect(0, 0, LCD_WIDTH * scale, LCD_HEIGHT * scale);
		for (i = 0, r = 0; r < LCD_ROWS; r++) {
			y = r * 8 * scale;
			for (x = 0; x < LCD_WIDTH; x++) {
				rect_x = x * scale;
				for (j = 0; j < 8; j++, i++) {
					rect_y = y + j * scale;
					if (getValue(lcd_bitmap + i, 'i8'))
						ctx.putImageData(pixel, rect_x, rect_y);
				}
			}
		}
	};

})();
