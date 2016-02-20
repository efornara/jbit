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
 * Reserved id range: 1xx
 */
public interface JBitSvc {

	String TAG = "JBit";

	/**
	 * <b>MIDlet opO(0, null)</b>
	 * 
	 * <p>Get the MIDlet.
	 */
	int OP_GET_MIDLET = 101;

	/**
	 * <b>Display opO(0, null)</b>
	 * 
	 * <p>Get the Display.
	 */
	int OP_GET_DISPLAY = 102;

	/**
	 * <b>Module opO(0, String service)</b>
	 * 
	 * <p>Find a module providing the given service.
	 */
	int OP_FIND_SERVICE = 103;

	/**
	 * <b>void opI(0, String service)</b>
	 * 
	 * <p>Deactivate the current active module and replace it with
	 * a module providing the given service.
	 * If service is null, the module to activate is chosen by JBit.
	 */
	int OP_REPLACE_WITH_SERVICE = 104;
	
	/**
	 * <b>void opI(vmStartMode, Module back)</b>
	 * 
	 * <p>Deactivate the current active module and replace it with the VM.
	 * For valid values for vmStartMode see VMSvc.
	 * When the VM terminates the module 'back' is activated.
	 */
	int OP_REPLACE_WITH_VM = 105;

	/**
	 * <b>void opI(0, [] { String id, Object value } )</b>
	 * 
	 * <p>Put a value into the pool (slot: id).
	 * If value is null the slot is emptied.
	 */
	int OP_POOL_PUT = 106;

	/**
	 * <b>Object opO(0, String id)</b>
	 * 
	 * <p>Get a value from to the pool (slot: id).
	 * Returns null if the slot is empty.
	 */
	int OP_POOL_GET = 107;
	
	/**
	 * <b>int opI(0, byte[][] file)</b>
	 *
	 * <p>Helper function.
	 * Put a new program into PROGRAM_FILE, increasing PROGRAM_ID and
	 * setting PROGRAM_MODIFIED to false.
	 * Returns the program Id. 
	 */
	int OP_NEW_PROGRAM = 108;
	
	/**
	 * <b>String opO(0, InputStream is)</b>
	 * 
	 * <p>Helper function.
	 * Load a JB file and put it into PROGRAM_FILE, increasing PROGRAM_ID and
	 * setting PROGRAM_MODIFIED to false.
	 * Returns null on success and an error message on failure.
	 */
	int OP_LOAD_PROGRAM = 109;
	
	/**
	 * <p>(java.lang.Integer) The Program Identity (> 0).
	 * 
	 * <p>Increases every time a program is created/loaded.
	 */
	String POOL_ID_PROGRAM_ID = "PI";
	
	/**
	 * <p>(java.lang.Boolean) The program has been modified since
	 * the last Load/Save.
	 */
	String POOL_ID_PROGRAM_MODIFIED = "PM";
	
	/**
	 * <p>(byte[][]) The rappresentation in memory of a JB file.
	 * 
	 * <ul>
	 * <li>byte[0][] is the header of the file
	 * <li>byte[1...n] is a page of code or data; single pages might be null
	 * (means that the page is filled with 0s).
	 * </ul>
	 */
	String POOL_ID_PROGRAM_FILE = "PF";
}
