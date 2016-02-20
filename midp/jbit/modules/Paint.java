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

import java.io.InputStream;
import java.util.Vector;
import javax.microedition.lcdui.*;


public final class Paint extends Canvas implements Module, CommandListener {
	
	private Command backCmd = new Command("Back", Command.BACK, 1);
	private Command infoCmd = new Command("Info", Command.ITEM, 1);
	private Command editCmd = new Command("Edit", Command.ITEM, 2);
	private Command deleteCmd = new Command("Delete", Command.ITEM, 3);
	private Command newCmd = new Command("New", Command.SCREEN, 1);
	private Command saveCmd = new Command("Save", Command.SCREEN, 2);
	private Command saveAsCmd = new Command("SaveAs", Command.SCREEN, 3);
	private Command setPaletteCmd = new Command("SetPalette", Command.SCREEN, 4);
	private Command putROMCmd = new Command("PutROM", Command.SCREEN, 5);
	private Command showCoordsCmd = new Command("(No)Coords", Command.SCREEN, 6);
	private Command setCursorCmd = new Command("SetCursor", Command.SCREEN, 7);
	
	private static final int INACTIVE = 0;
	private static final int LIST = 1;
	private static final int NEW = 2;
	private static final int EDIT = 3;
	private static final int SETPALETTE = 4;
	private static final int PUTROM = 5;
	private static final int ERROR = 6;
	
	private int status;
	private Form errorForm;
	private List imageList;
	private Form newForm;
	private Form putROMForm;
	private List setPaletteList;
	private boolean isRunning;
	
	private Displayable getDisplayable() {
		switch (status) {
		case ERROR:
			return errorForm;
		case LIST:
			return imageList;
		case NEW:
			return newForm;
		case SETPALETTE:
			return setPaletteList;
		case PUTROM:
			return putROMForm;
		}
		return this;
	}
	
	private int init() {
		if (findSaveSvc() != null) {
			addCommand(saveCmd);
			addCommand(saveAsCmd);
		}
		addCommand(setPaletteCmd);
		addCommand(putROMCmd);
		addCommand(showCoordsCmd);
		addCommand(setCursorCmd);
		addCommand(backCmd);
		setCommandListener(this);
		currentPalette = standardPalette;
		showCoords = true;
		status = INACTIVE;
		return 0;
	}
	
	private int activate() {
		attachData();
		if (isRunning) {
			status = EDIT;
			isRunning = false;
			return 0;
		}
		if (getDataSize() == 0)
			enterErrorStatus("No data available!");
		else
			enterListStatus();
		return 0;
	}
	
	private int deactivate() {
		detachData();
		releaseImages();
		errorForm = null;
		imageList = null;
		releaseNewForm();
		releasePutROMForm();
		releaseSetPaletteList();
		status = INACTIVE;
		return 0;
	}

	private void showError(String msg, Displayable nextDisplayable) {
		Alert alert = new Alert(null, msg + "!", null, AlertType.ERROR);
		alert.setTimeout(Alert.FOREVER);
		display.setCurrent(alert, nextDisplayable);
	}

	public void commandAction(Command c, Displayable d) {
		if (d != null && d == imageList) {
			handleListCommands(c);
		} else if (d != null && d == confirmDlg) {
			handleConfirmDialog(c);
		} else if (d != null && d == newForm) {
			handleNewCommands(c);
		} else if (d != null && d == setPaletteList) {
			handleSetPaletteCommands(c);
		} else if (d != null && d == putROMForm) {
			handlePutROMCommands(c);
		} else if (c == backCmd) {
			if (status == EDIT) {
				enterListStatus();
				display.setCurrent(imageList);
			} else {
				back();
			}
		} else if (c == setPaletteCmd) {
			enterSetPaletteStatus();
			display.setCurrent(setPaletteList);
		} else if (c == putROMCmd) {
			if (viewTileMode && !viewColorMode) {
				enterPutROMStatus();
				display.setCurrent(putROMForm);
			} else {
				showError("Only available in Tile mode", this);
			}
		} else if (c == showCoordsCmd) {
			showCoords = !showCoords;
			repaint();
		} else if (c == setCursorCmd) {
			cursorColor = getColor(fgIndex);
			repaint();
		} else if (c == saveCmd) {
			save(false);
		} else if (c == saveAsCmd) {
			save(true);
		} else if (c == saveOkCmd) {
			dataHasBeenSaved();
		}
	}

	private void enterErrorStatus(String msg) {
		errorForm = new Form("Error");
		errorForm.append(msg);
		errorForm.addCommand(backCmd);
		errorForm.setCommandListener(this);
		status = ERROR;
	}

	private Form confirmDlg;
	private int confirmImage;
	
	private void showConfirmDialog(int image) {
		confirmImage = image;
		confirmDlg = new Form("Delete");
		confirmDlg.append("Do you really want to delete image "
				+ getImageTag(image, false) + "?");
		confirmDlg.addCommand(okCmd);
		confirmDlg.addCommand(cancelCmd);
		confirmDlg.setCommandListener(this);
		display.setCurrent(confirmDlg);
	}
	private void handleConfirmDialog(Command c) {
		boolean retry = false;
		try {
			if (c == okCmd) {
				int image = confirmImage;
				int length = getData16(confirmImage - 2);
				for (int i = -2; i < length; i++)
					if (getData8(image + i) != 0)
						putData8(image + i, 0);
				enterListStatus();
			}
			display.setCurrent(imageList);
		} finally {
			if (!retry) {
				confirmDlg = null;
			}
		}
	}

	private void showInfoDialog(int image) {
		int depth = getImageDepth(image);
		int length = getData16(image - 2);
		int id = getData8(image + IO.PNG_HEADER_IMAGEID_OFFSET);
		int flags = getImageFlags(image);
		String info = addressToTag(image - 2) + "-"
				+ addressToTag(image + length - 1) + " (" +  getImageWidth(image) + " pixels x "
				+ getImageHeight(image) + " pixels x " + depth + " bit(s)), " + (length + 2)
				+ " bytes.\nImageId ("
				+ addressToTag(image + IO.PNG_HEADER_IMAGEID_OFFSET) + "): "
				+ id + ".\nFlags ("
				+ addressToTag(image + IO.PNG_HEADER_FLAGS_OFFSET) + "): ";
		if (flags != 0) {
			if ((flags & IO.VAL_IPNGGEN_FLAGS_ZOOM1) != 0)
				info += " ZOOM1(8)";
			if ((flags & IO.VAL_IPNGGEN_FLAGS_ZOOM0) != 0)
				info += " ZOOM0(4)";
			if ((flags & IO.VAL_IPNGGEN_FLAGS_PALREF) != 0)
				info += " PALREF(2)";
			if ((flags & IO.VAL_IPNGGEN_FLAGS_IDX0TRANSP) != 0)
				info += " IDX0TRANSP(1)";
		} else {
			info += "0";
		}
		info += ".";
		Alert alert = new Alert("Info", info, null, AlertType.INFO);
		alert.setTimeout(Alert.FOREVER);
		display.setCurrent(alert, imageList);
	}

	private void enterListStatus() {
		scanDataForImages();
		imageList = new List("Images", List.IMPLICIT);
		int n = getNumberOfImages();
		if (n == 0) {
			imageList.append("- none -", null);
		} else {
			for (int i = 0; i < n; i++)
				imageList.append(getImageTag(getImageByIndex(i), false), null);
		}
		imageList.addCommand(infoCmd);
		imageList.addCommand(editCmd);
		imageList.addCommand(deleteCmd);
		imageList.addCommand(backCmd);
		imageList.addCommand(newCmd);
		imageList.setCommandListener(this);
		status = LIST;
	}
	
	private void handleListCommands(Command c) {
		if (c == newCmd) {
			enterNewStatus();
			display.setCurrent(newForm);
		} else if (c == backCmd) {
			back();
		} else if (getNumberOfImages() != 0) {
			int image = getImageByIndex(imageList.getSelectedIndex());
			if (c == List.SELECT_COMMAND || c == editCmd) {
				enterEditStatus(image);
				display.setCurrent(this);
				repaint();
			} else if (c == infoCmd && image != -1) {
				showInfoDialog(image);
			} else if (c == deleteCmd) {
				showConfirmDialog(image);
			} 
		} 
	}
	
	private Command okCmd = new Command("OK", Command.OK, 1);
	private Command cancelCmd = new Command("Cancel", Command.CANCEL, 1);

	private TextField pageItem;
	private TextField offsetItem;
	private TextField widthItem;
	private TextField heightItem;
	private ChoiceGroup depthItem;
	private ChoiceGroup rgbItem;

	private void enterNewStatus() {
		newForm = new Form("New");
		pageItem = new TextField("Page", "", 3, TextField.NUMERIC);
		newForm.append(pageItem);
		offsetItem = new TextField("Offset", "", 3, TextField.NUMERIC);
		newForm.append(offsetItem);
		widthItem = new TextField("Width", "", 4, TextField.NUMERIC);
		newForm.append(widthItem);
		heightItem = new TextField("Height", "", 4, TextField.NUMERIC);
		newForm.append(heightItem);
		depthItem = new ChoiceGroup("Colors", Choice.POPUP);
		depthItem.append("2", null);
		depthItem.append("4", null);
		depthItem.append("16", null);
		newForm.append(depthItem);
		rgbItem = new ChoiceGroup("Mode", Choice.POPUP);
		rgbItem.append("Palette", null);
		rgbItem.append("RGB", null);
		newForm.append(rgbItem);
		newForm.addCommand(okCmd);
		newForm.addCommand(cancelCmd);
		newForm.setCommandListener(this);
		status = NEW;
	}
	
	private void releaseNewForm() {
		newForm = null;
		pageItem = null;
		offsetItem = null;
		widthItem = null;
		heightItem = null;
		depthItem = null;
	}

	private void handleNewCommands(Command c) {
		if (c == okCmd) {
			try {
				int page, offset, width, height, depth;
				try {
					page = Integer.parseInt(pageItem.getString());
					if (page < getFirstDataPage() || page > getLastDataPage())
						throw new Throwable();
				} catch (Throwable e) {
					throw new Exception("Invalid Page [" + getFirstDataPage()
							+ "," + getLastDataPage() + "]");
				}
				try {
					offset = Integer.parseInt(offsetItem.getString());
					if (offset < 0 || offset > 255)
						throw new Throwable();
				} catch (Throwable e) {
					throw new Exception("Invalid Offset [0,255]");
				}
				try {
					width = Integer.parseInt(widthItem.getString());
					if (width <= 0 || width > MAX_WIDTH)
						throw new Throwable();
				} catch (Throwable e) {
					throw new Exception("Invalid Width [1," + MAX_WIDTH + "]");
				}
				try {
					height = Integer.parseInt(heightItem.getString());
					if (height <= 0 || height > MAX_HEIGHT)
						throw new Throwable();
				} catch (Throwable e) {
					throw new Exception("Invalid Height [1," + MAX_HEIGHT + "]");
				}
				int colors = depthItem.getSelectedIndex();
				switch (colors) {
				case 1:
					depth = 2;
					break;
				case 2:
					depth = 4;
					break;
				default:
					depth = 1;
					break;
				}
				boolean rgb = rgbItem.getSelectedIndex() != 0;
				makeImage(page, offset, width, height, depth, rgb);
			} catch (Exception e) {
				showError(e.getMessage(), newForm);
				return;
			}
		}
		enterListStatus();
		display.setCurrent(imageList);			
	}
	
	private int currentPalette[];
	private Vector palettes;
	
	private void scanDataForPalettes() {
		palettes = new Vector();
		int n = getDataSize();
		for (int palette = 2; palette < n - 3; palette++) {
			if (getData8(palette) != IO.REQ_SETPAL)
				continue;
			int length = getData16(palette - 2);
			if (palette + length > n)
				continue;
			if (length < 1 + 3 || length > 1 + 256 * 3)
				continue;
			if (length % 3 != 1)
				continue;
			palettes.addElement(new Integer(palette));
		}
	}
	
	private int getPaletteByIndex(int index) {
		return ((Integer)palettes.elementAt(index)).intValue();
	}
	
	private String getPaletteTag(int palette) {
		return addressToTag(palette - 2) + " (" + getData16(palette - 2) / 3
				+ ")";
	}
	
	private void enterSetPaletteStatus() {
		scanDataForPalettes();
		setPaletteList = new List("Palettes", List.IMPLICIT);
		setPaletteList.append("- standard -", null);
		int n = palettes.size();
		for (int i = 0; i < n; i++)
			setPaletteList.append(getPaletteTag(getPaletteByIndex(i)), null);
		setPaletteList.addCommand(cancelCmd);
		setPaletteList.setCommandListener(this);
		status = SETPALETTE;
	}
	
	private void releaseSetPaletteList() {
		setPaletteList = null;
	}
	
	private void handleSetPaletteCommands(Command c) {
		if (c != cancelCmd) {
			try {
				int index = setPaletteList.getSelectedIndex();
				if (index <= 0) {
					currentPalette = standardPalette;
				} else {
					int palette = getPaletteByIndex(index - 1);
					int n = getData16(palette - 2) / 3;
					currentPalette = new int[n];
					for (int i = 0; i < n; i++) {
						int color = getData8(palette + 1);
						color <<= 8;
						color |= getData8(palette + 2);
						color <<= 8;
						color |= getData8(palette + 3);
						currentPalette[i] = color;
						palette += 3;
					}
				}
				selectCurrentImage(currentImage);
			} catch (Exception e) {
				showError(e.getMessage(), setPaletteList);
				return;
			}
		}
		status = EDIT;
		display.setCurrent(this);	
		repaint();
	}

	private TextField firstCharItem;
	private TextField nOfCharsItem;
	private TextField romItem;

	private void enterPutROMStatus() {
		putROMForm = new Form("PutROM");
		firstCharItem = new TextField("First char", "32", 3, TextField.NUMERIC);
		putROMForm.append(firstCharItem);
		nOfCharsItem = new TextField("# of chars", "96", 3, TextField.NUMERIC);
		putROMForm.append(nOfCharsItem);
		romItem = new TextField("ROM", "cga.rom", 16, TextField.ANY);
		putROMForm.append(romItem);
		putROMForm.addCommand(okCmd);
		putROMForm.addCommand(cancelCmd);
		putROMForm.setCommandListener(this);
		status = PUTROM;
	}
	
	private void releasePutROMForm() {
		putROMForm = null;
		firstCharItem = null;
		nOfCharsItem = null;
		romItem = null;
	}
	
	private int readROM(InputStream is) throws Exception {
		int c = is.read();
		if (c == -1)
			throw new Exception("Unexpected EOF");
		return c;
	}
	
	private void runPutROM(int firstChar, int nOfChars, InputStream is)
			throws Exception {
		for (int n = firstChar * 8; n > 0; n--)
			readROM(is);
		int tileId = getTileId(cursorX, cursorY);
		int nOfTiles = (imageWidth * imageHeight) >> 6;
		for (int n = 0; n < nOfChars; n++) {
			if (tileId >= nOfTiles)
				break;
			for (int row = 0; row < 8; row++)
				fillTileBuffer(row, readROM(is));
			tileReadWrite(tileId++, true, tileBuffer);
		}
	}

	private void handlePutROMCommands(Command c) {
		if (c == okCmd) {
			try {
				int firstChar, nOfChars;
				try {
					firstChar = Integer.parseInt(firstCharItem.getString());
					if (firstChar < 0)
						throw new Throwable();
				} catch (Throwable e) {
					throw new Exception("Invalid First char [>=0]");
				}
				try {
					nOfChars = Integer.parseInt(nOfCharsItem.getString());
					if (nOfChars <= 0)
						throw new Throwable();
				} catch (Throwable e) {
					throw new Exception("Invalid # of chars [>0]");
				}
				InputStream is = getClass().getResourceAsStream(
						romItem.getString());
				if (is == null)
					throw new Exception("ROM " + romItem.getString()
							+ " not found");
				try {
					runPutROM(firstChar, nOfChars, is);
				} finally {
					is.close();
				}
			} catch (Exception e) {
				showError(e.getMessage(), putROMForm);
				return;
			}
		}
		status = EDIT;
		display.setCurrent(this);	
		repaint();
	}
	
	private static final int PIXEL_OFFSET = 6;
	private static final int PIXEL_SIZE = 5;
	private static final int PIXEL_CENTER = 2;
	private static final int TILE_SIZE = PIXEL_OFFSET * 8;
	
	private int viewOX;
	private int viewOY;
	private int viewWidth;
	private int viewHeight;
	private boolean viewTileMode;
	private boolean viewColorMode;
	private int cursorX;
	private int cursorY;
	private int cursorColor;
	private int fgIndex;
	private int bgIndex;
	private boolean showCoords;
	private int[] tileBrush = new int[8 * 4];
	private int[] tileBuffer = new int[8 * 4];
	
	private void fillTileBuffer(int row, int pattern) {
		int value = 0;
		int bits = 0;
		int mask = 0x80;
		int j = row * imageDepth;
		for (int i = 0; i < 8; i++) {
			value <<= imageDepth;
			value |= (pattern & mask) != 0 ? fgIndex : bgIndex;
			bits += imageDepth;
			if (bits == 8) {
				tileBuffer[j++] = value;
				value = 0;
				bits = 0;
			}
			mask >>= 1;
		}
	}
	
	private void enterEditStatus(int image) {
		selectCurrentImage(image);
		int h = Font.getDefaultFont().getHeight();
		viewOX = 0;
		viewOY = 0;
		for (int row = 0; row < 8; row++)
			fillTileBuffer(row, 0);
		viewWidth = (getWidth() - 1) / PIXEL_OFFSET;
		viewHeight = (getHeight() - h - 1) / PIXEL_OFFSET;
		viewTileMode = false;
		viewColorMode = false;
		cursorX = 0;
		cursorY = 0;
		cursorColor = 0xC00000;
		fgIndex = 1;
		bgIndex = 0;
		status = EDIT;
	}
	
	private void updateView() {
		int cursorMin, cursorMax;
		if (viewTileMode) {
			cursorMin = cursorX & ~7;
			cursorMax = cursorMin + 7;
		} else {
			cursorMin = cursorX;
			cursorMax = cursorX;
		}
		if (cursorMin < viewOX)
			viewOX = cursorMin;
		else if (cursorMax >= viewOX + viewWidth)
			viewOX = cursorMax - viewWidth + 1;
		if (viewTileMode) {
			cursorMin = cursorY & ~7;
			cursorMax = cursorMin + 7;
		} else {
			cursorMin = cursorY;
			cursorMax = cursorY;
		}
		if (cursorMin < viewOY)
			viewOY = cursorMin;
		else if (cursorMax >= viewOY + viewHeight)
			viewOY = cursorMax - viewHeight + 1;
	}
	
	private void handleFire() {
		if (viewTileMode) {
			tileReadWrite(cursorX, cursorY, false, tileBuffer);
			int n = 8 * imageDepth;
			boolean isBrush = true;
			for (int i = 0; i < n; i++)
				if (tileBuffer[i] != tileBrush[i])
					isBrush = false;
			if (isBrush) {
				for (int row = 0; row < 8; row++)
					fillTileBuffer(row, 0);
				tileReadWrite(cursorX, cursorY, true, tileBuffer);
			} else {
				tileReadWrite(cursorX, cursorY, true, tileBrush);
			}
		} else {
			putPixel(cursorX, cursorY,
					getPixel(cursorX, cursorY) != fgIndex ? fgIndex : bgIndex);
		}
	}
	
	private boolean handleImageGameAction(int gameAction) {
		int delta = viewTileMode ? 8 : 1;
		switch (gameAction) {
		case UP:
			if (cursorY >= delta)
				cursorY -= delta;
			break;
		case DOWN:
			if (cursorY < imageHeight - delta)
				cursorY += delta;
			break;
		case LEFT:
			if (cursorX >= delta)
				cursorX -= delta;
			break;
		case RIGHT:
			if (cursorX < imageWidth - delta)
				cursorX += delta;
			break;
		case FIRE:
			if (!viewTileMode) {
				handleFire();
				return true;
			}
			break;
		}
		return false;
	}
	
	private int normalizeIndex(int index) {
		int lastIndex = (1 << imageDepth) - 1;
		if (index < 0)
			index = lastIndex;
		else if (index > lastIndex)
			index = 0;
		return index;
	}

	private void imageKeyPressed(int keyCode) {
		boolean repaintStatusBar = false;
		boolean repaintArea = false;
		boolean repaintAll = false;
		int lastViewOX = viewOX;
		int lastViewOY = viewOY;
		int lastCursorX = cursorX;
		int lastCursorY = cursorY;
		int gameAction = 0;
		switch (keyCode) {
		case '1':
			fgIndex = normalizeIndex(fgIndex + 1);
			repaintStatusBar = true;
			break;
		case '2':
			gameAction = UP;
			break;
		case '3':
			bgIndex = normalizeIndex(bgIndex + 1);
			repaintStatusBar = true;
			break;
		case '4':
			gameAction = LEFT;
			break;
		case '5':
			handleFire();
			repaintArea = true;
			break;
		case '6':
			gameAction = RIGHT;
			break;
		case '7':
			if (!viewTileMode) {
				fgIndex = getPixel(cursorX, cursorY);
				repaintStatusBar = true;
			} else {
				tileReadWrite(cursorX, cursorY, false, tileBrush);
			}
			break;
		case '8':
			gameAction = DOWN;
			break;
		case '9':
			if (!viewTileMode) {
				bgIndex = getPixel(cursorX, cursorY);
				repaintStatusBar = true;
			}
			break;
		default:
			gameAction = -1;
			break;
		}
		if (gameAction != 0) {
			if (gameAction == -1)
				gameAction = getGameAction(keyCode);
			if (handleImageGameAction(gameAction))
				repaintArea = true;
		}
		updateView();
		if (lastCursorX != cursorX || lastCursorY != cursorY)
			repaintArea = true;
		if (lastViewOX != viewOX || lastViewOY != viewOY)
			repaintAll = true;
		if (repaintAll
				|| (repaintArea && (viewTileMode || showCoords))) {
			repaint();
		} else if (repaintArea) {
			int minX = lastCursorX < cursorX ? lastCursorX : cursorX;
			int minY = lastCursorY < cursorY ? lastCursorY : cursorY;
			int maxX = lastCursorX > cursorX ? lastCursorX : cursorX;
			int maxY = lastCursorY > cursorY ? lastCursorY : cursorY;
			minX -= viewOX;
			minY -= viewOY;
			maxX -= viewOX;
			maxY -= viewOY;
			maxX++;
			maxY++;
			repaint(minX * PIXEL_OFFSET, Font.getDefaultFont().getHeight()
					+ minY * PIXEL_OFFSET, (maxX - minX) * PIXEL_OFFSET + 2,
					(maxY - minY) * PIXEL_OFFSET + 2);
		} else if (repaintStatusBar) {
			repaint(0, 0, getWidth(), Font.getDefaultFont().getHeight());
		}
	}
	
	private int handleColorGameAction(int gameAction, int index) {
		switch (gameAction) {
		case UP:
			if (!imageRGB)
				index++;
			break;
		case DOWN:
			if (!imageRGB)
				index--;
			break;
		case LEFT:
			if (fgIndex > 0)
				fgIndex--;
			break;
		case RIGHT:
			if (fgIndex < (1 << imageDepth) - 1)
				fgIndex++;
			break;
		}
		return index;
	}
	
	private int normalizeRGB(int value) {
		if (value < 0)
			return 0;
		else if (value > 255)
			return 255;
		return value;
	}
	
	private void colorKeyPressed(int keyCode) {
		int red = 0, green = 0, blue = 0;
		int value = getPaletteEntry(fgIndex);
		int oldValue = value;
		if (imageRGB) {
			red = value >> 16;
			green = (value >> 8) & 0xFF;
			blue = value & 0xFF;
			switch (keyCode) {
			case '1':
				red += 0x10;
				break;
			case '2':
				green += 0x10;
				break;
			case '3':
				blue += 0x10;
				break;
			case '4':
				handleColorGameAction(LEFT, 0);
				break;
			case '6':
				handleColorGameAction(RIGHT, 0);
				break;
			case '7':
				if (red == 0xFF)
					red = 0x100;
				red -= 0x10;
				break;
			case '8':
				if (green == 0xFF)
					green = 0x100;
				green -= 0x10;
				break;
			case '9':
				if (blue == 0xFF)
					blue = 0x100;
				blue -= 0x10;
				break;
			default:
				handleColorGameAction(getGameAction(keyCode), value);
				break;
			}
			if (value == oldValue)
				value = (normalizeRGB(red) << 16) + (normalizeRGB(green) << 8)
						+ normalizeRGB(blue);
		} else {
			switch (keyCode) {
			case '1':
			case '2':
			case '3':
				value = handleColorGameAction(UP, value);
				break;
			case '7':
			case '8':
			case '9':
				value = handleColorGameAction(DOWN, value);
				break;
			default:
				value = handleColorGameAction(getGameAction(keyCode), value);
				break;
			}
		}
		if (value != oldValue) {
			if (imageRGB) {
				setPaletteEntry(fgIndex, value);
			} else {
				if (value < 0)
					value = 0;
				else if (value >= currentPalette.length)
					value = currentPalette.length - 1;
				setPaletteEntry(fgIndex, value);
				colors[fgIndex] = currentPalette[value];
			}
		}
		repaint();
	}
	
	public int getGameAction(int keyCode) {
		try {
			return super.getGameAction(keyCode);
		} catch (Throwable e) {
			return 0;
		}
	}

	protected void keyPressed(int keyCode) {
		if (status != EDIT)
			return;
		if (keyCode == '*') {
			isRunning = true;
			runProgram();
		} else if (keyCode == '0') {
			viewColorMode = !viewColorMode;
			repaint();
		} else if (keyCode == '#') {
			if (!viewColorMode && (imageWidth & 7) == 0
					&& (imageHeight & 7) == 0)
				viewTileMode = !viewTileMode;
			repaint();
		} else {
			if (viewColorMode)
				colorKeyPressed(keyCode);
			else
				imageKeyPressed(keyCode);
		}
	}
	
	private void imagePaint(Graphics g) {
		g.setColor(BLACK);
		int ooy = Font.getDefaultFont().getHeight() + 1;
		int oy = ooy;
		if (g.getClipY() < oy) {
			g.drawString(getImageTag(currentImage, true), 0, 0, Graphics.TOP
					| Graphics.LEFT);
			g.setColor(getColor(fgIndex));
			g.fillRect(getWidth() - PIXEL_OFFSET * 2, 1, PIXEL_SIZE, ooy - 3);
			g.setColor(getColor(bgIndex));
			g.fillRect(getWidth() - PIXEL_OFFSET, 1, PIXEL_SIZE, ooy - 3);
			g.setColor(BLACK);
			int right = getWidth() - PIXEL_OFFSET * 2 - 1;
			if (viewTileMode) {
				g.drawString("T" + (getTileId(cursorX, cursorY) + 1), right, 0,
						Graphics.TOP | Graphics.RIGHT);
			} else {
				if (showCoords)
					g.drawString(cursorX + "," + cursorY, right, 0,
							Graphics.TOP | Graphics.RIGHT);
			}
		}
		for (int y = viewOY; y < imageHeight && (y - viewOY) < viewHeight; y++, oy += PIXEL_OFFSET) {
			if (oy + PIXEL_OFFSET < g.getClipY())
				continue;
			if (oy  > g.getClipY() + g.getClipHeight())
				continue;
			int ox = 1;
			for (int x = viewOX; x < imageWidth && (x - viewOX) < viewWidth; x++, ox += PIXEL_OFFSET) {
				if (ox + PIXEL_OFFSET < g.getClipX())
					continue;
				if (ox  > g.getClipX() + g.getClipWidth())
					continue;
				int pixel = getPixel(x, y);
				g.setColor(getColor(pixel));
				g.fillRect(ox, oy, PIXEL_SIZE, PIXEL_SIZE);
				if (pixel == 0) {
					g.setColor(getColor(1));
					g.drawLine(ox + PIXEL_CENTER, oy + PIXEL_CENTER,
							ox + PIXEL_CENTER, oy + PIXEL_CENTER);
				}
			}
		}
		if (display.isColor())
			g.setColor(cursorColor);
		else
			g.setStrokeStyle(Graphics.DOTTED);
		oy = ooy;
		int ox = 1;
		if (viewTileMode) {			
			ox += (cursorX - viewOX - (cursorX & 7)) * PIXEL_OFFSET;
			oy += (cursorY - viewOY - (cursorY & 7)) * PIXEL_OFFSET;
			g.drawRect(ox - 1, oy - 1, TILE_SIZE, TILE_SIZE);	
		} else {
			ox += (cursorX - viewOX) * PIXEL_OFFSET;
			oy += (cursorY - viewOY) * PIXEL_OFFSET;
			g.drawRect(ox - 1, oy - 1, PIXEL_OFFSET, PIXEL_OFFSET);	
		}
	}

	private void colorPaint(Graphics g) {
		g.setColor(BLACK);
		int ooy = Font.getDefaultFont().getHeight() + 1;
		g.drawString(getImageTag(currentImage, true), 0, 0, 
				Graphics.TOP | Graphics.LEFT);
		g.drawString("C" + fgIndex, getWidth(), 0,
				Graphics.TOP | Graphics.RIGHT);
		int cx = getWidth() >> 1;
		int oy = getHeight() >> 1;
		int ox = cx - PIXEL_CENTER;
		g.setColor(getColor(fgIndex));
		g.fillRect(0, ooy, getWidth(), oy - ooy);
		g.setColor(cursorColor);
		g.drawRect(ox - 1, oy - 1, PIXEL_OFFSET, PIXEL_OFFSET);
		int n = 1 << imageDepth;
		for (int i = 0; i < n; i++) {
			g.setColor(getColor(i));
			g.fillRect(ox + PIXEL_OFFSET * (i - fgIndex), oy, PIXEL_SIZE,
					PIXEL_SIZE);
		}
		if (imageRGB) {
			int w4 = getWidth() >> 2;
			int h4 = getHeight() >> 2;
			int w8 = w4 >> 1;
			int h8 = h4 >> 1;
			g.setColor(0xFF0000);
			g.fillRect(0, oy + h4 - h8, w4, h4);
			g.setColor(0x00FF00);
			g.fillRect(cx - w8, oy + h4 - h8, w4, h4);
			g.setColor(0x0000FF);
			g.fillRect(getWidth() - w4, oy + h4 - h8, w4, h4);
		}
	}

	protected void paint(Graphics g) {
		g.setColor(WHITE);
		g.fillRect(0, 0, getWidth(), getHeight());
		if (status != EDIT)
			return;
		if (viewColorMode)
			colorPaint(g);
		else
			imagePaint(g);
		touchDraw(false, g);
	}

	// adapted from Vice (changes: White has been made pure)
	private static final int standardPalette[] = {
		0x000000, // Black
		0xFFFFFF, // White
		0xBE1A24, // Red
		0x30E6C6, // Cyan
		0xB41AE2, // Purple
		0x1FD21E, // Green
		0x211BAE, // Blue
		0xDFF60A, // Yellow
		0xB84104, // Orange
		0x6A3304, // Brown
		0xFE4A57, // Light Red
		0x424540, // Dark Gray
		0x70746F, // Medium Gray
		0x59FE59, // Light Green
		0x5F53FE, // Light Blue
		0xA4A7A2, // Light Gray		
	};

	private static final int WHITE = 0xFFFFFF;
	private static final int BLACK = 0x000000;
	
	private static final int MAX_WIDTH = 1024;
	private static final int MAX_HEIGHT = 1024;
	
	private Vector images;
	private int rowLength;
	private int imageWidth;
	private int imageHeight;
	private int imageDepth;
	private boolean imageRGB;
	private int imageDataAddress;
	private int colors[];
	
	private int getImageLength(int width, int height, int depth, boolean rgb) {
		return IO.PNG_HEADER_SIZE + 1 + (1 << depth) * (rgb ? 3 : 1)
				+ ((width * depth + 7) >> 3) * height;
	}

	private void scanDataForImages() {
		images = new Vector();
		int n = getDataSize();
		for (int image = 2; image < n - IO.PNG_HEADER_SIZE; image++) {
			if (getData8(image) != IO.REQ_IPNGGEN)
				continue;
			int length = getData16(image - 2);
			if (image + length > n)
				continue;
			int width = getImageWidth(image);
			if (width == 0 || width > MAX_WIDTH)
				continue;
			int height = getImageHeight(image);
			if (height == 0 || height > MAX_HEIGHT)
				continue;
			int depth = getImageDepth(image);
			if (depth != 1 && depth != 2 && depth != 4)
				continue;
			boolean rgb = (getImageFlags(image)
					& IO.VAL_IPNGGEN_FLAGS_PALREF) == 0;
			if (length != getImageLength(width, height, depth, rgb))
				continue;
			images.addElement(new Integer(image));
		}
	}

	private void releaseImages() {
		images = null;
	}
	
	private int getNumberOfImages() {
		return images == null ? 0 : images.size();
	}
	
	private int getImageByIndex(int index) {
		return ((Integer)images.elementAt(index)).intValue();
	}
	
	private int getImageWidth(int image) {
		return getData16(image + IO.PNG_HEADER_WIDTH_OFFSET);
	}

	private int getImageHeight(int image) {
		return getData16(image + IO.PNG_HEADER_HEIGHT_OFFSET);
	}
	
	private int getImageDepth(int image) {
		return getData8(image + IO.PNG_HEADER_DEPTH_OFFSET);
	}
	
	private int getImageFlags(int image) {
		return getData8(image + IO.PNG_HEADER_FLAGS_OFFSET);
	}
	
	private String getImageTag(int image, boolean compact) {
		String tag = addressToTag(image - 2);
		if (!compact)
			tag += " (" + getImageWidth(image) + "x" + getImageHeight(image)
					+ ")";
		return tag;
	}
	
	private int currentImage;
	
	private void selectCurrentImage(int image) {
		currentImage = image;
		imageWidth = getImageWidth(image);
		imageHeight = getImageHeight(image);
		imageDepth = getImageDepth(image);
		imageRGB = (getImageFlags(image) & IO.VAL_IPNGGEN_FLAGS_PALREF) == 0;
		rowLength = ((imageWidth + 7) >> 3) * imageDepth;
		imageDataAddress = image + IO.PNG_HEADER_SIZE + 1
				+ (getData8(image + IO.PNG_HEADER_SIZE) + 1)
				* (imageRGB ? 3 : 1);
		if (imageRGB) {
			colors = null;
		} else { 
			int n = 1 << imageDepth;
			colors = new int[n];
			for (int i = 0; i < n; i++) {
				int index = getPaletteEntry(i);
				if (index < currentPalette.length)
					colors[i] = currentPalette[index];
				else
					colors[i] = BLACK;
			}
		}
	}
	
	private int getPaletteEntry(int index) {
		if (imageRGB) {
			int address = currentImage + IO.PNG_HEADER_SIZE + 1 + index * 3;
			return (getData8(address + 0) << 16) | (getData8(address + 1) << 8)
					| getData8(address + 2);
		} else {
			return getData8(currentImage + IO.PNG_HEADER_SIZE + 1 + index);
		}
	}
	
	private void setPaletteEntry(int index, int value) {
		if (imageRGB) {
			int address = currentImage + IO.PNG_HEADER_SIZE + 1 + index * 3;
			putData8(address + 0, value >> 16);
			putData8(address + 1, (value >> 8) & 0xFF);
			putData8(address + 2, value & 0xFF);
		} else {
			putData8(currentImage + IO.PNG_HEADER_SIZE + 1 + index, value);
		}
	}
	
	private int getColor(int index) {
		if (imageRGB) {
			return getPaletteEntry(index);
		} else {
			if (index >= colors.length)
				return 0;
			return colors[index];
		}
	}
	
	private int pixelReadWrite(int x, int y, int index) {
		int address = imageDataAddress + y * rowLength;
		int mask;
		int _x;
		switch (imageDepth) {
		case 4:
			address += x >> 1;
			x &= 0x1;
			_x = x ^ 0x1;
			mask = 0xF0;
			break;
		case 2:
			address += x >> 2;
			x &= 0x3;
			_x = x ^ 0x3;
			mask = 0xC0;
			break;
		default:
			address += x >> 3;
			x &= 0x7;
			_x = x ^ 0x7;
			mask = 0x80;
			break;
		}
		mask >>= x * imageDepth;
		int value = getData8(address);
		if (index != -1) {
			// write
			value &= mask ^ 0xFF;
			value |= index << (_x * imageDepth);
			putData8(address, value);
			return index;
		} else {
			// read
			return (value & mask) >> (_x * imageDepth);
		}
	}
	
	private void putPixel(int x, int y, int index) {
		pixelReadWrite(x, y, index);
	}
	
	private int getPixel(int x, int y) {
		return pixelReadWrite(x, y, -1);
	}
	
	private int getTileId(int x, int y) {
		return (y >> 3) * (rowLength / imageDepth) + (x >> 3);
	}
	
	private void tileReadWrite(int x, int y, boolean isWrite, int[] buffer) {
		int address = imageDataAddress + (y & ~7) * rowLength;
		int col = x & ~7;
		switch (imageDepth) {
		case 4:
			address += col >> 1;
			break;
		case 2:
			address += col >> 2;
			break;
		default:
			address += col >> 3;
			break;
		}
		int k = 0;
		for (int i = 0; i < 8; i++) {
			if (isWrite) {
				for (int j = 0; j < imageDepth; j++)
					putData8(address + i * rowLength + j, buffer[k++]);
			} else {
				for (int j = 0; j < imageDepth; j++)
					buffer[k++] = getData8(address + i * rowLength + j);
			}
		}
	}
	
	private void tileReadWrite(int tileId, boolean isWrite,
			int[] buffer) {
		int x = (tileId % (rowLength / imageDepth)) << 3;
		int y = (tileId / (rowLength / imageDepth)) << 3;
		tileReadWrite(x, y, isWrite, buffer);
	}
	
	private void makeImage(int page, int offset, int width, int height,
			int depth, boolean rgb) throws Exception {
		int address = makeAddress(page, offset);
		if (address < 0)
			throw new Exception("Starting address is out of range");
		int length = getImageLength(width, height, depth, rgb);
		if (address + length + 2 > getDataSize())
			throw new Exception("Image is too big to fit in data");
		for (int i = 0; i < length + 2; i++)
			if (getData8(address + i) != 0)
				throw new Exception("Target area is not empty");
		putData16(address, length);
		int image = address + 2;
		putData8(image, IO.REQ_IPNGGEN);
		putData8(image + IO.PNG_HEADER_IMAGEID_OFFSET, 1);
		putData16(image + IO.PNG_HEADER_WIDTH_OFFSET, width);
		putData16(image + IO.PNG_HEADER_HEIGHT_OFFSET, height);
		putData8(image + IO.PNG_HEADER_DEPTH_OFFSET, depth);
		putData8(image + IO.PNG_HEADER_COLOR_OFFSET,
				IO.VAL_IPNGGEN_CT_INDEXED_COLOR);
		putData8(image + IO.PNG_HEADER_FLAGS_OFFSET,
				IO.VAL_IPNGGEN_FLAGS_IDX0TRANSP
						+ (rgb ? 0 : IO.VAL_IPNGGEN_FLAGS_PALREF));
		address = image + IO.PNG_HEADER_SIZE;
		int n = (1 << depth) - 1;
		putData8(address, n);
		for (int i = 0; i <= n; i++) {
			int palIndex = i < 2 ? 1 - i : i;
			if (rgb) {
				int color = standardPalette[palIndex];
				putData8(address + 1 + i * 3 + 0, color >> 16);
				putData8(address + 1 + i * 3 + 1, (color >> 8) & 0xFF);
				putData8(address + 1 + i * 3 + 2, color & 0xFF);
			} else {
				putData8(address + 1 + i, palIndex);
			}
		}
	}

	private boolean programAlreadyMarkedAsModified;
	
	private void attachData() {
		attachProgram();
		programAlreadyMarkedAsModified = false;
	}
	
	private void detachData() {
		detachProgram();
	}
	
	private void dataHasBeenSaved() {
		programAlreadyMarkedAsModified = false;
	}

	private String addressToTag(int address) {
		int page = 3 + (programFile[0][JBFile.OFFSET_CODEPAGES] & 0xFF);
		page += address >> 8;
		int offset = address & 0xFF;
		return page + ":" + offset;
	}
	
	private int makeAddress(int page, int offset) {
		page -= 3 + (programFile[0][JBFile.OFFSET_CODEPAGES] & 0xFF);
		int address = (page << 8) + offset;
		if (address < 0 || address > getDataSize())
			return -1;
		return address;
	}
	
	private int getDataSize() {
		if (programFile == null)
			return 0;
		return (programFile[0][JBFile.OFFSET_DATAPAGES] & 0xFF) << 8;
	}
	
	private int getFirstDataPage() {
		return 3 + (programFile[0][JBFile.OFFSET_CODEPAGES] & 0xFF);
	}
	
	private int getLastDataPage() {
		return getFirstDataPage() - 1
				+ (programFile[0][JBFile.OFFSET_DATAPAGES] & 0xFF);
	}
	
	private byte[] getPage(int address, boolean create) {
		int programPage = 1 + (address >> 8) +
				(programFile[0][JBFile.OFFSET_CODEPAGES] & 0xFF);
		byte[] page = programFile[programPage];
		if (page == null && create)
			page = programFile[programPage] = new byte[256];
		return page;
	}
	
	private void putData8(int address, int value) {
		if (!programAlreadyMarkedAsModified) {
			markProgramAsModified();
			programAlreadyMarkedAsModified = true;
		}
		getPage(address, true)[address & 0xFF] = (byte)value;
	}

	private int getData8(int address) {
		byte[] page = getPage(address, false);
		if (page == null)
			return 0;
		return page[address & 0xFF] & 0xFF;
	}
	
	private void putData16(int address, int value) {
		putData8(address, value & 0xFF);
		putData8(address + 1, value >> 8);
	}
	
	private int getData16(int address) {
		return getData8(address) | (getData8(address + 1) << 8);
	}
	
	private Command saveOkCmd = new Command("", Command.SCREEN, 0);
	private Command saveFailedCmd = new Command("", Command.SCREEN, 0);

	private Module findSaveSvc() {
		return (Module)jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, SaveSvc.TAG);
	}
	
	private void save(boolean saveAs) {
		findSaveSvc().opI(SaveSvc.OP_SAVE, 0, new Object[] {
				new Boolean(saveAs),
				this,
				this,
				saveOkCmd,
				saveFailedCmd
		});
	}

	private byte[][] programFile;

	private void attachProgram() {
		programFile = (byte[][])jbit.opO(JBitSvc.OP_POOL_GET, 0,
				JBitSvc.POOL_ID_PROGRAM_FILE);
	}
	
	private void detachProgram() {
		programFile = null;
	}
	
	private void runProgram() {
		jbit.opI(JBitSvc.OP_REPLACE_WITH_VM, VMSvc.START_MODE_RUN, this);
	}
	
	private void markProgramAsModified() {
		jbit.opI(JBitSvc.OP_POOL_PUT, 0, new Object[] {
				JBitSvc.POOL_ID_PROGRAM_MODIFIED, new Boolean(true)
		});
	}
	
	private Object getMetadata(int metadataId) {
		switch (metadataId) {
		case METADATA_ID_LABEL:
			return "Paint";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_TOP_LEVEL);
		case METADATA_ID_SERVICES:
			return new String[] {};
		default:
			return null;
		}
	}
	
	private void back() {
		jbit.opI(JBitSvc.OP_REPLACE_WITH_SERVICE, 0, null);
	}
	
	private final Integer touchContext = new Integer(TouchSvc.CONTEXT_NAVIGATION);
	private Module touch;
	private Object[] touchArgs = new Object[3];
	
	protected void touchDraw(boolean bold, Graphics g) {
		if (touch == null)
			return;
		touchArgs[0] = touchContext;
		touchArgs[1] = this;
		touchArgs[2] = g;
		try {
			touch.opI(TouchSvc.OP_DRAW, bold ? -1 : 0, touchArgs);
		} finally {
			touchArgs[2] = null;
		}
	}
	
	protected void pointerPressed(int x, int y) {
		if (touch == null)
			return;
		touchArgs[0] = touchContext;
		touchArgs[1] = this;
		touchArgs[2] = null;
		int keyCode = touch.opI(TouchSvc.OP_POINTER_PRESSED,
				((y & 0xFFFF) << 16) | (x & 0xFFFF), touchArgs);
		if (keyCode != 0)
			keyPressed(keyCode);
		else
			repaint();
	}

	private Module jbit;
	private Display display;
	
	public Object opO(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_GET_METADATA:
			return getMetadata(iArg);
		case OP_GET_DISPLAYABLE:
			return getDisplayable();
		}
		return null;
	}
	
	public int opI(int opId, int iArg, Object oArg) {
		switch (opId) {
		case OP_INIT:
			jbit = (Module)oArg;
			touch = (Module)jbit.opO(JBitSvc.OP_FIND_SERVICE, 0, TouchSvc.TAG);
			display = (Display)jbit.opO(JBitSvc.OP_GET_DISPLAY, 0, null);
			return init();
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
