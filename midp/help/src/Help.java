/*
	JBDoc - Documentation for JBit
	Copyright (C) 2012-2017  Emanuele Fornara
	
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
import java.util.Vector;

import javax.microedition.lcdui.Alert;
import javax.microedition.lcdui.AlertType;
import javax.microedition.lcdui.Canvas;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;
import javax.microedition.lcdui.List;

public class Help extends Canvas implements CommandListener, Runnable {

	private final static int MSG_BUFFER_SIZE = 32;

	private final static int MSG_BOOK = 1;
	private final static int MSG_PREV_LINE = 2;
	private final static int MSG_NEXT_LINE = 3;
	private final static int MSG_PREV_PAGE = 4;
	private final static int MSG_NEXT_PAGE = 5;
	private final static int MSG_EXIT = 100;

	private int[] msgs = new int[MSG_BUFFER_SIZE];

	private synchronized void enqueMsg(int msg) {
		int i;
		for (i = MSG_BUFFER_SIZE - 1; i >= 0; i--) {
			if (msgs[i] != 0)
				break;
		}
		if (i == MSG_BUFFER_SIZE - 1)
			return;
		msgs[i + 1] = msg;
		notifyAll();
	}

	private synchronized int dequeMsg() {
		int msg = msgs[0];
		for (int i = 1; i < MSG_BUFFER_SIZE; i++)
			msgs[i - 1] = msgs[i];
		return msg;
	}

	private boolean hasPendingMsg() {
		return msgs[0] != 0;
	}

	private int[] tocSection;
	private String[] tocLabel;

	private int curBook;
	private int curLine;
	private int curColumn;

	private byte[] data;

	private String[] books;

	private Image initFont(String resName) {
		try {
			Image image = Image.createImage(resName);
			charWidth = image.getWidth() / 16;
			charHeight = image.getHeight() / 6;
			screenColumns = getWidth() / charWidth;
			screenRows = getHeight() / charHeight;
			return image;
		} catch (Throwable e) {
			return null;
		}
	}

	private void initScreen() {
		if (getWidth() > 200 || getHeight() > 200)
			font = initFont("/vga.png");
		else
			font = initFont("/fixed.png");
		if (font == null)
			font = initFont("/font.png");
		if (font == null)
			state = FAILED;
	}

	private void loadBookList() {
		Vector vb = new Vector();
		StringBuffer book = new StringBuffer();

		InputStream is = null;
		byte[] buf = new byte[64];
		int i, n;

		try {
			is = getClass().getResourceAsStream("help.txt");
			while ((n = is.read(buf)) >= 0) {
				for (i = 0; i < n; i++) {
					byte c = buf[i];
					if (c == '\r') {
						// be nice to windows
					} else if (c == '\n') {
						vb.addElement(book.toString());
						book.setLength(0);
					} else {
						book.append((char) c);
					}
				}
			}
		} catch (Throwable e) {
			state = FAILED;
		} finally {
			if (is != null)
				try {
					is.close();
				} catch (Exception e) {
				}
		}

		n = vb.size();
		books = new String[n];
		for (i = 0; i < n; i++)
			books[i] = (String) vb.elementAt(i);
	}

	private void init() {
		initScreen();
		loadBookList();
	}

	private void loadBook() {
		// TOC temporary vectors and buffers
		Vector vp = new Vector();
		Vector vl = new Vector();
		StringBuffer label = new StringBuffer();

		InputStream is = null;
		ByteArrayOutputStream output = new ByteArrayOutputStream();
		byte[] buf = new byte[512];
		byte[] line = new byte[256];
		int i, n, len = 0, section = 0;
		byte mode = 0;
		boolean toc = false;

		data = null;
		tocSection = null;
		tocLabel = null;

		try {
			is = getClass().getResourceAsStream(books[curBook] + ".dat");
			while ((n = is.read(buf)) >= 0) {
				for (i = 0; i < n; i++) {
					byte c = buf[i];

					// process end of section
					if (c == 0) {
						// add entry to TOC, if necessary
						if (toc) {
							vp.addElement(new Integer(section));
							vl.addElement(label.toString());
							label.setLength(0);
						}

						// add last line to output
						if (len != 0) {
							output.write(line, 0, len);
							output.write(0);
						}
						len = 0;

						// add end of section to output
						char underline = 0;
						if (mode == 'A' || mode == 'a')
							underline = '=';
						else if (mode == 'B' || mode == 'b')
							underline = '-';
						if (underline != 0) {
							for (int j = 0; j < screenColumns; j++)
								output.write(underline);
							output.write(0);
						}
						output.write(0);
						section = output.size();

						// ready for next section
						mode = 0;
						continue;
					}

					// process start of section
					if (mode == 0) {
						mode = c;
						toc = (mode != '#' && Character
								.isUpperCase((char) mode));

						// start a new line
						if (mode == 'i') {
							line[0] = 127;
							line[1] = ' ';
							len = 2;
						} else {
							len = 0;
						}
						continue;
					}

					// process section
					if (toc)
						label.append((char) c);

					// explicit new lines are easy
					if (c == '\n') {
						output.write(line, 0, len);
						output.write(0);
						len = 0;
						continue;
					}

					// formatted sections are also easy
					if (mode == '#') {
						line[len++] = c;
						continue;
					}

					// handle split
					if (len == 0 && c == ' ')
						continue;
					line[len++] = c;
					if (len > screenColumns) {
						int split = len;
						for (int j = len - 1; j >= 0; j--)
							if (line[j] == ' ') {
								split = j;
								break;
							}
						output.write(line, 0, split++);
						output.write(0);
						int start;
						for (start = 0; split < len; split++, start++)
							line[start] = line[split];
						len = start;
					}
				}
			}
			data = output.toByteArray();
		} catch (Throwable e) {
			state = FAILED;
			return;
		} finally {
			if (is != null)
				try {
					is.close();
				} catch (Exception e) {
				}
		}

		// Copy TOC
		n = vp.size();
		tocSection = new int[n];
		tocLabel = new String[n];
		for (i = 0; i < n; i++) {
			tocSection[i] = ((Integer) vp.elementAt(i)).intValue();
			tocLabel[i] = (String) vl.elementAt(i);
		}

		// Ready to render the top of the book
		curLine = 0;
		state = READY;
	}

	private Command gotoCmd = new Command("GoTo", Command.SCREEN, 1);
	private Command booksCmd = new Command("Books", Command.SCREEN, 3);
	private Command aboutCmd = new Command("About", Command.SCREEN, 4);
	private Command exitCmd = new Command("Exit", Command.EXIT, 5);

	private JBDoc midlet;
	private Display display;
	private Image font;

	public Help(JBDoc midlet) {
		this.midlet = midlet;
		display = Display.getDisplay(midlet);
		addCommand(gotoCmd);
		addCommand(booksCmd);
		addCommand(aboutCmd);
		addCommand(exitCmd);
		setCommandListener(this);
	}

	private int bgColor = 0xFFFFFF;
	private int fgColor = 0x000000;
	private int llColor = 0xC0C0C0;

	private int charWidth;
	private int charHeight;
	private int screenColumns;
	private int screenRows;

	private final static int INITIALIZING = 0;
	private final static int FAILED = 1;
	private final static int PAUSED = 2;
	private final static int LOADING = 3;
	private final static int READY = 4;

	private int state = INITIALIZING;

	protected void paint(Graphics g) {
		int w = getWidth(), h = getHeight();
		g.setClip(0, 0, w, h);
		g.setColor(bgColor);
		g.fillRect(0, 0, w, h);
		g.setColor(fgColor);
		String msg = null;
		switch (state) {
		case INITIALIZING:
			msg = "Initializing...";
			break;
		case FAILED:
			msg = "Failed";
			break;
		case PAUSED:
			msg = "Paused";
			break;
		case LOADING:
			msg = "Loading...";
			break;
		}
		if (msg != null) {
			g.drawString(msg, w >> 1, h >> 1, Graphics.HCENTER
					| Graphics.BASELINE);
		} else {
			int x = 0, col = 0, y = 0, row = 0, di = curLine;
			while (di < data.length) {
				char c = (char) data[di++];
				if (c == 0) {
					y += charHeight;
					row++;
					x = 0;
					col = 0;
					if (row == screenRows)
						return;
					continue;
				}
				if (col++ < curColumn)
					continue;
				boolean longline = false;
				if (col == curColumn + screenColumns) {
					while (di < data.length)
						if (data[di++] == 0) {
							di--;
							break;
						} else {
							longline = true;
						}
				}
				int cx = (c & 15) * charWidth;
				int cy = ((c >> 4) - 2) * charHeight;
				g.setClip(x, y, charWidth, charHeight);
				if (longline) {
					g.setColor(llColor);
					g.fillRect(x, y, charWidth, charHeight);
					g.setColor(fgColor);
				}
				g.drawImage(font, x - cx, y - cy, Graphics.TOP | Graphics.LEFT);
				x += charWidth;
			}
		}
	}

	private Command cancelCmd = new Command("Cancel", Command.CANCEL, 1);

	private List gotoList;

	private void enterGoto() {
		gotoList = new List("GoTo", List.IMPLICIT);
		int n = tocSection.length;
		for (int i = 0; i < n; i++)
			gotoList.append(tocLabel[i], null);
		gotoList.addCommand(cancelCmd);
		gotoList.setCommandListener(this);
		display.setCurrent(gotoList);
	}

	private void handleGoto(Command c) {
		if (c != cancelCmd) {
			int i = gotoList.getSelectedIndex();
			curLine = tocSection[i];
			repaint();
		}
		gotoList = null;
		display.setCurrent(this);
	}

	private List booksList;

	private void enterBooks() {
		booksList = new List("Books", List.IMPLICIT);
		int n = books.length;
		for (int i = 0; i < n; i++)
			booksList.append(books[i], null);
		booksList.addCommand(cancelCmd);
		booksList.setCommandListener(this);
		display.setCurrent(booksList);
	}

	private void handleBooks(Command c) {
		if (c != cancelCmd) {
			curBook = booksList.getSelectedIndex();
			state = LOADING;
			repaint();
			enqueMsg(MSG_BOOK);
		}
		booksList = null;
		display.setCurrent(this);
	}

	private static final String aboutText = "JBDoc - Documentation for JBit.\n"
			+ "JBDoc comes with ABSOLUTELY NO WARRANTY.\n"
			+ "http://jbit.sf.net/m\n\n"
			+ "Keypad: 1: prev page, 3: next page, 2: prev line, 8: next line, 4: scroll left, 6: scroll right, 7: start of line\n"
			+ "Touchpad: (middle edges) left: prev page, right: next page, top: prev line, bottom: next line; (corners) top-left: scroll left, top-right: scroll right, bottom-left: start of line\n";

	private void handleAbout() {
		Alert alert = new Alert("JBDoc", aboutText, null, AlertType.INFO);
		alert.setTimeout(Alert.FOREVER);
		display.setCurrent(alert, this);
	}

	protected void keyPressed(int keyCode) {
		switch (keyCode) {
		case '1':
			enqueMsg(MSG_PREV_PAGE);
			break;
		case '2':
			enqueMsg(MSG_PREV_LINE);
			break;
		case '3':
			enqueMsg(MSG_NEXT_PAGE);
			break;
		case '4':
			if (curColumn > 0)
				curColumn--;
			repaint();
			break;
		case '6':
			curColumn++;
			repaint();
			break;
		case '7':
			curColumn = 0;
			repaint();
			break;
		case '8':
			enqueMsg(MSG_NEXT_LINE);
			break;
		}
	}

	protected void pointerPressed(int x, int y) {
		int w = getWidth(), h = getHeight();
		int ws = w >> 2, hs = h >> 2;
		if (y < hs) {
			// top
			if (x < ws)
				keyPressed('4');
			else if (x > (w - ws))
				keyPressed('6');
			else
				keyPressed('2');
		} else if (y > (h - hs)) {
			// bottom
			if (x < ws)
				keyPressed('7');
			else if (x > (w - ws))
				;
			else
				keyPressed('8');
		} else {
			// center
			if (x < ws)
				keyPressed('1');
			else if (x > (w - ws))
				keyPressed('3');
			else
				;
		}
	}

	public void commandAction(Command c, Displayable d) {
		if (d == gotoList) {
			handleGoto(c);
		} else if (d == booksList) {
			handleBooks(c);
		} else if (c == gotoCmd) {
			if (state != READY)
				return;
			enterGoto();
		} else if (c == booksCmd) {
			if (state != READY)
				return;
			enterBooks();
		} else if (c == aboutCmd) {
			handleAbout();
		} else if (c == exitCmd) {
			enqueMsg(MSG_EXIT);
		}
	}

	private void nextLine() {
		int di;
		for (di = curLine; data[di] != 0; di++)
			;
		if (di != data.length - 1)
			curLine = di + 1;
	}

	private void prevLine() {
		if (curLine == 0)
			return;
		int di;
		for (di = curLine - 2; di > 0 && data[di] != 0; di--)
			;
		if (di > 0)
			curLine = di + 1;
		else
			curLine = 0;
	}

	private void nextPage() {
		for (int i = 0; i < screenRows; i++)
			nextLine();
	}

	private void prevPage() {
		for (int i = 0; i < screenRows; i++)
			prevLine();
	}

	private boolean initialized = false;

	public void run() {
		boolean exitRequested = false;
		if (!initialized) {
			init();
			initialized = true;
		}
		if (state != FAILED) {
			state = LOADING;
			curBook = 0;
			enqueMsg(MSG_BOOK);
		}
		repaint();
		while (midlet.thread == Thread.currentThread()) {
			int msg;
			while ((msg = dequeMsg()) != 0) {
				switch (msg) {
				case MSG_BOOK:
					loadBook();
					repaint();
					break;
				case MSG_NEXT_LINE:
					nextLine();
					repaint();
					break;
				case MSG_PREV_LINE:
					prevLine();
					repaint();
					break;
				case MSG_NEXT_PAGE:
					nextPage();
					repaint();
					break;
				case MSG_PREV_PAGE:
					prevPage();
					repaint();
					break;
				case MSG_EXIT:
					exitRequested = true;
					break;
				}
			}
			if (exitRequested)
				break;
			synchronized (this) {
				try {
					if (!hasPendingMsg())
						wait();
				} catch (InterruptedException e) {
				}
			}
		}
		midlet.threadTerminated();
		if (exitRequested)
			midlet.notifyDestroyed();
		else
			state = PAUSED;
	}
}
