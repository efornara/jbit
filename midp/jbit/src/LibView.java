// #condition !JBIT_RUNTIME 

/*
	JBit - A 6502 framework for mobile phones
	Copyright (C) 2007-2016  Emanuele Fornara
	
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

import javax.microedition.lcdui.Canvas;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Font;
import javax.microedition.lcdui.Graphics;


public abstract class LibView extends Canvas {

	protected static final int AM_INV = 0;
	protected static final int AM_IMP = 1; // TODO split IMP/ACC
	protected static final int AM_REL = 2;
	protected static final int AM_ABS = 3;
	protected static final int AM_IMM = 4;
	protected static final int AM_IND_X = 5;
	protected static final int AM_IND_Y = 6;
	protected static final int AM_ZPG = 7;
	protected static final int AM_ZPG_X = 8;
	protected static final int AM_ZPG_Y = 9;
	protected static final int AM_ABS_Y = 10;
	protected static final int AM_IND = 11;
	protected static final int AM_ABS_X = 12;
	// {{{ optable !
protected static final String opcAddressingModes[] = {
 /*  0 */ "",
 /*  1 */ "",
 /*  2 */ "r",
 /*  3 */ "n:n",
 /*  4 */ "#n",
 /*  5 */ "(n,X)",
 /*  6 */ "(n),Y",
 /*  7 */ "n",
 /*  8 */ "n,X",
 /*  9 */ "n,Y",
 /* 10 */ "n:n,Y",
 /* 11 */ "(n:n)",
 /* 12 */ "n:n,X",
};
protected static final String opcMnemonics[] = {
 /*  0 */ "???",
 /*  1 */ "BRK",
 /*  2 */ "BPL",
 /*  3 */ "JSR",
 /*  4 */ "BMI",
 /*  5 */ "RTI",
 /*  6 */ "BVC",
 /*  7 */ "RTS",
 /*  8 */ "BVS",
 /*  9 */ "BCC",
 /* 10 */ "LDY",
 /* 11 */ "BCS",
 /* 12 */ "CPY",
 /* 13 */ "BNE",
 /* 14 */ "CPX",
 /* 15 */ "BEQ",
 /* 16 */ "ORA",
 /* 17 */ "AND",
 /* 18 */ "EOR",
 /* 19 */ "ADC",
 /* 20 */ "STA",
 /* 21 */ "LDA",
 /* 22 */ "CMP",
 /* 23 */ "SBC",
 /* 24 */ "LDX",
 /* 25 */ "BIT",
 /* 26 */ "STY",
 /* 27 */ "ASL",
 /* 28 */ "ROL",
 /* 29 */ "LSR",
 /* 30 */ "ROR",
 /* 31 */ "STX",
 /* 32 */ "DEC",
 /* 33 */ "INC",
 /* 34 */ "PHP",
 /* 35 */ "CLC",
 /* 36 */ "PLP",
 /* 37 */ "SEC",
 /* 38 */ "PHA",
 /* 39 */ "CLI",
 /* 40 */ "PLA",
 /* 41 */ "SEI",
 /* 42 */ "DEY",
 /* 43 */ "TYA",
 /* 44 */ "TAY",
 /* 45 */ "CLV",
 /* 46 */ "INY",
 /* 47 */ "CLD",
 /* 48 */ "INX",
 /* 49 */ "SED",
 /* 50 */ "TXA",
 /* 51 */ "TXS",
 /* 52 */ "TAX",
 /* 53 */ "TSX",
 /* 54 */ "DEX",
 /* 55 */ "NOP",
 /* 56 */ "JMP",
};
protected static final int opcDefinitionTable[] = {
 /*   0 */  257, // BRK 
 /*   1 */ 1360, // ORA (n,X)
 /*   2 */    0, 
 /*   3 */    0, 
 /*   4 */    0, 
 /*   5 */ 1872, // ORA n
 /*   6 */ 1883, // ASL n
 /*   7 */    0, 
 /*   8 */  290, // PHP 
 /*   9 */ 1104, // ORA #n
 /*  10 */  283, // ASL 
 /*  11 */    0, 
 /*  12 */    0, 
 /*  13 */  912, // ORA n:n
 /*  14 */  923, // ASL n:n
 /*  15 */    0, 
 /*  16 */  578, // BPL r
 /*  17 */ 1616, // ORA (n),Y
 /*  18 */    0, 
 /*  19 */    0, 
 /*  20 */    0, 
 /*  21 */ 2128, // ORA n,X
 /*  22 */ 2139, // ASL n,X
 /*  23 */    0, 
 /*  24 */  291, // CLC 
 /*  25 */ 2704, // ORA n:n,Y
 /*  26 */    0, 
 /*  27 */    0, 
 /*  28 */    0, 
 /*  29 */ 3216, // ORA n:n,X
 /*  30 */ 3227, // ASL n:n,X
 /*  31 */    0, 
 /*  32 */  899, // JSR n:n
 /*  33 */ 1361, // AND (n,X)
 /*  34 */    0, 
 /*  35 */    0, 
 /*  36 */ 1881, // BIT n
 /*  37 */ 1873, // AND n
 /*  38 */ 1884, // ROL n
 /*  39 */    0, 
 /*  40 */  292, // PLP 
 /*  41 */ 1105, // AND #n
 /*  42 */  284, // ROL 
 /*  43 */    0, 
 /*  44 */  921, // BIT n:n
 /*  45 */  913, // AND n:n
 /*  46 */  924, // ROL n:n
 /*  47 */    0, 
 /*  48 */  580, // BMI r
 /*  49 */ 1617, // AND (n),Y
 /*  50 */    0, 
 /*  51 */    0, 
 /*  52 */    0, 
 /*  53 */ 2129, // AND n,X
 /*  54 */ 2140, // ROL n,X
 /*  55 */    0, 
 /*  56 */  293, // SEC 
 /*  57 */ 2705, // AND n:n,Y
 /*  58 */    0, 
 /*  59 */    0, 
 /*  60 */    0, 
 /*  61 */ 3217, // AND n:n,X
 /*  62 */ 3228, // ROL n:n,X
 /*  63 */    0, 
 /*  64 */  261, // RTI 
 /*  65 */ 1362, // EOR (n,X)
 /*  66 */    0, 
 /*  67 */    0, 
 /*  68 */    0, 
 /*  69 */ 1874, // EOR n
 /*  70 */ 1885, // LSR n
 /*  71 */    0, 
 /*  72 */  294, // PHA 
 /*  73 */ 1106, // EOR #n
 /*  74 */  285, // LSR 
 /*  75 */    0, 
 /*  76 */  952, // JMP n:n
 /*  77 */  914, // EOR n:n
 /*  78 */  925, // LSR n:n
 /*  79 */    0, 
 /*  80 */  582, // BVC r
 /*  81 */ 1618, // EOR (n),Y
 /*  82 */    0, 
 /*  83 */    0, 
 /*  84 */    0, 
 /*  85 */ 2130, // EOR n,X
 /*  86 */ 2141, // LSR n,X
 /*  87 */    0, 
 /*  88 */  295, // CLI 
 /*  89 */ 2706, // EOR n:n,Y
 /*  90 */    0, 
 /*  91 */    0, 
 /*  92 */    0, 
 /*  93 */ 3218, // EOR n:n,X
 /*  94 */ 3229, // LSR n:n,X
 /*  95 */    0, 
 /*  96 */  263, // RTS 
 /*  97 */ 1363, // ADC (n,X)
 /*  98 */    0, 
 /*  99 */    0, 
 /* 100 */    0, 
 /* 101 */ 1875, // ADC n
 /* 102 */ 1886, // ROR n
 /* 103 */    0, 
 /* 104 */  296, // PLA 
 /* 105 */ 1107, // ADC #n
 /* 106 */  286, // ROR 
 /* 107 */    0, 
 /* 108 */ 3000, // JMP (n:n)
 /* 109 */  915, // ADC n:n
 /* 110 */  926, // ROR n:n
 /* 111 */    0, 
 /* 112 */  584, // BVS r
 /* 113 */ 1619, // ADC (n),Y
 /* 114 */    0, 
 /* 115 */    0, 
 /* 116 */    0, 
 /* 117 */ 2131, // ADC n,X
 /* 118 */ 2142, // ROR n,X
 /* 119 */    0, 
 /* 120 */  297, // SEI 
 /* 121 */ 2707, // ADC n:n,Y
 /* 122 */    0, 
 /* 123 */    0, 
 /* 124 */    0, 
 /* 125 */ 3219, // ADC n:n,X
 /* 126 */ 3230, // ROR n:n,X
 /* 127 */    0, 
 /* 128 */    0, 
 /* 129 */ 1364, // STA (n,X)
 /* 130 */    0, 
 /* 131 */    0, 
 /* 132 */ 1882, // STY n
 /* 133 */ 1876, // STA n
 /* 134 */ 1887, // STX n
 /* 135 */    0, 
 /* 136 */  298, // DEY 
 /* 137 */    0, 
 /* 138 */  306, // TXA 
 /* 139 */    0, 
 /* 140 */  922, // STY n:n
 /* 141 */  916, // STA n:n
 /* 142 */  927, // STX n:n
 /* 143 */    0, 
 /* 144 */  585, // BCC r
 /* 145 */ 1620, // STA (n),Y
 /* 146 */    0, 
 /* 147 */    0, 
 /* 148 */ 2138, // STY n,X
 /* 149 */ 2132, // STA n,X
 /* 150 */ 2399, // STX n,Y
 /* 151 */    0, 
 /* 152 */  299, // TYA 
 /* 153 */ 2708, // STA n:n,Y
 /* 154 */  307, // TXS 
 /* 155 */    0, 
 /* 156 */    0, 
 /* 157 */ 3220, // STA n:n,X
 /* 158 */    0, 
 /* 159 */    0, 
 /* 160 */ 1098, // LDY #n
 /* 161 */ 1365, // LDA (n,X)
 /* 162 */ 1112, // LDX #n
 /* 163 */    0, 
 /* 164 */ 1866, // LDY n
 /* 165 */ 1877, // LDA n
 /* 166 */ 1880, // LDX n
 /* 167 */    0, 
 /* 168 */  300, // TAY 
 /* 169 */ 1109, // LDA #n
 /* 170 */  308, // TAX 
 /* 171 */    0, 
 /* 172 */  906, // LDY n:n
 /* 173 */  917, // LDA n:n
 /* 174 */  920, // LDX n:n
 /* 175 */    0, 
 /* 176 */  587, // BCS r
 /* 177 */ 1621, // LDA (n),Y
 /* 178 */    0, 
 /* 179 */    0, 
 /* 180 */ 2122, // LDY n,X
 /* 181 */ 2133, // LDA n,X
 /* 182 */ 2392, // LDX n,Y
 /* 183 */    0, 
 /* 184 */  301, // CLV 
 /* 185 */ 2709, // LDA n:n,Y
 /* 186 */  309, // TSX 
 /* 187 */    0, 
 /* 188 */ 3210, // LDY n:n,X
 /* 189 */ 3221, // LDA n:n,X
 /* 190 */ 2712, // LDX n:n,Y
 /* 191 */    0, 
 /* 192 */ 1100, // CPY #n
 /* 193 */ 1366, // CMP (n,X)
 /* 194 */    0, 
 /* 195 */    0, 
 /* 196 */ 1868, // CPY n
 /* 197 */ 1878, // CMP n
 /* 198 */ 1888, // DEC n
 /* 199 */    0, 
 /* 200 */  302, // INY 
 /* 201 */ 1110, // CMP #n
 /* 202 */  310, // DEX 
 /* 203 */    0, 
 /* 204 */  908, // CPY n:n
 /* 205 */  918, // CMP n:n
 /* 206 */  928, // DEC n:n
 /* 207 */    0, 
 /* 208 */  589, // BNE r
 /* 209 */ 1622, // CMP (n),Y
 /* 210 */    0, 
 /* 211 */    0, 
 /* 212 */    0, 
 /* 213 */ 2134, // CMP n,X
 /* 214 */ 2144, // DEC n,X
 /* 215 */    0, 
 /* 216 */  303, // CLD 
 /* 217 */ 2710, // CMP n:n,Y
 /* 218 */    0, 
 /* 219 */    0, 
 /* 220 */    0, 
 /* 221 */ 3222, // CMP n:n,X
 /* 222 */ 3232, // DEC n:n,X
 /* 223 */    0, 
 /* 224 */ 1102, // CPX #n
 /* 225 */ 1367, // SBC (n,X)
 /* 226 */    0, 
 /* 227 */    0, 
 /* 228 */ 1870, // CPX n
 /* 229 */ 1879, // SBC n
 /* 230 */ 1889, // INC n
 /* 231 */    0, 
 /* 232 */  304, // INX 
 /* 233 */ 1111, // SBC #n
 /* 234 */  311, // NOP 
 /* 235 */    0, 
 /* 236 */  910, // CPX n:n
 /* 237 */  919, // SBC n:n
 /* 238 */  929, // INC n:n
 /* 239 */    0, 
 /* 240 */  591, // BEQ r
 /* 241 */ 1623, // SBC (n),Y
 /* 242 */    0, 
 /* 243 */    0, 
 /* 244 */    0, 
 /* 245 */ 2135, // SBC n,X
 /* 246 */ 2145, // INC n,X
 /* 247 */    0, 
 /* 248 */  305, // SED 
 /* 249 */ 2711, // SBC n:n,Y
 /* 250 */    0, 
 /* 251 */    0, 
 /* 252 */    0, 
 /* 253 */ 3223, // SBC n:n,X
 /* 254 */ 3233, // INC n:n,X
 /* 255 */    0, 
};
	// }}}

	/*
	 * definition is: 00000000000000000000AAAASSMMMMMM
	 * where:
	 *   MMMMMM is the mnemonic (0 if invalid opcode)
	 *   SS is the size-1 (0 if invalid opcode)
	 *   AAAA is the addressing mode (0 if invalid opcode)
	 */
		
	protected int opcGetDefinition(int opcode) {
		return opcDefinitionTable[opcode & 0xFF];
	}
	
	protected String opcGetMnemonic(int definition) {
		return opcMnemonics[definition & 63];
	}
	
	protected int opcGetSize(int opcode) {
		return ((opcGetDefinition(opcode) >> 6) & 3) + 1;
	}
	
	protected int opcGetAddressingMode(int definition) {
		return definition >> 8;
	}
	
	protected String opcGetAddressingModeTag(int definition) {
		return opcAddressingModes[definition >> 8];
	}
	
	protected boolean opcIsAddressingModeAbsolute(int addressingMode) {
		switch (addressingMode) {
		case AM_ABS:
		case AM_ABS_X:
		case AM_ABS_Y:
		case AM_IND:
			return true;
		default:
			return false;
		}
	}

	protected String opcByteToDecString(int v) {
		return Integer.toString(v);
	}
	
	protected String opcIntToPagString(int v) {
		return Integer.toString((v >> 8) & 0xFF) + ":" + Integer.toString(v & 0xFF);
	}

	protected String opcGetTemplate(int opcode) {
		int definition = opcGetDefinition(opcode);
		return opcGetMnemonic(definition) + " "
				+ opcGetAddressingModeTag(definition);
	}
	
	protected String opcDisassembly(int pc, int m0, int m1, int m2) {
		int definition = opcGetDefinition(m0);
		String mnemonic = opcGetMnemonic(definition);
		String addressMode = opcGetAddressingModeTag(definition);
		if (addressMode.equals("")) {
			return mnemonic;
		} else if (addressMode.equals("r")) {
			if (m1 != 0)
				return mnemonic + " " + opcIntToPagString((pc + 2 + (byte)m1) & 0xFFFF);
			else
				return mnemonic + " !!!!";
		} else {
			int i;
			
			if ((i = addressMode.indexOf("n:n")) != -1) {
				int m = (m2 << 8) | m1;
				return mnemonic + " "
					+ addressMode.substring(0, i)
					+ opcIntToPagString(m)
					+ addressMode.substring(i + 3);
			} else if ((i = addressMode.indexOf("n")) != -1) {
				return mnemonic + " "
					+ addressMode.substring(0, i)
					+ opcByteToDecString(m1)
					+ addressMode.substring(i + 1);
			} else
				return "???"; // NOT REACHED
		}
	}

	protected boolean opcCharMatches(char source, char key) {
		switch (Character.toUpperCase(key)) {
		case '2':
			return source == 'A' || source == 'B' || source == 'C';
		case '3':
			return source == 'D' || source == 'E' || source == 'F';
		case '4':
			return source == 'G' || source == 'H' || source == 'I';
		case '5':
			return source == 'J' || source == 'K' || source == 'L';
		case '6':
			return source == 'M' || source == 'N' || source == 'O';
		case '7':
			return source == 'P' || source == 'Q' || source == 'R'
				|| source == 'S';
		case '8':
			return source == 'T' || source == 'U' || source == 'V';
		case '9':
			return source == 'W' || source == 'X' || source == 'Y'
				|| source == 'Z';
		default:
			return source == key;
		}
	}
	
	protected boolean opcMatches(byte opcode, char k1, char k2, char k3) {
		String d = opcGetMnemonic(opcGetDefinition(opcode));
		if (!opcCharMatches(d.charAt(0), k1))
			return false;
		if (!opcCharMatches(d.charAt(1), k2))
			return false;
		if (!opcCharMatches(d.charAt(2), k3))
			return false;
		return true;
	}

	// TODO split type and color 
	protected static final int FMT_NORMAL = 0xFFFFFF;
	protected static final int FMT_ACTIVE = 0x909090;
	protected static final int FMT_EDIT = 0x9090FF;
	
	protected static final int FMT_J_LEFT = 1;
	protected static final int FMT_J_RIGHT = 2;

	protected Module jbit;
	protected Display display;
	protected boolean isColor;
	protected Font fmtFont;
	protected int fmtSpace;
	protected int fmtCellSpace = 3; // TODO compute CellSpace
	protected int fmtByteWidth;
	//protected int fmtMnemonicWidth;

	protected Graphics fmtG;
	protected int fmtXL;
	protected int fmtXR;
	protected int fmtY;
	protected int fmtJ;

	protected void fmtInitByteWidth() {
		int n, max = 0;
		char digit = '0';
		for (char c = '0'; c <= '9'; c++)
			if ((n = fmtFont.charWidth(c)) > max) {
				max = n;
				digit = c;
			}
		fmtByteWidth = fmtFont.stringWidth(new String(
				new char[] { '2', digit, digit}));
	}
	
	/*
	protected void fmtInitMnemonicWidth() {
		fmtMnemonicWidth = fmtFont.stringWidth("STA");
	}
	*/

	protected void viewInit(Module jbit) {
		this.jbit = jbit;
		touch = (Module)jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, TouchSvc.TAG);
		display = (Display)jbit.opO(JBitSvc.OP_GET_DISPLAY, 0, null);
		// #if MICROEMULATOR
		fmtFont = Font.getFont(Font.FACE_MONOSPACE, Font.STYLE_BOLD,
				Font.SIZE_MEDIUM);
		// #else
//@		fmtFont = Font.getFont(Font.FACE_MONOSPACE, Font.STYLE_BOLD,
//@				Font.SIZE_SMALL);
		// #endif
		isColor = display.isColor();
		fmtInitByteWidth();
		//fmtInitMnemonicWidth();
		fmtSpace = fmtFont.charWidth('W') >> 1;
	}
	

	protected void fmtNewLine() {
		fmtJ = FMT_J_LEFT;
		fmtXL = 0;
		fmtXR = getWidth();
		fmtY += fmtFont.getHeight();
	}
	
	protected void fmtBegin(Graphics g) {
		g.setFont(fmtFont);
		fmtG = g;
		fmtNewLine();
		fmtY = 0;
	}
	
	protected void fmtEnd() {
		fmtG = null;
	}
	
	protected int fmtPutChar(char c) {
		int width = fmtFont.charWidth(c);
		if (fmtJ == FMT_J_LEFT) {
			fmtG.drawChar(c, fmtXL, fmtY, Graphics.TOP|Graphics.LEFT);
			fmtXL += width;
		} else {
			fmtG.drawChar(c, fmtXR, fmtY, Graphics.TOP|Graphics.RIGHT);
			fmtXR -= width;
		}
		return width;
	}
	
	protected int fmtPutString(String s) {
		int width = fmtFont.stringWidth(s);
		if (fmtJ == FMT_J_LEFT) {
			fmtG.drawString(s, fmtXL, fmtY, Graphics.TOP|Graphics.LEFT);
			fmtXL += width;
		} else {
			fmtG.drawString(s, fmtXR, fmtY, Graphics.TOP|Graphics.RIGHT);
			fmtXR -= width;
		}
		return width;
	}
	
	protected void fmtPutDecoration(int decoration, int width) {
		int x;
		if (fmtJ == FMT_J_LEFT)
			x = fmtXL;
		else
			x = fmtXR - width;
		if (isColor) {
			int color = decoration;
			if (color != FMT_NORMAL) {
				int last = fmtG.getColor();
				fmtG.setColor(color);
				fmtG.fillRect(x, fmtY, width, fmtFont.getHeight());
				fmtG.setColor(last);
			}
		} else {
			switch (decoration) {
			case FMT_ACTIVE:
				fmtG.setStrokeStyle(Graphics.DOTTED);
				break;
			case FMT_EDIT:
				fmtG.setStrokeStyle(Graphics.SOLID);
				break;
			default:
				return;
			}
			fmtG.drawRect(x - 1, fmtY - 1, width, fmtFont.getHeight());
			fmtG.setStrokeStyle(Graphics.SOLID);
		}
	}
	
	protected int fmtPutPagedAddress(int address) {
		return fmtPutString((address >> 8) + ":" + (address & 0xFF));
	}

	// left justified assumed
	protected int fmtPutByte(int value) {
		fmtG.drawString(Integer.toString(value & 0xFF),
				fmtXL + fmtByteWidth, fmtY, Graphics.TOP|Graphics.RIGHT);
		fmtXL += fmtByteWidth;
		return fmtByteWidth;
	}
	
	// left justified assumed
	protected int fmtPutCell(String value) {
		fmtG.drawString(value, fmtXL, fmtY, Graphics.TOP|Graphics.LEFT);
		fmtXL += fmtByteWidth;
		return fmtByteWidth;
	}	

	public int getGameAction(int keyCode) {
		try {
			return super.getGameAction(keyCode);
		} catch (Throwable e) {
			return 0;
		}
	}

	protected int getJBitGameAction(int keyCode) {
		switch (keyCode) {
		case '2':
			return UP;
		case '4':
			return LEFT;
		case '5':
			return FIRE;
		case '6':
			return RIGHT;
		case '8':
			return DOWN;
		case TouchSvc.SOFT_KEY_BREAK:
			return 0;
		default:
			return getGameAction(keyCode);
		}
	}
	
	protected Integer touchContext = null;
	private Module touch;
	private Object[] touchArgs = new Object[3];
	
	protected void touchDraw(boolean bold, Graphics g) {
		if (touch == null)
			return;
		touchArgs[0] = touchContext;
		touchArgs[1] = this;
		touchArgs[2] = g;
		try {
			touch.opI(TouchSvc.OP_DRAW, bold ? -1 : 0, touchArgs);
		} finally {
			touchArgs[2] = null;
		}
	}
	
	protected void pointerPressed(int x, int y) {
		if (touch == null)
			return;
		touchArgs[0] = touchContext;
		touchArgs[1] = this;
		touchArgs[2] = null;
		int keyCode = touch.opI(TouchSvc.OP_POINTER_PRESSED,
				((y & 0xFFFF) << 16) | (x & 0xFFFF), touchArgs);
		if (keyCode != 0)
			keyPressed(keyCode);
		else
			repaint();
	}
}
