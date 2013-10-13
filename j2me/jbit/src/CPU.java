/*
	JBit - A 6502 framework for mobile phones
	Copyright (C) 2007-2010  Emanuele Fornara
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

public class CPU
//#if !JBIT_RUNTIME
implements Module
//#endif
{
	
	/*
	 * The bulk of this method is generated using the C preprocessor
	 * starting from include/opcodes.p
	 * It could be implemented in a few ways trading speed and code size,
	 * for now this would do even if probably is the biggest and the slowest.
	 */
	public final void step() {
		int address, tmp;
		int value;
		boolean flag;
		// #if TRACE_CPU
//@		System.out.println((pc >> 8) + ":" + (pc & 0xFF) + "  " +
//@				m.get(pc + 0) + " " +
//@				m.get(pc + 1) + " " +
//@				m.get(pc + 2) + " " +
//@				"- " +
//@				"A:" + a + " " + 
//@				"X:" + x + " " + 
//@				"Y:" + y + " " + 
//@		"");
		// #endif
		byte opcode = (byte)m.get(pc++);
		switch (opcode) {
		/// {{{ cpp opcodes.p !
case (byte)0x00:
 status = CPUSvc.HALT;
 nc += 2;
 pc--;
break;
case (byte)0x10:
 value = m.get(pc++);
 if (!n)
  pc += (byte)value;
 nc += 2;
break;
case (byte)0x20:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 m.put(s | 0x100, (byte)(((pc - 1) & 0xFF00) >> 8));
 m.put((s + -1) & 0xFF | 0x100, (byte)((pc - 1) & 0xFF));
 s -= 2;
 s = s & 0xFF;
 pc = address;
 nc += 6;
break;
case (byte)0x30:
 value = m.get(pc++);
 if (n)
  pc += (byte)value;
 nc += 2;
break;
case (byte)0x40:
 status = CPUSvc.UNSUPPORTED_OPCODE; pc--;
break;
case (byte)0x50:
 value = m.get(pc++);
 if (!v)
  pc += (byte)value;
 nc += 2;
break;
case (byte)0x60:
 pc = ((m.get((s + 1) & 0xFF | 0x100) & 0xFF) | (m.get((s + 2) & 0xFF | 0x100) << 8)) + 1;
 s += 2;
 s = s & 0xFF;
 nc += 6;
break;
case (byte)0x70:
 value = m.get(pc++);
 if (v)
  pc += (byte)value;
 nc += 2;
break;
case (byte)0x90:
 value = m.get(pc++);
 if (!c)
  pc += (byte)value;
 nc += 2;
break;
case (byte)0xA0:
 y = m.get(pc++);
 n = y >= 0x80; z = y == 0;
 nc += 2;
break;
case (byte)0xB0:
 value = m.get(pc++);
 if (c)
  pc += (byte)value;
 nc += 2;
break;
case (byte)0xC0:
 value = m.get(pc++);
 tmp = y - value;
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 2;
break;
case (byte)0xD0:
 value = m.get(pc++);
 if (!z)
  pc += (byte)value;
 nc += 2;
break;
case (byte)0xE0:
 value = m.get(pc++);
 tmp = x - value;
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 2;
break;
case (byte)0xF0:
 value = m.get(pc++);
 if (z)
  pc += (byte)value;
 nc += 2;
break;
case (byte)0x01:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case (byte)0x11:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case (byte)0x21:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case (byte)0x31:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case (byte)0x41:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case (byte)0x51:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case (byte)0x61:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case (byte)0x71:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case (byte)0x81:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 m.put(address, a);
 nc += 6;
break;
case (byte)0x91:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 m.put(address, a);
 nc += 5;
break;
case (byte)0xA1:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case (byte)0xB1:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case (byte)0xC1:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 6;
break;
case (byte)0xD1:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 5;
break;
case (byte)0xE1:
 address = (m.get(pc++) + x) & 0xFF; address = m.get(address) | (m.get((address + 1) & 0xFF) << 8);
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 6;
break;
case (byte)0xF1:
 address = m.get(pc++); tmp = m.get(address) + y; address = (tmp + (m.get((address + 1) & 0xFF) << 8)) & 0xFFFF;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 5;
break;
case (byte)0xA2:
 x = m.get(pc++);
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case (byte)0x24:
 address = m.get(pc++);
 value = m.get(address);
 n = (value & 0x80) != 0; v = (value & 0x40) != 0; z = (value & a) != 0;
 nc += 3;
break;
case (byte)0x84:
 address = m.get(pc++);
 m.put(address, y);
 nc += 3;
break;
case (byte)0x94:
 address = (m.get(pc++) + x) & 0xFF;
 m.put(address, y);
 nc += 4;
break;
case (byte)0xA4:
 address = m.get(pc++);
 y = m.get(address);
 n = y >= 0x80; z = y == 0;
 nc += 3;
break;
case (byte)0xB4:
 address = (m.get(pc++) + x) & 0xFF;
 y = m.get(address);
 n = y >= 0x80; z = y == 0;
 nc += 4;
break;
case (byte)0xC4:
 address = m.get(pc++);
 tmp = y - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 3;
break;
case (byte)0xE4:
 address = m.get(pc++);
 tmp = x - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 3;
break;
case (byte)0x05:
 address = m.get(pc++);
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case (byte)0x15:
 address = (m.get(pc++) + x) & 0xFF;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x25:
 address = m.get(pc++);
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case (byte)0x35:
 address = (m.get(pc++) + x) & 0xFF;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x45:
 address = m.get(pc++);
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case (byte)0x55:
 address = (m.get(pc++) + x) & 0xFF;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x65:
 address = m.get(pc++);
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case (byte)0x75:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x85:
 address = m.get(pc++);
 m.put(address, a);
 nc += 3;
break;
case (byte)0x95:
 address = (m.get(pc++) + x) & 0xFF;
 m.put(address, a);
 nc += 4;
break;
case (byte)0xA5:
 address = m.get(pc++);
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case (byte)0xB5:
 address = (m.get(pc++) + x) & 0xFF;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0xC5:
 address = m.get(pc++);
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 3;
break;
case (byte)0xD5:
 address = (m.get(pc++) + x) & 0xFF;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case (byte)0xE5:
 address = m.get(pc++);
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 3;
break;
case (byte)0xF5:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x06:
 address = m.get(pc++);
 value = m.get(address);
 c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case (byte)0x16:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x26:
 address = m.get(pc++);
 value = m.get(address);
 flag = c; c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; if (flag) value |= 0x01; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case (byte)0x36:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 flag = c; c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; if (flag) value |= 0x01; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x46:
 address = m.get(pc++);
 value = m.get(address);
 c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case (byte)0x56:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x66:
 address = m.get(pc++);
 value = m.get(address);
 flag = c; c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; if (flag) value |= 0x80; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case (byte)0x76:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 flag = c; c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; if (flag) value |= 0x80; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x86:
 address = m.get(pc++);
 m.put(address, x);
 nc += 3;
break;
case (byte)0x96:
 address = (m.get(pc++) + y) & 0xFF;
 m.put(address, x);
 nc += 4;
break;
case (byte)0xA6:
 address = m.get(pc++);
 x = m.get(address);
 n = x >= 0x80; z = x == 0;
 nc += 3;
break;
case (byte)0xB6:
 address = (m.get(pc++) + y) & 0xFF;
 x = m.get(address);
 n = x >= 0x80; z = x == 0;
 nc += 4;
break;
case (byte)0xC6:
 address = m.get(pc++);
 value = m.get(address);
 value--;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case (byte)0xD6:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 value--;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0xE6:
 address = m.get(pc++);
 value = m.get(address);
 value++;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 5;
break;
case (byte)0xF6:
 address = (m.get(pc++) + x) & 0xFF;
 value = m.get(address);
 value++;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x08:
 m.put(s | 0x100, (byte)getP());
 s--;
 s = s & 0xFF;
 nc += 3;
break;
case (byte)0x18:
 c = false;
 nc += 2;
break;
case (byte)0x28:
 s++;
 s = s & 0xFF;
 value = m.get(s | 0x100);
 putP(value);
 nc += 4;
break;
case (byte)0x38:
 c = true;
 nc += 2;
break;
case (byte)0x48:
 m.put(s | 0x100, a);
 s--;
 s = s & 0xFF;
 nc += 3;
break;
case (byte)0x58:
 i = false;
 nc += 2;
break;
case (byte)0x68:
 s++;
 s = s & 0xFF;
 a = m.get(s | 0x100);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x78:
 i = true;
 nc += 2;
break;
case (byte)0x88:
 y--;
 y = y & 0xFF;
 n = y >= 0x80; z = y == 0;
 nc += 2;
break;
case (byte)0x98:
 a = y;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0xA8:
 y = a;
 n = y >= 0x80; z = y == 0;
 nc += 2;
break;
case (byte)0xB8:
 v = false;
 nc += 2;
break;
case (byte)0xC8:
 y++;
 y = y & 0xFF;
 n = y >= 0x80; z = y == 0;
 nc += 2;
break;
case (byte)0xD8:
 d = false;
 nc += 2;
break;
case (byte)0xE8:
 x++;
 x = x & 0xFF;
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case (byte)0xF8:
 status = CPUSvc.UNSUPPORTED_OPCODE; pc--;
break;
case (byte)0x09:
 value = m.get(pc++);
 a |= value;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x19:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x29:
 value = m.get(pc++);
 a &= value;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x39:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x49:
 value = m.get(pc++);
 a ^= value;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x59:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x69:
 value = m.get(pc++);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x79:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x99:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 m.put(address, a);
 nc += 5;
break;
case (byte)0xA9:
 a = m.get(pc++);
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0xB9:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0xC9:
 value = m.get(pc++);
 tmp = a - value;
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 2;
break;
case (byte)0xD9:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case (byte)0xE9:
 value = m.get(pc++);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0xF9:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x0A:
 c = (a & 0x80) != 0; a <<= 1; a = a & 0xFF; n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x2A:
 flag = c; c = (a & 0x80) != 0; a <<= 1; a = a & 0xFF; if (flag) a |= 0x01; n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x4A:
 c = (a & 0x01) != 0; a >>= 1; a = a & 0xFF; n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x6A:
 flag = c; c = (a & 0x01) != 0; a >>= 1; a = a & 0xFF; if (flag) a |= 0x80; n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x8A:
 a = x;
 n = a >= 0x80; z = a == 0;
 nc += 2;
break;
case (byte)0x9A:
 s = x;
 n = s >= 0x80; z = s == 0;
 nc += 2;
break;
case (byte)0xAA:
 x = a;
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case (byte)0xBA:
 x = s;
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case (byte)0xCA:
 x--;
 x = x & 0xFF;
 n = x >= 0x80; z = x == 0;
 nc += 2;
break;
case (byte)0xEA:
 nc += 2;
break;
case (byte)0x2C:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 n = (value & 0x80) != 0; v = (value & 0x40) != 0; z = (value & a) != 0;
 nc += 4;
break;
case (byte)0x4C:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 pc = address;
 nc += 3;
break;
case (byte)0x6C:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 pc = ((m.get(address) & 0xFF) | (m.get(address+1) << 8));
 nc += 5;
break;
case (byte)0x8C:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 m.put(address, y);
 nc += 4;
break;
case (byte)0xAC:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 y = m.get(address);
 n = y >= 0x80; z = y == 0;
 nc += 4;
break;
case (byte)0xBC:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 y = m.get(address);
 n = y >= 0x80; z = y == 0;
 nc += 4;
break;
case (byte)0xCC:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 tmp = y - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case (byte)0xEC:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 tmp = x - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case (byte)0x0D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x1D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 a |= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x2D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x3D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 a &= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x4D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x5D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 a ^= m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x6D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x7D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 tmp = value + a; if (c) tmp++; v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); c = tmp > 0xFF; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x8D:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 m.put(address, a);
 nc += 4;
break;
case (byte)0x9D:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 m.put(address, a);
 nc += 5;
break;
case (byte)0xAD:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0xBD:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 a = m.get(address);
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0xCD:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case (byte)0xDD:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 tmp = a - m.get(address);
 n = (tmp & 0x80) != 0; z = tmp == 0; c = tmp >= 0;
 nc += 4;
break;
case (byte)0xED:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0xFD:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 tmp = a - value; if (!c) tmp--; v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); c = tmp >= 0; a = tmp & 0xFF; n = a >= 0x80; z = a == 0;
 n = a >= 0x80; z = a == 0;
 nc += 4;
break;
case (byte)0x0E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x1E:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case (byte)0x2E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 flag = c; c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; if (flag) value |= 0x01; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x3E:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 flag = c; c = (value & 0x80) != 0; value <<= 1; value = value & 0xFF; if (flag) value |= 0x01; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case (byte)0x4E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x5E:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case (byte)0x6E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 flag = c; c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; if (flag) value |= 0x80; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0x7E:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 flag = c; c = (value & 0x01) != 0; value >>= 1; value = value & 0xFF; if (flag) value |= 0x80; n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case (byte)0x8E:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 m.put(address, x);
 nc += 6;
break;
case (byte)0xAE:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 x = m.get(address);
 n = x >= 0x80; z = x == 0;
 nc += 6;
break;
case (byte)0xBE:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += y;
 x = m.get(address);
 n = x >= 0x80; z = x == 0;
 nc += 7;
break;
case (byte)0xCE:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 value--;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0xDE:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 value--;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
case (byte)0xEE:
 address = m.get(pc++); address |= m.get(pc++) << 8;
 value = m.get(address);
 value++;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 6;
break;
case (byte)0xFE:
 address = m.get(pc++); address |= m.get(pc++) << 8; address += x;
 value = m.get(address);
 value++;
 value = value & 0xFF;
 n = value >= 0x80; z = value == 0;
 m.put(address, value);
 nc += 7;
break;
		/// }}}
		default: // Invalid opcode
			status = CPUSvc.INVALID_OPCODE;
			pc--;
			return;
		}
		ni++;
	}

	// #if !JBIT_RUNTIME
	private Module m;
	// #else
//@	public VM m;
	// #endif

	public int status;
	public int pc;
	private int a, x, y, s;
	private boolean n, v, b, d, i, z, c;
	private boolean dummyFlag;
	
	private long nc; // = 0; number of cycles 
	private long ni; // = 0; number of instructions 
	
	private int getP() {
		  return (n ? 0x80 : 0) |
		  (v ? 0x40 : 0) |
		  (dummyFlag ? 0x20 : 0) |
		  (b ? 0x10 : 0) |
		  (d ? 0x08 : 0) |
		  (i ? 0x04 : 0) |
		  (z ? 0x02 : 0) |
		  (c ? 0x01 : 0);
	}
	
	private void putP(int value) {
		 n = (value & 0x80) != 0;
		 v = (value & 0x40) != 0;
		 dummyFlag = (value & 0x20) != 0;
		 b = (value & 0x10) != 0;
		 d = (value & 0x08) != 0;
		 i = (value & 0x04) != 0;
		 z = (value & 0x02) != 0;
		 c = (value & 0x01) != 0;
	}

	public void reset() {
		status = CPUSvc.OK;
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
	}
	
	// #if !JBIT_RUNTIME

	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "CPU";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_SUPPORT);
		case METADATA_ID_SERVICES:
			return new String[] { CPUSvc.TAG };
		default:
			return null;
		}
	}
	
	public Object opO(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_GET_METADATA:
			return getMetadata(iArg);
		case CPUSvc.OP_GET_COUNTER:
			switch (iArg) {
			case CPUSvc.COUNTER_CYCLES:
				return new Long(nc);
			case CPUSvc.COUNTER_INSTRUCTIONS:
				return new Long(ni);
			}
		}
		return null;
	}

	public int opI(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_INIT:
		case OP_ACTIVATE:
		case OP_DEACTIVATE:
			return 0;
		case CPUSvc.OP_RESET:
			reset();
			return 0;
		case CPUSvc.OP_SET_ADDRESS_SPACE:
			m = (Module)oArg;
			return 0;
		case CPUSvc.OP_STEP:
			step();
			return 0;
		case CPUSvc.OP_GET_STATUS:
			return status;
		case CPUSvc.OP_SET_STATUS:
			status = iArg;
			return 0;
		}
		return -1;
	}

	public int get(int address) {
		switch (address) {
		case CPUSvc.ADDRESS_PCLO:
			return pc & 0xFF;
		case CPUSvc.ADDRESS_PCHI:
			return pc >> 8;
		case CPUSvc.ADDRESS_S:
			return s;
		case CPUSvc.ADDRESS_P:
			return getP();
		case CPUSvc.ADDRESS_A:
			return a;
		case CPUSvc.ADDRESS_X:
			return x;
		case CPUSvc.ADDRESS_Y:
			return y;
		case CPUSvc.ADDRESS_PC:
			return pc;
		}
		return -1;
	}

	public int put(int address, int value) {
		switch (address) {
		case CPUSvc.ADDRESS_PCLO:
			pc = (pc & 0xFF00) | value;
			break;
		case CPUSvc.ADDRESS_PCHI:
			pc = (pc & 0x00FF) | (value << 8);
			break;
		case CPUSvc.ADDRESS_S:
			s = value;
			break;
		case CPUSvc.ADDRESS_P:
			putP(value);
		case CPUSvc.ADDRESS_A:
			a = value;
			break;
		case CPUSvc.ADDRESS_X:
			x = value;
			break;
		case CPUSvc.ADDRESS_Y:
			y = value;
			break;
		case CPUSvc.ADDRESS_PC:
			pc = value;
			break;
		default:
			return -1;
		}
		return 0;
	}
	
	// #endif

}
