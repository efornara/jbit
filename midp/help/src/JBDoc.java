/*
	JBDoc - Documentation for JBit
	Copyright (C) 2012-2016  Emanuele Fornara
	
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

import javax.microedition.lcdui.Display;
import javax.microedition.midlet.MIDlet;
import javax.microedition.midlet.MIDletStateChangeException;

public class JBDoc extends MIDlet {

	Help help = null;
	Thread thread = null;

	synchronized void terminateThread() {
		thread = null;
		try {
			wait(2000);
		} catch (InterruptedException e) {
		}
	}

	synchronized void threadTerminated() {
		notifyAll();
	}

	protected void destroyApp(boolean unconditional)
			throws MIDletStateChangeException {
		terminateThread();
	}

	protected void pauseApp() {
		terminateThread();
	}

	protected void startApp() throws MIDletStateChangeException {
		if (help == null)
			help = new Help(this);
		thread = new Thread(help);
		thread.start();
		Display.getDisplay(this).setCurrent(help);
	}
}
