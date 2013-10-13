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

//////////////////////////////////////////////////////////////////////////////
// MACROS
//////////////////////////////////////////////////////////////////////////////


#define BEGIN(_opcode_, _descr_) \
	case (byte)0x ## _opcode_:
	
#define END \
	break;


#define CYCLES(_n_) \
	nc += _n_;

#define MEM_READ(_address_) \
	m.get(_address_)

#define MEM_READ0(_address_) \
	m.get(_address_)

#define MEM_WRITE(_address_, _value_) \
	m.put(_address_, _value_);

#define MEM_WRITE0(_address_, _value_) \
	m.put(_address_, _value_);

#define STACK_READ(_offset_) \
	m.get((s + _offset_) & 0xFF | 0x100)

#define STACK_WRITE(_offset_, _value_) \
	m.put((s + _offset_) & 0xFF | 0x100, _value_);

#define STACK_READ0 \
	m.get(s | 0x100)

#define STACK_WRITE0(_value_) \
	m.put(s | 0x100, _value_);


#define GET_IMM(_var_) \
	_var_ = MEM_READ(pc++);

#define GET_ABS(_var_) \
	_var_ = MEM_READ(pc++); \
	_var_ |= MEM_READ(pc++) << 8;

#define GET_ABS_X(_var_) \
	_var_ = MEM_READ(pc++); \
	_var_ |= MEM_READ(pc++) << 8; \
	_var_ += x;

#define GET_ABS_Y(_var_) \
	_var_ = MEM_READ(pc++); \
	_var_ |= MEM_READ(pc++) << 8; \
	_var_ += y;

#define GET_ZPAGE(_var_) \
	_var_ = MEM_READ(pc++);

#define GET_ZPAGE_X(_var_) \
	_var_ = (MEM_READ(pc++) + x) & 0xFF; // stays in Z-page?

#define GET_ZPAGE_Y(_var_) \
	_var_ = (MEM_READ(pc++) + y) & 0xFF; // stays in Z-page?

#define GET_IND_X(_var_) \
	address = (MEM_READ(pc++) + x) & 0xFF; \
	_var_ = MEM_READ0(address) | (MEM_READ0((address + 1) & 0xFF) << 8);

// TODO: add instr. dep. cycles
#define GET_IND_Y(_var_) \
	address = MEM_READ(pc++); \
	tmp =  MEM_READ(address) + y; \
	_var_ = (tmp + (MEM_READ0((address + 1) & 0xFF) << 8)) & 0xFFFF;


#define CHECK_NZ(_ref_) \
	n = _ref_ >= 0x80; \
	z = _ref_ == 0;

#define CHECK_CMP \
	n = (tmp & 0x80) != 0; \
	z = tmp == 0; \
	c = tmp >= 0;

#define DO_ADC \
	tmp = value + a; \
	if (c) \
		tmp++; \
	v = (((a ^ value) & 0x80) == 0) && (((a ^ tmp) & 0x80) != 0); \
	c = tmp > 0xFF; \
	a = tmp & 0xFF; \
	CHECK_NZ(a)

#define DO_SBC \
	tmp = a - value; \
	if (!c) \
		tmp--; \
	v = (((a ^ value) & 0x80) != 0) && (((a ^ tmp) & 0x80) != 0); \
	c = tmp >= 0; \
	a = tmp & 0xFF; \
	CHECK_NZ(a)

#define DO_ASL(_value_) \
	c = (_value_ & 0x80) != 0; \
	_value_ <<= 1; \
	_value_ = _value_ & 0xFF; \
	CHECK_NZ(_value_)

#define DO_ROL(_value_) \
	flag = c; \
	c = (_value_ & 0x80) != 0; \
	_value_ <<= 1; \
	_value_ = _value_ & 0xFF; \
	if (flag) \
		_value_ |= 0x01; \
	CHECK_NZ(_value_)

#define DO_LSR(_value_) \
	c = (_value_ & 0x01) != 0; \
	_value_ >>= 1; \
	_value_ = _value_ & 0xFF; \
	CHECK_NZ(_value_)

#define DO_ROR(_value_) \
	flag = c; \
	c = (_value_ & 0x01) != 0; \
	_value_ >>= 1; \
	_value_ = _value_ & 0xFF; \
	if (flag) \
		_value_ |= 0x80; \
	CHECK_NZ(_value_)

#define DO_BIT(_value_) \
	n = (_value_ & 0x80) != 0; \
	v = (_value_ & 0x40) != 0; \
	z = (_value_ & a) != 0;

#define UNSUPPORTED \
	status = CPUSvc.UNSUPPORTED_OPCODE; \
	pc--;


//////////////////////////////////////////////////////////////////////////////
// *0
//////////////////////////////////////////////////////////////////////////////

BEGIN(00, "BRK")
	status = CPUSvc.HALT;
	CYCLES(2)
	pc--;
END

BEGIN(10, "BPL r")
	GET_IMM(value)
	if (!n)
		pc += (byte)value;
	CYCLES(2)
END

BEGIN(20, "JSR n:n")
	GET_ABS(address)
	STACK_WRITE0((byte)(((pc - 1) & 0xFF00) >> 8))
	STACK_WRITE(-1, (byte)((pc - 1) & 0xFF))
	s -= 2;
	s = s & 0xFF;
	pc = address;
	CYCLES(6)
END

BEGIN(30, "BMI r")
	GET_IMM(value)
	if (n)
		pc += (byte)value;
	CYCLES(2)
END

BEGIN(40, "RTI")
	UNSUPPORTED
END

BEGIN(50, "BVC r")
	GET_IMM(value)
	if (!v)
		pc += (byte)value;
	CYCLES(2)
END

BEGIN(60, "RTS")
	pc = ((STACK_READ(1) & 0xFF) | (STACK_READ(2) << 8)) + 1;
	s += 2;
	s = s & 0xFF;
	CYCLES(6)
END

BEGIN(70, "BVS r")
	GET_IMM(value)
	if (v)
		pc += (byte)value;
	CYCLES(2)
END

BEGIN(90, "BCC r")
	GET_IMM(value)
	if (!c)
		pc += (byte)value;
	CYCLES(2)
END

BEGIN(A0, "LDY #n")
	GET_IMM(y)
	CHECK_NZ(y)
	CYCLES(2)
END

BEGIN(B0, "BCS r")
	GET_IMM(value)
	if (c)
		pc += (byte)value;
	CYCLES(2)
END

BEGIN(C0, "CPY #n")
	GET_IMM(value)
	tmp = y - value;
	CHECK_CMP
	CYCLES(2)
END

BEGIN(D0, "BNE r")
	GET_IMM(value)
	if (!z)
		pc += (byte)value;
	CYCLES(2)
END

BEGIN(E0, "CPX #n")
	GET_IMM(value)
	tmp = x - value;
	CHECK_CMP
	CYCLES(2)
END

BEGIN(F0, "BEQ r")
	GET_IMM(value)
	if (z)
		pc += (byte)value;
	CYCLES(2)
END


//////////////////////////////////////////////////////////////////////////////
// *1
//////////////////////////////////////////////////////////////////////////////

BEGIN(01, "ORA (n,X)")
	GET_IND_X(address)
	a |= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(6)
END

BEGIN(11, "ORA (n),Y")
	GET_IND_Y(address)
	a |= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(5)
END

BEGIN(21, "AND (n,X)")
	GET_IND_X(address)
	a &= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(6)
END

BEGIN(31, "AND (n),Y")
	GET_IND_Y(address)
	a &= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(5)
END

BEGIN(41, "EOR (n,X)")
	GET_IND_X(address)
	a ^= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(6)
END

BEGIN(51, "EOR (n),Y")
	GET_IND_Y(address)
	a ^= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(5)
END

BEGIN(61, "ADC (n,X)")
	GET_IND_X(address)
	value = MEM_READ(address);
	DO_ADC
	CHECK_NZ(a)
	CYCLES(6)
END

BEGIN(71, "ADC (n),Y")
	GET_IND_Y(address)
	value = MEM_READ(address);
	DO_ADC
	CHECK_NZ(a)
	CYCLES(5)
END

BEGIN(81, "STA (n,X)")
	GET_IND_X(address)
	MEM_WRITE(address, a)
	CYCLES(6)
END

BEGIN(91, "STA (n),Y")
	GET_IND_Y(address)
	MEM_WRITE(address, a)
	CYCLES(5)
END

BEGIN(A1, "LDA (n,X)")
	GET_IND_X(address)
	a = MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(6)
END

BEGIN(B1, "LDA (n),Y")
	GET_IND_Y(address)
	a = MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(5)
END

BEGIN(C1, "CMP (n,X)")
	GET_IND_X(address)
	tmp = a - MEM_READ(address);
	CHECK_CMP
	CYCLES(6)
END

BEGIN(D1, "CMP (n),Y")
	GET_IND_Y(address)
	tmp = a - MEM_READ(address);
	CHECK_CMP
	CYCLES(5)
END

BEGIN(E1, "SBC (n,X)")
	GET_IND_X(address)
	value = MEM_READ(address);
	DO_SBC
	CHECK_NZ(a)
	CYCLES(6)
END

BEGIN(F1, "SBC (n),Y")
	GET_IND_Y(address)
	value = MEM_READ(address);
	DO_SBC
	CHECK_NZ(a)
	CYCLES(5)
END


//////////////////////////////////////////////////////////////////////////////
// *2
//////////////////////////////////////////////////////////////////////////////

BEGIN(A2, "LDX #n")
	GET_IMM(x)
	CHECK_NZ(x)
	CYCLES(2)
END


//////////////////////////////////////////////////////////////////////////////
// *3
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// *4
//////////////////////////////////////////////////////////////////////////////

BEGIN(24, "BIT n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	DO_BIT(value)
	CYCLES(3)
END

BEGIN(84, "STY n")
	GET_ZPAGE(address)
	MEM_WRITE0(address, y)
	CYCLES(3)
END

BEGIN(94, "STY n,X")
	GET_ZPAGE_X(address)
	MEM_WRITE0(address, y)
	CYCLES(4)
END

BEGIN(A4, "LDY n")
	GET_ZPAGE(address)
	y = MEM_READ0(address);
	CHECK_NZ(y)
	CYCLES(3)
END

BEGIN(B4, "LDY n,X")
	GET_ZPAGE_X(address)
	y = MEM_READ0(address);
	CHECK_NZ(y)
	CYCLES(4)
END

BEGIN(C4, "CPY n")
	GET_ZPAGE(address)
	tmp = y - MEM_READ0(address);
	CHECK_CMP
	CYCLES(3)
END

BEGIN(E4, "CPX n")
	GET_ZPAGE(address)
	tmp = x - MEM_READ0(address);
	CHECK_CMP
	CYCLES(3)
END


//////////////////////////////////////////////////////////////////////////////
// *5
//////////////////////////////////////////////////////////////////////////////

BEGIN(05, "ORA n")
	GET_ZPAGE(address)
	a |= MEM_READ0(address);
	CHECK_NZ(a)
	CYCLES(3)
END

BEGIN(15, "ORA n,X")
	GET_ZPAGE_X(address)
	a |= MEM_READ0(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(25, "AND n")
	GET_ZPAGE(address)
	a &= MEM_READ0(address);
	CHECK_NZ(a)
	CYCLES(3)
END

BEGIN(35, "AND n,X")
	GET_ZPAGE_X(address)
	a &= MEM_READ0(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(45, "EOR n")
	GET_ZPAGE(address)
	a ^= MEM_READ0(address);
	CHECK_NZ(a)
	CYCLES(3)
END

BEGIN(55, "EOR n,X")
	GET_ZPAGE_X(address)
	a ^= MEM_READ0(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(65, "ADC n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	DO_ADC
	CHECK_NZ(a)
	CYCLES(3)
END

BEGIN(75, "ADC n,X")
	GET_ZPAGE_X(address)
	value = MEM_READ0(address);
	DO_ADC
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(85, "STA n")
	GET_ZPAGE(address)
	MEM_WRITE0(address, a)
	CYCLES(3)
END

BEGIN(95, "STA n,X")
	GET_ZPAGE_X(address)
	MEM_WRITE0(address, a)
	CYCLES(4)
END

BEGIN(A5, "LDA n")
	GET_ZPAGE(address)
	a = MEM_READ0(address);
	CHECK_NZ(a)
	CYCLES(3)
END

BEGIN(B5, "LDA n,X")
	GET_ZPAGE_X(address)
	a = MEM_READ0(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(C5, "CMP n")
	GET_ZPAGE(address)
	tmp = a - MEM_READ0(address);
	CHECK_CMP
	CYCLES(3)
END

BEGIN(D5, "CMP n,X")
	GET_ZPAGE_X(address)
	tmp = a - MEM_READ0(address);
	CHECK_CMP
	CYCLES(4)
END

BEGIN(E5, "SBC n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	DO_SBC
	CHECK_NZ(a)
	CYCLES(3)
END

BEGIN(F5, "SBC n,X")
	GET_ZPAGE_X(address)
	value = MEM_READ0(address);
	DO_SBC
	CHECK_NZ(a)
	CYCLES(4)
END


//////////////////////////////////////////////////////////////////////////////
// *6
//////////////////////////////////////////////////////////////////////////////

BEGIN(06, "ASL n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	DO_ASL(value)
	MEM_WRITE0(address, value)
	CYCLES(5)
END

BEGIN(16, "ASL n,X")
	GET_ZPAGE_X(address)
	value = MEM_READ0(address);
	DO_ASL(value)
	MEM_WRITE0(address, value)
	CYCLES(6)
END

BEGIN(26, "ROL n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	DO_ROL(value)
	MEM_WRITE0(address, value)
	CYCLES(5)
END

BEGIN(36, "ROL n,X")
	GET_ZPAGE_X(address)
	value = MEM_READ0(address);
	DO_ROL(value)
	MEM_WRITE0(address, value)
	CYCLES(6)
END

BEGIN(46, "LSR n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	DO_LSR(value)
	MEM_WRITE0(address, value)
	CYCLES(5)
END

BEGIN(56, "LSR n,X")
	GET_ZPAGE_X(address)
	value = MEM_READ0(address);
	DO_LSR(value)
	MEM_WRITE0(address, value)
	CYCLES(6)
END

BEGIN(66, "ROR n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	DO_ROR(value)
	MEM_WRITE0(address, value)
	CYCLES(5)
END

BEGIN(76, "ROR n,X")
	GET_ZPAGE_X(address)
	value = MEM_READ0(address);
	DO_ROR(value)
	MEM_WRITE0(address, value)
	CYCLES(6)
END

BEGIN(86, "STX n")
	GET_ZPAGE(address)
	MEM_WRITE0(address, x)
	CYCLES(3)
END

BEGIN(96, "STX n,Y")
	GET_ZPAGE_Y(address)
	MEM_WRITE0(address, x)
	CYCLES(4)
END

BEGIN(A6, "LDX n")
	GET_ZPAGE(address)
	x = MEM_READ0(address);
	CHECK_NZ(x)
	CYCLES(3)
END

BEGIN(B6, "LDX n,Y")
	GET_ZPAGE_Y(address)
	x = MEM_READ0(address);
	CHECK_NZ(x)
	CYCLES(4)
END

BEGIN(C6, "DEC n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	value--;
	value = value & 0xFF;
	CHECK_NZ(value)
	MEM_WRITE0(address, value)
	CYCLES(5)
END

BEGIN(D6, "DEC n,X")
	GET_ZPAGE_X(address)
	value = MEM_READ0(address);
	value--;
	value = value & 0xFF;
	CHECK_NZ(value)
	MEM_WRITE0(address, value)
	CYCLES(6)
END

BEGIN(E6, "INC n")
	GET_ZPAGE(address)
	value = MEM_READ0(address);
	value++;
	value = value & 0xFF;
	CHECK_NZ(value)
	MEM_WRITE0(address, value)
	CYCLES(5)
END

BEGIN(F6, "INC n,X")
	GET_ZPAGE_X(address)
	value = MEM_READ0(address);
	value++;
	value = value & 0xFF;
	CHECK_NZ(value)
	MEM_WRITE0(address, value)
	CYCLES(6)
END


//////////////////////////////////////////////////////////////////////////////
// *7
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// *8
//////////////////////////////////////////////////////////////////////////////

BEGIN(08, "PHP")
	STACK_WRITE0((byte)getP())
	s--;
	s = s & 0xFF;
	CYCLES(3)
END

BEGIN(18, "CLC")
	c = false;
	CYCLES(2)
END

BEGIN(28, "PLP")
	s++;
	s = s & 0xFF;
	value = STACK_READ0;
	putP(value);
	CYCLES(4)
END

BEGIN(38, "SEC")
	c = true;
	CYCLES(2)
END

BEGIN(48, "PHA")
	STACK_WRITE0(a)
	s--;
	s = s & 0xFF;
	CYCLES(3)
END

BEGIN(58, "CLI")
	i = false;
	CYCLES(2)
END

BEGIN(68, "PLA")
	s++;
	s = s & 0xFF;
	a = STACK_READ0;
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(78, "SEI")
	i = true;
	CYCLES(2)
END

BEGIN(88, "DEY")
	y--;
	y = y & 0xFF;
	CHECK_NZ(y)
	CYCLES(2)
END

BEGIN(98, "TYA")
	a = y;
	CHECK_NZ(a)
	CYCLES(2)
END

BEGIN(A8, "TAY")
	y = a;
	CHECK_NZ(y)
	CYCLES(2)
END

BEGIN(B8, "CLV")
	v = false;
	CYCLES(2)
END

BEGIN(C8, "INY")
	y++;
	y = y & 0xFF;
	CHECK_NZ(y)
	CYCLES(2)
END

BEGIN(D8, "CLD")
	d = false;
	CYCLES(2)
END

BEGIN(E8, "INX")
	x++;
	x = x & 0xFF;
	CHECK_NZ(x)
	CYCLES(2)
END

BEGIN(F8, "SED")
	UNSUPPORTED
	//d = true;
	//CYCLES(2)
END


//////////////////////////////////////////////////////////////////////////////
// *9
//////////////////////////////////////////////////////////////////////////////

BEGIN(09, "ORA #n")
	GET_IMM(value)
	a |= value;
	CHECK_NZ(a)
	CYCLES(2)
END

BEGIN(19, "ORA n:n,Y")
	GET_ABS_Y(address)
	a |= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(29, "AND #n")
	GET_IMM(value)
	a &= value;
	CHECK_NZ(a)
	CYCLES(2)
END

BEGIN(39, "AND n:n,Y")
	GET_ABS_Y(address)
	a &= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(49, "EOR #n")
	GET_IMM(value)
	a ^= value;
	CHECK_NZ(a)
	CYCLES(2)
END

BEGIN(59, "EOR n:n,Y")
	GET_ABS_Y(address)
	a ^= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(69, "ADC #n")
	GET_IMM(value)
	DO_ADC
	CHECK_NZ(a)
	CYCLES(2)
END

BEGIN(79, "ADC n:n,Y")
	GET_ABS_Y(address)
	value = MEM_READ(address);
	DO_ADC
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(99, "STA n:n,Y")
	GET_ABS_Y(address)
	MEM_WRITE(address, a)
	CYCLES(5)
END

BEGIN(A9, "LDA #n")
	GET_IMM(a)
	CHECK_NZ(a)
	CYCLES(2)
END

BEGIN(B9, "LDA n:n,Y")
	GET_ABS_Y(address)
	a = MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(C9, "CMP #n")
	GET_IMM(value)
	tmp = a - value;
	CHECK_CMP
	CYCLES(2)
END

BEGIN(D9, "CMP n:n,Y")
	GET_ABS_Y(address)
	tmp = a - MEM_READ(address);
	CHECK_CMP
	CYCLES(4)
END

BEGIN(E9, "SBC #n")
	GET_IMM(value)
	DO_SBC
	CHECK_NZ(a)
	CYCLES(2)
END

BEGIN(F9, "SBC n:n,Y")
	GET_ABS_Y(address)
	value = MEM_READ(address);
	DO_SBC
	CHECK_NZ(a)
	CYCLES(4)
END


//////////////////////////////////////////////////////////////////////////////
// *A
//////////////////////////////////////////////////////////////////////////////

BEGIN(0A, "ASL")
	DO_ASL(a)
	CYCLES(2)
END

BEGIN(2A, "ROL")
	DO_ROL(a)
	CYCLES(2)
END

BEGIN(4A, "LSR")
	DO_LSR(a)
	CYCLES(2)
END

BEGIN(6A, "ROR")
	DO_ROR(a)
	CYCLES(2)
END

BEGIN(8A, "TXA")
	a = x;
	CHECK_NZ(a)
	CYCLES(2)
END

BEGIN(9A, "TXS")
	s = x;
	CHECK_NZ(s)
	CYCLES(2)
END

BEGIN(AA, "TAX")
	x = a;
	CHECK_NZ(x)
	CYCLES(2)
END

BEGIN(BA, "TSX")
	x = s;
	CHECK_NZ(x)
	CYCLES(2)
END

BEGIN(CA, "DEX")
	x--;
	x = x & 0xFF;
	CHECK_NZ(x)
	CYCLES(2)
END

BEGIN(EA, "NOP")
	CYCLES(2)
END


//////////////////////////////////////////////////////////////////////////////
// *B
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// *C
//////////////////////////////////////////////////////////////////////////////

BEGIN(2C, "BIT n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	DO_BIT(value)
	CYCLES(4)
END

BEGIN(4C, "JMP n:n")
	GET_ABS(address)
	pc = address;
	CYCLES(3)
END

BEGIN(6C, "JMP (n:n)")
	GET_ABS(address)
	pc = ((MEM_READ(address) & 0xFF) | (MEM_READ(address+1) << 8));
	CYCLES(5)
END

BEGIN(8C, "STY n:n")
	GET_ABS(address)
	MEM_WRITE(address, y)
	CYCLES(4)
END

BEGIN(AC, "LDY n:n")
	GET_ABS(address)
	y = MEM_READ(address);
	CHECK_NZ(y)
	CYCLES(4)
END

BEGIN(BC, "LDY n:n,X")
	GET_ABS_X(address)
	y = MEM_READ(address);
	CHECK_NZ(y)
	CYCLES(4)
END

BEGIN(CC, "CPY n:n")
	GET_ABS(address)
	tmp = y - MEM_READ(address);
	CHECK_CMP
	CYCLES(4)
END

BEGIN(EC, "CPX n:n")
	GET_ABS(address)
	tmp = x - MEM_READ(address);
	CHECK_CMP
	CYCLES(4)
END


//////////////////////////////////////////////////////////////////////////////
// *D
//////////////////////////////////////////////////////////////////////////////

BEGIN(0D, "ORA n:n")
	GET_ABS(address)
	a |= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(1D, "ORA n:n,X")
	GET_ABS_X(address)
	a |= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(2D, "AND n:n")
	GET_ABS(address)
	a &= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(3D, "AND n:n,X")
	GET_ABS_X(address)
	a &= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(4D, "EOR n:n")
	GET_ABS(address)
	a ^= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(5D, "EOR n:n,X")
	GET_ABS_X(address)
	a ^= MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(6D, "ADC n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	DO_ADC
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(7D, "ADC n:n,X")
	GET_ABS_X(address)
	value = MEM_READ(address);
	DO_ADC
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(8D, "STA n:n")
	GET_ABS(address)
	MEM_WRITE(address, a)
	CYCLES(4)
END

BEGIN(9D, "STA n:n,X")
	GET_ABS_X(address)
	MEM_WRITE(address, a)
	CYCLES(5)
END

BEGIN(AD, "LDA n:n")
	GET_ABS(address)
	a = MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(BD, "LDA n:n,X")
	GET_ABS_X(address)
	a = MEM_READ(address);
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(CD, "CMP n:n")
	GET_ABS(address)
	tmp = a - MEM_READ(address);
	CHECK_CMP
	CYCLES(4)
END

BEGIN(DD, "CMP n:n,X")
	GET_ABS_X(address)
	tmp = a - MEM_READ(address);
	CHECK_CMP
	CYCLES(4)
END

BEGIN(ED, "SBC n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	DO_SBC
	CHECK_NZ(a)
	CYCLES(4)
END

BEGIN(FD, "SBC n:n,X")
	GET_ABS_X(address)
	value = MEM_READ(address);
	DO_SBC
	CHECK_NZ(a)
	CYCLES(4)
END

//////////////////////////////////////////////////////////////////////////////
// *E
//////////////////////////////////////////////////////////////////////////////

BEGIN(0E, "ASL n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	DO_ASL(value)
	MEM_WRITE(address, value)
	CYCLES(6)
END

BEGIN(1E, "ASL n:n,X")
	GET_ABS_X(address)
	value = MEM_READ(address);
	DO_ASL(value)
	MEM_WRITE(address, value)
	CYCLES(7)
END

BEGIN(2E, "ROL n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	DO_ROL(value)
	MEM_WRITE(address, value)
	CYCLES(6)
END

BEGIN(3E, "ROL n:n,X")
	GET_ABS_X(address)
	value = MEM_READ(address);
	DO_ROL(value)
	MEM_WRITE(address, value)
	CYCLES(7)
END

BEGIN(4E, "LSR n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	DO_LSR(value)
	MEM_WRITE(address, value)
	CYCLES(6)
END

BEGIN(5E, "LSR n:n,X")
	GET_ABS_X(address)
	value = MEM_READ(address);
	DO_LSR(value)
	MEM_WRITE(address, value)
	CYCLES(7)
END

BEGIN(6E, "ROR n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	DO_ROR(value)
	MEM_WRITE(address, value)
	CYCLES(6)
END

BEGIN(7E, "ROR n:n,X")
	GET_ABS_X(address)
	value = MEM_READ(address);
	DO_ROR(value)
	MEM_WRITE(address, value)
	CYCLES(7)
END

BEGIN(8E, "STX n:n")
	GET_ABS(address)
	MEM_WRITE(address, x)
	CYCLES(6)
END

BEGIN(AE, "LDX n:n")
	GET_ABS(address)
	x = MEM_READ(address);
	CHECK_NZ(x)
	CYCLES(6)
END

BEGIN(BE, "LDX n:n,Y")
	GET_ABS_Y(address)
	x = MEM_READ(address);
	CHECK_NZ(x)
	CYCLES(7)
END

BEGIN(CE, "DEC n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	value--;
	value = value & 0xFF;
	CHECK_NZ(value)
	MEM_WRITE(address, value)
	CYCLES(6)
END

BEGIN(DE, "DEC n:n,X")
	GET_ABS_X(address)
	value = MEM_READ(address);
	value--;
	value = value & 0xFF;
	CHECK_NZ(value)
	MEM_WRITE(address, value)
	CYCLES(7)
END

BEGIN(EE, "INC n:n")
	GET_ABS(address)
	value = MEM_READ(address);
	value++;
	value = value & 0xFF;
	CHECK_NZ(value)
	MEM_WRITE(address, value)
	CYCLES(6)
END

BEGIN(FE, "INC n:n,X")
	GET_ABS_X(address)
	value = MEM_READ(address);
	value++;
	value = value & 0xFF;
	CHECK_NZ(value)
	MEM_WRITE(address, value)
	CYCLES(7)
END


//////////////////////////////////////////////////////////////////////////////
// *F
//////////////////////////////////////////////////////////////////////////////

