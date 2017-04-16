/*
	JBit - A 6502 framework for mobile phones
	Copyright (C) 2007-2017  Emanuele Fornara
	
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

/**
 * Reserved opId range: 3xx
 */
public interface CPUSvc {

	String TAG = "CPU";

	int OK = 0;
	int HALT = 1;
	int WAIT = 2;
	int INVALID_OPCODE = 3;
	int UNSUPPORTED_OPCODE = 4;
	
	int ADDRESS_PCLO = 1;
	int ADDRESS_PCHI = 2;
	int ADDRESS_S = 3;
	int ADDRESS_P = 4;
	int ADDRESS_A = 5;
	int ADDRESS_X = 6;
	int ADDRESS_Y = 7;
	int ADDRESS_PC = 8;

	int COUNTER_CYCLES = 1;
	int COUNTER_INSTRUCTIONS = 2;

	/**
	 * <b>void opI(0, null)</b>
	 * 
	 * <p>Execute one step.
	 */
	int OP_STEP = 301;

	/**
	 * <b>void opI(0, null)</b>
	 * 
	 * <p>Reset the CPU.
	 */
	int OP_RESET = 302;

	/**
	 * <b>void opI(0, module)</b>
	 * 
	 * <p>Set the module providing the address space for the CPU.
	 */
	int OP_SET_ADDRESS_SPACE = 303;

	/**
	 * <b>value opI(0, null)</b>
	 * 
	 * <p>Return the status of the CPU.
	 */
	int OP_GET_STATUS = 304;

	/**
	 * <b>value opI(status, null)</b>
	 * 
	 * <p>Set the status of the CPU.
	 */
	int OP_SET_STATUS = 305;

	/**
	 * <b>Long opO(COUNTER_x, null)</b>
	 * 
	 * <p>Return the value of a counter.
	 */
	int OP_GET_COUNTER = 306;
}
