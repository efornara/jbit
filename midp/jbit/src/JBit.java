/*
	JBit - A 6502 framework for mobile phones
	Copyright (C) 2007-2017  Emanuele Fornara
	
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
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.Vector;

import javax.microedition.lcdui.*;
import javax.microedition.midlet.MIDlet;
import javax.microedition.midlet.MIDletStateChangeException;

//#if !JBIT_RUNTIME 
import java.util.Hashtable;
//#endif

public final class JBit extends MIDlet implements CommandListener
// #if !JBIT_RUNTIME
, Module
// #endif
{
	
	/*
	 * If fatalError is not null JBit is in serious trouble.
	 * The only sensible thing we can do is to show the error to the user
	 * and terminate the MIDlet when the user acknowledges the error.
	 * 
	 * Alerts in MIDP1 would not do; we need to be notified when the user
	 * acknowledges the error and, afaik, Alerts in MIDL1 cannot do that.
	 */

	private Displayable fatalError;

	private void setupFatalError(String error) {
		// #if JavaPlatform == "MIDP/1.0"
//@		Form form = new Form("JBit");
//@		form.append("Error:\n");
//@		form.append(error);
//@		form.setCommandListener(this);
//@		fatalError = form;
		// #else
		Alert alert = new Alert("JBit", error, null, AlertType.ERROR);
		alert.setTimeout(Alert.FOREVER);
		alert.setCommandListener(this);
		fatalError = alert;
		// #endif
		fatalError.addCommand(new Command("Exit", Command.EXIT, 1));
	}
	
	private void showFatalError(String error, Throwable e) {
		String msg;
		if (e != null)
			msg = error + " (" + e.getMessage() + ") !";
		else
			msg = error + "!";
		setupFatalError(msg);
		Display.getDisplay(this).setCurrent(fatalError);
	}
	
	/*
	 * Including the full license would be nice, but since we are in a very
	 * limited platform we opt for a few attributes that should tell enough
	 * to a lot of people.
	 * 
	 * JBit-URL is all an interested party needs to know to find out more
	 * about JBit.
	 * 
	 * JBit-License is enough for most programmers and, I presume,
	 * a significant number of users to grasp the terms of how JBit
	 * is distributed even without bothering to visit the web site.
	 */

	private static final String JBitURLName = "JBit-URL";
	private static final String JBitURLValue = "http://jbit.sourceforge.net/";
	private static final String JBitLicName = "JBit-License";
	private static final String JBitLicValue = "GNU LGPL 2.1";
	
	private boolean checkAttribute(String name, String value) {
		String s;
		s = getAppProperty(name);
		if (s == null || !s.equals(value)) {
			setupFatalError("Attribute " + name
					+ " in descriptor must be set to " + value);
			return false;
		}
		return true;
	}

	private String autorun = null;
	
	private void parseAutoRun() {
		autorun = getAppProperty("JBit-AutoRun");
		if (autorun == null) {
			// #if JBIT_RUNTIME
//@			setupFatalError("AutoRun not configured in descriptor");
			// #endif
		}
	}

	/*
	 * The standard JBit has the full loading and unloading of modules
	 * enabled and is by far the preferred version of JBit.
	 * 
	 * Still... a requirement is that JBit could be packed in a 30K Jar.
	 * It is true that the standard JBit without interactive tools is below 30K,
	 * but on that kind of devices every K counts and dynamic configuration
	 * and flexible loading of modules might need to be disabled.
	 * 
	 * We might well end up scrapping JBIT_RUNTIME... time will tell.
	 */
	
	// #if !JBIT_RUNTIME

	private static final int UNLOADED = 0;
	private static final int LOADED = 1;
	private static final int INITIALIZED = 2;
	private static final int ACTIVE = 3;
	private static final int FAILED = 4;
	
	private Hashtable pool = new Hashtable();
	private String[] classes;
	private String[] labels;
	private int[] types;
	private Module[] modules;
	private String[][] services;
	private int states[];
	private int activeId = -1;	
	private int n;

	private void parseModuleList(Vector c) {
		c.addElement("JBit");
		n++;
		while (true) {
			String name = getAppProperty("JBit-Module-" + n);
			if (name == null)
				break;
			for (int j = 0; j < c.size(); j++) {
				if (name.equals(c.elementAt(j))) {
					setupFatalError("Duplicated module " + name);
					return;
				}
			}
			c.addElement(name);
			n++;
		}
		if (n == 1) {
			c.addElement("VM");
			c.addElement("CPU");
			n += 2;
		}
	}
	
	private void setupModules(Vector c) {
		classes = new String[n];
		for (int i = 0; i < n; i++)
			classes[i] = (String)c.elementAt(i);
		labels = new String[n];
		labels[0] = "JBit";
		types = new int[n];
		types[0] = TYPE_MIDLET;
		modules = new Module[n];
		modules[0] = this;
		services = new String[n][];
		services[0] = new String[] { "JBit" };
		states = new int[n];
		states[0] = INITIALIZED;
		for (int i = 1; i < n; i++)
			states[i] = UNLOADED;
	}
	
	private Module loadModuleByClassName(String name, boolean mandatory) {
		try {
			Class cls = Class.forName(name);
			return (Module)cls.newInstance();
		} catch (Throwable e) {
			if (mandatory)
				showFatalError(name + " could not be loaded", e);
			return null;
		}
	}
	
	private boolean initModules() {
		for (int i = 0; i < n; i++) {
			if (modules[i] == null && states[i] != FAILED) {
				modules[i] = loadModuleByClassName(classes[i], false);
				if (modules[i] == null)
					states[i] = FAILED;
				else {
					try {
						labels[i] = (String)modules[i].opO(OP_GET_METADATA, METADATA_ID_LABEL, null);
						types[i] = ((Integer)modules[i].opO(OP_GET_METADATA, METADATA_ID_TYPE, null)).intValue();
						services[i] = (String[])modules[i].opO(OP_GET_METADATA, METADATA_ID_SERVICES, null);
						states[i] = LOADED;
					} catch (RuntimeException e) {
						states[i] = FAILED;
					}
				}
			}
		}
		for (int i = 0; i < n; i++) {
			if (states[i] == LOADED) {
				try {
					if (modules[i].opI(OP_INIT, 0, this) == 0)
						states[i] = INITIALIZED;
					else
						states[i] = FAILED;
				} catch (RuntimeException e) {
					states[i] = FAILED;
				}
			}
		}
		return true;
	}

	/*
	private Module getModuleByClassName(String name) {
		for (int i = 0; i < n; i++)
			if (classes[i].equals(name))
				return modules[i];
		return null;
	}
	*/
	
	private int getModuleIdByService(String service) {
		for (int i = 0; i < n; i++) {
			String[] s = services[i];
			if (s != null) {
				for (int j = 0; j < s.length; j++)
					if (s[j].equals(service))
						return i;
			}
		}
		return -1;
	}
	
	private Module getModuleById(int id) {
		return modules[id];
	}
	
	// id is the module to activate, or -1 if pauseApp has been called and
	// the current active module should be deactivated without
	// any other module taking its place.
	private boolean activateModule(int id) {
		Module m;
		try {
			if (activeId != -1) {
				m = modules[activeId];
				m.opI(OP_DEACTIVATE, 0, null);
				states[activeId] = INITIALIZED;
			}
		} catch (RuntimeException e) {
			showFatalError("Unexpected exception during deactivation of "
					+ modules[activeId], e);
			return false;
		}
		if (id == -1)
			return true;
		try {
			m = modules[id];
			if (states[id] == INITIALIZED) {
				if (m.opI(OP_ACTIVATE, 0, null) == 0)
					states[id] = ACTIVE;
				else
					states[id] = INITIALIZED;
			}
			if (states[id] != ACTIVE) {
				// TODO call GET_ERROR
				throw new RuntimeException("OP_ACTIVATE failed");
			}
			activeId = id;
			Displayable d = (Displayable)m.opO(OP_GET_DISPLAYABLE, 0, null);
			Display.getDisplay(this).setCurrent(d);
			return true;
		} catch (RuntimeException e) {
			showFatalError("Unexpected exception during activation of "
					+ modules[id], e);
			return false;
		}
	}
	
	// #else
//@	
//@	private static final int MODULE_VM = 1;
//@
//@	public byte[][] file;
//@	private VM vm;
//@	
//@	private boolean initModules() {
//@		if (vm != null)
//@			return true;
//@		vm = new VM();
//@		boolean ok = true;
//@		Throwable t = null;
//@		try {
//@			if (vm.init(this) != 0)
//@				ok = false;
//@		} catch (Throwable e) {
//@			t = e;
//@			ok = false;
//@		} finally {
//@			if (!ok)
//@				showFatalError("VM could not be initialized", t);
//@		}
//@		return ok;
//@	}
//@
//@	// only VM can be (de)activated
//@	private boolean activateModule(int id) {
//@		try {
//@			if (id == MODULE_VM) {
//@				vm.activate();
//@				Display.getDisplay(this).setCurrent(
//@						(Displayable)vm.getDisplayable());
//@			} else {
//@				vm.deactivate();
//@			}
//@			return true;
//@		} catch (RuntimeException e) {
//@			showFatalError("Unexpected exception during (de)activation of VM", e);
//@			return false;
//@		}
//@	}
//@	
	// #endif

	public JBit() {
		if (!checkAttribute(JBitURLName, JBitURLValue))
			return;
		if (!checkAttribute(JBitLicName, JBitLicValue))
			return;
		// #if !JBIT_RUNTIME
		Vector c = new Vector();
		parseModuleList(c);
		parseAutoRun();
		setupModules(c);
		pool.put(JBitSvc.POOL_ID_PROGRAM_ID, new Integer(1));
		pool.put(JBitSvc.POOL_ID_PROGRAM_MODIFIED, new Boolean(false));
		// #else
//@		parseAutoRun();
		// #endif
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
			// #if JBIT_RUNTIME
//@			this.file = file;
			// #else
			newProgram(file);
			// #endif
			return null;
		} catch (Throwable e) {
			return e.getMessage();
		}
	}

	private boolean loadJBFile(String resName) {
		String error;
		InputStream is = getClass().getResourceAsStream(resName);
		if (is == null)
			error = "Resource " + resName + " not found";
		else
			error = load(is);
		if (error != null) {
			showFatalError(error, null);
			return false;
		}
		return true;
	}
	
	protected void startApp() throws MIDletStateChangeException {
		if (fatalError != null) {
			Display.getDisplay(this).setCurrent(fatalError);
			return;
		}
		if (!initModules())
			return;
		if (autorun != null) {
			// #if !JBIT_RUNTIME
			byte[][] file = (byte[][])pool.get(JBitSvc.POOL_ID_PROGRAM_FILE);
			// #endif
			if (file == null && !loadJBFile(autorun))
				return;
		}
		// #if !JBIT_RUNTIME
		int id;
		if (activeId != -1) {
			// resume operation after pauseApp
			id = activeId;
			activeId = 1;
			activateModule(id);
			return;
		}
		if (autorun != null) {
			// Run Program
			if ((id = getModuleIdByService("VM")) == -1) {
				showFatalError("VM service not available", null);
				return;
			}
		} else {
			// JBit (tool selection)
			id = 0;
		}
		activateModule(id);
		// #else
//@		activateModule(1);
		// #endif
	}

	/*
	 * Current pauseApp() handling is quite buggy (e.g. the VM should
	 * not be restarted); nevertheless, it should be good enough
	 * to allow the user to save the program and to restart JBit. 
	 */
	protected void pauseApp() {
		activateModule(-1);
	}

	protected void destroyApp(boolean unconditional) throws MIDletStateChangeException {
		activateModule(-1);
	}
	
	// #if !JBIT_RUNTIME

	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "JBit";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_MIDLET);
		case METADATA_ID_SERVICES:
			return new String[] { JBitSvc.TAG };
		default:
			return null;
		}
	}
	
	private List list;
	
	private int activate() {
		list = new List("JBit", List.IMPLICIT);
		for (int i = 0; i < n; i++)
			if (types[i] == TYPE_TOP_LEVEL && states[i] != FAILED)
				list.append(labels[i], null);
		list.append("About", null);
		list.addCommand(new Command("Exit", Command.EXIT, 1));
		list.setCommandListener(this);
		return 0;
	}
	
	private int deactivate() {
		list = null;
		return 0;
	}
	
	private int backModuleId;
	
	private int replaceWidth(String service) {
		int id;
		if (service != null)
			id = getModuleIdByService(service);
		else
			id = backModuleId;
		backModuleId = 0;
		if (id == -1) {
			showFatalError(service + " service not available", null);
			return -1;
		} else {
			return activateModule(id) ? 0 : -1;
		}
	}
	
	private int replaceWithVM(int vmStartMode, Module back) {
		backModuleId = 0;
		for (int i = 0; i < n; i++)
			if (modules[i] == back)
				backModuleId = i;
		Module vm = findService("VM");
		vm.opI(VMSvc.OP_SET_START_MODE, vmStartMode, null);
		return activateModule(getModuleIdByService("VM")) ? 0 : -1;
	}
	
	private Module findService(Object oArg) {
		try {
			String arg = (String)oArg;
			int id = getModuleIdByService(arg);
			if (id == -1)
				return null;
			return getModuleById(id);
		} catch (RuntimeException e) {
			return null;
		}
	}

	private Object loadProgram(Object oArg) {
		try {
			InputStream is = (InputStream)oArg;
			return load(is);
		} catch (Throwable e) {
			return e.toString();
		}
	}

	private int newProgram(Object oArg) {
		Integer id = (Integer)pool.get(JBitSvc.POOL_ID_PROGRAM_ID);
		id = new Integer(id.intValue() + 1);
		pool.put(JBitSvc.POOL_ID_PROGRAM_ID, id);
		pool.put(JBitSvc.POOL_ID_PROGRAM_FILE, oArg);
		pool.put(JBitSvc.POOL_ID_PROGRAM_MODIFIED, new Boolean(false));
		return id.intValue();
	}

	private int poolPut(Object oArg) {
		try {
			Object[] args = (Object[])oArg;
			String id = (String)args[0];
			Object value = args[1];
			if (value != null)
				pool.put(id, value);
			else
				pool.remove(id);
			return 0;
		} catch (RuntimeException e) {
			return -1;
		}
	}
	
	public Object opO(int opId, int iArg, Object oArg) {
		switch (opId) {
		case JBitSvc.OP_GET_MIDLET:
			return this;
		case JBitSvc.OP_GET_DISPLAY:
			return Display.getDisplay(this);
		case JBitSvc.OP_POOL_GET:
			return pool.get(oArg);
		case OP_GET_METADATA:
			return getMetadata(iArg);
		case OP_GET_DISPLAYABLE:
			return list;
		case JBitSvc.OP_FIND_SERVICE:
			return findService(oArg);
		case JBitSvc.OP_LOAD_PROGRAM:
			return loadProgram(oArg);
		}
		return null;
	}
	
	public int opI(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_ACTIVATE:
			return activate();
		case OP_DEACTIVATE:
			return deactivate();
		case JBitSvc.OP_REPLACE_WITH_SERVICE:
			return replaceWidth((String)oArg);
		case JBitSvc.OP_REPLACE_WITH_VM:
			return replaceWithVM(iArg, (Module)oArg);
		case JBitSvc.OP_POOL_PUT:
			return poolPut(oArg);
		case JBitSvc.OP_NEW_PROGRAM:
			return newProgram(oArg);
		default:
			return -1;
		}
	}

	public int get(int address) {
		return -1;
	}

	public int put(int address, int value) {
		return -1;
	}	
	
	private static final String AboutText =
		"JBit - A 6502 Framework for Mobile Phones.\n" +
		"JBit comes with ABSOLUTELY NO WARRANTY.\n" +
		"http://jbit.sourceforge.net/\n\n";
	
	public void showAboutDialog() {
		StringBuffer text = new StringBuffer();
		text.append(AboutText);
		try {
			InputStream is = getClass().getResourceAsStream("about.txt");
			InputStreamReader ir = new InputStreamReader(is);
			int c;
			while ((c = ir.read()) != -1) {
				if (c == '\r')
					continue;
				text.append((char)c);
			}
			is.close();
		} catch (Throwable e) {
		}
		Alert about = new Alert("JBit", text.toString(), null, AlertType.INFO);
		about.setTimeout(Alert.FOREVER);
		Display.getDisplay(this).setCurrent(about);
	}
	
	private void showExitConfirmDialog() {
		Form form = new Form("Exit");
		form.append("Program has been modified.\n"
				+ "Do you really want to exit?");
		form.addCommand(new Command("OK", Command.OK, 1));
		form.addCommand(new Command("Cancel", Command.CANCEL, 2));
		form.setCommandListener(this);
		Display.getDisplay(this).setCurrent(form);
	}
	
	// #endif
	
	public void commandAction(Command c, Displayable d) {
		if (d == fatalError) {
			notifyDestroyed();
			return;
		}
		// #if !JBIT_RUNTIME
		if (d == list) {
			if (c.getCommandType() == Command.EXIT) {
				if (findService("Save") != null && ((Boolean)pool.get(
						JBitSvc.POOL_ID_PROGRAM_MODIFIED)).booleanValue()) {
					showExitConfirmDialog();
				} else {
					notifyDestroyed();
				}
				return;
			}
			int index = list.getSelectedIndex();
			int last = list.size();
			if (index == last - 1) {
				showAboutDialog();
				return;
			} else {
				String s = list.getString(index);
				for (int i = 0; i < n; i++) {
					if (s.equals(labels[i])) {
						activateModule(i);
						return;
					}
				}
			}				
		} else {
			if (c.getCommandType() == Command.OK)
				notifyDestroyed();
			else
				Display.getDisplay(this).setCurrent(list);
		}
		// #endif
	}
}
