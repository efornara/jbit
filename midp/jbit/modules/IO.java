/*
	JBit - A 6502 framework for mobile phones
	Copyright (C) 2007-2013  Emanuele Fornara
	
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

import javax.microedition.lcdui.*;


public final class IO extends Canvas implements Module {

	private static final long MAXRAND = 0xFFFFFFFFFFFFL;

	private long rndSeed0;
	private long rndSeed1;
	private int rndN;
	private long rndDivisor;
	
	private void randomReset() {	
		rndSeed0 = System.currentTimeMillis() & MAXRAND;
		rndSeed1 = 0;
		doRandomPut(255);
	}

	private long rndNext() {
		rndSeed0 = (rndSeed0 * 0x5DEECE66DL + 0xBL) & MAXRAND;
		return rndSeed0;
	}
	
	private int doRandomGet() {
		long n;
		while (rndN <= (n = rndNext() / rndDivisor))
			;
		return (int)n;
	}
	
	private void doRandomPut(int max) {
		if (max == 0) {
			long t = rndSeed0;
			rndSeed0 = rndSeed1;
			rndSeed1 = t;
		} else {
			rndN = max + 1;
			rndDivisor = MAXRAND / rndN;
		}
	}

	private byte[] keyBuf = new byte[KEYBUF_SIZE];

	private synchronized void keyBufReset() {
		for (int i = 0; i < KEYBUF_SIZE; i++)
			keyBuf[i] = 0;
	}

	private synchronized void doKeyBufPut() {
		for (int i = 0; i < 7; i++)
			keyBuf[i] = keyBuf[i + 1];
		keyBuf[7] = 0;
	}

	private int doKeyBufGet(int address) {
		return keyBuf[address - REG_KEYBUF] & 0xFF;
	}

	private synchronized void enqueKeyCode(int keyCode) {
		for (int i = 0; i < KEYBUF_SIZE; i++)
			if (keyBuf[i] == 0) {
				keyBuf[i] = (byte)keyCode;
				return;
			}
	}
	
	protected void keyPressed(int keyCode) {
		if (vmStatus == IOSvc.VM_STATUS_MONITOR || keyCode < 0)
			signalUserBreak = true;
		else
			enqueKeyCode(keyCode);
	}

	private byte[] video = new byte[CONVIDEO_SIZE];

	private void videoReset() {
		for (int i = 0; i < CONVIDEO_SIZE; i++)
			video[i] = ' ';
	}
	
	private void videoDrawLineArt(Graphics g, int fontWidth, int fontHeight,
			int x, int y, int chr) {
		int px = (fontWidth - 2) >> 1;
		int py = (fontHeight - 2) >> 1;
		g.fillRect(x + px, y + py, 2, 2);
		if ((chr & 0x1) != 0)
			g.fillRect(x + px, y, 2, py);
		if ((chr & 0x2) != 0)
			g.fillRect(x, y + py, px, 2);
		if ((chr & 0x4) != 0)
			g.fillRect(x + px + 2, y + py, fontWidth - px - 2, 2);
		if ((chr & 0x8) != 0)
			g.fillRect(x + px, y + py + 2, 2, fontHeight - py - 2);
	}

	private void videoDraw(Graphics g) {
		g.setColor(0xFFFFFF);
		g.fillRect(0, 0, getWidth(), getHeight());
		g.setColor(0x000000);
		Font font = Font.getFont(Font.FACE_MONOSPACE, Font.STYLE_BOLD,
				Font.SIZE_SMALL);
		int fontWidth = font.charWidth('W');
		int fontHeight = font.getHeight();
		int ox = (getWidth() - COLS * fontWidth) >> 1;
		int oy = (getHeight() - ROWS * fontHeight) >> 1;
		g.setFont(font);
		int x, y = oy, index = 0;
		for (int r = 0; r < ROWS; r++) {
			x = ox;
			for (int c = 0; c < COLS; c++) {
				int chr = video[index++] & 0xFF;
				if (chr != 0) {
					if ((chr & 0xF0) == 0x80)
						videoDrawLineArt(g, fontWidth, fontHeight, x, y, chr);
					else
						g.drawChar((char)chr, x, y, 0);
				}
				x += fontWidth;
			}
			y += fontHeight;
		}
	}
	
	private void doVideoPut(int address, int value) {
		video[address - REG_CONVIDEO] =  (byte)value;
	}
	
	private int doVideoGet(int address) {
		return video[address - REG_CONVIDEO] & 0xFF;
	}

	protected void paint(Graphics g) {
		videoDraw(g);
	}

	private int init() {
		return 0;
	}

	private int activate() {
		return 0;
	}

	private int deactivate() {
		return 0;
	}

	private int reset() {
		frameReset();
		randomReset();
		keyBufReset();
		videoReset();
		return 0;
	}
	
	private int flush() {
		repaint();
		serviceRepaints();
		return 0;
	}

	private int vmStatus = IOSvc.VM_STATUS_RUNNING;
	private boolean signalUserBreak;
	private boolean signalSuspendCPU;
	private boolean signalResumeCPU;

	private int align(int vmStatus) {
		this.vmStatus = vmStatus;
		if (signalUserBreak) {
			signalUserBreak = false;
			return IOSvc.ALIGN_USER_BREAK;
		}
		if (signalSuspendCPU) {
			signalSuspendCPU = false;
			return IOSvc.ALIGN_SUSPEND_CPU;
		}
		if (signalResumeCPU) {
			signalResumeCPU = false;
			return IOSvc.ALIGN_RESUME_CPU;
		}
		return IOSvc.ALIGN_ACTIVE;
	}
	
	private int setAddressSpace(Object oArg) {
		return 0;
	}

	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "MicroIO";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_VM_PLUGIN);
		case METADATA_ID_SERVICES:
			return new String[] { IOSvc.TAG };
		default:
			return null;
		}
	}
	
	public Object opO(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_GET_METADATA:
			return getMetadata(iArg);
		case OP_GET_DISPLAYABLE:
			return this;
		}
		return null;
	}

	public int opI(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_INIT:
			return init();
		case OP_ACTIVATE:
			return activate();
		case OP_DEACTIVATE:
			return deactivate();
		case IOSvc.OP_RESET:
			return reset();
		case IOSvc.OP_FLUSH:
			return flush();
		case IOSvc.OP_ALIGN:
			return align(iArg);
		case IOSvc.OP_DO_SOME_WORK:
			return doSomeWork();
		case IOSvc.OP_SET_ADDRESS_SPACE:
			return setAddressSpace(oArg);
		}
		return -1;
	}

	private static final int FRAME_MIN_WAIT = 10;

	private int fps;	
	private long nextFrame;
	private int frameInterval;
	private boolean waitingForFrame;
	
	private void frameReset() {
		doFrmFpsPut(40);
	}

	private int doSomeWork() {
		int wait = -1;
		long now = System.currentTimeMillis();
		if (nextFrame != 0 && now >= nextFrame - FRAME_MIN_WAIT) {
			do
				nextFrame += frameInterval;
			while (nextFrame < now);
			flush();
			if (waitingForFrame) {
				waitingForFrame = false;
				signalResumeCPU = true;
			}
			wait = 0;
		}
		if (waitingForFrame) {
			wait = (int)(nextFrame - now - (FRAME_MIN_WAIT >> 1));
		}
		return wait;
	}

	private int doFrmFpsGet() {
		return fps;
	}

	private void doFrmFpsPut(int value) {
		fps = value;
		if (value == 0) {
			frameInterval = 0;
			nextFrame = 0;
		} else {
			frameInterval = 4000 / value;
			nextFrame = System.currentTimeMillis() + frameInterval;
		}
	}

	private void doFrmDrawPut() {
		if (frameInterval == 0) {
			flush();
		} else {
			signalSuspendCPU = true;
			waitingForFrame = true;
		}
	}

	public static final int REG_FRMFPS = 17;
	public static final int REG_FRMDRAW = 18;
	public static final int REG_RANDOM = 23;
	public static final int REG_KEYBUF = 24;
	public static final int REG_CONVIDEO = 40;

	private static final int KEYBUF_SIZE = 8;

	private static final int COLS = 10;
	private static final int ROWS = 4;
	private static final int CONVIDEO_SIZE = COLS * ROWS;

	public int get(int address) {
		switch (address) {
		case REG_FRMFPS:
			return doFrmFpsGet();
		case REG_RANDOM:
			return doRandomGet();
		default:
			if (address >= REG_KEYBUF
					&& address < REG_KEYBUF + KEYBUF_SIZE) {
				return doKeyBufGet(address);
			} else if (address >= REG_CONVIDEO
					&& address < REG_CONVIDEO + CONVIDEO_SIZE) {
				return doVideoGet(address);
			}
		}			
		return 0;
	}

	public int put(int address, int value) {
		switch (address) {
		case REG_FRMFPS:
			doFrmFpsPut(value);
			break;
		case REG_FRMDRAW:
			doFrmDrawPut();
			break;
		case REG_RANDOM:
			doRandomPut(value);
			break;
		case REG_KEYBUF:
			doKeyBufPut();
			break;
		default:
			if (address >= REG_CONVIDEO
					&& address < REG_CONVIDEO + CONVIDEO_SIZE)
				doVideoPut(address, value);
		}			
		return 0;
	}
}
