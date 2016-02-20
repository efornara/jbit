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
 * This is the "Module" interface and its name and the name of its methods
 * should be kept during obfuscation.
 * The class names of implementations of Module should also be kept. 
 * 
 * Reserved opId range: 0xx
 */
public interface Module {
	
	int METADATA_ID_LABEL = 1;
	int METADATA_ID_TYPE = 2;
	int METADATA_ID_SERVICES = 3;
	
	int TYPE_MIDLET = 1;
	int TYPE_TOP_LEVEL = 2;
	int TYPE_SUPPORT = 3;
	int TYPE_VM_PLUGIN = 4;

	/**
	 * <b>Object opO(0, String )</b>
	 * 
	 * <p>returns an Object depending on metadataId or null if the metadataId
	 * is not recognized
	 */
	int OP_GET_METADATA = 1;
	
	/**
	 * TODO implement GET_ERROR
	 */
	int OP_GET_ERROR = 2;
	
	/**
	 * <b>retCode opI(0, Object jbit)</b>
	 * 
	 * <p>A module should become operational. This operation
	 * is called just after the module is instantiated.
	 * <p>Returns 0 on success, < 0 on failure.
	 *
	 * <p>jbit:
	 *   if JBIT_RUNTIME is defined jbit can be cast to JBit
	 *   otherwise jbit should be considered a MIDlet on IO modules
	 *   and a Module providing the JBitSvc service on other modules
	 * <p>retCode:
	 *   0: success; < 0: failed 
	 */
	int OP_INIT = 3;
	
	/**
	 * <b>retCode opI(0, null)</b>
	 * 
	 * <p>An activable module should prepare to take charge of the Display.
	 * If the activation is successful GET_DISPLAYABLE is called.
	 * 
	 * <p>retCode:
	 *   0: success; < 0: failed 
	 */
	int OP_ACTIVATE = 4;

	/**
	 * <b>void opI(0, null)</b>
	 * 
	 * <p>An active module is no more in charge of the Display. 
	 * It should release any references to the other modules. 
	 * 
	 */
	int OP_DEACTIVATE = 5;
	
	/**
	 * <b>Displayable opO(0, null)</b>
	 * 
	 * <p>returns a Displayable currently associated with this module
	 * or null if not Displayable is available.
	 */
	int OP_GET_DISPLAYABLE = 6;
	
	/**
	 * <p>For the future...
	 * 
	 * <p>The suggested protocol for multiple integer in/out parameter
	 * passing is to put an int[PAR_STANDARD_SIZE] in oPar and
	 * the length of the significant part of it in iPar.
	 * The return value can then be a negative number to indicate failure,
	 * or the length of the relevant part of the array.
	 * 
	 * <p>Every caller is advised to keep an int[PAR_STANDARD_SIZE] as
	 * a scratchpad.
	 */
	int OP_PAR_STANDARD_SIZE = 16;

	/**
	 * Do some generic operation on this module.
	 */
	Object opO(int opId, int iArg, Object oArg);

	/**
	 * Do some generic operation on this module (GC friendly version).
	 */
	int opI(int opId, int iArg, Object oArg);
	
	/**
	 * Get one byte from the address space of the module.
	 * 
	 * <p><em>NOTE:</em> The address space might not be a mere
	 * container of data and this operation might cause side effects.
	 * 
	 * @param address The address (>=0)
	 * @return A byte in the 0-255 range on success or a negative number
	 * on failure
	 */
	int get(int address);

	/**
	 * Put one byte in the address space of the module.
	 * 
	 * <p><em>NOTE:</em> The address space might not be a mere
	 * container of data and this operation might cause side effects.
	 * 
	 * @param address The address (>=0)
	 * @param value A byte in the 0-255 range
	 * @return 0 on success or a negative number on failure
	 */
	int put(int address, int value);
}
