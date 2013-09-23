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

var UI = function() {
	return {
	
		space: 5,
		
		width: 180,
		
		height: 240,
		
		createPage: function(id) {
			var e;
			e = document.createElement("div");
			e.className = "page";
			e.id = "jb_page_" + id;
			document.getElementById("jb_top").appendChild(e);
			return e;
		},	
		
		getPage: function(id) {
			return document.getElementById("jb_page_" + id);
		},
		
		switchToPage: function(id) {
			var c, i;
			c = document.getElementById("jb_top").children;
			for (i in c) {
				if (c[i].nodeName != "DIV")
					continue;
				if (c[i].id == "jb_page_" + id)
					c[i].style.display = "block";
				else
					c[i].style.display = "none";
			}
		},
		
		createButton: function(parent, id, title, callback) {
			var e;
			e = document.createElement("input");
			e.id = "jb_btn_" + id;
			e.className = "button";
			e.type = "button";
			e.style.position = "absolute";
			if (title)
				e.value = title;
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
		},
		
		createTextArea: function(parent, id, callback) {
			var e;
			e = document.createElement("textarea");
			e.id = "jb_txt_" + id;
			e.className = "text";
			e.setAttribute("wrap", "off");
			parent.appendChild(e);
			return e;
		}
		
	};
}();

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
	
	function reflow() {
	
		var y;
	
		function reflowHeader() {
			var width, e;
			width = 60;
			y += UI.space;
			e = document.getElementById("jb_btn_halt");
			e.style.left = (UI.width - width - UI.space) + "px";
			e.style.top = y + "px";
			e.style.width = width + "px";
			e.style.fontSize = "10px";
			e.style.height = "18px";
			y += 18;
			y += UI.space;
		}
		
		function reflowLCD() {
		
			var width, height, e;
			
			function reflowGlass() {
				width = UI.width - UI.space * 2;
				height = 16 * 4 + 12;
				e = document.getElementById("jb_vm_lcd");
				e.style.left = UI.space + "px";
				e.style.top = y + "px";
				e.style.width = (width - 4) + "px";
				e.style.height = (height - 3) + "px";
				y += height;
				y += UI.space;
			}
			
			function reflowCells() {
				var ox, i, row, column, y, x;
				ox = (width - 10 * 8) / 2;
				i = 0;
				y = 5; // TODO
				for (row = 0; row < 4; row++) {
					x = ox;
					for (col = 0; col < 10; col++) {
						e = cells[i++];
						e.style.left = x + "px";
						e.style.top = y + "px";
						e.style.width = 8 + "px";
						e.style.height = 16 + "px";
						x += 8;
					}
					y += 16;
				}
			}

			reflowGlass();
			reflowCells();
		}
		
		function reflowKeypad() {
			var width, height, column, x, i, b, id, e;
			width = Math.floor((UI.width - (UI.space * 4)) / 3);
			height = Math.floor((UI.height - y - (UI.space * 4)) / 4);
			column = 0;
			x = UI.space;
			for (i in buttons) {
				b = buttons[i];
				id = "k_" + (b.hasOwnProperty('id') ? b.id : b.label);
				e = document.getElementById("jb_btn_" + id);
				e.style.left = x + "px";
				e.style.top = y + "px";
				e.style.width = width + "px";
				e.style.height = height + "px";
				x += width + UI.space;
				column++;
				if (column == 3) {
					column = 0;
					x = UI.space;
					y += height + UI.space;
				}
			}
		}
		
		y = 0;
		reflowHeader();
		reflowLCD();
		reflowKeypad();
	}
	
	function create() {
	
		var page;
		
		function createHeader() {
			var e;
			UI.createButton(page, "halt", "BREAK", halt);
			e = document.createElement("div");
			e.id = "jb_vm_status";
			e.style.position = "absolute";
			e.style.left = UI.space + "px";
			e.style.top = UI.space + "px";
			e.style.width = "6px";
			e.style.height = "18px";
			page.appendChild(e);
			updateStatus("HALTED");
		}
		
		function createLCD() {
		
			var glass;
			
			function createGlass() {
				var e;
				e = document.createElement("div");
				e.id = "jb_vm_lcd";
				e.className = "lcd";
				page.appendChild(e);
				return e;
			}
		
			function createCells() {
				var i, e;
				for (i = 0; i < 40; i++) {
					e = document.createElement("div");
					e.id = "jb_vm_lcd_" + i;
					e.className = "cell";
					e.style.backgroundImage = "url(vga.png)";
					e.style.backgroundPosition = "0px 0px";
					glass.appendChild(e);
					cells[i] = e;
				}
			}

			glass = createGlass();
			createCells();
		}
		
		function createKeypad() {
			var i, b, id, e;
			for (i in buttons) {
				b = buttons[i];
				id = "k_" + (b.hasOwnProperty('id') ? b.id : b.label); 
				e = UI.createButton(page, id, b.label);
				e.onclick = onclickHandler;
			}
		}

		page = UI.createPage("vm");
		createHeader();
		createLCD();
		createKeypad();
	}

	create();
	reflow();
	
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
			x = -8 * col;
			y = -16 * ((c - col) / 16);
			cells[i].style.backgroundPosition = x + "px " + y + "px";
		}
	};
	
	this.setVMStatus = function(status) {
		updateStatus(status);
	};
	
	this.resize = function() {
		reflow();
	};
}

function Simulator(vm_) {

	var vm = vm_;
	var pendingTimeouts;

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

	this.load = function(prog) {
		vm.reset();
		vm.load(prog);
	}

	this.start = function() {
		vm.start();
		pendingTimeouts = 1;
		timeoutHandler();
	}
}

window.onload = function() {

	var vmPage;
	var editPage;
	var demoPage;

	vmPage = new VMPage();

	var io = new MicroIO();
	io.init();
	var vm = new VM(io, vmPage);
	vm.init();

	vmPage.setCore(vm, io);

	var sim = new Simulator(vm);

	sim.load(jbProgram);
	UI.switchToPage("vm");
	sim.start();
};
