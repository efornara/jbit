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

#include "embd.h"

#include <string.h>

// mostly a search & replace and type optimization starting from LibView.java

#define AM_INV 0
#define AM_IMP 1 // TODO split IMP/ACC
#define AM_REL 2
#define AM_ABS 3
#define AM_IMM 4
#define AM_IND_X 5
#define AM_IND_Y 6
#define AM_ZPG 7
#define AM_ZPG_X 8
#define AM_ZPG_Y 9
#define AM_ABS_Y 10
#define AM_IND 11
#define AM_ABS_X 12

// {{{ optable !
const char *const opcAddressingModes[] PROGMEM = {
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
const char *const opcMnemonics[] PROGMEM = {
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
const uint16_t opcDefinitionTable[] PROGMEM = {
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

uint16_t opcGetDefinition(uint8_t opcode) {
	return pgm_read_word(&(opcDefinitionTable[opcode]));
}

const char *opcGetMnemonic(uint16_t definition) {
	return (const char *)pgm_read_word(&(opcMnemonics[definition & 63]));
}

uint8_t opcGetSize(uint8_t opcode) {
	return ((opcGetDefinition(opcode) >> 6) & 3) + 1;
}

uint8_t opcGetAddressingMode(uint16_t definition) {
	return definition >> 8;
}

const char *opcGetAddressingModeTag(uint16_t definition) {
	return (const char *)pgm_read_word(&(opcAddressingModes[definition >> 8]));
}

uint8_t opcIsAddressingModeAbsolute(uint8_t addressingMode) {
	switch (addressingMode) {
	case AM_ABS:
	case AM_ABS_X:
	case AM_ABS_Y:
	case AM_IND:
		return 1;
	default:
		return 0;
	}
}

void opcByteToDecString(char *s, uint8_t v) {
	sprintf(s, "%d", v);
}

void opcIntToPagString(char *s, uint16_t v) {
	sprintf(s, "%d:%d", v >> 8, v & 0xFF);
}

void opcGetTemplate(char *s, uint8_t opcode) {
	uint16_t definition = opcGetDefinition(opcode);
	sprintf(s, "%s %s",
	  opcGetMnemonic(definition), opcGetAddressingModeTag(definition));
}

void opcDisassembly(char *s, uint16_t pc, uint8_t m0, uint8_t m1, uint8_t m2) {
	uint16_t definition = opcGetDefinition(m0);
	const char *mnemonic = opcGetMnemonic(definition);
	const char *addressMode = opcGetAddressingModeTag(definition);
	if (strlen(addressMode) == 0) {
		strcpy(s, mnemonic);
	} else if (!strcmp(addressMode, "r")) {
		if (m1 != 0) {
			char buf[8];
			opcIntToPagString(buf, (pc + 2 + (int8_t)m1) & 0xFFFF);
			sprintf(s, "%s %s", mnemonic, buf);
		} else {
			sprintf(s, "%s !!!!", mnemonic);
		}
	} else {
		const char *p;
		
		if ((p = strstr(addressMode, "n:n")) != NULL) {
			char buf[8];
			uint16_t m = (m2 << 8) | m1;
			int i = p - addressMode;
			strcpy(s, mnemonic);
			strcat(s, " ");
			memcpy(buf, addressMode, i);
			buf[i] = '\0';
			strcat(s, buf);
			opcIntToPagString(buf, m);
			strcat(s, buf);
			strcat(s, &addressMode[i + 3]);
		} else if ((p = strstr(addressMode, "n")) != NULL) {
			char buf[8];
			int i = p - addressMode;
			strcpy(s, mnemonic);
			strcat(s, " ");
			memcpy(buf, addressMode, i);
			buf[i] = '\0';
			strcat(s, buf);
			opcByteToDecString(buf, m1);
			strcat(s, buf);
			strcat(s, &addressMode[i + 1]);
		} else {
			strcpy(s, "???"); // NOT REACHED
		}
	}
}

