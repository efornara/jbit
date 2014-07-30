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

JBIT = {};

(function() {

	var SCALE = 5,
		LCD_WIDTH = 84,
		LCD_HEIGHT = 48,
		LCD_ROWS = 5,
		sim_step,
		lcd_bitmap,
		pixel;

	JBIT.init = function() {
		var ctx,
			i;

		ctx = document.getElementById('display').getContext('2d');
		pixel = ctx.createImageData(SCALE, SCALE);
		for (i = 0; i < SCALE * SCALE; i++) {
			pixel.data[i*4 + 0] = 0;
			pixel.data[i*4 + 1] = 0;
			pixel.data[i*4 + 2] = 0;
			pixel.data[i*4 + 3] = 255;
		}
		Module.ccall('sim_init', 'number', [], []);
		lcd_bitmap = Module.ccall('lcd_get_bitmap', 'number', [], []);
		window.setInterval(JBIT.update, 100);
	};

	JBIT.update = function() {
		var ctx,
			rect_x, rect_y,
			x, y, r, i, j;

		Module.ccall('sim_step', 'number', [], []);
		ctx = document.getElementById('display').getContext('2d');
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

window.onload = function() {
	JBIT.init();
};
