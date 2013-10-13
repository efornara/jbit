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
import javax.microedition.lcdui.ChoiceGroup;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Form;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.TextField;

public class Monitor extends LibView implements Module, CommandListener {
	
	public static final int VIEW_MEMORY = 1;
	public static final int VIEW_CPU = 2;
	
	public static final int HEADER_SEPARATOR_HEIGHT = 3;
	
	private Command editCmd = new Command("Edit", Command.SCREEN, 1);
	private Command gotoCmd = new Command("GoTo", Command.SCREEN, 2);
	private Command stepSizeCmd = new Command("StepSize", Command.SCREEN, 3);
	private Command continueCmd = new Command("Continue", Command.SCREEN, 4);
	private Command abortCmd = new Command("Abort", Command.SCREEN, 5);
	private Command brkptsCmd = new Command("BrkPt", Command.SCREEN, 6);
	private Command okCmd = new Command("OK", Command.OK, 1);
	private Command unsetCmd = new Command("UnSet", Command.SCREEN, 2);
	private Command cancelCmd = new Command("Cancel", Command.CANCEL, 10);

	private Module vm;
	private Module cpu;
	private Module m;
	
	private static final int memStart = 0x0000;
	private static final int memEnd = 0xFF00;

	private int view;
	private int rows, cols;
	private int screenStart;
	private int cpuStart;
	
	private int cursor;
	private int stepSize = 10;
	
	private int brkCode;
	private int brkStack;

	public Monitor() {
		setCommandListener(this);
	}
	
	private void setupCommands() {
		addCommand(gotoCmd);
		addCommand(editCmd);
		addCommand(stepSizeCmd);
		addCommand(continueCmd);
		addCommand(abortCmd);
		addCommand(brkptsCmd);
	}
	
	private void drawAssemblyView() {
		fmtPutString("PC");
		fmtXL += fmtSpace;
		int pc = cpu.get(CPUSvc.ADDRESS_PC);
		fmtPutPagedAddress(pc);
		fmtXL += fmtSpace;
		int cpuStatus = cpu.opI(CPUSvc.OP_GET_STATUS, 0, null);
		switch (cpuStatus) {
		case CPUSvc.OK:
			fmtPutChar('R');
			break;
		case CPUSvc.WAIT:
			fmtPutChar('W');
			break;
		default:
			fmtPutChar('E');
			break;
		}
		fmtJ = FMT_J_RIGHT;
		fmtPutString("CPU");
		fmtNewLine();
		fmtY += HEADER_SEPARATOR_HEIGHT;
		int i = 0, j = 0;
		if (i >= cpuStart && j < rows) {
			fmtXL += fmtSpace;
			fmtPutString(opcDisassembly(pc,
					m.get(pc),
					m.get(pc + 1),
					m.get(pc + 2)
			));
			fmtNewLine();
			j++;
		}
		i++;
		if (i >= cpuStart && j < rows) {
			fmtXL += fmtSpace;
			fmtPutChar('A');
			fmtXL += fmtSpace;
			fmtPutString(opcByteToDecString(cpu.get(CPUSvc.ADDRESS_A)));
			fmtPutChar(' ');
			fmtPutChar('X');
			fmtXL += fmtSpace;
			fmtPutString(opcByteToDecString(cpu.get(CPUSvc.ADDRESS_X)));
			fmtPutChar(' ');
			fmtPutChar('Y');
			fmtXL += fmtSpace;
			fmtPutString(opcByteToDecString(cpu.get(CPUSvc.ADDRESS_Y)));
			fmtNewLine();
			j++;
		}
		i++;
		if (i >= cpuStart && j < rows) {
			fmtXL += fmtSpace;
			int mask = 0x80;
			int w = fmtFont.charWidth('W');
			int p = cpu.get(CPUSvc.ADDRESS_P);
			for (int k = 0; k < 8; k++) {
				fmtXL = fmtSpace + k * w;
				fmtPutChar((p & mask) != 0 ? "NV?BDIZC".charAt(k) : '-');
				mask >>= 1;
			}
			fmtNewLine();
			j++;
		}
		i++;
		if (i >= cpuStart && j < rows) {
			fmtXL += fmtSpace;
			fmtPutString("S");
			fmtXL += fmtSpace;
			fmtPutString(opcByteToDecString(cpu.get(CPUSvc.ADDRESS_S)));
			fmtNewLine();
			j++;
		}
		i++;
		if (i >= cpuStart && j < rows) {
			fmtXL += fmtSpace;
			fmtPutString("I#");
			fmtXL += fmtSpace;
			fmtPutString(((Long)cpu.opO(CPUSvc.OP_GET_COUNTER,
					CPUSvc.COUNTER_INSTRUCTIONS, null)).toString());
			fmtNewLine();
			j++;
		}
		i++;
		if (i >= cpuStart && j < rows) {
			fmtXL += fmtSpace;
			fmtPutString("C#");
			fmtXL += fmtSpace;
			fmtPutString(((Long)cpu.opO(CPUSvc.OP_GET_COUNTER,
					CPUSvc.COUNTER_CYCLES, null)).toString());
			fmtNewLine();
			j++;
		}
		i++;
	}

	private void drawMemoryView() {
		int address = screenStart;
		fmtPutPagedAddress(cursor);
		fmtJ = FMT_J_RIGHT;
		fmtPutString("MEM");
		fmtNewLine();
		fmtY += HEADER_SEPARATOR_HEIGHT;
		for (int y = 0; y < rows; y++) {
			fmtXL = fmtCellSpace;
			for (int x = 0; x < cols; x++) {
				int decoration = FMT_NORMAL;
					if (address == cursor)
						decoration = FMT_ACTIVE;
				fmtPutDecoration(decoration, fmtByteWidth);
				fmtPutByte(m.get(address));
				fmtXL += fmtCellSpace;
				address++;
			}
			fmtNewLine();
		}
	}

	protected void paint(Graphics g) {
		try {
			g.setColor(0xFFFFFF);
			g.fillRect(0, 0, getWidth(), getHeight());
			touchDraw(false, g);
			fmtBegin(g);
			g.setColor(0x000000);
			switch (view) {
			case VIEW_CPU:
				drawAssemblyView();
				break;
			case VIEW_MEMORY:
				drawMemoryView();
				break;
			}
		} finally {
			fmtEnd();
		}
	}
	
	private void initMemoryWindow() {
		screenStart = screenStart / cols * cols;
		updateMemoryWindow();
	}
	
	private void updateMemoryWindow() {
		if (cursor < screenStart)
			screenStart = cursor / cols * cols;
		while (cursor >= screenStart + rows * cols)
			screenStart += cols;
		repaint();
	}
	
	private void setupWindow() {
		initMemoryWindow();
	}
	
	private TextField offsetItem;
	private TextField pageItem;
	private TextField valueItem;

	private Form gotoDlg;

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
					int to = 0, page, offset;
					try {
						page = Integer.parseInt(pageItem.getString());
						if (page < 0 || page > 254)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Page [0,254]");
					}							
					try {
						offset = Integer.parseInt(offsetItem.getString());
						if (offset < 0 || offset > 255)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Offset [0,255]");
					}
					to = (page << 8) + offset;
					if (to >= memStart	&& to <= memEnd) {
						cursor = to;
						setupWindow();
					} else {
						throw new Exception("Out of range");
					}
					display.setCurrent(this);
				} catch (Exception e) {
					retry = true;
					showError(e.getMessage(), gotoDlg);
				}
			} else {
				display.setCurrent(this);
			}
		} finally {
			if (!retry) {
				gotoDlg = null;
				pageItem = null;
				offsetItem = null;
			}
		}
	}

	private Form editDlg;
	private ChoiceGroup itemItem;
	
	private static final int ITEM_MEMORY = 0;
	private static final int ITEM_A = 1;
	private static final int ITEM_X = 2;
	private static final int ITEM_Y = 3;
	private static final int ITEM_Z = 4;
	private static final int ITEM_N = 5;
	private static final int ITEM_C = 6;
	private static final int ITEM_V = 7;
	private static final int ITEM_PC = 8;
	private static final int ITEM_S = 9;

	/*
	 * Some tricks here with the focus and the order of the items.
	 * On MIDP1 we cannot set the focus of the item and we don't have a POPUP
	 * item, so in memory view we move the long ChoiceGroup at the bottom
	 * (in CPU view the ChoiceGroup is useful at the top).
	 */
	private void showEditDialog() {
		editDlg = new Form("Edit");
		itemItem = new ChoiceGroup("Item", Const.CHOICE_GROUP_POPUP);
		itemItem.append("Memory (V,O,P)", null);
		itemItem.append("A (V)", null);
		itemItem.append("X (V)", null);
		itemItem.append("Y (V)", null);
		itemItem.append("Z (V)", null);
		itemItem.append("N (V)", null);
		itemItem.append("C (V)", null);
		itemItem.append("V (V)", null);
		itemItem.append("PC (O,P)", null);
		itemItem.append("S (V)", null);
		if (view == VIEW_CPU)
			itemItem.setSelectedIndex(ITEM_A, true);
		else
			itemItem.setSelectedIndex(ITEM_MEMORY, true);
		// #if JavaPlatform == "MIDP/1.0"
//@		if (view == VIEW_CPU)
		// #endif
		editDlg.append(itemItem);
		valueItem = new TextField("Value",
				"",
				4, TextField.NUMERIC);
		editDlg.append(valueItem);
		int offset, page;
		if (view == VIEW_CPU) {
			offset = cpu.get(CPUSvc.ADDRESS_PCLO);
			page = cpu.get(CPUSvc.ADDRESS_PCHI);
		} else {
			offset = cursor & 0xFF;
			page = cursor >> 8;
		}
		offsetItem = new TextField("Offset",
				Integer.toString(offset),
				3, TextField.NUMERIC);
		editDlg.append(offsetItem);
		pageItem = new TextField("Page",
				Integer.toString(page),
				3, TextField.NUMERIC);
		editDlg.append(pageItem);
		// #if JavaPlatform == "MIDP/1.0"
//@		if (view == VIEW_MEMORY)
//@			editDlg.append(itemItem);
		// #endif
		editDlg.addCommand(okCmd);
		editDlg.addCommand(cancelCmd);
		editDlg.setCommandListener(this);
		// #if JavaPlatform != "MIDP/1.0" && !MICROEMULATOR
//@		if (view == VIEW_MEMORY)
//@			display.setCurrentItem(valueItem);
//@		else
		// #endif
			display.setCurrent(editDlg);
	}
	
	private void updateP(int mask, int value) {
		int p = cpu.get(CPUSvc.ADDRESS_P);
		if (value != 0)
			p |= mask;
		else
			p &= ~mask;
		cpu.put(CPUSvc.ADDRESS_P, p);
	}

	private void handleEditDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				try {
					int index, page = 0, offset = 0, value = 0;
					index = itemItem.getSelectedIndex();
					// value
					switch (index) {
					case ITEM_MEMORY:
					case ITEM_A:
					case ITEM_X:
					case ITEM_Y:
					case ITEM_S:
						try {
							value = Integer.parseInt(valueItem.getString());
							if (value < -128 || value > 255)
								throw new Throwable();
							value &= 0xFF;
						} catch (Throwable e) {
							throw new Exception("Invalid Value [-128,255]");
						}
						break;
					case ITEM_Z:
					case ITEM_N:
					case ITEM_C:
					case ITEM_V:
						try {
							value = Integer.parseInt(valueItem.getString());
							if (value < 0 || value > 1)
								throw new Throwable();
						} catch (Throwable e) {
							throw new Exception("Invalid Value [0,1]");
						}
						break;
					}
					// page and offset
					switch (index) {
					case ITEM_MEMORY:
					case ITEM_PC:
						try {
							page = Integer.parseInt(pageItem.getString());
							if (page < 0 || page > 254)
								throw new Throwable();
						} catch (Throwable e) {
							throw new Exception("Invalid Page [0,254]");
						}							
						try {
							offset = Integer.parseInt(offsetItem.getString());
							if (offset < 0 || offset > 255)
								throw new Throwable();
						} catch (Throwable e) {
							throw new Exception("Invalid Offset [0,255]");
						}
						break;
					}
					// set
					switch (index) {
					case ITEM_MEMORY:
						m.put((page << 8) | offset, value);
						break;
					case ITEM_A:
						cpu.put(CPUSvc.ADDRESS_A, value);
						break;
					case ITEM_X:
						cpu.put(CPUSvc.ADDRESS_X, value);
						break;
					case ITEM_Y:
						cpu.put(CPUSvc.ADDRESS_Y, value);
						break;
					case ITEM_Z:
						updateP(0x02, value);
						break;
					case ITEM_N:
						updateP(0x80, value);
						break;
					case ITEM_C:
						updateP(0x01, value);
						break;
					case ITEM_V:
						updateP(0x40, value);
						break;
					case ITEM_PC:
						cpu.put(CPUSvc.ADDRESS_PCHI, page);
						cpu.put(CPUSvc.ADDRESS_PCLO, offset);
						break;
					case ITEM_S:
						cpu.put(CPUSvc.ADDRESS_S, value);
						break;
					}
					repaint();
					display.setCurrent(this);
				} catch (Exception e) {
					retry = true;
					showError(e.getMessage(), editDlg);
				}
			} else {
				display.setCurrent(this);
			}
		} finally {
			if (!retry) {
				editDlg = null;
				itemItem = null;
				valueItem = null;
				offsetItem = null;
				pageItem = null;
			}
		}
	}
	
	private Form stepSizeDlg;

	private void showStepSizeDialog() {
		stepSizeDlg = new Form("StepSize");
		valueItem = new TextField("Value",
				Integer.toString(stepSize),
				5, TextField.NUMERIC);
		stepSizeDlg.append(valueItem);
		stepSizeDlg.addCommand(okCmd);
		stepSizeDlg.addCommand(cancelCmd);
		stepSizeDlg.setCommandListener(this);
		display.setCurrent(stepSizeDlg);
	}
	
	private void handleStepSizeDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				try {
					int value;
					try {
						value = Integer.parseInt(valueItem.getString());
						if (value < 1 || value > 1000)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Value [1,1000]");
					}
					stepSize = value;
					display.setCurrent(this);
				} catch (Exception e) {
					retry = true;
					showError(e.getMessage(), stepSizeDlg);
				}
			} else {
				display.setCurrent(this);
			}
		} finally {
			if (!retry) {
				stepSizeDlg = null;
				valueItem = null;
			}
		}
	}
	
	private Form brkPtsSizeDlg;

	private void showBrkPtsDialog() {
		brkPtsSizeDlg = new Form("BrkPt");
		pageItem = new TextField("Page", brkCode == -1 ? "" : Integer.toString(brkCode >> 8), 3, TextField.NUMERIC);
		brkPtsSizeDlg.append(pageItem);
		offsetItem = new TextField("Offset", brkCode == -1 ? "" : Integer.toString(brkCode & 0xFF), 3, TextField.NUMERIC);
		brkPtsSizeDlg.append(offsetItem);
		brkPtsSizeDlg.addCommand(okCmd);
		brkPtsSizeDlg.addCommand(unsetCmd);
		brkPtsSizeDlg.addCommand(cancelCmd);
		brkPtsSizeDlg.setCommandListener(this);
		display.setCurrent(brkPtsSizeDlg);
	}
	
	private void handleBrkPtsDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				try {
					int page, offset;
					try {
						page = Integer.parseInt(pageItem.getString());
						if (page < 3 || page > 254)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Page [3,254]");
					}							
					try {
						offset = Integer.parseInt(offsetItem.getString());
						if (offset < 0 || offset > 255)
							throw new Throwable();
					} catch (Throwable e) {
						throw new Exception("Invalid Offset [0,255]");
					}
					brkCode = (page << 8) + offset;
					display.setCurrent(this);
				} catch (Exception e) {
					retry = true;
					showError(e.getMessage(), brkPtsSizeDlg);
				}
			} else {
				if (c == unsetCmd)
					brkCode = -1;
				display.setCurrent(this);
			}
		} finally {
			if (!retry) {
				brkPtsSizeDlg = null;
				pageItem = null;
				offsetItem = null;
			}
		}
	}
	
	private int checkBrkPts() {
		if (brkCode != -1 && brkCode == cpu.get(CPUSvc.ADDRESS_PC))
			;
		else if (brkStack != -1 && brkStack <= cpu.get(CPUSvc.ADDRESS_S))
			;
		else 
			return 0;
		brkStack = -1;
		return -1;
	}

	/* NAV
	 *  1 step 1
	 *  2 up
	 *  3 step n
	 *  4 left (memory view only)
	 *  5 ?
	 *  6 right (memory view only)
	 *  7 next
	 *  8 down
	 *  9 step out
	 *  0 memory/cpu
	 *  * continue
	 *  # show video
	 */
	
	private void gameActionPressed(int gameAction) {
		if (view == VIEW_CPU) {
			switch (gameAction) {
			case UP:
				cpuStart--;
				if (cpuStart < 0)
					cpuStart++;
				else
					repaint();
				break;
			case DOWN:
				cpuStart++;
				if (cpuStart > 6 - rows) 
					cpuStart--;
				else
					repaint();
				break;
			}
		} else {
			switch (gameAction) {
			case UP:
				cursor -= cols;
				if (cursor < memStart)
					cursor += cols;
				else
					updateMemoryWindow();
				break;
			case DOWN:
				cursor += cols;
				if (cursor > memEnd) 
					cursor -= cols;
				else
					updateMemoryWindow();
				break;
			case LEFT:
				cursor--;
				if (cursor < memStart)
					cursor++;
				else
					updateMemoryWindow();
				break;
			case RIGHT:
				cursor++;
				if (cursor > memEnd)
					cursor--;
				else
					updateMemoryWindow();
				break;
			case FIRE:
				showEditDialog();
				break;
			}
		}
	}
	
	protected void keyPressed(int keyCode) {
		switch (keyCode) {
		case '1':
			vm.opI(VMSvc.OP_STEP, 1, null);
			break;
		case '3':
			vm.opI(VMSvc.OP_STEP, stepSize, null);
			break;
		case '7':
			if (m.get(cpu.get(CPUSvc.ADDRESS_PC)) == 32) { // JSR
				brkStack = cpu.get(CPUSvc.ADDRESS_S);
				vm.opI(VMSvc.OP_CONTINUE, 0, null);
			} else {
				vm.opI(VMSvc.OP_STEP, 1, null);
			}
			break;
		case '9':
			brkStack = (cpu.get(CPUSvc.ADDRESS_S) + 2) & 0xFF;
			vm.opI(VMSvc.OP_CONTINUE, 0, null);
			break;
		case '*':
			vm.opI(VMSvc.OP_CONTINUE, 0, null);
			break;
		case '0':
			if (view == VIEW_MEMORY)
				view = VIEW_CPU;
			else
				view = VIEW_MEMORY;
			setupWindow();
			return;
		case '#':
		case Const.AUX_MODE_KEY:
			vm.opI(VMSvc.OP_VIDEO, 0, null);
			break;
		default:
			gameActionPressed(getJBitGameAction(keyCode));
			break;
		}
	}
	
	public void commandAction(Command c, Displayable d) {
		if (d == editDlg) {
			handleEditDialog(c);
		} else if (d == gotoDlg) {
			handleGotoDialog(c);
		} else if (d == stepSizeDlg) {
			handleStepSizeDialog(c);
		} else if (d == brkPtsSizeDlg) {
			handleBrkPtsDialog(c);
		} else if (c == editCmd) {
			showEditDialog();
		} else if (c == gotoCmd) {
			showGotoDialog();
		} else if (c == stepSizeCmd) {
			showStepSizeDialog();
		} else if (c == continueCmd) {
			vm.opI(VMSvc.OP_CONTINUE, 0, null);
		} else if (c == abortCmd) {
			vm.opI(VMSvc.OP_ABORT, 0, null);
		} else if (c == brkptsCmd) {
			showBrkPtsDialog();
		}
	}
	
	private void showError(String msg, Displayable nextDisplayable) {
		Alert alert = new Alert(null, msg + "!", null, AlertType.ERROR);
		alert.setTimeout(Alert.FOREVER);
		display.setCurrent(alert, nextDisplayable);
	}

	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "Monitor";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_SUPPORT);
		case METADATA_ID_SERVICES:
			return new String[] { MonitorSvc.TAG };
		default:
			return null;
		}
	}
	
	private int init(Module jbit) {
		viewInit(jbit);
		touchContext = new Integer(TouchSvc.CONTEXT_NAVIGATION);
		vm = (Module)jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, VMSvc.TAG);
		rows = (getHeight() - fmtFont.getHeight() - HEADER_SEPARATOR_HEIGHT)
				/ fmtFont.getHeight();
		cols = (getWidth() - fmtCellSpace) / (fmtByteWidth + fmtCellSpace);
		cursor = 0x300;
		brkCode = -1;
		brkStack = -1;
		return 0;
	}
	
	private int activate() {
		setupCommands();
		view = VIEW_CPU;
		cpuStart = 0;
		brkStack = -1;
		/*
		byte[][] file = (byte[][])jbit.opO(JBitSvc.OP_POOL_GET, 0,
				JBitSvc.POOL_ID_PROGRAM_FILE);
		cursor = 0x300 + ((file[0][JBFile.OFFSET_CODEPAGES] & 0xFF) << 8);
		*/
		//cpu.opI(CPUSvc.OP_SET_ADDRESS_SPACE, 0, this);
		return 0;
	}

	private int deactivate() {
		//cpu.opI(CPUSvc.OP_SET_ADDRESS_SPACE, 0, m);
		return 0;
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
			return init((Module)oArg);
		case OP_ACTIVATE:
			return activate();
		case OP_DEACTIVATE:
			return deactivate();
		case MonitorSvc.OP_SET_CPU:
			cpu = (Module)oArg;
			break;
		case MonitorSvc.OP_SET_ADDRESS_SPACE:
			m = (Module)oArg;
			break;
		case MonitorSvc.OP_REFRESH:
			repaint();
			display.setCurrent(this);
			break;
		case MonitorSvc.OP_SET_BRKPT:
			brkCode = iArg;
			break;
		case MonitorSvc.OP_HAS_BRKPT:
			return brkCode != -1 || brkStack != -1 ? -1 : 0;
		case MonitorSvc.OP_CHECK_BRKPT:
			return checkBrkPts();
		}
		return -1;
	}

	public int get(int address) {
		return m.get(address);
	}

	public int put(int address, int value) {
		return m.put(address, value);
	}
	
}
