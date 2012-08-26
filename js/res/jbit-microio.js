/*
 * Copyright (C) 2012  Emanuele Fornara
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


/****************************************************************************
 ****************************************************************************
 MicroIO
 ****************************************************************************
 ****************************************************************************/

function MicroIO() {

	/************************************************************************
	   PRIVATE
	 ************************************************************************/

	// Random (no 64-bit integers available, so different values are used)
	
	var MODULE = 0x100000000;
	
	var rndSeed0;
	var rndSeed1;
	var rndN;
	
	function randomReset() {	
		rndSeed0 = Math.floor(Math.random() * (MODULE - 1));
		rndSeed1 = 0;
		doRandomPut(255);
	}

	function rndNext() {
		rndSeed0 = (rndSeed0 * 1103515245 + 12345) % MODULE;
		return rndSeed0;
	}
	
	function doRandomGet() {
		return Math.floor(rndN * (rndNext() / MODULE));
	}
	
	function doRandomPut(max) {
		var t;
		if (max == 0) {
			t = rndSeed0;
			rndSeed0 = rndSeed1;
			rndSeed1 = t;
		} else {
			rndN = max + 1;
		}
	}

	// Key Buffer

	var keyBuf = new Array();

	function keyBufReset() {
		var i;
		
		for (i = 0; i < 8; i++)
			keyBuf[i] = 0;
	}

	function doKeyBufPut() {
		var i;
		
		for (i = 0; i < 7; i++)
			keyBuf[i] = keyBuf[i + 1];
		keyBuf[7] = 0;
	}

	function doKeyBufGet(address) {
		return keyBuf[address - 24];
	}

	function enqueKeyCode(keyCode) {
		var i;
		
		for (i = 0; i < 8; i++)
			if (keyBuf[i] == 0) {
				keyBuf[i] = keyCode;
				return;
			}
	}

	// Console
	
	var video = new Array();

	function videoReset() {
		var i;
		
		for (i = 0; i < 40; i++)
			video[i] = 32;
	}
	
	function doVideoPut(address, value) {
		video[address - 40] =  value;
	}
	
	function doVideoGet(address) {
		return video[address - 40];
	}

	// FPS
	
	var fps;	
	var frameInterval;
	var wait;
	
	function frameReset() {
		doFrmFpsPut(40);
		wait = 0;
	}

	function doFrmFpsGet() {
		return fps;
	}

	function doFrmFpsPut(value) {
		fps = value;
		if (value == 0) {
			frameInterval = 0;
			wait = 0;
		} else {
			frameInterval = 4000 / value;
			wait = frameInterval;
		}
	}

	function doFrmDrawPut() {
		if (frameInterval == 0) {
			flush();
		} else {
			wait = frameInterval;
		}
	}


	/************************************************************************
	   PUBLIC
	 ************************************************************************/
	 
	// VM INTERFACE

	this.init = function() {
		this.reset();
	};

	this.reset = function() {
		frameReset();
		randomReset();
		keyBufReset();
		videoReset();
	};

	this.get = function(address) {
		if (address == 17)
			return doFrmFpsGet();
		else if (address == 23)
			return doRandomGet();
		else if (address >= 24 && address <= 31)
			return doKeyBufGet(address);
		else if (address >= 40 && address <= 79)
			return doVideoGet(address);
	};
	
	this.put = function(address, value) {
		if (address == 17)
			doFrmFpsPut(value);
		else if (address == 18)
			doFrmDrawPut(value);
		else if (address == 23)
			doRandomPut(value);
		else if (address == 24)
			doKeyBufPut();
		else if (address >= 40 && address <= 79)
			doVideoPut(address, value);
	};
	
	this.wakeup = function() {
		wait = 0;
	};
	
	this.getWait = function() {
		return wait;
	};
	
	// SKIN INTERFACE

	this.keyPress = function(keyCode) {
		enqueKeyCode(keyCode);
	};

	// for quick display updates (read-only)
	this.getVideoBuffer = function() {
		return video;
	};
}
