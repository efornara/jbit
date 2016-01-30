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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.util.Vector;

import javax.microedition.lcdui.Alert;
import javax.microedition.lcdui.AlertType;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Form;
import javax.microedition.lcdui.List;
import javax.microedition.lcdui.Screen;
import javax.microedition.lcdui.TextBox;
import javax.microedition.lcdui.TextField;
import javax.microedition.rms.RecordEnumeration;
import javax.microedition.rms.RecordStore;


public class Store implements Module, CommandListener {

	private Command infoCmd = new Command("Info", Command.ITEM, 1);
	private Command editCmd = new Command("Load&Edit", Command.ITEM, 2);
	private Command runCmd = new Command("Load&Run", Command.ITEM, 3);
	private Command loadCmd = new Command("Load", Command.ITEM, 4);
	private Command copyCmd = new Command("Copy", Command.ITEM, 5);
	private Command renameCmd = new Command("Rename", Command.ITEM, 7);
	private Command deleteCmd = new Command("Delete", Command.ITEM, 6);
	private Command overwriteCmd = new Command("Overwrite", Command.ITEM, 8);
	private Command saveCmd = new Command("Save", Command.SCREEN, 1);
	private Command listRecsCmd = new Command("ListRecs", Command.SCREEN, 2);
	private Command backCmd = new Command("Back", Command.BACK, 1);
	private Command okCmd = new Command("OK", Command.OK, 1);
	private Command cancelCmd = new Command("Cancel", Command.CANCEL, 2);

	private Module jbit;
	private Display display;
	private boolean canEdit;
	private boolean canRun;

	private RecordStore store;
	private int used = -1;
	private Vector recNames = new Vector();
	private Vector recIds = new Vector();
	private int progId = -1;
	private String progName;

	private byte[][] file;
	private List list;
	
	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "Store";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_TOP_LEVEL);
		case METADATA_ID_SERVICES:
			return new String[] { SaveSvc.TAG };
		default:
			return null;
		}
	}
	
	private void openStore() throws Throwable {
		if (store == null) {
			store = RecordStore.openRecordStore(Const.STORE_NAME, true);
		}
	}
	
	private void closeStore() {
		if (store != null) {
			try {
				try {
					int size = store.getSize();
					int available = store.getSizeAvailable();
					if (size + available != 0)
						used = (1000 * size / (size + available) + 5) / 10;
				} catch (Throwable e) {
				}
				store.closeRecordStore();
			} catch (Throwable e) {
			}
			store = null;
		}
	}

	private int getRecordSize(String name) throws Throwable {
		boolean wasOpen = true;
		try {
			if (store == null) {
				wasOpen = false;
				openStore();
			}
			for (int i = 0; i < recNames.size(); i++)
				if (recNames.elementAt(i).equals(name)) {
					int recId = ((Integer)recIds.elementAt(i)).intValue();
					return store.getRecordSize(recId);
				}
			return 0;
		} finally {
			if (!wasOpen)
				closeStore();
		}
	}

	private byte[] loadRecord(String name) throws Throwable {
		boolean wasOpen = true;
		try {
			if (store == null) {
				wasOpen = false;
				openStore();
			}
			for (int i = 0; i < recNames.size(); i++)
				if (recNames.elementAt(i).equals(name)) {
					int recId = ((Integer)recIds.elementAt(i)).intValue();
					return store.getRecord(recId);
				}
			return null;
		} finally {
			if (!wasOpen)
				closeStore();
		}
	}

	private int saveRecord(int recId, byte[] data) throws Throwable {
		boolean wasOpen = true;
		try {
			if (store == null) {
				wasOpen = false;
				openStore();
			}
			if (recId <= 0)
				return store.addRecord(data, 0, data.length);
			else {
				store.setRecord(recId, data, 0, data.length);
				return recId;
			}
		} finally {
			if (!wasOpen)
				closeStore();
		}
	}
	
	private void saveRecord(String name, byte[] data) throws Throwable {
		boolean wasOpen = true;
		try {
			if (store == null) {
				wasOpen = false;
				openStore();
			}
			for (int i = 0; i < recNames.size(); i++)
				if (recNames.elementAt(i).equals(name)) {
					int recId = ((Integer)recIds.elementAt(i)).intValue();
					saveRecord(recId, data);
					return;
				}
		} finally {
			if (!wasOpen)
				closeStore();
		}
	}	

	public void deleteRecord(String name) throws Throwable {
		boolean wasOpen = true;
		try {
			if (store == null) {
				wasOpen = false;
				openStore();
			}
			for (int i = 0; i < recNames.size(); i++)
				if (recNames.elementAt(i).equals(name)) {
					int recId = ((Integer)recIds.elementAt(i)).intValue();
					store.deleteRecord(recId);
				}
		} finally {
			if (!wasOpen)
				closeStore();
		}
	}

	private String parseStore(char parseType) {
		String ret = "Records:\n";
		boolean found = false;
		try {
			openStore();
			RecordEnumeration re = store.enumerateRecords(null, null, false);
			while (re.hasNextElement()) {
				int recId = re.nextRecordId();
				byte[] data = store.getRecord(recId);
				ByteArrayInputStream bais = new ByteArrayInputStream(data);
				DataInputStream is = new DataInputStream(bais);
				String name = is.readUTF();
				if (name.length() == 0)
					continue;
				char type = name.charAt(0);
				if (type != parseType)
					continue;
				if (type == Const.RECORD_TYPE_PROGRAM) {
					recNames.addElement(name);
					recIds.addElement(new Integer(recId));
				} else { // Const.RECORD_TYPE_DATA
					ret += name.substring(1) + " (" + store.getRecordSize(recId) + ")\n";
					found = true;
				}
			}
			if (!found)
				ret += "- none -";
			return ret;
		} catch (Throwable e) {
			return null;
		} finally {
			closeStore();
		}
	}
	
	private int init(Module jbit) {
		this.jbit = jbit;
		display = (Display)jbit.opO(JBitSvc.OP_GET_DISPLAY, 0, null);
		if (jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, EditSvc.TAG) != null)
			canEdit = true;
		if (jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, VMSvc.TAG) != null)
			canRun = true;
		return parseStore(Const.RECORD_TYPE_PROGRAM) == null ? -1 : 0;
	}
	
	private void updateView() {
		list = new List(null, List.IMPLICIT);
		if (used == -1)
			list.setTitle("Store");
		else
			list.setTitle("Store (" + used + "% used)");
		for (int i = 0; i < recNames.size(); i++)
			list.append(((String)recNames.elementAt(i)).substring(1), null);
		if (list.size() == 0) {
			list.append("- empty - ", null);
		} else {
			list.addCommand(infoCmd);
			if (canEdit)
				list.addCommand(editCmd);
			if (canRun)
				list.addCommand(runCmd);
			list.addCommand(loadCmd);
			if (file != null)
				list.addCommand(overwriteCmd);
			list.addCommand(copyCmd);
			list.addCommand(renameCmd);
			list.addCommand(deleteCmd);
		}
		if (file != null)
			list.addCommand(saveCmd);
		list.addCommand(listRecsCmd);
		list.addCommand(backCmd);
		list.setCommandListener(this);
	}
	
	private int activate() {
		file = (byte[][])jbit.opO(JBitSvc.OP_POOL_GET, 0,
				JBitSvc.POOL_ID_PROGRAM_FILE);
		updateView();
		return 0;
	}

	private int deactivate() {
		list = null;
		file = null;
		return 0;
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

	private void showConfirmation(String msg, Displayable nextDisplayable) {
		Alert alert = new Alert("", msg + "!", null, AlertType.CONFIRMATION);
		// #if MICROEMULATOR
		// workaround; "abc" get stuck in microemulator
		// display.setCurrent(nextDisplayable); fixed?
		// #endif
		display.setCurrent(alert, nextDisplayable);
	}

	private boolean isNameUnique(String name) {
		for (int i = 0; i < recNames.size(); i++)
			if (recNames.elementAt(i).equals(name))
				return false;
		return true;
	}
	
	private void saved(String name) {
		progName = name;
		progId = ((Integer)jbit.opO(JBitSvc.OP_POOL_GET,
				0, JBitSvc.POOL_ID_PROGRAM_ID)).intValue();
		jbit.opI(JBitSvc.OP_POOL_PUT, 0, new Object[] {
				JBitSvc.POOL_ID_PROGRAM_MODIFIED, new Boolean(false)
		});
	}
	
	/*
	 * Every record is the name of the file, followed by the format version
	 * and by a stream of pages so encoded:
	 * first there is a single unsigned byte storing:
	 *   - 0, if the page is empty
	 *   - 1-255, if the page is 2-256 byte long
	 * then the page content (0 or 2-256 bytes) follows.
	 * The first page is the header, followed by the code pages,
	 * followed by the data pages.
	 */
	
	private static final byte RECORD_FORMAT = 1;

	private static final String UNEXPECTED_EOF = "Unexpected EOF";
	private static final String INVALID_HEADER_LENGTH = "Invalid Header Length";

	private void createFileFromRecord(String name) throws Throwable {
		byte[] data = loadRecord(name);
		ByteArrayInputStream bais = new ByteArrayInputStream(data);
		DataInputStream dis = new DataInputStream(bais);
		dis.readUTF(); // skip file name
		dis.read(); // skip version
		Vector v = new Vector();
		int n = 0, len;
		while ((len = dis.read()) != -1) {
			if (len == 0) {
				if (n == 0)
					throw new Exception(INVALID_HEADER_LENGTH);
				v.addElement(null);
			} else {
				byte[] p = new byte[n == 0 ? len + 1 : 256];
				for (int i = 0; i <= len; i++) {
					int value = dis.read();
					if (value == -1)
						throw new Exception(UNEXPECTED_EOF);
					p[i] = (byte)value;
				}
				v.addElement(p);
			}
			n++;
		}
		if (n == 0)
			throw new Exception(UNEXPECTED_EOF);
		file = new byte[n][];
		for (int i = 0; i < n; i++)
			file[i] = (byte[])v.elementAt(i);
		return;
	}
	
	private byte[] createRecordFromFile(String name) throws Throwable {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		DataOutputStream os = new DataOutputStream(baos);
		os.writeUTF(name);
		os.write(RECORD_FORMAT);
		os.write(file[0].length - 1);
		os.write(file[0]);
		int codePages = file[0][JBFile.OFFSET_CODEPAGES] & 0xFF;
		int dataPages = file[0][JBFile.OFFSET_DATAPAGES] & 0xFF;
		int pages = codePages + dataPages;
		for (int i = 1; i <= pages; i++) {
			if (file[i] != null) {
				byte[] p = file[i];
				int n;
				for (n = 255; n >= 0; n--)
					if (p[n] != 0)
						break;
				if (n == -1) {
					os.write(0);
				} else {
					n++; // n is now the number of significant bytes (1-256)
					if (n == 1) // normalize n to 2-256
						n++;
					os.write(n - 1);
					os.write(p, 0, n);
				}
			} else {
				os.write(0);
			}
		}
		os.flush();
		return baos.toByteArray();
	}

	private byte[] createRecordFromTemplate(String name, InputStream is)
			throws Throwable {
		DataInputStream dis = new DataInputStream(is);
		dis.readUTF();
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		DataOutputStream os = new DataOutputStream(baos);
		os.writeUTF(name);
		int c;
		while ((c = dis.read()) != -1)
			os.write(c);
		os.flush();
		return baos.toByteArray();
	}

	private void showInfoDialog(String name) {
		StringBuffer info = new StringBuffer();
		String label = name.substring(1);
		int used = -1, recSize, size;
		try {
			openStore();
			recSize = getRecordSize(name);
			size = store.getSize() + store.getSizeAvailable();
			if (size != 0)
				used = (1000 * recSize / size + 5) / 10;
		} catch (Throwable e) {
			showError("Failed loading " + label, list);
			return;
		} finally {
			closeStore();
		}
		info.append(label + " uses " + recSize + " bytes of your store");
		if (used != -1) {
			info.append(" (" + used + "% of " + size + " bytes).\n");
		} else {
			info.append(".\n");
		}
		Alert alert = new Alert("Info", info.toString(), null, AlertType.INFO);
		alert.setTimeout(Alert.FOREVER);
		display.setCurrent(alert, list);
	}

	private void showListRecsDialog() {
		Alert alert = new Alert("ListRecs", parseStore(Const.RECORD_TYPE_DATA), null, AlertType.INFO);
		alert.setTimeout(Alert.FOREVER);
		display.setCurrent(alert, list);
	}

	private boolean handleLoad(String name) {
		try {
			createFileFromRecord(name);
			progId = jbit.opI(JBitSvc.OP_NEW_PROGRAM, 0, file);
			progName = name;
			return true;
		} catch (Throwable e) {
			showError("Failed loading " + name.substring(1), list);
			return false;
		}
	}
	
	private static final int MAX_NAME_LENGTH = 32;
	
	private static final String ACTION_SAVE = "Save";
	private static final String ACTION_COPY = "Copy";
	private static final String ACTION_RENAME = "Rename";
	private static final String ACTION_DELETE = "Delete";
	private static final String ACTION_OVERWRITE = "Overwrite";

	private TextBox namedOpDlg;
	private String namedOpAction;
	private String namedOpName;
	
	private void showNamedOpDialog(String action, String name) {
		namedOpDlg = new TextBox("Name (" + action + ")",
				"", MAX_NAME_LENGTH, TextField.ANY);
		namedOpAction = action;
		namedOpName = name;
		namedOpDlg.addCommand(okCmd);
		namedOpDlg.addCommand(cancelCmd);
		namedOpDlg.setCommandListener(this);
		display.setCurrent(namedOpDlg);
	}
	
	private void handleNamedOpDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				String label = namedOpDlg.getString();
				if (label.length() == 0) {
					retry = true;
					showError("Name cannot be empty", namedOpDlg);
					return;
				}
				String name = Const.RECORD_TYPE_PROGRAM + label;
				if (!isNameUnique(name)) {
					retry = true;
					showError(label + " already exists", namedOpDlg);
					return;
				}
				byte[] data = null;
				if (namedOpAction.equals(ACTION_COPY)
						|| namedOpAction.equals(ACTION_RENAME)) {
					InputStream is;
					try {
						data = loadRecord(namedOpName);
						is = new ByteArrayInputStream(data);
					} catch (Throwable t) {
						retry = true;
						showError("Failed loading "
								+ namedOpName.substring(1), namedOpDlg);
						return;
					}
					try {
						data = createRecordFromTemplate(name, is);
					} catch (Throwable t) {
						retry = true;
						showError("Failed creating a record from "
								+ namedOpName.substring(1), namedOpDlg);
						return;
					}
				} else {
					// ACTION_SAVE
					try {
						data = createRecordFromFile(name);
					} catch (Throwable t) {
						retry = true;
						showError("Failed creating a new record",
								namedOpDlg);
						return;
					}
				}
				try {
					if (namedOpAction.equals(ACTION_RENAME)) {
						saveRecord(namedOpName, data);
						for (int i = 0; i < recNames.size(); i++) {
							if (recNames.elementAt(i).equals(namedOpName)) {
								recNames.setElementAt(name, i);
								break;
							}
						}
						updateView();
						showConfirmation(namedOpName.substring(1)
								+ " renamed " + label, list);
					} else {
						// ACTION_COPY or ACTION_SAVE
						int recId = saveRecord(0, data);
						recNames.addElement(name);
						recIds.addElement(new Integer(recId));
						updateView();
						if (namedOpAction.equals(ACTION_COPY)) {
							showConfirmation(namedOpName.substring(1)
									+ " copied to " + label, list);
						} else {
							saved(name);
							showConfirmation(label + " saved", list);
						}
					}
				} catch (Throwable t) {
					retry = true;
					showError("Failed saving " + label, namedOpDlg);
					return;
				}
			} else {
				display.setCurrent(list);
			}
		} finally {
			if (!retry) {
				namedOpDlg = null;
				namedOpAction = null;
				namedOpName = null;
			}
		}
	}
	
	private Screen confirmedOpDlg;
	private String confirmedOpAction;
	private String confirmedOpName;
	
	private void showConfirmedOpDialog(String action, String name) {
		confirmedOpAction = action;
		confirmedOpName = name;
		Form form = new Form(action);
		form.append("Do you really want to " + action.toLowerCase() + " "
				+ name.substring(1) + "?");
		confirmedOpDlg = form;
		confirmedOpDlg.addCommand(okCmd);
		confirmedOpDlg.addCommand(cancelCmd);
		confirmedOpDlg.setCommandListener(this);
		display.setCurrent(confirmedOpDlg);
	}
	
	private void handleConfirmedOpDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				try {
					String label = confirmedOpName.substring(1);
					if (confirmedOpAction == ACTION_DELETE) {
						deleteRecord(confirmedOpName);
						for (int i = 0; i < recNames.size(); i++) {
							if (recNames.elementAt(i).equals(confirmedOpName)) {
								recNames.removeElementAt(i);
								recIds.removeElementAt(i);
								break;
							}
						}
						updateView();
						showConfirmation(label + " deleted", list);
					} else {
						// ACTION_OVERWRITE
						byte[] data = createRecordFromFile(confirmedOpName);
						saveRecord(confirmedOpName, data);
						saved(confirmedOpName);
						updateView();
						showConfirmation(label + " overwritten", list);
					}
				} catch (Throwable t) {
					retry = true;
					showError("Failed to "
							+ confirmedOpAction.toLowerCase() + " "
							+ confirmedOpName.substring(1),
							confirmedOpDlg);
					return;
				}
			} else {
				display.setCurrent(list);
			}
		} finally {
			if (!retry) {
				confirmedOpDlg = null;
				confirmedOpAction = null;
				confirmedOpName = null;
			}
		}
	}
	
	private boolean saveNewName;
	private Displayable saveNextDisplayable;
	private CommandListener saveListener;
	private Command saveOkCommand;
	private Command saveFailedCommand;
	
	private void disposeSaveResources() {
		file = null;
		saveNextDisplayable = null;
		saveListener = null;
		saveOkCommand = null;
		saveFailedCommand = null;
	}
	
	private void doSave() {
		Command saveCmd = saveFailedCommand;
		String label = progName.substring(1);
		try {
			byte[] data;
			data = createRecordFromFile(progName);
			saveRecord(progName, data);
			saved(progName);
			showConfirmation(label + " saved", saveNextDisplayable);
			saveCmd = saveOkCommand;
		} catch (Throwable t) {
			showError("Failed to save " + label, saveNextDisplayable);
		} finally {
			if (saveListener != null)
				saveListener.commandAction(saveCmd, null);
			disposeSaveResources();			
		}
	}
	
	private TextBox saveAsDlg;
	
	private void showSaveAsDialog() {
		saveAsDlg = new TextBox("Name (Save)",
				"", MAX_NAME_LENGTH, TextField.ANY);
		saveAsDlg.addCommand(okCmd);
		saveAsDlg.addCommand(cancelCmd);
		saveAsDlg.setCommandListener(this);
		display.setCurrent(saveAsDlg);
	}
	
	private void handleSaveAsDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				String label = saveAsDlg.getString();
				if (label.length() == 0) {
					retry = true;
					showError("Name cannot be empty", saveAsDlg);
					return;
				}
				String name = Const.RECORD_TYPE_PROGRAM + label;
				if (!isNameUnique(name)) {
					retry = true;
					showError(label + " already exists", saveAsDlg);
					return;
				}
				byte[] data;
				try {
					data = createRecordFromFile(name);
				} catch (Throwable t) {
					retry = true;
					showError("Failed creating a new record",
							saveAsDlg);
					return;
				}
				try {
					int recId = saveRecord(0, data);
					recNames.addElement(name);
					recIds.addElement(new Integer(recId));
					saved(name);
					showConfirmation(label + " saved", saveNextDisplayable);
				} catch (Throwable t) {
					retry = true;
					showError("Failed to save " + label, saveAsDlg);
					return;
				}
			}
			if (saveListener != null) {
				Command saveCmd;
				if (c == okCmd)
					saveCmd = saveOkCommand;
				else
					saveCmd = saveFailedCommand;
				saveListener.commandAction(saveCmd, null);
			}
			if (c != okCmd)
				display.setCurrent(saveNextDisplayable);
			disposeSaveResources();
		} finally {
			if (!retry) {
				saveAsDlg = null;
			}
		}
	}
	
	private int save(Object oArg) {
		try {
			file = (byte[][])jbit.opO(JBitSvc.OP_POOL_GET, 0,
					JBitSvc.POOL_ID_PROGRAM_FILE);
			Object[] args = (Object[])oArg;
			saveNewName = ((Boolean)args[0]).booleanValue();
			saveNextDisplayable = (Displayable)args[1];
			saveListener = (CommandListener)args[2];
			saveOkCommand = (Command)args[3];
			saveFailedCommand = (Command)args[4];
			int id = ((Integer)jbit.opO(JBitSvc.OP_POOL_GET,
					0, JBitSvc.POOL_ID_PROGRAM_ID)).intValue();
			if (id == progId && progName != null && !saveNewName) {
				try {
					doSave();
				} catch (Throwable e) {
					if (saveListener != null)
						saveListener.commandAction(saveFailedCommand, null);
				} finally {
					disposeSaveResources();
				}
				return SaveSvc.RETCODE_EXECUTED;
			} else {
				showSaveAsDialog();
				return SaveSvc.RETCODE_PENDING;
			}
		} catch (Throwable e) {
			disposeSaveResources();
			return SaveSvc.RETCODE_INVPAR;
		}
	}
	
	public void commandAction(Command c, Displayable d) {
		if (d == saveAsDlg) {
			handleSaveAsDialog(c);
		} else if (d == namedOpDlg) {
			handleNamedOpDialog(c);
		} else if (d == confirmedOpDlg) {
			handleConfirmedOpDialog(c);
		} else if (d == list) {
			int index = list.getSelectedIndex();
			String name = Const.RECORD_TYPE_PROGRAM + list.getString(index);
			if (c == List.SELECT_COMMAND && recNames.size() != 0) {
				boolean loaded = false;
				if (canEdit || canRun)
					loaded = handleLoad(name);
				else
					showInfoDialog(name);
				if (loaded) {
					if (canEdit)
						jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, EditSvc.TAG);
					else
						jbit.opI(JBitSvc.OP_REPLACE_WITH_VM,
								VMSvc.START_MODE_RUN, this);
				}
			} else if (c == infoCmd) {
				showInfoDialog(name);
			} else if (c == editCmd) {
				if (handleLoad(name))
					jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, EditSvc.TAG);
			} else if (c == runCmd) {
				if (handleLoad(name))
					jbit.opI(JBitSvc.OP_REPLACE_WITH_VM,
							VMSvc.START_MODE_RUN, this);
			} else if (c == loadCmd) {
				if (handleLoad(name))
					showConfirmation(name.substring(1) + " loaded", list);
			} else if (c == copyCmd) {
				showNamedOpDialog(ACTION_COPY, name);
			} else if (c == renameCmd) {
				showNamedOpDialog(ACTION_RENAME, name);
			} else if (c == deleteCmd) {
				showConfirmedOpDialog(ACTION_DELETE, name);
			} else if (c == overwriteCmd) {
				showConfirmedOpDialog(ACTION_OVERWRITE, name);
			} else if (c == saveCmd) {
				showNamedOpDialog(ACTION_SAVE, null);
			} else if (c == listRecsCmd) {
				showListRecsDialog();
			} else if (c == backCmd) {
				jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
			} else {
			}
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
		case SaveSvc.OP_SAVE:
			return save(oArg);
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
