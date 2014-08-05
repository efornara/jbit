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

JBEMBD = {};

(function() {

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

	function createDisplay(parent) {
		var e;

		e = document.createElement("canvas");
		e.id = "jb_display";
		e.className = "jbit_embd jbit_embd_display";
		parent.appendChild(e);
		return e;
	}

	function createKeys(parent) {
		var i, b, id;

		keys = [];
		for (i = 0; i < buttons.length; i++) {
			b = buttons[i];
			id = b.hasOwnProperty('id') ? b.id : b.label;
			e = document.createElement("div");
			e.id = "jb_key_" + id;
			e.className = "jbit_embd jbit_embd_key";
			e.innerHTML = b.label;
			parent.appendChild(e);
			keys[i] = e;
		}
	}

	function createKeypad(parent) {
		var e;

		e = document.createElement("div");
		e.id = "jb_keypad";
		e.className = "jbit_embd jbit_embd_keypad";
		e.style.position = "relative";
		createKeys(e);
		parent.appendChild(e);
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
			code = 48 + 8;
			break;
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
		if (id == "jb_key_STAR")
			key = "*";
		else if (id == "jb_key_SHARP")
			key = "#";
		else
			key = id.substring(7);
		keypad_update(is_down, key.charCodeAt(0));
		return false;
	}

	function addMouseEvents() {
		var i,
			b,
			id,
			e;

		for (i = 0; i < buttons.length; i++) {
			b = buttons[i];
			id = b.hasOwnProperty('id') ? b.id : b.label;
			e = document.getElementById("jb_key_" + id);
			e.addEventListener('mouseup', function(e) { onMouse(false, e); }, false);
			e.addEventListener('mousedown', function(e) { onMouse(true, e); }, false);
			e.addEventListener('touchend', function(e) { onMouse(false, e); }, false);
			e.addEventListener('touchstart', function(e) { onMouse(true, e); }, false);
		}
	}

	function reflowDisplay(s) {
		var w, h;

		w = LCD_WIDTH * s;
		h = LCD_HEIGHT * s;
		display.width = w + "";
		display.height = h + "";
		display.style.width = w + "px";
		display.style.height = h + "px";
	}

	function reflowKeys(w, h, mx, my) {
		var column,
			x, y, i, e;

		column = 0;
		x = 0;
		y = 0;
		w = Math.floor(w);
		h = Math.floor(h);
		for (i = 0; i < keys.length; i++) {
			e = keys[i];
			e.style.left = x + "px";
			e.style.top = y + "px";
			e.style.width = w + "px";
			e.style.height = h + "px";
			x += w + mx;
			column++;
			if (column == 3) {
				column = 0;
				x = 0
				y += h + my;
			}
		}
	}

	function reflow() {
		var MARGIN_X = 10,
			MARGIN_Y = 5,
			ctx,
			display_max_width,
			keypad_max_width,
			keypad_max_height,
			sc, i, w, h;

		if (width < 100 || height < 200) {
			width = 100;
			height = 200;
		}

		if (height > width) {
			// portrait
			display_max_width = width;
			keypad_max_width = width;
			keypad_max_height = width; // TODO
		} else {
			// landscape (TODO)
			display_max_width = width / 2;
			keypad_max_width = width / 2;
			keypad_max_height = width / 3;
		}

		for (sc = 1; sc < 8; sc++) {
			if ((LCD_WIDTH + 3) * (sc + 1) > display_max_width)
				break;
		}
		reflowDisplay(sc);
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

		w = (keypad_max_width - MARGIN_X * 2) / 3;
		h = (keypad_max_height - MARGIN_Y * 3) / 4;
		reflowKeys(w, h, MARGIN_X, MARGIN_Y);
	}

	function resize() {
		var w,
			h;

		if (typeof(window.innerWidth) == 'number') {
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

	JBEMBD.init = function() {
		var e;

		e = document.getElementById('sim');
		display = createDisplay(e);
		createKeypad(e);
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
