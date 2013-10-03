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

JBIT = {};

function VMPage() {

	var video;
	var keyPress;
	var vm;
	var cells = new Array(); // cache cell elements for speeed

	var buttons = [
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

	function createKey(parent, id, title, callback) {
		var e;
		e = document.createElement("div");
		e.id = "jb_btn_" + id;
		e.className = "key";
		e.style.position = "absolute";
		e.style.width = "10px";
		e.style.height = "10px";
		if (callback)
			e.onclick = function() {
				try {
					callback();
				} catch(e) {
				}
				return false;
			};
		parent.appendChild(e);
		return e;
	}

	function onclickHandler() {
		var id, key;
		id = this.id;
		if (id == "jb_btn_k_STAR")
			key = "*";
		else if (id == "jb_btn_k_SHARP")
			key = "#";
		else
			key = id.substring(9);
		keyPress(key.charCodeAt(0));
		return false;
	}
	
	function updateStatus(status) {
		var e;
		e = document.getElementById("jb_vm_status");
		switch (status) {
		case "HALTED":
			e.style.backgroundColor = "gray";
			break;
		case "ERROR":
			e.style.backgroundColor = "red";
			break;
		case "OK":
			e.style.backgroundColor = "green";
			break;
		}
	}

	function halt() {
		vm.halt();
	}
	
	function create() {
	
		var top;
		
		function createHeader() {
			var e;
			e = createKey(top, "halt", "BREAK", halt);
			e.style.left = "134px";
			e.style.top = "7px";
			e.style.width = "56px";
			e.style.height = "28px";
			e = document.createElement("div");
			e.id = "jb_vm_status";
			e.style.position = "absolute";
			e.style.left = "15px";
			e.style.top = "15px";
			e.style.width = "12px";
			e.style.height = "16px";
			top.appendChild(e);
			updateStatus("HALTED");
		}
		
		function createLCD() {
		
			function createCells() {
				var i, e, ox = 16, oy = 45, x = 0, y = 0;
				for (i = 0; i < 40; i++) {
					e = document.createElement("div");
					e.id = "jb_vm_lcd_" + i;
					e.className = "cell";
					e.style.left = ox + x + "px";
					e.style.top = oy + y + "px";
					e.style.width = 16 + "px";
					e.style.height = 28 + "px";
					e.style.backgroundImage = "url(images/vga14_x2.png)";
					e.style.backgroundPosition = "0px 0px";
					top.appendChild(e);
					cells[i] = e;
					if (i % 10 == 9) {
						x = 0;
						y += 29;
					} else {
						x += 17;
					}
				}
			}

			createCells();
		}
		
		function createKeypad() {
			var width = 56, height = 40, column, x, y, i, b, id, e;
			column = 0;
			x = 10;
			y = 170;
			for (i = 0; i < buttons.length; i++) {
				b = buttons[i];
				id = "k_" + (b.hasOwnProperty('id') ? b.id : b.label); 
				e = createKey(top, id, b.label);
				e.style.left = x + "px";
				e.style.top = y + "px";
				e.style.width = width + "px";
				e.style.height = height + "px";
				x += width + 5;
				column++;
				if (column == 3) {
					column = 0;
					x = 10
					y += height + 5;
				}
				e.onclick = onclickHandler;
			}
		}

		top = document.getElementById("jb_top");
		createHeader();
		createLCD();
		createKeypad();
	}

	create();
	
	this.setCore = function(vm_, io) {
		vm = vm_;
		video = io.getVideoBuffer();
		keyPress = io.keyPress;
	};
	
	this.update = function() {
		var i, c, col, x, y;
		for (i = 0; i < 40; i++) {
			c = video[i];
			col = c % 16;
			x = -16 * col;
			y = -28 * ((c - col) / 16);
			cells[i].style.backgroundPosition = x + "px " + y + "px";
		}
	};
	
	this.setVMStatus = function(status) {
		updateStatus(status);
	};
}

function Simulator(vm_) {

	var vm = vm_;
	var pendingTimeouts = 0;

	function timeoutHandler() {
		pendingTimeouts--;
		if (pendingTimeouts > 0)
			return;
		var ms = vm.advance();
		if (vm.isRunning()) {
			pendingTimeouts++;
			window.setTimeout("vmTimeoutHandler()", ms);
		}
	}

	window.vmTimeoutHandler = timeoutHandler;

	this.run = function(prog) {
		var msg = document.getElementById("jb_msg");
		vm.reset();
		if (JBIT.core_parse(prog) == 0) {
			var	p = JBIT.core_get_prg_data(),
				n = JBIT.core_get_prg_length(),
				i = 0,
				org = 0x300;
			for (i = 0; i < n; i++) {
				vm.put(org + i, Module.getValue(p + i, "i8") & 0xff);
				// make browser happy
			}
			msg.innerHTML = "OK";
			vm.start();
			pendingTimeouts++;
			timeoutHandler();
			return -1;
		} else {
			var lineno = JBIT.core_get_error_lineno();
			msg.innerHTML = lineno + ":" + JBIT.core_get_error_colno() + ": " + JBIT.core_get_error_msg();
			return lineno;
		}
	}
}

JBIT.program_id = "";

JBIT.start = function() {

	JBIT.core_parse = Module.cwrap('core_parse', 'number', ['string']);
	JBIT.core_get_prg_data = Module.cwrap('core_get_prg_data', 'number', []);
	JBIT.core_get_prg_length = Module.cwrap('core_get_prg_length', 'number', []);
	JBIT.core_get_error_lineno = Module.cwrap('core_get_error_lineno', 'number', []);
	JBIT.core_get_error_colno = Module.cwrap('core_get_error_colno', 'number', []);
	JBIT.core_get_error_msg = Module.cwrap('core_get_error_msg', 'string', []);

	var vmPage;
	var editPage;
	var demoPage;

	vmPage = new VMPage();

	var io = new MicroIO();
	io.init();
	var vm = new VM(io, vmPage);
	vm.init();

	vmPage.setCore(vm, io);

	var editor = ace.edit("jb_source");
	editor.setTheme("ace/theme/monokai");
	editor.setShowPrintMargin(true);
	editor.getSession().setTabSize(8);
	document.getElementById('jb_source').style.fontSize='14px';

	function selectPlayground() {
		JBIT.program_id = "playground";
		editor.setValue(JBIT.playground, -1);
		editor.setReadOnly(false);
		$("#jb_toolbox li").removeClass("jb_current_file");
		$("#jb_playground").addClass("jb_current_file");
	}

	function selectProgram(e) {
		var	el = e.target,
			id = $(el).text();
		if (JBIT.program_id === "playground")
			JBIT.playground = editor.getValue();
		if (id === "playground") {
			selectPlayground();
		} else {
			$.get("res/" + id + ".asm", function(prg) {
				JBIT.program_id = id;
				editor.setValue(prg, -1);
				editor.setReadOnly(true);
				$("#jb_toolbox li").removeClass("jb_current_file");
				$(el).addClass("jb_current_file");
			});
		}
	}

	$.get("res/playground.asm", function(prg) {
		JBIT.playground = prg;
		selectPlayground();
	});

	$("#jb_toolbox li").click(selectProgram);

	var sim = new Simulator(vm);

	document.getElementById("jb_run").onclick = function() {
		var prog = editor.getValue();
		var lineno = sim.run(prog);
		if (lineno > 0)
			editor.gotoLine(lineno);
		return false;
	};
};
