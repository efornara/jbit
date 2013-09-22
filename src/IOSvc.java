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
 * Reserved opId range: 4xx
 */
public interface IOSvc {

	String TAG = "IO";

	int ALIGN_ACTIVE = 0;
	int ALIGN_FAILED = -1;	
	int ALIGN_USER_BREAK = -2;
	int ALIGN_SUSPEND_CPU = -3;
	int ALIGN_RESUME_CPU = -4;
	
	int VM_STATUS_RUNNING = 0;
	int VM_STATUS_MONITOR = -1;

	/**
	 * <b>void opI(0, null)</b>
	 * 
	 * <p>Reset the IO chip.
	 */
	int OP_RESET = 401;


	/**
	 * <b>void opI(0, null)</b>
	 * 
	 * <p>Flush the display.
	 */
	int OP_FLUSH = 402;

	/**
	 * <b>ioStatus opI(vmStatus, null)</b>
	 * 
	 * <p>Called after every get/put and periodically.
	 */
	int OP_ALIGN = 403;

	/**
	 * <b>retCode opI(0, null)</b>
	 * 
	 * <p>Called after each batch of CPU instructions have been executed.
	 * retCode is taken as a hint on how long the VM thread should wait.
	 * If retCode is < 0, no hint is provided.
	 * If retCode is 0, no waiting is necessary (the IO has already performed a
	 * Thread.wait() or equivalent).
	 * If retCode is > 0, this is the amount of waiting suggested (in ms).
	 */
	int OP_DO_SOME_WORK = 404;


	/**
	 * <b>void opI(0, module)</b>
	 * 
	 * <p>Set the module providing the address space (used for DMA).
	 */
	int OP_SET_ADDRESS_SPACE = 405;

}
