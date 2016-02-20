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

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Vector;

import javax.microedition.io.Connector;
import javax.microedition.io.file.FileConnection;
import javax.microedition.io.file.FileSystemRegistry;
import javax.microedition.lcdui.Alert;
import javax.microedition.lcdui.AlertType;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.List;
import javax.microedition.lcdui.TextBox;
import javax.microedition.lcdui.TextField;


public final class FileIE implements Module, CommandListener, Runnable {
	
	private static final String ROOT = "file:///";	
	private static final String DUMMY_LABEL = "...";
	private static final String UP_LABEL = "[up]";
	private static final String ROOT_LABEL = "[root]";

	private static final int WORKER_WAIT = 1;
	private static final int WORKER_INIT = 2;
	private static final int WORKER_LIST = 3;
	private static final int WORKER_LOAD = 4;
	private static final int WORKER_SAVE = 5;
	private static final int WORKER_OVERWRITE = 6;

	private Command saveCmd = new Command("Save", Command.SCREEN, 1);
	private Command loadCmd = new Command("Load", Command.SCREEN, 2);
	private Command overwriteCmd = new Command("Overwrite", Command.SCREEN, 3);
	private Command backCmd = new Command("Back", Command.BACK, 1);
	private Command okCmd = new Command("OK", Command.OK, 1);
	private Command cancelCmd = new Command("Cancel", Command.CANCEL, 2);

	private volatile int workerStatus;
	private volatile boolean terminate;
	private volatile String path = ROOT;
	private volatile String fileName = null;
	private final char separator = '/';
	
	private Module jbit;
	private Display display;
	private Thread thread;
	private List list;
	private byte[][] file;

	private void back() {
		jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
	}

	private Object getMetadata(int metadataId) {
		switch (metadataId) {
		case METADATA_ID_LABEL:
			return "Files";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_TOP_LEVEL);
		case METADATA_ID_SERVICES:
			return new String[] {};
		}
		return null;
	}
	
	private void makeList(String title, Enumeration files) {
		list = new List(title, List.IMPLICIT);
		if (file != null)
			list.addCommand(saveCmd);
		list.addCommand(loadCmd);
		if (file != null)
			list.addCommand(overwriteCmd);
		list.addCommand(backCmd);
		if (files != null) {
			while (files.hasMoreElements()) {
				String file = (String) files.nextElement();
				list.append(file, null);
			}
			if (!ROOT.equals(title) ) {
				list.append(UP_LABEL, null);
				list.append(ROOT_LABEL, null);
			}
		}
		list.setCommandListener(this);
		if (!DUMMY_LABEL.equals(title))
			display.setCurrent(list);
	}
	
	private void showError(Throwable e) {
		Alert alert = new Alert("Error", e.getMessage(),
				null, AlertType.ERROR);
		alert.setTimeout(Alert.FOREVER);
		display.setCurrent(alert);		
	}

	private void showInfo(String msg) {
		Alert alert = new Alert("Status Report", msg, null, AlertType.INFO);
		alert.setTimeout(Alert.FOREVER);
		alert.setCommandListener(this);
		display.setCurrent(alert);		
	}

	private int init(Module jbit) {
		this.jbit = jbit;
		display = (Display)jbit.opO(JBitSvc.OP_GET_DISPLAY, 0, null);
		return 0;
	}
	
	private int activate() {
		file = (byte[][])jbit.opO(JBitSvc.OP_POOL_GET, 0,
				JBitSvc.POOL_ID_PROGRAM_FILE);
		makeList(DUMMY_LABEL, null);
		terminate = false;
		workerStatus = WORKER_INIT;
		thread = new Thread(this);
		thread.start();
		return 0;
	}

	private int deactivate() {
		file = null;
		terminate = true;
		list = null;
		return 0;
	}

	private static final int MAX_NAME_LENGTH = 32;
	
	public void commandAction(Command c, Displayable d) {
		if (c == backCmd || d instanceof Alert) {
			back();
			return;
		}
		try {
			if (workerStatus != WORKER_WAIT)
				throw new Exception("Busy");
			if (c == List.SELECT_COMMAND) {
				String selected = list.getString(list.getSelectedIndex());
				if (ROOT_LABEL.equals(selected)) {
					path = ROOT;
				} else if (UP_LABEL.equals(selected)) {
					int index = path.lastIndexOf(separator, path.length() - 2);
					path = path.substring(0, index + 1);
				} else {
					if (selected.charAt(selected.length() - 1) != separator)
						return;
					path += selected;
				}
				workerStatus = WORKER_LIST;
			} else if (c == loadCmd || c == overwriteCmd) {
				String selected = list.getString(list.getSelectedIndex());
				if (ROOT_LABEL.equals(selected) || UP_LABEL.equals(selected)
						|| selected.charAt(selected.length() - 1) == separator)
					return;
				fileName = selected;
				if (c == loadCmd)
					workerStatus = WORKER_LOAD;
				else if (!selected.toLowerCase().endsWith(".jb"))
					throw new Exception("File must end with .jb");
				else
					workerStatus = WORKER_OVERWRITE;
			} else if (c == saveCmd) {
				TextBox dialog = new TextBox("Save",
						"", MAX_NAME_LENGTH, TextField.ANY);
				dialog.addCommand(okCmd);
				dialog.addCommand(cancelCmd);
				dialog.setCommandListener(this);
				display.setCurrent(dialog);
			} else if (c == okCmd || c == cancelCmd) {
				display.setCurrent(list);
				if (c == okCmd) {
					TextBox dialog = (TextBox)d;
					String selected = dialog.getString();
					if (selected.length() == 0 || selected.indexOf(separator) != -1)
						throw new Exception("Invalid name");
					if (!selected.toLowerCase().endsWith(".jb"))
						selected += ".jb";
					fileName = selected;
					workerStatus = WORKER_SAVE;
				}
			}
		} catch (Throwable e) {
			showError(e);
		}
	}

	public Object opO(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_GET_METADATA:
			return getMetadata(iArg);
		case OP_GET_DISPLAYABLE:
			return list;
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

	private void newProgram(Object oArg) {
		jbit.opI(JBitSvc.OP_NEW_PROGRAM, 0, oArg);
	}

	private static final String UNEXPECTED_EOF = "Unexpected EOF";
	private static final String INVALID_SIGNATURE = "Invalid Signature";
	private static final String INVALID_HEADER_LENGTH = "Invalid Header Length";
	private static final String INVALID_PROGRAM_LENGTH = "Invalid Program Length";
	
	private int copyByte(InputStream is, OutputStream os) throws Exception {
		int c = is.read();
		if (c == -1)
			throw new Exception(UNEXPECTED_EOF);
		os.write(c);
		return c;
	}

	private String load(InputStream is) {
		try {
			ByteArrayOutputStream os = new ByteArrayOutputStream();
			int i, j;
			if (copyByte(is, os) != JBFile.SIGNATURE_0
					|| copyByte(is, os) != JBFile.SIGNATURE_1
					|| copyByte(is, os) != JBFile.SIGNATURE_2
					|| copyByte(is, os) != JBFile.SIGNATURE_3)
				throw new Exception(INVALID_SIGNATURE);
			if (copyByte(is, os) != 0)
				throw new Exception(INVALID_HEADER_LENGTH);
			// if length reaches 256, it's time to upgrade JBit anyway
			int length = copyByte(is, os);
			if (length < 12)
				throw new Exception(INVALID_HEADER_LENGTH);
			copyByte(is, os); // Version Major
			copyByte(is, os); // Version Minor
			int codePages = copyByte(is, os);
			int dataPages = copyByte(is, os);
			int pages = codePages + dataPages;
			if (pages > 253)
				throw new Exception(INVALID_PROGRAM_LENGTH);
			for (i = 10; i < length; i++)
				copyByte(is, os);
			Vector v = new Vector();
			v.addElement(os.toByteArray());
			for (i = 0; i < pages; i++) {
				os.reset();				
				for (j = 0; j < 256; j++)
					copyByte(is, os);
				byte[] data = os.toByteArray();
				for (j = 255; j >= 0; j--)
					if (data[j] != 0)
						break;
				if (j == -1) {
					v.addElement(null);
				} else {
					v.addElement(data);
				}
			}
			is.close();
			// junk at the end is good; it might get meaning later on
			// (hint: adding/stripping a symbol table without touching the
			// rest of the file would be nice...)
			byte[][] file = new byte[v.size()][];
			for (i = 0; i < file.length; i++)
				file[i] = (byte[])v.elementAt(i);
			newProgram(file);
			return null;
		} catch (Throwable e) {
			return e.getMessage();
		}
	}
	
	private void writePage(OutputStream os, byte[] page) throws Exception {
		if (page != null)
			os.write(page);
		else
			os.write(new byte[256]);
	}
	
	private void save(OutputStream os) throws Exception {
		int codePages = file[0][JBFile.OFFSET_CODEPAGES] & 0xff;
		int dataPages = file[0][JBFile.OFFSET_DATAPAGES] & 0xff;
		int n = 1;
		os.write(file[0]);
		for (int i = 0; i < codePages; i++)
			writePage(os, file[n++]);
		for (int i = 0; i < dataPages; i++)
			writePage(os, file[n++]);
	}
	
	public void run() {
		while (!terminate) {
			FileConnection file = null;
			InputStream is = null;
			OutputStream os = null;
			String error;
			try {
				switch (workerStatus) {
				case WORKER_INIT:
					Thread.yield();
				case WORKER_LIST:
					try {
						if (ROOT.equals(path)) {
							makeList(path, FileSystemRegistry.listRoots());
						} else {
							file = (FileConnection)Connector.open(path, Connector.READ);
							makeList(path, file.list());
						}
					} catch (Exception e) {
						path = ROOT;
						makeList(path, FileSystemRegistry.listRoots());
					}
					workerStatus = WORKER_WAIT;
					break;
				case WORKER_LOAD:
					file = (FileConnection)Connector.open(path + fileName, Connector.READ);
					is = file.openInputStream();
					if ((error = load(is)) != null)
						throw new Exception(error);
					showInfo(fileName + " loaded.");
					workerStatus = WORKER_WAIT;
					break;
				case WORKER_OVERWRITE:
					file = (FileConnection)Connector.open(path + fileName, Connector.WRITE);
					os = file.openOutputStream();
					save(os);
					showInfo(fileName + " overwritten.");
					workerStatus = WORKER_WAIT;
					break;
				case WORKER_SAVE:
					file = (FileConnection)Connector.open(path + fileName, Connector.READ_WRITE);
					if (file.exists())
						throw new Exception("File already exists");
					file.create();
					os = file.openOutputStream();
					save(os);
					showInfo(fileName + " saved.");
					workerStatus = WORKER_WAIT;
					break;
				case WORKER_WAIT:
					try {
						Thread.sleep(100);
					} catch (Exception e) {
					}
					break;
				}
			} catch (Throwable e) {
				workerStatus = WORKER_WAIT;
				showError(e);
			} finally {
				if (is != null)
					try {
						is.close();
					} catch (Exception e) {
					}
				if (os != null)
					try {
						os.close();
					} catch (Exception e) {
					}
				if (file != null)
					try {
						file.close();
					} catch (Exception e) {
					}
			}
		}
	}

	public int get(int address) {
		return -1;
	}

	public int put(int address, int value) {
		return -1;
	}
}
