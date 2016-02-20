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
 * Reserved opId range: 2xx
 */
public interface VMSvc {
	
	String TAG = "VM";

	int START_MODE_RUN = 1;
	int START_MODE_DEBUG = 2;

	/**
	 * <b>void opI(startMode, null)</b>
	 */
	int OP_SET_START_MODE = 201;
	
	/**
	 * <b>void opI(0, null)</b>
	 */
	int OP_CONTINUE = 202;
	
	/**
	 * <b>void opI(size, null)</b>
	 */
	int OP_STEP = 203;
	
	/**
	 * <b>void opI(0, null)</b>
	 */
	int OP_ABORT = 204;
	
	/**
	 * <b>void opI(0, null)</b>
	 */
	int OP_VIDEO = 205;

}
