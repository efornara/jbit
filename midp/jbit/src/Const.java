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

import javax.microedition.lcdui.ChoiceGroup;


public interface Const {
	
	// #if JavaPlatform == "MIDP/2.0"
	int CHOICE_GROUP_POPUP = ChoiceGroup.POPUP;
	// #else
//@	int CHOICE_GROUP_POPUP = ChoiceGroup.EXCLUSIVE;
	// #endif
	
	// For BlackBerry (no #)
	int AUX_MODE_KEY = '@';

	int MSG_SIZE = 4;
	
	int MSG_VM_GO = 1;
	int MSG_VM_DEBUG = 2;
	int MSG_VM_STOP = 3;
	int MSG_VM_STEP = 4;
	int MSG_VM_QUIT = 5;
	int MSG_VM_TERMINATED = 6;

	int MSG_USER_BREAK = 10;
	int MSG_USER_CONTINUE = 11;
	int MSG_USER_ABORT = 12;
	int MSG_USER_DEBUG = 13;

	String STORE_NAME = "JBit";
	
	char RECORD_TYPE_PROGRAM = 'P';
	char RECORD_TYPE_DATA = 'D';

}
