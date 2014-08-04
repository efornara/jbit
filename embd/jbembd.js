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

	var SCALE = 1,
		LCD_WIDTH = 84,
		LCD_HEIGHT = 48,
		LCD_ROWS = 6,
		jbit_step,
		keypad_update,
		lcd_bitmap,
		buttons,
		display,
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
		e.className = "jbit_embd_display";
		e.width = "84";
		e.height = "48";
		e.style.position = "relative";
		e.style.width = "84px";
		e.style.height = "48px";
		parent.appendChild(e);
		return e;
	}

	function createKeys(parent) {
		var width = 20, height = 20, column, x, y, i, b, id, e;

		column = 0;
		x = 0;
		y = 0;
		for (i = 0; i < buttons.length; i++) {
			b = buttons[i];
			id = b.hasOwnProperty('id') ? b.id : b.label;
			e = document.createElement("div");
			e.id = "jb_key_" + id;
			e.className = "jbit_embd_key";
			e.innerHTML = b.label;
			e.style.position = "absolute";
			e.style.left = x + "px";
			e.style.top = y + "px";
			e.style.width = width + "px";
			e.style.height = height + "px";
			parent.appendChild(e);
			x += width + 2;
			column++;
			if (column == 3) {
				column = 0;
				x = 0
				y += height + 2;
			}
		}
	}

	function createKeypad(parent) {
		var e;

		e = document.createElement("div");
		e.id = "jb_keypad";
		e.className = "jbit_embd_key";
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
		}
	}

	JBEMBD.init = function() {
		var e,
			ctx,
			i;

		e = document.getElementById('sim');
		display = createDisplay(e);
		createKeypad(e);
		ctx = display.getContext('2d');
		pixel = ctx.createImageData(SCALE, SCALE);
		for (i = 0; i < SCALE * SCALE; i++) {
			pixel.data[i*4 + 0] = 0;
			pixel.data[i*4 + 1] = 0;
			pixel.data[i*4 + 2] = 0;
			pixel.data[i*4 + 3] = 255;
		}
		Module.ccall('jbit_init', 'number', [], []);
		lcd_bitmap = Module.ccall('lcd_get_bitmap', 'number', [], []);
		window.addEventListener('keyup', function(e) { onKey(false, e); }, false);
		window.addEventListener('keydown', function(e) { onKey(true, e); }, false);
		addMouseEvents();
		window.setInterval(JBEMBD.update, 100);
	};

	JBEMBD.update = function() {
		var ctx,
			rect_x, rect_y,
			x, y, r, i, j;

		jbit_step();
		ctx = display.getContext('2d');
		ctx.clearRect(0, 0, LCD_WIDTH * SCALE, LCD_HEIGHT * SCALE);
		for (i = 0, r = 0; r < LCD_ROWS; r++) {
			y = r * 8 * SCALE;
			for (x = 0; x < LCD_WIDTH; x++) {
				rect_x = x * SCALE;
				for (j = 0; j < 8; j++, i++) {
					rect_y = y + j * SCALE;
					if (getValue(lcd_bitmap + i, 'i8'))
						ctx.putImageData(pixel, rect_x, rect_y);
				}
			}
		}
	};

})();
