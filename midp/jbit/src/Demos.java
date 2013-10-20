// #condition !JBIT_RUNTIME && !JBIT_MICRO

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

import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Vector;

import javax.microedition.lcdui.Alert;
import javax.microedition.lcdui.AlertType;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.List;


public class Demos implements Module, CommandListener {
	
	private Command infoCmd = new Command("Info", Command.ITEM, 1);
	private Command runCmd = new Command("Load&Run", Command.ITEM, 2);
	private Command debugCmd = new Command("Load&Debug", Command.ITEM, 3);
	private Command loadCmd = new Command("Load", Command.ITEM, 4);
	private Command backCmd = new Command("Back", Command.BACK, 1);
	
	private Module jbit;
	private Display display;

	private String labels[];
	private String files[];
	private String infos[];
	private List list;
	private boolean hasFolders;
	private int folder = -1;
	private int item = -1;
	
	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "Demos";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_TOP_LEVEL);
		case METADATA_ID_SERVICES:
			return new String[] {};
		default:
			return null;
		}
	}
	
	private String[] vectorToArray(Vector v) {
		int n = v.size();
		String a[] = new String[n];
		for (int i = 0; i < n; i++)
			a[i] = (String)v.elementAt(i);
		return a;
	}
	
	private int parseDemosFile() {
		try {
			Vector lv = new Vector();
			Vector fv = new Vector();
			Vector iv = new Vector();
			StringBuffer line = new StringBuffer();
			String label = null, file = null;
			StringBuffer info = new StringBuffer();
			InputStream is = getClass().getResourceAsStream("demos.txt");
			InputStreamReader ir = new InputStreamReader(is);
			int c;
			while ((c = ir.read()) != -1) {
				if (c == '\r')
					continue;
				if (c == '\n') {
					if (line.charAt(0) == '#') {
						if (label != null) {
							lv.addElement(label);
							fv.addElement(file);
							iv.addElement(info.toString());
						}
						file = line.toString().substring(1);
						int index = file.indexOf('#'); 
						if (index != -1) {
							label = file.substring(index + 1);
							file = file.substring(0, index);
						} else {
							index = file.indexOf(".jb");
							if (index != -1)
								label = file.substring(0, index);
							else
								label = file;
						}
						if (file.equals("/"))
							hasFolders = true;
						info.setLength(0);
					} else {
						info.append(line.toString());
						info.append('\n');
					}
					line.setLength(0);
				} else {
					line.append((char)c);
				}
			}
			is.close();
			if (label != null) {
				// no empty folders allowed
				lv.addElement(label);
				fv.addElement(file);
				iv.addElement(info.toString());
			}
			labels = vectorToArray(lv);
			files = vectorToArray(fv);
			infos = vectorToArray(iv);
			return 0;
		} catch (Throwable e) {
			return -1;
		}
	}
	
	private void makeList() {
		String title;
		if (folder == -1)
			title = "Demos";
		else
			title = labels[folder] + " (Demos)";		
		list = new List(title, List.IMPLICIT);
		for (int i = 0; i < labels.length; i++) {
			if (!hasFolders || folder != -1) {
				// show files
				if (i <= folder)
					continue; // skip prev. files/folders
				if (files[i].equals("/"))
					break; // stop on next folder
			} else {
				// show folders
				if (!files[i].equals("/"))
					continue; // skip files
			}
			list.append(labels[i], null);
		}
		if (item != -1 && item < list.size())
			list.setSelectedIndex(item, true);
		list.addCommand(infoCmd);
		if (!hasFolders || folder != -1) {
			if (jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, VMSvc.TAG) != null)
				list.addCommand(runCmd);
			if (jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, MonitorSvc.TAG) != null)
				list.addCommand(debugCmd);
			list.addCommand(loadCmd);
		}
		list.addCommand(backCmd);
		list.setCommandListener(this);
	}
	
	private void enterFolder(int folder) {
		this.folder = folder;
		item = -1;
		makeList();
		display.setCurrent(list);
	}
	
	private int listIndexToDemoIndex(int listIndex) {
		int demoIndex;
		if (hasFolders && folder == -1) {
			listIndex++;
			for (demoIndex = 0; demoIndex < files.length; demoIndex++) {
				if (files[demoIndex].equals("/"))
					listIndex--;
				if (listIndex == 0)
					break;
			}
		} else {
			demoIndex = folder + listIndex + 1;
		}
		return demoIndex;
	}
	
	private int init(Module jbit) {
		this.jbit = jbit;
		display = (Display)jbit.opO(JBitSvc.OP_GET_DISPLAY, 0, null);
		return parseDemosFile();
	}
	
	private int activate() {
		makeList();
		return 0;
	}

	private int deactivate() {
		list = null;
		return 0;
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
	
	private boolean load(String label, String file) {
		String error;
		InputStream is = getClass().getResourceAsStream(file);
		if (is == null)
			error = "Resource " + file + " not found";
		else
			error = (String)jbit.opO(JBitSvc.OP_LOAD_PROGRAM, 0, is);
		if (error != null) {
			Alert alert = new Alert(label, error + "!",
					null, AlertType.ERROR);
			alert.setTimeout(Alert.FOREVER);
			display.setCurrent(alert);
			return false;
		}
		return true;
	}

	public void commandAction(Command c, Displayable d) {
		if (d == list) {
			item = list.getSelectedIndex();
			int index = listIndexToDemoIndex(item);
			boolean isFolder = files[index].equals("/"); 
			if (c == List.SELECT_COMMAND) {
				if (isFolder) {
					enterFolder(index);
				} else {
					if (load(labels[index], files[index])) {
						if (jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, VMSvc.TAG) != null)
							jbit.opI(JBitSvc.OP_REPLACE_WITH_VM,
									VMSvc.START_MODE_RUN, this);
						else
							jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
					}
				}
			} else if (c == infoCmd) {
				String text = infos[index];
				if (text.length() == 0)
					text = "No info available.";
				Alert info = new Alert(labels[index],
						text, null, AlertType.INFO);
				info.setTimeout(Alert.FOREVER);
				display.setCurrent(info);
			} else if (c == runCmd && !isFolder) {
				if (load(labels[index], files[index]))
					jbit.opI(JBitSvc.OP_REPLACE_WITH_VM,
							VMSvc.START_MODE_RUN, this);
			} else if (c == debugCmd && !isFolder) {
				if (load(labels[index], files[index]))
					jbit.opI(JBitSvc.OP_REPLACE_WITH_VM,
							VMSvc.START_MODE_DEBUG, this);
			} else if (c == loadCmd && !isFolder) {
				if (load(labels[index], files[index])) {
					jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
					folder = -1;
					item = -1;
				}
			} else if (c == backCmd) {
				if (folder == -1)
					jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
				else
					enterFolder(-1);
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
