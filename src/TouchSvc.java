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

/**
 * Reserved opId range: 7xx
 */
public interface TouchSvc {

	String TAG = "Touch";
	
	int CONTEXT_USER1 = 0;
	int CONTEXT_USER2 = 1;
	int CONTEXT_USER3 = 2;
	int CONTEXT_USER4 = 3;
	
	int CONTEXT_EMULATOR = 4;
	int CONTEXT_NAVIGATION = 5;
	int CONTEXT_EDITING = 6;
	
	int N_OF_CONTEXTS = 8;

	int SOFT_KEY_BREAK = -999999;
	
	/**
	 * <b>int opI((y << 16) | x, Object[] { Integer context, Canvas canvas (,...) } )</b>
	 * 
	 * <p>Handle a pointerPressed event (where 0 <= x, y <= 65535).
	 * Returns the keyCode.
	 */
	int OP_POINTER_PRESSED = 701;

	/**
	 * <b>void opI(bold ? -1 : 0, Object[] { Integer context, Canvas canvas, Graphics graphics (,...) } )</b>
	 * 
	 * <p>Draw labels.
	 */
	int OP_DRAW = 702;

}
