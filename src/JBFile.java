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
 * A JBFile is a 12 bytes header optionally followed by some pages of code
 * and/or by some pages of data; a page is a block of 256 bytes.
 * 
 * JBFiles rely on the container (e.g. the Jar file) for compression.
 * Unused space in pages should be 0-filled to increase the compression ratio.
 */
public interface JBFile {
	
	int OFFSET_HEADER = 0;
	int SIZE_HEADER = 12;

	int OFFSET_SIGNATURE = 0;
	int SIZE_SIGNATURE = 4;
	
	byte SIGNATURE_0 = 'J';
	byte SIGNATURE_1 = 'B';
	byte SIGNATURE_2 = 'i';
	byte SIGNATURE_3 = 't';
	
	int OFFSET_HEADERLENGTH = 4;
	int SIZE_HEADERLENGTH = 2;

	byte HEADERLENGTH_0_HIGH = 0;
	byte HEADERLENGTH_1_LOW = SIZE_HEADER;	

	int OFFSET_VERSION = 6;
	int SIZE_VERSION = 2;

	byte VERSION_0_MAJOR = 1;
	byte VERSION_1_MINOR = 0;	

	int OFFSET_CODEPAGES = 8;
	int SIZE_CODEPAGES = 1;
	
	int OFFSET_DATAPAGES = 9;
	int SIZE_DATAPAGES = 1;

}
