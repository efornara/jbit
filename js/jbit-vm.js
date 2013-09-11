/*
 * Copyright (C) 2012-2013  Emanuele Fornara
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
 VM
 ****************************************************************************
 ****************************************************************************/

JBIT = {};

vm_reset = Module.cwrap('vm_reset', undefined, []);
vm_step = Module.cwrap('vm_step', 'number', []);
vm_load_begin = Module.cwrap('vm_load_begin', undefined, []);
vm_load_line = Module.cwrap('vm_load_line', undefined, ['string']);
vm_load_end = Module.cwrap('vm_load_end', undefined, []);

function VM(io_, skin_) {

	/************************************************************************
	   PRIVATE
	 ************************************************************************/

	var running;
	var io = io_;
	var skin = skin_;
	
	function loadProgramFromText(text) {
		var lines = text.split("\n");
		vm_load_begin();
		for (var i in lines)
			vm_load_line(lines[i]);
		vm_load_end();
	}

	function advanceImpl() {
		var vm_status;
		if (!running) {
			skin.setVMStatus("HALTED");
			return;
		}
		io.wakeup();
		for (i = 0; i < 2000; i++) {
			vm_status = vm_step();
			if (io.getWait() != 0)
				break;
		}
		skin.update();
		switch (vm_status) {
		case 0: // OK
			break;
		case 1: // HALT
			skin.setVMStatus("HALTED");
			running = false;
			break;
		default:
			skin.setVMStatus("ERROR");
			running = false;
			break;
		}
	}

	/************************************************************************
	   PUBLIC
	 ************************************************************************/

	// ENVIRONMENT INTERFACE
	
	this.init = function() {
		this.reset();
	};
	
	this.reset = function() {
		vm_reset();
	};
	
	this.load = function(text) {
		loadProgramFromText(text);
	};

	this.start = function() {
		skin.setVMStatus("OK");
		running = true;
	};
	
	this.halt = function() {
		running = false;
	};
	
	this.isRunning = function() {
		return running;
	}

	this.advance = function() {
		advanceImpl();
		return io.getWait();
	};

	// VM INTERFACE

	JBIT.io_reset = function() {
		return io.reset();
	}

	JBIT.io_get = function(address, value) {
		return io.get(address);
	}

	JBIT.io_put = function(address, value) {
		io.put(address, value);
	}
}
