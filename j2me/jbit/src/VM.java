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

import java.util.Vector;

import javax.microedition.lcdui.Canvas;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Font;
import javax.microedition.lcdui.Graphics;


public class VM extends Canvas implements Runnable, CommandListener, Module
{
	
	private static final int WAITLENGTH = 20;
	private static final int BATCHSIZE = 20000;
	
	private static final int READY = 1;
	private static final int RUNNING = 2;
	private static final int DEBUGGING = 3;
	private static final int HALTED = 4;
	private static final int TERMINATED = 5;
	private static final int ERROR = 6;
	
	private static final int VM = 1;
	private static final int IO = 2;
	private static final int MONITOR = 3;
	
	private static final byte PAGE_EMPTY = 1;
	private static final byte PAGE_FULL = 2;
	private static final byte PAGE_PROGRAM = 3;

	private static final int bgColor = 0xFFFFFF;
	private static final int fgColor = 0x000000;

	private Command continueCmd = new Command("Continue", Command.SCREEN, 1);
	private Command abortCmd = new Command("Abort", Command.SCREEN, 2);
	private Command endCmd = new Command("End", Command.SCREEN, 2);
	private Command videoCmd = new Command("Video", Command.SCREEN, 3);
	// #if !JBIT_RUNTIME
	private Command debugCmd = new Command("Debug", Command.SCREEN, 4);
	// #endif

	private int status;
	private int displayOwner;
	private Thread thread;
	private String msg;
	
	// #if !JBIT_RUNTIME
	private Module jbit;
	private int startMode;
	private Module monitor;
	private Module cpu;
	private boolean hasBrkPts;
	// #else
//@	private JBit jbit;
//@	private CPU cpu;
	// #endif

	private Module io;

	private Display display;

	private byte[][] memory;
	private byte[] page0;
	private byte[] stack;
	private byte[] pageType;

	public VM() {
		addCommand(endCmd);
		addCommand(videoCmd);
		setCommandListener(this);
		displayOwner = VM;
	}

	// #if !JBIT_RUNTIME
	public int init(Module jbit) {
		this.jbit = jbit;
		display = (Display)jbit.opO(JBitSvc.OP_GET_DISPLAY, 0, null);
		return 0;
	}
	// #else
//@	public int init(JBit jbit) {
//@		this.jbit = jbit;
//@		display = Display.getDisplay(jbit);
//@		return 0;
//@	}
	// #endif

	private Module loadModuleByClassName(String name) {
		try {
			Class cls = Class.forName(name);
			return (Module)cls.newInstance();
		} catch (Throwable e) {
			return null;
		}
	}

	public int activate() {
		// get the program file
		// #if !JBIT_RUNTIME
		byte[][] file = (byte[][])jbit.opO(JBitSvc.OP_POOL_GET, 0,
				JBitSvc.POOL_ID_PROGRAM_FILE);
		// #else
//@		byte[][] file = jbit.file;
		// #endif
		if (file == null)
			return -1;
		int codePages = (file[0][JBFile.OFFSET_CODEPAGES] & 0xFF);
		int dataPages = (file[0][JBFile.OFFSET_DATAPAGES] & 0xFF);
		int pages = codePages + dataPages;

		// setup memory
		pageType = new byte[256];
		memory = new byte[256][];
		page0 = memory[0] = new byte[256];
		stack = memory[1] = new byte[256];
		pageType[0] = PAGE_FULL;
		pageType[1] = PAGE_FULL;
		int i;
		for (i = 3; i < pages + 3; i++) {
			memory[i] = file[i - 2];
			if (memory[i] == null)
				pageType[i] = PAGE_EMPTY;
			else
				pageType[i] = PAGE_PROGRAM;
		}
		for (; i < 256; i++)
			pageType[i] = PAGE_EMPTY;

		// setup CPU
		// #if !JBIT_RUNTIME
		if ((cpu = (Module)jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, CPUSvc.TAG)) == null)
			return -1;
		cpu.opI(Module.OP_INIT, 0, jbit);
		cpu.opI(CPUSvc.OP_SET_ADDRESS_SPACE, 0, this);
		cpu.opI(CPUSvc.OP_RESET, 0, null);
		// #else
//@		cpu = new CPU();
//@		cpu.m = this;
//@		cpu.reset();
		// #endif
		
		// setup IO
		if ((io = (Module)loadModuleByClassName("IO")) == null) {
			deactivate();
			return -1;
		}
		io.opI(Module.OP_INIT, 0, jbit);
		io.opI(IOSvc.OP_SET_ADDRESS_SPACE, 0, this);
		io.opI(Module.OP_ACTIVATE, 0, null);
		io.opI(IOSvc.OP_RESET, 0, null);

		// setup Monitor
		displayOwner = IO;
		// #if !JBIT_RUNTIME
		monitor = (Module)jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, MonitorSvc.TAG);
		if (monitor != null) {
			monitor.opI(MonitorSvc.OP_SET_CPU, 0, cpu);
			monitor.opI(MonitorSvc.OP_SET_ADDRESS_SPACE, 0, this);
		}
		if (startMode == VMSvc.START_MODE_DEBUG) {
			if (monitor.opI(OP_ACTIVATE, 0, null) == 0)
				displayOwner = MONITOR;
		}
		// #endif
		
		// start the VM
		msg = "";
		status = READY;
		thread = new Thread(this);
		thread.start();
		int[] msg = new int[Const.MSG_SIZE];
		msg[0] = displayOwner == MONITOR ? Const.MSG_VM_DEBUG : Const.MSG_VM_GO;
		enqueMsg(msg);

		return 0;
	}
	
	private void releaseResources() {
		// cleanup VM
		displayOwner = VM;
		msg = null;

		// cleanup Monitor
		// #if !JBIT_RUNTIME
		monitor = null;
		// #endif
		
		// cleanup IO
		if (io != null)
			io.opI(Module.OP_DEACTIVATE, 0, null);
		io = null;
		
		// cleanup CPU
		cpu = null;
		
		// release memory
		memory = null;
		page0 = null;
		stack = null;
		pageType = null;
	}

	public int deactivate() {

		// stop the VM
		if (thread != null) {
			int[] msg = new int[Const.MSG_SIZE];
			msg[0] = Const.MSG_VM_QUIT;
			enqueMsg(msg);
			thread = null;
		} else {
			status = TERMINATED;
			releaseResources();
		}
		return 0;
	}

	private void updateDisplayOwner(int displayOwner) {
		this.displayOwner = displayOwner;
		if (displayOwner == VM) {
			if (status == READY || status == DEBUGGING) {
				addCommand(continueCmd);
				addCommand(abortCmd);
				removeCommand(endCmd);
			} else {
				removeCommand(continueCmd);
				removeCommand(abortCmd);
				addCommand(endCmd);
			}
			// #if !JBIT_RUNTIME
			if (status == READY && monitor != null) {
				addCommand(debugCmd);
			} else {
				removeCommand(debugCmd);
			}
			// #endif
			repaint();
		} else if (displayOwner == IO) {
			io.opI(IOSvc.OP_FLUSH, 0, null);
		}
		display.setCurrent((Displayable)getDisplayable());
	}

	protected void paint(Graphics g) {
		g.setColor(bgColor);
		g.fillRect(0, 0, getWidth(), getHeight());
		if (msg != null) {
			Font font = Font.getDefaultFont();
			g.setColor(fgColor);
			int nl, y = 2, start = 0;
			while ((nl = msg.indexOf('\n', start)) != -1) {
				g.drawSubstring(msg, start, nl - start, 2, y, 0);
				y += font.getHeight();
				start = nl + 1;
			}
			if (start < msg.length())
				g.drawSubstring(msg, start, msg.length() - start, 2, y, 0);
		}
	}

	private String addressToString(int address) {
		return ((address >> 8) & 0xFF) + ":" + (address & 0xFF);
	}
	
	private void alignIO(Module io) {
		int vmStatus;
		if (status == RUNNING)
			vmStatus = IOSvc.VM_STATUS_RUNNING;
		else
			vmStatus = IOSvc.VM_STATUS_MONITOR;
		int ioStatus = io.opI(IOSvc.OP_ALIGN, vmStatus, null); 
		if (ioStatus == IOSvc.ALIGN_USER_BREAK) {
			if (status == RUNNING) {
				status = READY;
				msg = "PAUSED\n";
			}
			if (status == DEBUGGING)
				updateDisplayOwner(MONITOR);
			else
				updateDisplayOwner(VM);
			ioStatus = io.opI(IOSvc.OP_ALIGN, IOSvc.VM_STATUS_MONITOR, null);
		}
		if (ioStatus == IOSvc.ALIGN_SUSPEND_CPU) {
			// #if !JBIT_RUNTIME
			int cpuStatus = cpu.opI(CPUSvc.OP_GET_STATUS, 0, null);
			if (cpuStatus == CPUSvc.OK)
				cpu.opI(CPUSvc.OP_SET_STATUS, CPUSvc.WAIT, null);
			// #else
//@			if (cpu.status == CPUSvc.OK)
//@				cpu.status = CPUSvc.WAIT;
			// #endif
		} else if (ioStatus == IOSvc.ALIGN_RESUME_CPU) {
			// #if !JBIT_RUNTIME
			int cpuStatus = cpu.opI(CPUSvc.OP_GET_STATUS, 0, null);
			if (cpuStatus == CPUSvc.WAIT) {
				cpu.opI(CPUSvc.OP_SET_STATUS, CPUSvc.OK, null);
				/*
				if (status == DEBUGGING)
					monitor.opI(MonitorSvc.OP_REFRESH, 0, null);
				*/
			}
			// #else
//@			if (cpu.status == CPUSvc.WAIT)
//@				cpu.status = CPUSvc.OK;
			// #endif
		}
	}

	public Object getDisplayable() {
			switch (displayOwner) {
			case IO:
				return io.opO(Module.OP_GET_DISPLAYABLE, 0, null);
			// #if !JBIT_RUNTIME
			case MONITOR:
				return monitor.opO(OP_GET_DISPLAYABLE, 0, null);
			// #endif
			default:
				return this;
			}
	}

	// #if !JBIT_RUNTIME

	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "JBit VM";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_SUPPORT);
		case METADATA_ID_SERVICES:
			return new String[] { VMSvc.TAG };
		default:
			return null;
		}
	}
	
	public Object opO(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_GET_METADATA:
			return getMetadata(iArg);
		case OP_GET_DISPLAYABLE:
			return getDisplayable();
		}
		return null;
	}

	public int opI(int opId, int iArg, Object oArg) {
		int msgType = 0;

		switch (opId) {
		case OP_INIT:
			return init((Module)oArg);
		case OP_ACTIVATE:
			return activate();
		case OP_DEACTIVATE:
			return deactivate();
		case VMSvc.OP_CONTINUE:
			msgType = Const.MSG_USER_CONTINUE;
			updateDisplayOwner(IO);
			break;
		case VMSvc.OP_STEP:
			msgType = Const.MSG_VM_STEP;
			break;
		case VMSvc.OP_ABORT:
			msgType = Const.MSG_USER_ABORT;
			jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
			break;
		case VMSvc.OP_VIDEO:
			updateDisplayOwner(IO);
			break;
		case VMSvc.OP_SET_START_MODE:
			startMode = iArg;
		default:
			return - 1;
		}
		if (msgType != 0) {
			int[] msg = new int[Const.MSG_SIZE];
			msg[0] = msgType;
			if (msgType == Const.MSG_VM_STEP)
				msg[1] = iArg;
			enqueMsg(msg);
		}
		return 0;
	}

	// #else
//@
//@	// If JBIT_RUNTIME is defined, VM is just an "Address Space" (for IO DMA).
//@	// opO and opI should not be called.
//@
//@	public Object opO(int opId, int iArg, Object oArg) {
//@		return null;
//@	}
//@
//@	public int opI(int opId, int iArg, Object oArg) {
//@		return 0;
//@	}
//@
	// #endif

	public int get(int address) {
		int page = address >> 8;
		int offset = address & 0xFF;
		switch (page) {
		case 0:
			return page0[offset] & 0xFF;
		case 1:
			return stack[offset] & 0xFF;
		case 2:
			int result = io.get(offset);
			alignIO(io);
			return result;
		}
		// #if INTERNAL_CHECK
//@		if (page > 255)
//@			throw new RuntimeException("VM.get(address) is invalid");
		// #endif
		byte[] p = memory[page];
		switch (pageType[page]) {
		case PAGE_FULL:
		case PAGE_PROGRAM:
			return p[offset] & 0xFF;
		default: // PAGE_EMPTY
			return 0;
		}
	}

	public int put(int address, int value) {
		int page = address >> 8;
		int offset = address & 0xFF;
		switch (page) {
		case 0:
			page0[offset] = (byte)value;
			return 0;
		case 1:
			stack[offset] = (byte)value;
			return 0;
		case 2:
			int result = io.put(offset, value);
			alignIO(io);
			return result;
		}
		byte[] p = memory[page];
		switch (pageType[page]) {
		case PAGE_PROGRAM:
		case PAGE_EMPTY:
			// new page
			byte[] ram = new byte[256];
			if (p != null)
				System.arraycopy(p, 0, ram, 0, p.length);
			p = memory[page] = ram;
			pageType[page] = PAGE_FULL;
		case PAGE_FULL:
			p[offset] = (byte)value;
			return 0;
		default:
			// #if INTERNAL_CHECK
//@			throw new RuntimeException("VM.put() found an invalid pageType");
			// #else
			return -1;
			// #endif
		}
	}
	
	private void checkStatus(int cpuStatus, int prevStatus) {
		if (cpuStatus != CPUSvc.OK && cpuStatus != CPUSvc.WAIT) {
			// #if !JBIT_RUNTIME
			int pc = cpu.get(CPUSvc.ADDRESS_PC);
			// #else
//@			int pc = cpu.pc;
			// #endif
			if (cpuStatus == CPUSvc.HALT) {
				io.opI(IOSvc.OP_FLUSH, 0, null);
				// #if JBIT_RUNTIME && !MICROEMULATOR
//@				jbit.notifyDestroyed();
//@				return;
				// #else
				status = HALTED;
				msg = "HALTED\n" +
				addressToString(pc) + "\n";
				// #endif
			} else {
				int opcode = get(pc) & 0xFF;
				status = ERROR;
				msg = (cpuStatus == CPUSvc.INVALID_OPCODE ?
						"INV.OP.\n" : "UNSUPP.OP.\n") +
				addressToString(pc) + "\n" +
				"Opcode "+ opcode + "\n";
			}
			updateDisplayOwner(VM);
		}
	}
	
	private int batch(int n, int prevStatus) {
		int i = 0, cpuStatus;
		// #if !JBIT_RUNTIME
		while (((cpuStatus = cpu.opI(CPUSvc.OP_GET_STATUS, 0, null))
				== CPUSvc.OK) && status == prevStatus && i < n) {
			cpu.opI(CPUSvc.OP_STEP, 0, null);
			i++;
			if (hasBrkPts)
				if (monitor.opI(MonitorSvc.OP_CHECK_BRKPT, 0, null) != 0) {
					if (monitor.opI(OP_ACTIVATE, 0, null) == 0) {
						status = DEBUGGING;
						updateDisplayOwner(MONITOR);
					}
					return i;
				}
		}
		// #else
//@		while (((cpuStatus = cpu.status)
//@				== CPUSvc.OK) && status == prevStatus && i < n) {
//@			cpu.step();
//@			i++;
//@		}
		// #endif
		checkStatus(cpuStatus, prevStatus);
		return i;
	}

	public void run() {
		int pendingSteps = 0;
		while (true) {
			int[] msg = dequeMsg();
			if (msg != null) {
				int msgType = msg[0]; 
				switch (msgType) {
				case Const.MSG_USER_CONTINUE:
					pendingSteps = 0;
				case Const.MSG_VM_GO:
					if (status == READY || status == DEBUGGING)
						status = RUNNING;
					// #if !JBIT_RUNTIME
					hasBrkPts = monitor != null && monitor.opI(MonitorSvc.OP_HAS_BRKPT, 0, null) != 0;
					// #endif
					break;
				case Const.MSG_USER_DEBUG:
				case Const.MSG_USER_ABORT: // another status?
				case Const.MSG_VM_DEBUG:
				case Const.MSG_VM_STOP:
					if (status == RUNNING)
						status = READY;
					if (status == READY
							&& (msgType == Const.MSG_USER_DEBUG
							|| msgType == Const.MSG_VM_DEBUG))
						status = DEBUGGING;
					break;
				case Const.MSG_VM_STEP:
					if (pendingSteps == 0)
						pendingSteps = msg[1];
					break;
				case Const.MSG_VM_QUIT:
					status = TERMINATED;
					releaseResources();
					return;
				}
			}
			// #if !JBIT_RUNTIME
			if (status == DEBUGGING && pendingSteps != 0) {
				pendingSteps -= batch(pendingSteps, DEBUGGING);
				if (status == DEBUGGING && pendingSteps == 0)
					monitor.opI(MonitorSvc.OP_REFRESH, 0, null);
			}
			// #endif
			if (status == RUNNING) {
				batch(BATCHSIZE, RUNNING);
			}
			int wait = io.opI(IOSvc.OP_DO_SOME_WORK, 0, null);
			// #if !JBIT_RUNTIME
			if (status == DEBUGGING &&
					pendingSteps == 0 &&
					cpu.opI(CPUSvc.OP_GET_STATUS, 0, null) == CPUSvc.WAIT) {
				alignIO(io);
				if (cpu.opI(CPUSvc.OP_GET_STATUS, 0, null) == CPUSvc.OK)
					monitor.opI(MonitorSvc.OP_REFRESH, 0, null);
			} else
				// #endif
				alignIO(io);
			if (wait < 0)
				wait = WAITLENGTH;
			if (wait > 100)
				wait = 100;
			if (wait != 0) {
				try {
					Thread.sleep(wait);
				} catch (InterruptedException e) {
				}
			}
		}
	}

	public void commandAction(Command c, Displayable d) {
		int msgType = 0;
		if (c == continueCmd) {
			msgType = Const.MSG_USER_CONTINUE;
			updateDisplayOwner(IO);
		} else if (c == abortCmd || c == endCmd) {
			msgType = Const.MSG_USER_ABORT;
			// #if !JBIT_RUNTIME
			jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
			// #else
//@			jbit.notifyDestroyed();
			// #endif
		} else if (c == videoCmd) {
			updateDisplayOwner(IO);
		// #if !JBIT_RUNTIME
		} else if (c == debugCmd) {
			if (monitor.opI(OP_ACTIVATE, 0, null) == 0) {
				msgType = Const.MSG_USER_DEBUG;
				updateDisplayOwner(MONITOR);
			}
		// #endif
		}
		if (msgType != 0) {
			int[] msg = new int[Const.MSG_SIZE];
			msg[0] = msgType;
			enqueMsg(msg);
		}
	}
	
	private Vector msgQueue = new Vector();
	
	public synchronized void enqueMsg(int[] msg) {
		msgQueue.addElement(msg);
	}
	
	private synchronized int[] dequeMsg() {
		if (msgQueue.size() == 0)
			return null;
		int[] value = (int[])msgQueue.firstElement();
		msgQueue.removeElementAt(0);
		return value;
	}

}
