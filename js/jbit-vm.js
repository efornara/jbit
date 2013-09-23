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
 CPU
 ****************************************************************************
 ****************************************************************************/

function CPU() {

	/************************************************************************
	   PRIVATE
	 ************************************************************************/
	
	var m;
	var status;
	var pc;
	var a, x, y, s;
	var n, v, b, d, i, z, c;
	var dummyFlag;
	
	var nc; // = 0; number of cycles 
	var ni; // = 0; number of instructions 

	function getP() {
		  return (n ? 0x80 : 0) |
		  (v ? 0x40 : 0) |
		  (dummyFlag ? 0x20 : 0) |
		  (b ? 0x10 : 0) |
		  (d ? 0x08 : 0) |
		  (i ? 0x04 : 0) |
		  (z ? 0x02 : 0) |
		  (c ? 0x01 : 0);
	}
	
	function putP(value) {
		 n = (value & 0x80) != 0;
		 v = (value & 0x40) != 0;
		 dummyFlag = (value & 0x20) != 0;
		 b = (value & 0x10) != 0;
		 d = (value & 0x08) != 0;
		 i = (value & 0x04) != 0;
		 z = (value & 0x02) != 0;
		 c = (value & 0x01) != 0;
	}

	function stepImpl() {
		var address, tmp;
		var value;
		var flag;
		/*
console.log((pc >> 8) + ":" + (pc & 0xFF) + "  " +
				m.get(pc + 0) + " " +
				m.get(pc + 1) + " " +
				m.get(pc + 2) + " " +
				"- " +
				"A:" + a + " " + 
				"X:" + x + " " + 
				"Y:" + y + " " + 
"");
*/
		var opcode = m.get(pc++);
		switch (opcode) {
		//
case 0x00:
 status = "HALT";
 nc += 2;
 pc--;
break;
case 0x10:
 value = m.get(pc++);
 if (!n)
  pc += value > 127 ? value - 256 : value;
 nc += 2;
break;
case 0x20:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 m.put(s | 0x100, (((pc - 1) & 0xFF00) >> 8));
 m.put((s + -1) & 0xFF | 0x100, ((pc - 1) & 0xFF));
 s -= 2;
 s = s & 0xFF;
 pc = address;
 nc += 6;
break;
case 0x30:
 value = m.get(pc++);
 if (n)
  pc += value > 127 ? value - 256 : value;
 nc += 2;
break;
case 0x40:
 status = "UNSUPPORTED_OPCODE"; pc--;
break;
case 0x50:
 value = m.get(pc++);
 if (!v)
  pc += value > 127 ? value - 256 : value;
 nc += 2;
break;
case 0x60:
 pc = ((m.get((s + 1) & 0xFF | 0x100) & 0xFF) | (m.get((s + 2) & 0xFF | 0x100) << 8)) + 1;
 s += 2;
 s = s & 0xFF;
 nc += 6;
break;
case 0x70:
 value = m.get(pc++);
 if (v)
  pc += value > 127 ? value - 256 : value;
 nc += 2;
break;
case 0x90:
 value = m.get(pc++);
 if (!c)
  pc += value > 127 ? value - 256 : value;
 nc += 2;
break;
case 0xA0:
 y = m.get(pc++);
 n = y >= 0x80; z = y == 0;
 nc += 2;
break;
case 0xB0:
 value = m.get(pc++);
 if (c)
  pc += value > 127 ? value - 256 : value;
 nc += 2;
break;
case 0xC0:
 value = m.get(pc++);
 tmp = y - value;
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 2;
break;
case 0xD0:
 value = m.get(pc++);
 if (!z)
  pc += value > 127 ? value - 256 : value;
 nc += 2;
break;
case 0xE0:
 value = m.get(pc++);
 tmp = x - value;
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 2;
break;
case 0xF0:
 value = m.get(pc++);
 if (z)
  pc += value > 127 ? value - 256 : value;
 nc += 2;
break;
case 0x01:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case 0x11:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case 0x21:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case 0x31:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case 0x41:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case 0x51:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case 0x61:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case 0x71:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case 0x81:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 m.put(address, a);
 nc += 6;
break;
case 0x91:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 m.put(address, a);
 nc += 5;
break;
case 0xA1:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case 0xB1:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case 0xC1:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 6;
break;
case 0xD1:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 5;
break;
case 0xE1:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case 0xF1:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case 0xA2:
 x = m.get(pc++);
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case 0x24:
 address = m.get(pc++);
 value = m.get(address);
 n = (value & 0x80) != 0; v = (value & 0x40) != 0; z = (value & a) != 0;
 nc += 3;
break;
case 0x84:
 address = m.get(pc++);
 m.put(address, y);
 nc += 3;
break;
case 0x94:
 address = (m.get(pc++) + x) & 0xFF;
 m.put(address, y);
 nc += 4;
break;
case 0xA4:
 address = m.get(pc++);
 y = m.get(address);
 n = y >= 0x80; z = y == 0;
 nc += 3;
break;
case 0xB4:
 address = (m.get(pc++) + x) & 0xFF;
 y = m.get(address);
 n = y >= 0x80; z = y == 0;
 nc += 4;
break;
case 0xC4:
 address = m.get(pc++);
 tmp = y - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 3;
break;
case 0xE4:
 address = m.get(pc++);
 tmp = x - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 3;
break;
case 0x05:
 address = m.get(pc++);
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case 0x15:
 address = (m.get(pc++) + x) & 0xFF;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x25:
 address = m.get(pc++);
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case 0x35:
 address = (m.get(pc++) + x) & 0xFF;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x45:
 address = m.get(pc++);
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case 0x55:
 address = (m.get(pc++) + x) & 0xFF;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x65:
 address = m.get(pc++);
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case 0x75:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x85:
 address = m.get(pc++);
 m.put(address, a);
 nc += 3;
break;
case 0x95:
 address = (m.get(pc++) + x) & 0xFF;
 m.put(address, a);
 nc += 4;
break;
case 0xA5:
 address = m.get(pc++);
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case 0xB5:
 address = (m.get(pc++) + x) & 0xFF;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0xC5:
 address = m.get(pc++);
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 3;
break;
case 0xD5:
 address = (m.get(pc++) + x) & 0xFF;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case 0xE5:
 address = m.get(pc++);
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case 0xF5:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x06:
 address = m.get(pc++);
 value = m.get(address);
 c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case 0x16:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x26:
 address = m.get(pc++);
 value = m.get(address);
 flag = c; c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; if (flag) value |= 0x01; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case 0x36:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 flag = c; c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; if (flag) value |= 0x01; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x46:
 address = m.get(pc++);
 value = m.get(address);
 c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case 0x56:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x66:
 address = m.get(pc++);
 value = m.get(address);
 flag = c; c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; if (flag) value |= 0x80; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case 0x76:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 flag = c; c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; if (flag) value |= 0x80; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x86:
 address = m.get(pc++);
 m.put(address, x);
 nc += 3;
break;
case 0x96:
 address = (m.get(pc++) + y) & 0xFF;
 m.put(address, x);
 nc += 4;
break;
case 0xA6:
 address = m.get(pc++);
 x = m.get(address);
 n = x >= 0x80; z = x == 0;
 nc += 3;
break;
case 0xB6:
 address = (m.get(pc++) + y) & 0xFF;
 x = m.get(address);
 n = x >= 0x80; z = x == 0;
 nc += 4;
break;
case 0xC6:
 address = m.get(pc++);
 value = m.get(address);
 value--;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case 0xD6:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 value--;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0xE6:
 address = m.get(pc++);
 value = m.get(address);
 value++;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case 0xF6:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 value++;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x08:
 m.put(s | 0x100, getP());
 s--;
 s = s & 0xFF;
 nc += 3;
break;
case 0x18:
 c = false;
 nc += 2;
break;
case 0x28:
 s++;
 s = s & 0xFF;
 value = m.get(s | 0x100);
 putP(value);
 nc += 4;
break;
case 0x38:
 c = true;
 nc += 2;
break;
case 0x48:
 m.put(s | 0x100, a);
 s--;
 s = s & 0xFF;
 nc += 3;
break;
case 0x58:
 i = false;
 nc += 2;
break;
case 0x68:
 s++;
 s = s & 0xFF;
 a = m.get(s | 0x100);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x78:
 i = true;
 nc += 2;
break;
case 0x88:
 y--;
 y = y & 0xFF;
 n = y >= 0x80; z = y == 0;
 nc += 2;
break;
case 0x98:
 a = y;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0xA8:
 y = a;
 n = y >= 0x80; z = y == 0;
 nc += 2;
break;
case 0xB8:
 v = false;
 nc += 2;
break;
case 0xC8:
 y++;
 y = y & 0xFF;
 n = y >= 0x80; z = y == 0;
 nc += 2;
break;
case 0xD8:
 d = false;
 nc += 2;
break;
case 0xE8:
 x++;
 x = x & 0xFF;
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case 0xF8:
 status = "UNSUPPORTED_OPCODE"; pc--;
break;
case 0x09:
 value = m.get(pc++);
 a |= value;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x19:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x29:
 value = m.get(pc++);
 a &= value;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x39:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x49:
 value = m.get(pc++);
 a ^= value;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x59:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x69:
 value = m.get(pc++);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x79:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x99:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 m.put(address, a);
 nc += 5;
break;
case 0xA9:
 a = m.get(pc++);
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0xB9:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0xC9:
 value = m.get(pc++);
 tmp = a - value;
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 2;
break;
case 0xD9:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case 0xE9:
 value = m.get(pc++);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0xF9:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x0A:
 c = (a & 0x80) != 0; a <<= 1; a = a & 0xFF; n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x2A:
 flag = c; c = (a & 0x80) != 0; a <<= 1; a = a & 0xFF; if (flag) a |= 0x01; n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x4A:
 c = (a & 0x01) != 0; a >>= 1; a = a & 0xFF; n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x6A:
 flag = c; c = (a & 0x01) != 0; a >>= 1; a = a & 0xFF; if (flag) a |= 0x80; n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x8A:
 a = x;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case 0x9A:
 s = x;
 n = s >= 0x80; z = s == 0;
 nc += 2;
break;
case 0xAA:
 x = a;
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case 0xBA:
 x = s;
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case 0xCA:
 x--;
 x = x & 0xFF;
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case 0xEA:
 nc += 2;
break;
case 0x2C:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 n = (value & 0x80) != 0; v = (value & 0x40) != 0; z = (value & a) != 0;
 nc += 4;
break;
case 0x4C:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 pc = address;
 nc += 3;
break;
case 0x6C:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 pc = ((m.get(address) & 0xFF) | (m.get(address+1) << 8));
 nc += 5;
break;
case 0x8C:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 m.put(address, y);
 nc += 4;
break;
case 0xAC:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 y = m.get(address);
 n = y >= 0x80; z = y == 0;
 nc += 4;
break;
case 0xBC:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 y = m.get(address);
 n = y >= 0x80; z = y == 0;
 nc += 4;
break;
case 0xCC:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 tmp = y - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case 0xEC:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 tmp = x - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case 0x0D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x1D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x2D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x3D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x4D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x5D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x6D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x7D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x8D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 m.put(address, a);
 nc += 4;
break;
case 0x9D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 m.put(address, a);
 nc += 5;
break;
case 0xAD:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0xBD:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0xCD:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case 0xDD:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case 0xED:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0xFD:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case 0x0E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x1E:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case 0x2E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 flag = c; c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; if (flag) value |= 0x01; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x3E:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 flag = c; c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; if (flag) value |= 0x01; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case 0x4E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x5E:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case 0x6E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 flag = c; c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; if (flag) value |= 0x80; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0x7E:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 flag = c; c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; if (flag) value |= 0x80; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case 0x8E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 m.put(address, x);
 nc += 6;
break;
case 0xAE:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 x = m.get(address);
 n = x >= 0x80; z = x == 0;
 nc += 6;
break;
case 0xBE:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 x = m.get(address);
 n = x >= 0x80; z = x == 0;
 nc += 7;
break;
case 0xCE:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 value--;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0xDE:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 value--;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case 0xEE:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 value++;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case 0xFE:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 value++;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
		//
		default: // Invalid opcode
			status = "INVALID_OPCODE";
			pc--;
			return;
		}
		ni++;
	}

	/************************************************************************
	   PUBLIC
	 ************************************************************************/

	this.setAddressSpace = function(addressSpace) {
		m = addressSpace;
	};

	this.reset = function() {
		status = "OK";
		a = 0;
		x = 0;
		y = 0;
		n = false;
		v = false;
		b = false;
		d = false;
		i = false;
		z = false;
		c = false;
		pc = 0x300;
		s = 0xFF;
		nc = 0;
		ni = 0;
	};
	
	this.step = function() {
		stepImpl();
	};
	
	this.getStatus = function() {
		return status;
	};
}


/****************************************************************************
 ****************************************************************************
 VM
 ****************************************************************************
 ****************************************************************************/

function VM(io_, skin_) {

	/************************************************************************
	   PRIVATE
	 ************************************************************************/

	var running;
	var cpu = new CPU();
	var io = io_;
	var skin = skin_;
	
	var m = new Array();
	
	function memoryReset() {
		var i = 0;
		
		for (i = 0; i < 65536; i++)
			m[i] = 0;
	}
	
	function loadProgramFromText(text) {
		var pattern, code = -1, data, page, exc;
		var lines = text.split("\n");
		for (var i in lines) {
			var line = lines[i];
			var c0 = line.charAt(0);
			switch (c0) {
			case 'S':
				pattern = /^Size: (\d+) code pages?, (\d+) data pages?./;
				if (exc = pattern.exec(line)) {
					code = parseInt(exc[1], 10);
					data = parseInt(exc[2], 10);
					page = -1;
				}
				break;
			case 'C':
			case 'D':
				pattern = /^[CD] (\d+)/;
				if (code != -1 && (exc = pattern.exec(line))) {
					page = parseInt(exc[1], 10) * 256;
				}
				break;
			case '0':
			case '1':
			case '2':
				pattern = /^(\d\d\d): /;
				if (page != -1 && (exc = pattern.exec(line))) {
					var address = page + parseInt(exc[1], 10);
					var fields = line.split(" ");
					for (var j = 1; j < fields.length; j++) {
						value = parseInt(fields[j], 10);
						m[address] = value;
						address++;
					}
				}
				break;
			}
		}
	}

	function advanceImpl() {
		if (!running) {
			skin.setVMStatus("HALTED");
			return;
		}
		io.wakeup();
		for (i = 0; i < 2000; i++) {
			cpu.step();
			if (io.getWait() != 0)
				break;
		}
		skin.update();
		switch (cpu.getStatus()) {
		case "OK":
			break;
		case "HALT":
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
		cpu.setAddressSpace(this);
		this.reset();
	};
	
	this.reset = function() {
		memoryReset();
		io.reset();
		cpu.reset();
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

	// CPU INTERFACE
	
	this.get = function(address) {
		if (address >= 512 && address < 768)
			return io.get(address - 512);
		else
			return m[address];
	};
	
	this.put = function(address, value) {
		if (address >= 512 && address < 768) {
			io.put(address - 512, value);
			// TODO: check status
		} else
			m[address] = value;
	};
}
