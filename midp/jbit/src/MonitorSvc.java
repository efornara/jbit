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

/**
 * Reserved opId range: 5xx
 */
public interface MonitorSvc {

	String TAG = "Monitor";

	/**
	 * <b>void opI(0, module)</b>
	 * 
	 * <p>Set the module providing the CPU.
	 */
	int OP_SET_CPU = 501;

	/**
	 * <b>void opI(0, module)</b>
	 * 
	 * <p>Set the module providing the address space for the CPU.
	 */
	int OP_SET_ADDRESS_SPACE = 502;

	/**
	 * <b>void opI(0, null)</b>
	 * 
	 * <p>Update the display (called from the VM thread).
	 */
	int OP_REFRESH = 503;

	/**
	 * <b>void opI(address, null)</b>
	 * 
	 * <p>Set breakpoint at given address.
	 */
	int OP_SET_BRKPT = 504;

	/**
	 * <b>void opI(address, null)</b>
	 * 
	 * <p>Has breakpoints? Returns 0 if not.
	 */
	int OP_HAS_BRKPT = 505;

	/**
	 * <b>void opI(address, null)</b>
	 * 
	 * <p>Has a breakpoint been reached? Returns 0 if not.
	 */
	int OP_CHECK_BRKPT = 506;
}
