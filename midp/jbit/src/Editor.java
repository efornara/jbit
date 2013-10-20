// #condition !JBIT_RUNTIME 

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

import javax.microedition.lcdui.Alert;
import javax.microedition.lcdui.AlertType;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Form;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.List;
import javax.microedition.lcdui.TextField;

public class Editor extends LibView implements Module, CommandListener {
	
	public static final int MODE_NAVIGATE = 1;
	public static final int MODE_EDIT = 2;

	public static final int VIEW_MEMORY = 1;
	public static final int VIEW_ASSEMBLY = 2;
	
	public static final int EDIT_BUFFER_INITIAL_LENGTH = 8;
	public static final int HEADER_SEPARATOR_HEIGHT = 3;
	
	public static final int FIRST_CODE_PAGE = 3;
	
	public static final int OPCODE_BRK = 0;
	public static final int OPCODE_NOP = 234;
	
	private final Integer navTouchContext = new Integer(TouchSvc.CONTEXT_NAVIGATION);
	private final Integer edtTouchContext = new Integer(TouchSvc.CONTEXT_EDITING);
	
	private Command putMarkCmd = new Command("PutMark", Command.SCREEN, 1);
	private Command gotoCmd = new Command("GoTo", Command.SCREEN, 2);
	private Command saveCmd = new Command("Save", Command.SCREEN, 3);
	private Command insertCmd = new Command("Insert", Command.SCREEN, 4);
	private Command deleteCmd = new Command("Delete", Command.SCREEN, 5);
	private Command editStringCmd = new Command("EditString", Command.SCREEN, 6);
	private Command debugCmd = new Command("Debug", Command.SCREEN, 7);
	private Command resizeCmd = new Command("Resize", Command.SCREEN, 8);
	private Command saveAsCmd = new Command("SaveAs", Command.SCREEN, 9);
	private Command newCmd = new Command("New", Command.SCREEN, 10);
	private Command brkptCmd = new Command("SetBrkPt", Command.SCREEN, 11);
	private Command backCmd = new Command("Back", Command.BACK, 1);
	private Command okCmd = new Command("OK", Command.OK, 1);
	private Command cancelCmd = new Command("Cancel", Command.CANCEL, 2);
	private Command saveOkCmd = new Command("", Command.SCREEN, 0);
	private Command saveFailedCmd = new Command("", Command.SCREEN, 0);

	private boolean canSave;
	private Module monitor;
	
	private byte[][] file;
	private boolean modified;

	private int mode;
	private int view;
	private int rows, cols;
	
	private int progId = -1;
	private int progStart;
	private int dataStart;	
	private int dataEnd;
	private int cursor;
	private int screenStart;
	private int screenEnd;
	private int mark;
	
	private byte[] editBuffer = new byte[EDIT_BUFFER_INITIAL_LENGTH];
	private int editStart;
	private int editLength;
	
	private char[] cellBuffer = new char[4];
	private int cellValue;
	private int cellLength;
	private boolean cellAssembly;
	private List opcodesDlg;
	
	public Editor() {
		setCommandListener(this);
	}
	
	private int getByte(int address) {
		if (address > dataEnd)
			return 0;
		int page = 1 + (address >> 8) - FIRST_CODE_PAGE;
		int offset = address & 0xFF;
		if (file[page] == null)
			return 0;
		return file[page][offset] & 0xFF;
	}
	
	private void putByte(int address, int value) {
		if (address > dataEnd)
			return;
		int page = 1 + (address >> 8) - FIRST_CODE_PAGE;
		int offset = address & 0xFF;
		if (file[page] == null)
			file[page] = new byte[256];
		if (file[page][offset] != (byte)value) {
			file[page][offset] = (byte)value;
			if (!modified) {
				jbit.opI(JBitSvc.OP_POOL_PUT, 0, new Object[] {
						JBitSvc.POOL_ID_PROGRAM_MODIFIED, new Boolean(true)
				});
				modified = true;
			}
		}
	}
	
	private int getWord(int address) {
		return getByte(address) + (getByte(address + 1) << 8);
	}
	
	private void putWord(int address, int value) {
		putByte(address, value & 0xFF);
		putByte(address + 1, value >> 8);
	}
	
	private void setupCommands() {
		if (mode == MODE_NAVIGATE) {
			touchContext = navTouchContext;
			addCommand(gotoCmd);
			if (canSave) {
				addCommand(saveCmd);
				addCommand(saveAsCmd);
			}
			if (monitor != null)
				addCommand(debugCmd);
			addCommand(putMarkCmd);
			addCommand(insertCmd);
			addCommand(deleteCmd);
			addCommand(editStringCmd);
			addCommand(resizeCmd);
			addCommand(newCmd);
			if (monitor != null)
				addCommand(brkptCmd);
			addCommand(backCmd);
			removeCommand(okCmd);
			removeCommand(cancelCmd);
		} else {
			touchContext = edtTouchContext;
			removeCommand(gotoCmd);
			removeCommand(saveCmd);
			removeCommand(saveAsCmd);
			removeCommand(putMarkCmd);
			removeCommand(insertCmd);
			removeCommand(deleteCmd);
			removeCommand(editStringCmd);
			removeCommand(resizeCmd);
			removeCommand(debugCmd);
			removeCommand(newCmd);
			removeCommand(brkptCmd);
			removeCommand(backCmd);
			addCommand(okCmd);
			addCommand(cancelCmd);
		}
	}

	private void drawHeader() {
		if (cursor < dataStart)
			fmtPutChar('C'); // TODO well-formed
		else
			fmtPutChar('D');
		fmtXL += fmtSpace;
		fmtPutPagedAddress(cursor);
		fmtJ = FMT_J_RIGHT;
		switch (view) {
		case VIEW_ASSEMBLY:
			fmtPutString("ASM");
			break;
		case VIEW_MEMORY:
			if (cellAssembly)
				fmtPutString("ASM");
			else
				fmtPutString("MEM");
			break;
		}
		fmtXR -= fmtSpace;
		fmtPutString(mode == MODE_NAVIGATE ? "NAV" : "EDT");
		fmtNewLine();
		fmtY += HEADER_SEPARATOR_HEIGHT;
	}
	
	private void drawAssemblyView() {
		int address = screenStart;
		for (int i = 0; i < rows; i++) {
			int opSize = opcGetSize(getByte(address));
			fmtXL++;
			if (cursor >= address && cursor < address + opSize)
				fmtPutDecoration(FMT_ACTIVE, getWidth() - 2);
			fmtXL++;
			fmtPutString(opcDisassembly(address,
					getByte(address),
					getByte(address + 1),
					getByte(address + 2)
			));
			address += opSize;
			fmtNewLine();
		}	
	}

	private void drawMemoryView() {
		int address = screenStart;
		for (int y = 0; y < rows; y++) {
			fmtXL = fmtCellSpace;
			for (int x = 0; x < cols; x++) {
				int decoration = FMT_NORMAL;
				if (mode == MODE_NAVIGATE) {
					if (address == cursor)
						decoration = FMT_ACTIVE;
				} else {
					if (address >= editStart && address < cursor)
						decoration = FMT_ACTIVE;
					else if  (address == cursor)
						decoration = FMT_EDIT;
				}
				fmtPutDecoration(decoration, fmtByteWidth);
				if (mode == MODE_EDIT && address == cursor) {
					String s = new String(new char[] {
							0 < cellLength ? cellBuffer[0] : ' ',
							1 < cellLength ? cellBuffer[1] : ' ',
							2 < cellLength ? cellBuffer[2] : ' ',
					});
					fmtPutCell(s);
				} else if (mode == MODE_EDIT
						&& address >= editStart && address < cursor) {
					fmtPutByte(editBuffer[address - editStart]);
				} else {
					fmtPutByte(getByte(address));
				}
				fmtXL += fmtCellSpace;
				address++;
			}
			fmtNewLine();
		}
	}
	
	protected void paint(Graphics g) {
		if (file == null)
			return;
		try {
			g.setColor(0xFFFFFF);
			g.fillRect(0, 0, getWidth(), getHeight());
			if (mode == MODE_NAVIGATE)
				touchDraw(false, g);
			fmtBegin(g);
			g.setColor(0x000000);
			drawHeader();
			switch (view) {
			case VIEW_ASSEMBLY:
				drawAssemblyView();
				break;
			case VIEW_MEMORY:
				drawMemoryView();
				break;
			}
			if (mode == MODE_EDIT)
				touchDraw(true, g);
		} finally {
			fmtEnd();
		}
	}
	
	private void initMemoryWindow() {
		screenStart = (screenStart - progStart) / cols * cols + progStart;
		updateMemoryWindow();
	}
	
	private void updateMemoryWindow() {
		if (cursor < screenStart)
			screenStart = (cursor - progStart) / cols * cols + progStart;
		while (cursor >= screenStart + rows * cols)
			screenStart += cols;
		screenEnd = -1;
		repaint();
	}
	
	private void updateInputValue() {
		cellValue = 0;
		if (cellAssembly)
			return;
		for (int i = 0; i < cellLength; i++) {
			cellValue *= 10;
			cellValue += cellBuffer[i] - '0';
		}
		if (cellValue > 255) {
			cellLength = 2;
			cellValue = (cellBuffer[0] - '0') * 10 + (cellBuffer[1] - '0');
		}
	}
	
	private void snapCursor() {
		int i = progStart;
		while (i <= dataEnd) {
			int opSize = opcGetSize(getByte(i));
			if (cursor >= i && cursor < i + opSize) {
				cursor = i;
				return;
			}
			i += opSize;
		}
	}
	
	private void computeAssemblyEnd() {
		screenEnd = screenStart;
		for (int i = 0; i < rows; i++)
			screenEnd += opcGetSize(getByte(screenEnd));
		screenEnd--;
	}
	
	private void initAssemblyWindow() {
		snapCursor();
		computeAssemblyEnd();
		updateAssemblyWindow();
	}

	private void updateAssemblyWindow() {
		if (cursor < screenStart) {
			screenStart = cursor;
			computeAssemblyEnd();
		}
		while (cursor > screenEnd) {
			screenStart += opcGetSize(getByte(screenStart));
			screenEnd += opcGetSize(getByte(screenEnd + 1));
		}
		repaint();
	}
	
	private void setupWindow() {
		if (view == VIEW_ASSEMBLY)
			initAssemblyWindow();
		else
			initMemoryWindow();
	}
	
	private void nextCell() {
		if (editLength == editBuffer.length) {
			byte[] b = new byte[editBuffer.length << 1];
			System.arraycopy(editBuffer, 0, b, 0, editLength);
			editBuffer = b;
		}
		editBuffer[editLength++] = (byte)cellValue;
		cellLength = 0;
		cellAssembly = false;
		updateInputValue();
		cursor++;
		if (cursor > dataEnd) {
			cursor--;
			repaint();
		} else
			updateMemoryWindow();
	}
	
	private void showOpcodesDialog() {
		opcodesDlg = new List("Matchings", List.IMPLICIT);
		for (int i = 0; i <= 255; i++)
			if (opcMatches((byte)i,
					cellBuffer[0], cellBuffer[1], cellBuffer[2])) {
				opcodesDlg.append(opcGetTemplate((byte)i), null);
		}
		if (opcodesDlg.size() != 0) {
			opcodesDlg.addCommand(okCmd);
			opcodesDlg.addCommand(cancelCmd);
			opcodesDlg.setCommandListener(this);
			display.setCurrent(opcodesDlg);
		} else {
			cellLength = 0;
		}
	}

	private void handleOpcodesDialog(Command c) {
		try {
			if (c == okCmd || c == List.SELECT_COMMAND) {
				int index = opcodesDlg.getSelectedIndex();
				if (index == -1)
					return;
				String s = opcodesDlg.getString(index);
				for (int i = 0; i <= 255; i++) {
					if (s.startsWith(opcGetMnemonic(opcGetDefinition((byte)i)))
							&& opcGetTemplate((byte)i).equals(s)) {
						cellValue = i;
						nextCell();
						return;
					}
				}
			}
		} finally {
			opcodesDlg = null;
			cellLength = 0;
			updateInputValue();
			repaint();
			display.setCurrent(this);
		}
	}

	private Form gotoDlg;
	private TextField pageItem;
	private TextField offsetItem;

	private void showGotoDialog() {
		gotoDlg = new Form("GoTo");
		pageItem = new TextField("Page", "", 3, TextField.NUMERIC);
		gotoDlg.append(pageItem);
		offsetItem = new TextField("Offset", "", 3, TextField.NUMERIC);
		gotoDlg.append(offsetItem);
		gotoDlg.addCommand(okCmd);
		gotoDlg.addCommand(cancelCmd);
		gotoDlg.setCommandListener(this);
		display.setCurrent(gotoDlg);
	}
	
	private void handleGotoDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				try {
					int page, offset;
					try {
						page = Integer.parseInt(pageItem.getString());
						if (page < 0 || page > 254)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Page [0,254]");
					}							
					try {
						String offsetString = offsetItem.getString();
						if (offsetString.length() == 0)
							offset = 0;
						else
							offset = Integer.parseInt(offsetString);
						if (offset < 0 || offset > 255)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Offset [0,255]");
					}
					int to = (page << 8) + offset;
					if (to >= progStart	&& to <= dataEnd) {
						cursor = to;
						setupWindow();
					} else {
						throw new Exception("Out of range");
					}
				} catch (Exception e) {
					retry = true;
					showError(e.getMessage(), gotoDlg);
				}
			}
		} finally {
			if (!retry) {
				gotoDlg = null;
				pageItem = null;
				offsetItem = null;
				display.setCurrent(this);
			}
		}
	}
	
	private void checkWellFormed() throws Exception {
		int i = progStart;
		while (i < dataStart) {
			int op = getByte(i);
			if (opcGetAddressingMode(opcGetDefinition(op)) == AM_INV) {
				throw new Exception("Invalid opcode " + op +
						" at " + opcIntToPagString(i));
			}
			i += opcGetSize(getByte(i));
		}
	}
	
	private int findCurrentInstruction() {
		int i = progStart;
		while (i < dataStart) {
			int opcode = getByte(i);
			int definition = opcGetDefinition(opcode);
			int addressingMode = opcGetAddressingMode(definition);
			if (addressingMode == AM_INV)
				return - 1;
			int size = opcGetSize(opcode);
			if (cursor >= i && cursor < i + size)
				return i;
			i += size;
		}
		return -1;
	}

	private void putMark() {
		if (mark == -1)
			return; // mark not set
		if (cursor >= dataStart) {
			// on a data page; put the absolute address
			putWord(cursor, mark);
		} else {
			// on a code page, update the current instruction
			int current = findCurrentInstruction();
			if (current == -1)
				return; // not well-formed
			int addressingMode = opcGetAddressingMode(
					opcGetDefinition(getByte(current)));
			if (addressingMode == AM_REL) {
				int offset = mark - (current + 2);
				if (offset < -128 || offset > 127)
					return; // out of range
				putByte(current + 1, offset & 0xFF);
			} else if (opcIsAddressingModeAbsolute(addressingMode))
				putWord(current + 1, mark);
		}
		repaint();
	}

	private int mapInsertDeleteAddress(boolean insert, int cells, int buffer,
	  int source, int target) throws Exception {
		// before editing point, address is unchanged
		if (target < cursor)
			return target;
		// starting from buffer, address is unchanged
		// (referencing inside the buffer is not handled)
		if (target >= buffer)
			return target;
		if (insert) {
			// referencing the insert point is left unchanged (unless it is
			// self-referencing, in which case an empty loop is assumed)
			if (target == cursor && target != source)
				return target;
			return target + cells;
		} else { // delete
			// referencing deleted code is invalid
			if (target < cursor + cells)
				throw new Exception(opcIntToPagString(source) +
						" references code to be deleted");
			return target - cells;
		}
	}
	
	private void insertDeleteProgram(boolean insert, int cells, boolean perform)
	  throws Exception {
		int after = insert ? cursor : cursor + cells;
		int i, buffer = 0;
		if (!perform) { // check
			// the resulting stream must be well formed
			i = after;
			while (i < dataStart) {
				int op = getByte(i);
				if (opcGetAddressingMode(opcGetDefinition(op)) == AM_INV) {
					throw new Exception("Opcode " + op +
							" at " + opcIntToPagString(i) +
							" would be invalid");
				}
				i += opcGetSize(getByte(i));
			}
		}
		// find the buffer (next block of BRKs)
		i = after;
		while (i < dataStart) {
			int op = getByte(i);
			if (op == OPCODE_BRK) {
				buffer = i;
				for (buffer = i; i < dataStart && getByte(i) == OPCODE_BRK; i++)
					;
				if (insert && i - buffer < cells)
					throw new Exception("BRK block is too small");
				break; // buffer is OK
			}
			i += opcGetSize(getByte(i));
		}
		if (insert && buffer == 0)
			throw new Exception("No BRK block found");
		if (buffer == 0)
			buffer = dataStart;
		// scan the resulting stream
		i = progStart;
		boolean gapSkipped = false;
		while (i < dataStart) {
			if (!insert && !gapSkipped && i >= cursor) {
				i = after;
				gapSkipped = true;
			}
			int opcode = getByte(i);
			int definition = opcGetDefinition(opcode);
			int addressingMode = opcGetAddressingMode(definition);
			// is the instruction referencing another location?
			int oldTarget = 0;
			if (addressingMode == AM_REL) {
				oldTarget = i + (byte)getByte(i + 1);
			} else if (opcIsAddressingModeAbsolute(addressingMode)) {
				oldTarget = getWord(i + 1);
			}
			if (oldTarget != 0) {
				// yes! map the address space
				int newTarget = mapInsertDeleteAddress(insert, cells,
						buffer, i, oldTarget);
				if (addressingMode == AM_REL) {
					int newSource = mapInsertDeleteAddress(insert, cells,
							buffer, i, i);
					int offset = newTarget - newSource;
					if (offset < -128 || offset > 127)
						throw new Exception("Branch at " +
							opcIntToPagString(i) + " would be too wide");
					if (perform)
						putByte(i + 1, offset & 0xFF);
				} else {
					if (perform)
						putWord(i + 1, newTarget);
				}
			}
			i += opcGetSize(getByte(i));
		}
		if (perform) { // perform
			// do the actual moving of the code
			if (insert) {
				// move to the right
				for (i = buffer + cells - 1; i >= cursor + cells; i--)
					putByte(i, getByte(i - cells));
				// fill the gap with NOPs
				for (i = cursor; i < cursor + cells; i++)
					putByte(i, OPCODE_NOP);
			} else { // delete
				// move to the left
				for (i = cursor; i < buffer - cells; i++)
					putByte(i, getByte(i + cells));
				// fill the gap with BRKs
				for (; i < buffer; i++)
					putByte(i, OPCODE_BRK);
			}
			// move the mark
			if (mark != -1) {
				try {
					mark = mapInsertDeleteAddress(insert, cells, buffer,
							progStart, mark);
				} catch (Throwable e) {
					mark = -1;
				}
			}
		}
	}
	
	private Form insertDeleteDlg;
	private boolean isInsertDlg;
	private TextField cellsItem;

	private void showInsertDeleteDialog(boolean insert) {
		try {
			checkWellFormed();
			if (cursor >= dataStart)
				throw new Exception("Only code is supported");
		} catch (Exception e) {
			showError(e.getMessage(), this);
			return;
		}
		insertDeleteDlg = new Form(insert ? "Insert" : "Delete");
		isInsertDlg = insert;
		int cells = 0;
		if (insert) {
			// suggest 10 cells
			cells = 10;
		} else {
			// suggest 1 instruction or the number of the following NOPs  
			int i = cursor;
			int op = getByte(i);
			if (op == OPCODE_NOP) {
				while (i < dataStart && getByte(i) == OPCODE_NOP) {
					cells++;
					i++;
				}
			} else {
				cells = opcGetSize(op);
			} 
		}
		cellsItem = new TextField("# of cells",
				Integer.toString(cells), 5, TextField.NUMERIC);
		insertDeleteDlg.append(cellsItem);
		insertDeleteDlg.addCommand(okCmd);
		insertDeleteDlg.addCommand(cancelCmd);
		insertDeleteDlg.setCommandListener(this);
		display.setCurrent(insertDeleteDlg);
	}
	
	private void handleInsertDeleteDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				try {
					int cells;
					try {
						cells = Integer.parseInt(cellsItem.getString());
						if (cells <= 0)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid # of cells (>0)");
					}
					insertDeleteProgram(isInsertDlg, cells, false);
					insertDeleteProgram(isInsertDlg, cells, true);
				} catch (Exception e) {
					retry = true;
					showError(e.getMessage(), insertDeleteDlg);
				}
			}
		} finally {
			if (!retry) {
				insertDeleteDlg = null;
				cellsItem = null;
				display.setCurrent(this);
			}
		}
	}

	private Form editStringDlg;
	private TextField stringItem;
	
	private boolean appendPrintableChar(StringBuffer s, int address) {
		int value = getByte(address);
		if ((value >= 32 && value < 128) || value >= 160) {
			s.append((char)value);
			return true;
		}
		return false;
	}

	private void showEditStringDialog() {
		try {
			StringBuffer s = new StringBuffer();
			int length;
			if (cursor < dataStart) {
				checkWellFormed();
				int address = findCurrentInstruction();
				if (address == -1)
					return; // it shoudn't happen
				int addressingMode = opcGetAddressingMode(
						opcGetDefinition(getByte(address)));
				if (addressingMode != AM_IMM)
					throw new Exception("Only supported in immediate mode");
				appendPrintableChar(s, address + 1);
				length = 1;
			} else {
				int i = cursor;
				length = 0;
				for (i = cursor; cursor <= dataEnd && length < 100; i++, length++) {
					if (!appendPrintableChar(s, i))
						break;
				}
				for (; cursor <= dataEnd && length < 100; i++, length++) {
					if (getByte(i) != 0)
						break;
				}
				if (length == 0)
					length = 1;
			}
			editStringDlg = new Form("PutStr");
			stringItem = new TextField("String", s.toString(), length,
					TextField.ANY);
			editStringDlg.append(stringItem);
			editStringDlg.addCommand(okCmd);
			editStringDlg.addCommand(cancelCmd);
			editStringDlg.setCommandListener(this);
			display.setCurrent(editStringDlg);
		} catch (Exception e) {
			showError(e.getMessage(), this);
			return;
		}
	}
	private void handleEditStringDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				try {
					int address;
					if (cursor < dataStart)
						address = findCurrentInstruction() + 1;
					else
						address = cursor;
					int length = stringItem.getMaxSize();
					String s = stringItem.getString();
					int i;
					for (i = 0; i < length && i < s.length(); i++)
						putByte(address + i, s.charAt(i) & 0xFF);
					for (; i < length; i++)
						if (getByte(address + i) != 0)
							putByte(address + i, 0);
				} catch (Exception e) {
					retry = true;
					showError(e.getMessage(), editStringDlg);
				}
			}
		} finally {
			if (!retry) {
				editStringDlg = null;
				display.setCurrent(this);
			}
		}
	}
	
	private void resetView() {
		int codePages = file[0][JBFile.OFFSET_CODEPAGES] & 0xFF;
		int dataPages = file[0][JBFile.OFFSET_DATAPAGES] & 0xFF;
		
		progStart = FIRST_CODE_PAGE << 8;
		dataStart = progStart + (codePages << 8);
		dataEnd = progStart + ((codePages + dataPages) << 8) - 1;
		
		cursor = progStart;
		screenStart = progStart;
		screenEnd = -1;
		mark = -1;

		view = VIEW_MEMORY;
	}
	
	private void newProgram(int codePages, int dataPages) {
		byte[] header = new byte[JBFile.SIZE_HEADER];
		header[JBFile.OFFSET_SIGNATURE + 0] = JBFile.SIGNATURE_0;
		header[JBFile.OFFSET_SIGNATURE + 1] = JBFile.SIGNATURE_1;
		header[JBFile.OFFSET_SIGNATURE + 2] = JBFile.SIGNATURE_2;
		header[JBFile.OFFSET_SIGNATURE + 3] = JBFile.SIGNATURE_3;
		header[JBFile.OFFSET_HEADERLENGTH + 0] = JBFile.HEADERLENGTH_0_HIGH;
		header[JBFile.OFFSET_HEADERLENGTH + 1] = JBFile.HEADERLENGTH_1_LOW;
		header[JBFile.OFFSET_VERSION + 0] = JBFile.VERSION_0_MAJOR;
		header[JBFile.OFFSET_VERSION + 1] = JBFile.VERSION_1_MINOR;
		header[JBFile.OFFSET_CODEPAGES] = (byte)codePages;
		header[JBFile.OFFSET_DATAPAGES] = (byte)dataPages;
		file = new byte[1 + codePages + dataPages][];
		file[0] = header;
		progId = jbit.opI(JBitSvc.OP_NEW_PROGRAM, 0, file);
		resetView();
	}
	
	private boolean isPageEmpty(int page) {
		byte[] p = file[1 + page - FIRST_CODE_PAGE];
		if (p != null) {
			for (int i = 0; i < 256; i++)
				if (p[i] != 0)
					return false;
		}
		return true;
	}
	
	private void resizeProgram(int codePages, int dataPages, boolean perform)
	  throws Exception {
		byte[] header = file[0];
		int oldCodePages = header[JBFile.OFFSET_CODEPAGES] & 0xFF;
		int oldDataPages = header[JBFile.OFFSET_DATAPAGES] & 0xFF;
		if (perform) { // perform
			byte[][] oldFile = file;
			// set new program size in header
			header[JBFile.OFFSET_CODEPAGES] = (byte)codePages;
			header[JBFile.OFFSET_DATAPAGES] = (byte)dataPages;
			// create a new file
			file = new byte[1 + codePages + dataPages][];
			file[0] = header;
			// copy the old code
			for (int i = 0; i < codePages; i++)
				if (i < oldCodePages)
					file[1 + i] = oldFile[1 + i];
			// copy the old data
			for (int i = 0; i < dataPages; i++)
				if (i < oldDataPages)
					file[1 + codePages + i] = oldFile[1 + oldCodePages + i];
			// register the new file and update the view
			progId = jbit.opI(JBitSvc.OP_NEW_PROGRAM, 0, file);
			resetView();
			// relocate the code
			int oldProgStart = FIRST_CODE_PAGE << 8;
			int oldDataStart = oldProgStart + (oldCodePages << 8);
			int oldDataEnd = oldProgStart
					+ ((oldCodePages + oldDataPages) << 8) - 1;
			int i = progStart;
			while (i < dataStart) {
				int op = getByte(i);
				if (opcIsAddressingModeAbsolute(
						opcGetAddressingMode(opcGetDefinition(op)))) {
					int target = getWord(i + 1);
					if (target >= oldDataStart && target <= oldDataEnd)
						putWord(i + 1, target + dataStart - oldDataStart);
				}
				i += opcGetSize(getByte(i));
			}
		} else { // check
			// deleted code pages must be empty
			if (codePages < oldCodePages) {
				for (int i = codePages; i < oldCodePages; i++)
					if (!isPageEmpty(FIRST_CODE_PAGE + i))
						throw new Exception("Code page "
								+ (FIRST_CODE_PAGE + i) + " is not empty");
			}
			// deleted data pages must be empty
			if (dataPages < oldDataPages) {
				for (int i = dataPages; i < oldDataPages; i++)
					if (!isPageEmpty(FIRST_CODE_PAGE + i + oldCodePages))
						throw new Exception("Data page "
								+ (FIRST_CODE_PAGE + i + oldCodePages)
								+ " is not empty");
			}
		}
	}
	
	private static final int NEW_RESIZE_MODE_INITIAL = 1;
	private static final int NEW_RESIZE_MODE_NEW = 2;
	private static final int NEW_RESIZE_MODE_RESIZE = 3;
	
	private Form newResizeDlg;
	private int newResizeMode;
	private TextField codeItem;
	private TextField dataItem;

	private void showNewResizeDialog(int mode) {
		if (mode == NEW_RESIZE_MODE_RESIZE) {
			try {
				checkWellFormed();
			} catch (Exception e) {
				showError(e.getMessage(), this);
				return;
			}
		}
		newResizeDlg = new Form(mode == NEW_RESIZE_MODE_RESIZE ?
				"Resize" : "Size (New)");
		newResizeMode = mode;
		codeItem = new TextField("Code Pages",
				"4", 3, TextField.NUMERIC);
		newResizeDlg.append(codeItem);
		dataItem = new TextField("Data Pages",
				"3", 3, TextField.NUMERIC);
		newResizeDlg.append(dataItem);
		newResizeDlg.addCommand(okCmd);
		newResizeDlg.addCommand(cancelCmd);
		newResizeDlg.setCommandListener(this);
		if (file != null)
			display.setCurrent(newResizeDlg);
	}
	
	private void handleNewResizeDialog(Command c) {
		boolean retry = false, back = false;
		try {
			if (c == okCmd) {
				try {
					int codePages;
					try {
						codePages = Integer.parseInt(codeItem.getString());
						if (codePages < 0 || codePages > 253)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Code Pages [0,253]");
					}
					int dataPages;
					try {
						dataPages = Integer.parseInt(dataItem.getString());
						if (dataPages < 0 || dataPages > 253)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Data Pages [0-253]");
					}
					if (codePages + dataPages > 253)
						throw new Exception("Program is too big");
					if (newResizeMode == NEW_RESIZE_MODE_RESIZE) {
						resizeProgram(codePages, dataPages, false);
						resizeProgram(codePages, dataPages, true);
					} else {
						newProgram(codePages, dataPages);
					}
				} catch (Exception e) {
					retry = true;
					showError(e.getMessage(), newResizeDlg);
				}
			} else {
				if (newResizeMode == NEW_RESIZE_MODE_INITIAL) {
					jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
					back = true;
				}
			}
		} finally {
			if (!retry) {
				newResizeDlg = null;
				codeItem = null;
				dataItem = null;
				if (!back)
					display.setCurrent(this);
			}
		}
	}

	private void save(boolean newName) {
		Module m = (Module)jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, SaveSvc.TAG);
		if (m != null) {
			m.opI(SaveSvc.OP_SAVE, 0, new Object[] {
					new Boolean(newName),
					this,
					this,
					saveOkCmd,
					saveFailedCmd
			});
		} else {
			showError("Save not available", this);
		}
	}
	
	/* NAV
	 *  1 prev snap point
	 *  2 up
	 *  3 next snap point
	 *  4 left
	 *  5 EDT mode
	 *  6 right
	 *  7 set mark
	 *  8 down
	 *  9 go to operand
	 *  0 swap cursor/mark
	 *  * run
	 *  # cycle submode: code:NEM/ASM data:DEC/HEX/TXT/BMP (?)
	 */
	
	/*
	 * snap points:
	 *  0, 50, 100, 150, 200
	 */
	private void goToSnapPoint(int direction) {
		int page = cursor >> 8;
		int offset = cursor & 0xFF;
		if (direction < 0) {
			// back
			if (offset == 0) {
				page--;
				offset = 200;
			} else if (offset <= 50)
				offset = 0;
			else if (offset <= 100)
				offset = 50;
			else if (offset <= 150)
				offset = 100;
			else if (offset <= 200)
				offset = 150;
			else
				offset = 200;
		} else {
			// forward
			if (offset >= 200)  {
				page++;
				offset = 0;
			} else if (offset >= 150)
				offset = 200;
			else if (offset >= 100)
				offset = 150;
			else if (offset >= 50)
				offset = 100;
			else
				offset = 50;
		}
		int destination = (page << 8) + offset;
		if (destination >= progStart && destination <= dataEnd) {
			cursor = destination;
			// Using setupWindow() would allow the cursor to snap
			// to a valid instruction in the Assembly View.
			// However, this would prevent multiple goToSnapPoint()
			// to be used (e.g. the cursor might be stuck to 3:49
			// when going forward).
			// With the implemented solution, there is a good chance
			// that the Assembly View would end up being invalid, but this
			// could be fixed quickly by the user be moving up the cursor.
			// Not being able to quickly jump to the next snap
			// point would be, IMO, a more annoying bug.
			screenStart = dataEnd; // invalidate screenStart
			if (view == VIEW_ASSEMBLY)
				updateAssemblyWindow();
			else
				updateMemoryWindow();
		}
	}
	
	private void goToOperand() {
		int current = findCurrentInstruction();
		if (current == -1)
			return; // not in code page or not well-formed
		int addressingMode = opcGetAddressingMode(
				opcGetDefinition(getByte(current)));
		int target = -1;
		if (addressingMode == AM_REL)
			target = current + 2 + (byte)getByte(current + 1);
		else if (opcIsAddressingModeAbsolute(addressingMode))
			target = getWord(current + 1);
		if (target >= progStart && target < dataEnd) {
			// target is valid; update cursor and view
			cursor = target;
			setupWindow();
		}
	}
	
	private void keyPressedGameAction(int gameAction) {
		if (view == VIEW_MEMORY) {
			switch (gameAction) {
			case FIRE:
				editStart = cursor;
				editLength = 0;
				cellLength = 0;
				cellAssembly = false;
				updateInputValue();
				mode = MODE_EDIT;
				setupCommands();
				repaint();
				break;
			case UP:
				cursor -= cols;
				if (cursor < progStart)
					cursor += cols;
				else
					updateMemoryWindow();
				break;
			case DOWN:
				cursor += cols;
				if (cursor > dataEnd)
					cursor -= cols;
				else
					updateMemoryWindow();
				break;
			case LEFT:
				cursor--;
				if (cursor < progStart)
					cursor++;
				else
					updateMemoryWindow();
				break;
			case RIGHT:
				cursor++;
				if (cursor > dataEnd)
					cursor--;
				else
					updateMemoryWindow();
				break;
			}
		} else {
			switch (gameAction) {
			case UP:
				cursor--;
				snapCursor();
				if (cursor < progStart)
					cursor = progStart;
				else
					updateAssemblyWindow();
				break;
			case DOWN:
				int opSize = opcGetSize(getByte(cursor));
				cursor += opSize;
				if (cursor > dataEnd)
					cursor -= opSize;
				else
					updateAssemblyWindow();
				break;
			}
		}
	}
	
	private void keyPressedNavigate(int keyCode) {
		switch (keyCode) {
		case '1':
			goToSnapPoint(-1);
			break;
		case '3':
			goToSnapPoint(1);
			break;
		case '7':
			mark = cursor;
			break;
		case '9':
			goToOperand();
			break;
		case '0':
			if (mark != -1) {
				int t = cursor;
				cursor = mark;
				mark = t;
				setupWindow();
			}
			break;
		case '*':
			jbit.opI(JBitSvc.OP_REPLACE_WITH_VM, VMSvc.START_MODE_RUN, this);
			return;
		case '#':
		case Const.AUX_MODE_KEY:
			if (view == VIEW_MEMORY)
				view = VIEW_ASSEMBLY;
			else
				view = VIEW_MEMORY;
			setupWindow();
			return;
		default:
			keyPressedGameAction(getJBitGameAction(keyCode));
		}
	}
	
	private void keyPressedEdit(int keyCode) {
		if (keyCode >= '0' && keyCode <= '9') {
			if (cellLength == 3)
				nextCell();
			cellBuffer[cellLength++] = (char)keyCode;
			if (cellLength == 3 && cellAssembly)
				showOpcodesDialog();
			updateInputValue();
			repaint();
		} else if (keyCode == '*') {
			nextCell();
		} else if (keyCode == '#' || keyCode == Const.AUX_MODE_KEY) {
			cellAssembly = !cellAssembly;
			cellLength = 0;
			updateInputValue();
			repaint();
		} else if ((keyCode >= 'a' && keyCode <= 'z')
				|| (keyCode >= 'A' && keyCode <= 'Z')) {
			if (!cellAssembly) {
				cellAssembly = !cellAssembly;
				cellLength = 0;
			}
			cellBuffer[cellLength++] = Character.toUpperCase((char)keyCode);
			if (cellLength == 3)
				showOpcodesDialog();
			updateInputValue();
			repaint();
		} else if (getGameAction(keyCode) == FIRE) {
			nextCell();
		}
	}

	protected void keyPressed(int keyCode) {
		if (mode == MODE_NAVIGATE)
			keyPressedNavigate(keyCode);
		else
			keyPressedEdit(keyCode);
	}
	
	public void commandAction(Command c, Displayable d) {
		if (d == null) {
			if (c == saveOkCmd)
				modified = false;
		} else if (d == opcodesDlg) {
			handleOpcodesDialog(c);
		} else if (d == gotoDlg) {
			handleGotoDialog(c);
		} else if (d == insertDeleteDlg) {
			handleInsertDeleteDialog(c);
		} else if (d == newResizeDlg) {
			handleNewResizeDialog(c);
		} else if (d == editStringDlg) {
			handleEditStringDialog(c);
		} else if (c == okCmd || c == cancelCmd) {
			if (c == okCmd) {
				for (int i = 0; i < editLength; i++)
					putByte(editStart + i, editBuffer[i]);
				if (cellLength != 0 && !cellAssembly)
					putByte(cursor, (byte)cellValue);
			} else {
				if (cellLength != 0) {
					cellLength = 0;
					updateInputValue();
					repaint();
					return;
				}
				if (editLength != 0) {
					cellLength = 0;
					editLength--;
					cursor--;
					updateMemoryWindow();
					return;
				}
			}
			if (editBuffer.length != EDIT_BUFFER_INITIAL_LENGTH)
				editBuffer = new byte[EDIT_BUFFER_INITIAL_LENGTH];
			mode = MODE_NAVIGATE;
			setupCommands();
			repaint();
		} else if (c == gotoCmd) {
			showGotoDialog();
		} else if (c == saveCmd) {
			save(false);
		} else if (c == saveAsCmd) {
			save(true);
		} else if (c == putMarkCmd) {
			putMark();
		} else if (c == insertCmd) {
			showInsertDeleteDialog(true);
		} else if (c == deleteCmd) {
			showInsertDeleteDialog(false);
		} else if (c == editStringCmd) {
			showEditStringDialog();
		} else if (c == debugCmd) {
			jbit.opI(JBitSvc.OP_REPLACE_WITH_VM, VMSvc.START_MODE_DEBUG, this);
		} else if (c == resizeCmd) {
			showNewResizeDialog(NEW_RESIZE_MODE_RESIZE);
		} else if (c == newCmd) {
			showNewResizeDialog(NEW_RESIZE_MODE_NEW);
		} else if (c == brkptCmd) {
			monitor.opI(MonitorSvc.OP_SET_BRKPT, cursor, null);
		} else if (c == backCmd) {
			jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
		}
	}
	
	private void showError(String msg, Displayable nextDisplayable) {
		Alert alert = new Alert(null, msg + "!", null, AlertType.ERROR);
		alert.setTimeout(Alert.FOREVER);
		// #if MICROEMULATOR
		// workaround; "abc" get stuck in microemulator
		// display.setCurrent(nextDisplayable); fixed?
		// #endif
		display.setCurrent(alert, nextDisplayable);
	}

	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "Editor";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_TOP_LEVEL);
		case METADATA_ID_SERVICES:
			return new String[] { EditSvc.TAG };
		default:
			return null;
		}
	}
	
	private int init(Module jbit) {
		viewInit(jbit);
		if (jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, SaveSvc.TAG) != null)
			canSave = true;
		monitor = (Module)jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, MonitorSvc.TAG);
		rows = (getHeight() - fmtFont.getHeight() - HEADER_SEPARATOR_HEIGHT)
				/ fmtFont.getHeight();
		cols = (getWidth() - fmtCellSpace) / (fmtByteWidth + fmtCellSpace); 
		return 0;
	}
	
	private int activate() {
		mode = MODE_NAVIGATE;
		setupCommands();
		file = (byte[][])jbit.opO(JBitSvc.OP_POOL_GET, 0,
				JBitSvc.POOL_ID_PROGRAM_FILE);
		if (file != null) {
			int id = ((Integer)jbit.opO(JBitSvc.OP_POOL_GET, 0,
					JBitSvc.POOL_ID_PROGRAM_ID)).intValue();
			if (id != progId) {
				progId = id;
				resetView();
			}
			modified = ((Boolean)jbit.opO(JBitSvc.OP_POOL_GET, 0,
					JBitSvc.POOL_ID_PROGRAM_MODIFIED)).booleanValue();
		} else {
			showNewResizeDialog(NEW_RESIZE_MODE_INITIAL);
		}
		return 0;
	}

	private int deactivate() {
		file = null;
		return 0;
	}

	public Object opO(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_GET_METADATA:
			return getMetadata(iArg);
		case OP_GET_DISPLAYABLE:
			if (newResizeDlg != null)
				return newResizeDlg;
			else
				return this;
		}
		return null;
	}

	public int opI(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_INIT:
			return init((Module)oArg);
		case OP_ACTIVATE:
			return activate();
		case OP_DEACTIVATE:
			return deactivate();
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
