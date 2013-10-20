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

import javax.microedition.lcdui.Canvas;
import javax.microedition.lcdui.Font;
import javax.microedition.lcdui.Graphics;

// Module must be thread-safe
public class Touch implements Module {

	private boolean[] show;
	private Font mainFont;
	private Font subFont;
	
	private void drawSimpleButton(Graphics g, int x, int y, int w, int h, String label, String subLabel) {
		int cx = x + (w >> 1);
		int cy = y + (h >> 1);
		int fh2 = mainFont.getHeight() >> 1;
		g.setFont(mainFont);
		if (subLabel == null) {
			g.drawString(label, cx, cy - fh2, Graphics.TOP | Graphics.HCENTER);
		} else {
			g.drawString(label, cx, cy, Graphics.BASELINE | Graphics.HCENTER);
			g.setFont(subFont);
			g.drawString(subLabel, cx, cy, Graphics.TOP | Graphics.HCENTER);
		}
	}
	
	private void drawButton(Graphics g, int x, int y, int w, int h, String label, String subLabel, boolean bold) {
//		g.drawRect(x + 1, y + 1, w - 2, h - 2);
		if (bold) {
			g.setColor(0xFFFFFF);
			for (int dy = -1; dy <= 1; dy++)
				for (int dx = -1; dx <= 1; dx++)
					if (dx != 0 && dy != 0)
						drawSimpleButton(g, dx + x, dy + y, w, h, label, subLabel);
			g.setColor(0x40D040);
			drawSimpleButton(g, x, y, w, h, label, subLabel);
		} else {
			g.setColor(0xD0D0D0);
			drawSimpleButton(g, x, y, w, h, label, subLabel);
		}
	}

	private final byte[] keypadPositions = {
		0, 0,
		1, 0,
		2, 0,
		0, 1,
		1, 1,
		2, 1,
		0, 2,
		1, 2,
		2, 2,
		0, 3,
		1, 3,
		2, 3,
		-1
	};
	
	private final String[] keypadLabels = {
		"1", "-",
		"2", "abc",
		"3", "def",
		"4", "ghi",
		"5", "jkl",
		"6", "mno",
		"7", "pqrs",
		"8", "tuv",
		"9", "wxyz",
		"*", "-",
		"0", "-",
		"#", "-",
	};

	// g == 0 => POINTER_PRESSED
	// g != 0 => DRAW
	private int mainOp(Integer context, Canvas canvas, Graphics g, boolean bold,
			int x, int y) {
		if (g != null && (!show[context.intValue()] || !canvas.hasPointerEvents()))
			return 0;
		int w = canvas.getWidth();
		int h = canvas.getHeight();
		boolean landscape = w > h;
		int switchSize = landscape ? w >> 3 : h >> 3;
		int ox, oy, pw, ph;
		if (landscape) {
			if (g != null) {
				drawButton(g, 0, 0, switchSize, h, "H", null, bold);
				drawButton(g, w - switchSize, 0, switchSize, h, "B", null, bold);
			}
			ox = switchSize;
			oy = 0;
			pw = w - (switchSize << 1);
			ph = h;
		} else {			
			if (g != null) {
				drawButton(g, 0, 0, w, switchSize, "Hide", null, bold);
				drawButton(g, 0, h - switchSize, w, switchSize, "Break", null, bold);
			}
			ox = 0;
			oy = switchSize;
			pw = w;
			ph = h - (switchSize << 1);
		}
		int bw = pw / 3, bh = ph >> 2;
		if (g != null) {
			for (int i = 0; keypadPositions[i] != -1; i += 2) {
				drawButton(g, ox + bw * keypadPositions[i], oy + bh
						* keypadPositions[i + 1], bw, bh, keypadLabels[i],
						keypadLabels[i + 1], bold);
			}
			return 0;
		} else {
			if ((landscape && x < switchSize) || (!landscape && y < switchSize)) {
				int n = context.intValue();
				show[n] = !show[n];
				return 0;
			} else if ((landscape && x > w - switchSize) || (!landscape && y > h - switchSize)) {
				return TouchSvc.SOFT_KEY_BREAK;
			} else {
				for (int i = 0; keypadPositions[i] != -1; i += 2) {
					int bx = ox + bw * keypadPositions[i];
					int by = oy + bh * keypadPositions[i + 1];
					if (x > bx && x < bx + bw && y > by && y < by + bh) {
						return keypadLabels[i].charAt(0);
					}
				}
				return 0;
			}
		}
	}
	
	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "Touch";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_SUPPORT);
		case METADATA_ID_SERVICES:
			return new String[] { TouchSvc.TAG };
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
		Object[] args;
		switch (opId) {
		case OP_INIT:
			show = new boolean[TouchSvc.N_OF_CONTEXTS];
			for (int i = 0; i < TouchSvc.N_OF_CONTEXTS; i++)
				show[i] = true;
			mainFont = Font.getFont(Font.FACE_SYSTEM, Font.STYLE_BOLD,
					Font.SIZE_LARGE);
			subFont = Font.getFont(Font.FACE_SYSTEM, Font.STYLE_BOLD,
					Font.SIZE_MEDIUM);
			return 0;
		case OP_ACTIVATE:
		case OP_DEACTIVATE:
			return 0;
		case TouchSvc.OP_POINTER_PRESSED:
			args = (Object[])oArg; 
			return mainOp((Integer) args[0], (Canvas) args[1], null, false,
					iArg & 0xFFFF, (iArg >> 16) & 0xFFFF);
		case TouchSvc.OP_DRAW:
			args = (Object[])oArg; 
			return mainOp((Integer) args[0], (Canvas) args[1],
					(Graphics) args[2], iArg != 0, 0, 0);
		}
		return -1;
	}

	public int get(int address) {
		return -1;
	}

	public int put(int address, int value) {
		return -1;
	}
}
