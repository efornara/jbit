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

//////////////////////////////////////////////////////////////////////////////
// Subsystems 
//////////////////////////////////////////////////////////////////////////////

// #if !IO_DISABLE_RECSTORE
// #define ENABLE_RECSTORE
// #endif

// #if !IO_DISABLE_IMAGE
// #define ENABLE_IMAGE
// #endif

// #if !IO_DISABLE_CONSOLE
// #define ENABLE_CONSOLE
// #endif

// #if !IO_DISABLE_GAMEAPI && JavaPlatform != "MIDP/1.0"
// #define ENABLE_GAMEAPI
// #define ENABLE_GAME_CANVAS
// #endif

// #if !IO_DISABLE_DUMMY
// #define ENABLE_IMAGE_DUMMY
// #endif

// #if !IO_DISABLE_PNGGEN
// #define ENABLE_IMAGE_PNGGEN
// #endif

// #if !IO_DISABLE_EFFECTS && JavaPlatform != "MIDP/1.0"
// #define ENABLE_EFFECTS
// #endif


// TODO find pragma to disable import warnings

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.util.Vector;

import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Font;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;
import javax.microedition.midlet.MIDlet;

//#if ENABLE_RECSTORE
import javax.microedition.rms.RecordEnumeration;
import javax.microedition.rms.RecordStore;
//#endif

// #if ENABLE_GAME_CANVAS
// #else
//@import javax.microedition.lcdui.Canvas;
// #endif
// #if ENABLE_GAMEAPI
import javax.microedition.lcdui.game.GameCanvas;
import javax.microedition.lcdui.game.Layer;
import javax.microedition.lcdui.game.Sprite;
import javax.microedition.lcdui.game.TiledLayer;
import javax.microedition.lcdui.game.LayerManager;
//#endif

//#if ENABLE_EFFECTS
import javax.microedition.media.Manager;
//#endif

// #if ENABLE_GAME_CANVAS
public class IO extends GameCanvas implements Module, CommandListener {
// #else
//@public class IO extends Canvas implements Module, CommandListener {
// #endif
// TODO nokia, siemens custom canvas

public static final int REG_REQPUT = 0x01;
public static final int REG_REQEND = 0x02;
public static final int REG_REQRES = 0x03;
public static final int REG_REQPTRLO = 0x04;
public static final int REG_REQPTRHI = 0x05;
public static final int REG_ENABLE = 0x10;
public static final int REG_FRMFPS = 0x11;
public static final int REG_FRMDRAW = 0x12;
public static final int REG_GKEY0 = 0x13;
public static final int REG_GKEY1 = 0x14;
public static final int REG_RANDOM = 0x17;
public static final int REG_KEYBUF = 0x18;
public static final int REG_CONCOLS = 0x20;
public static final int REG_CONROWS = 0x21;
public static final int REG_CONCX = 0x22;
public static final int REG_CONCY = 0x23;
public static final int REG_CONCCHR = 0x24;
public static final int REG_CONCFG = 0x25;
public static final int REG_CONCBG = 0x26;
public static final int REG_CONVIDEO = 0x28;
public static final int REG_CONROW0 = 0x28;
public static final int REG_CONROW1 = 0x32;
public static final int REG_CONROW2 = 0x3C;
public static final int REG_CONROW3 = 0x46;
public static final int REG_LID = 0x50;
public static final int REG_LCTL = 0x51;
public static final int REG_LX = 0x52;
public static final int REG_LY = 0x53;
public static final int REG_SFRAME = 0x54;
public static final int REG_STRANSFM = 0x55;
public static final int REG_SCWITH = 0x56;
public static final int REG_TCOLLO = 0x57;
public static final int REG_TCOLHI = 0x58;
public static final int REG_TROWLO = 0x59;
public static final int REG_TROWHI = 0x5A;
public static final int REG_TCELL = 0x5B;
public static final int REG_REQDAT = 0x60;
public static final byte REQ_NOREQ = (byte)0x00;
public static final byte REQ_TIME = (byte)0x02;
public static final byte REQ_LOADROM = (byte)0x06;
public static final byte REQ_RSFORMAT = (byte)0x08;
public static final byte REQ_RLOAD = (byte)0x09;
public static final byte REQ_RSAVE = (byte)0x0A;
public static final byte REQ_RDELETE = (byte)0x0B;
public static final byte REQ_DPYINFO = (byte)0x10;
public static final byte REQ_SETBGCOL = (byte)0x11;
public static final byte REQ_SETPAL = (byte)0x12;
public static final byte REQ_SETBGIMG = (byte)0x13;
public static final byte REQ_IDESTROY = (byte)0x14;
public static final byte REQ_IDIM = (byte)0x15;
public static final byte REQ_IINFO = (byte)0x16;
public static final byte REQ_ILOAD = (byte)0x17;
public static final byte REQ_IDUMMY = (byte)0x18;
public static final byte REQ_IPNGGEN = (byte)0x19;
public static final byte REQ_IEMPTY = (byte)0x1A;
public static final byte REQ_IMKIMMUT = (byte)0x1B;
public static final byte REQ_IRAWRGBA = (byte)0x1C;
public static final byte REQ_LMVIEW = (byte)0x20;
public static final byte REQ_LMPOS = (byte)0x21;
public static final byte REQ_LDESTROY = (byte)0x22;
public static final byte REQ_LDIM = (byte)0x23;
public static final byte REQ_LTILED = (byte)0x24;
public static final byte REQ_LSPRITE = (byte)0x25;
public static final byte REQ_LSETPOS = (byte)0x26;
public static final byte REQ_LGETPOS = (byte)0x27;
public static final byte REQ_LMOVE = (byte)0x28;
public static final byte REQ_LSETPRI = (byte)0x29;
public static final byte REQ_LGETPRI = (byte)0x2A;
public static final byte REQ_LTLANIM = (byte)0x2B;
public static final byte REQ_LTLFILL = (byte)0x2C;
public static final byte REQ_LTLPUT = (byte)0x2D;
public static final byte REQ_LTLSCRLL = (byte)0x2E;
public static final byte REQ_LSPCOPY = (byte)0x2F;
public static final byte REQ_LSPAPOS = (byte)0x30;
public static final byte REQ_LSPREFPX = (byte)0x31;
public static final byte REQ_LSPCLRCT = (byte)0x32;
public static final byte REQ_GAMESET = (byte)0x3C;
public static final byte REQ_FXTONE = (byte)0x40;
public static final byte REQ_FXVIBRA = (byte)0x41;
public static final byte REQ_FXFLASH = (byte)0x42;
public static final byte VAL_ENABLE_BGCOL = (byte)0x01;
public static final byte VAL_ENABLE_BGIMG = (byte)0x02;
public static final byte VAL_ENABLE_CONSOLE = (byte)0x04;
public static final byte VAL_ENABLE_LAYERS = (byte)0x08;
public static final byte VAL_TIME_ABS = (byte)0x01;
public static final byte VAL_TIME_RESET = (byte)0x02;
public static final byte VAL_TIME_1 = (byte)0x01;
public static final byte VAL_TIME_10 = (byte)0x02;
public static final byte VAL_TIME_100 = (byte)0x03;
public static final byte VAL_TIME_1000 = (byte)0x04;
public static final byte VAL_DPYINFO_WIDTH = (byte)0x00;
public static final byte VAL_DPYINFO_HEIGHT = (byte)0x02;
public static final byte VAL_DPYINFO_COLORDEPTH = (byte)0x04;
public static final byte VAL_DPYINFO_ALPHADEPTH = (byte)0x05;
public static final byte VAL_DPYINFO_FLAGS = (byte)0x06;
public static final byte VAL_DPYINFO_FLAGS_ISCOLOR = (byte)0x80;
public static final byte VAL_DPYINFO_FLAGS_ISMIDP2 = (byte)0x40;
public static final byte VAL_DPYINFO_FLAGS_ISTOUCH = (byte)0x20;
public static final byte VAL_SETPAL_STD = (byte)0x00;
public static final byte VAL_SETPAL_RGB = (byte)0x01;
public static final byte VAL_IINFO_WIDTH = (byte)0x00;
public static final byte VAL_IINFO_HEIGHT = (byte)0x02;
public static final byte VAL_IINFO_FLAGS = (byte)0x04;
public static final byte VAL_IINFO_FLAGS_ISMUTABLE = (byte)0x80;
public static final byte VAL_IDUMMY_SIMPLE = (byte)0x01;
public static final byte VAL_IDUMMY_SPRITE = (byte)0x02;
public static final byte VAL_IDUMMY_TILES = (byte)0x03;
public static final byte VAL_IPNGGEN_CT_GRAYSCALE = (byte)0x00;
public static final byte VAL_IPNGGEN_CT_TRUECOLOR = (byte)0x02;
public static final byte VAL_IPNGGEN_CT_INDEXED_COLOR = (byte)0x03;
public static final byte VAL_IPNGGEN_CT_GRAYSCALE_ALPHA = (byte)0x04;
public static final byte VAL_IPNGGEN_CT_TRUECOLOR_ALPHA = (byte)0x06;
public static final byte VAL_IPNGGEN_FLAGS_IDX0TRANSP = (byte)0x01;
public static final byte VAL_IPNGGEN_FLAGS_PALREF = (byte)0x02;
public static final byte VAL_IPNGGEN_FLAGS_ZOOM0 = (byte)0x04;
public static final byte VAL_IPNGGEN_FLAGS_ZOOM1 = (byte)0x08;
public static final byte VAL_IRAWRGBA_FLAGS_ALPHA = (byte)0x01;
public static final byte VAL_DATATYPE_U8 = (byte)0x01;
public static final byte VAL_DATATYPE_I8 = (byte)0x02;
public static final byte VAL_DATATYPE_U16 = (byte)0x03;
public static final byte VAL_DATATYPE_I16 = (byte)0x04;
public static final byte VAL_LCTL_SHIFTX0 = (byte)0x01;
public static final byte VAL_LCTL_SHIFTX1 = (byte)0x02;
public static final byte VAL_LCTL_SHIFTY0 = (byte)0x04;
public static final byte VAL_LCTL_SHIFTY1 = (byte)0x08;
public static final byte VAL_LCTL_PXLCOLL = (byte)0x40;
public static final byte VAL_LCTL_ENABLE = (byte)0x80;
public static final byte VAL_LCTL_SHIFTX_MASK = (byte)0x03;
public static final byte VAL_LCTL_SHIFTY_MASK = (byte)0x0C;
public static final byte VAL_LTRANSFM_NONE = (byte)0x00;
public static final byte VAL_LTRANSFM_ROT90 = (byte)0x05;
public static final byte VAL_LTRANSFM_ROT180 = (byte)0x03;
public static final byte VAL_LTRANSFM_ROT270 = (byte)0x06;
public static final byte VAL_LTRANSFM_MIRROR = (byte)0x02;
public static final byte VAL_LTRANSFM_MIRROR_ROT90 = (byte)0x07;
public static final byte VAL_LTRANSFM_MIRROR_ROT180 = (byte)0x01;
public static final byte VAL_LTRANSFM_MIRROR_ROT270 = (byte)0x04;
public static final byte VAL_GKEY0_UP = (byte)0x02;
public static final byte VAL_GKEY0_LEFT = (byte)0x04;
public static final byte VAL_GKEY0_RIGHT = (byte)0x20;
public static final byte VAL_GKEY0_DOWN = (byte)0x40;
public static final byte VAL_GKEY1_FIRE = (byte)0x01;
public static final byte VAL_GKEY1_A = (byte)0x02;
public static final byte VAL_GKEY1_B = (byte)0x04;
public static final byte VAL_GKEY1_C = (byte)0x08;
public static final byte VAL_GKEY1_D = (byte)0x10;
public static final byte VAL_GAMESET_COLS = (byte)0x00;
public static final byte VAL_GAMESET_ROWS = (byte)0x02;
public static final byte VAL_TILESET_SILK = (byte)0xFF;
public static final byte VAL_TILESET_FONT = (byte)0xFE;
public static final byte ALINE_TOP = (byte)0x01;
public static final byte ALINE_LEFT = (byte)0x02;
public static final byte ALINE_RIGHT = (byte)0x04;
public static final byte ALINE_BOTTOM = (byte)0x08;
public static final byte COLOR_BLACK = (byte)0x00;
public static final byte COLOR_WHITE = (byte)0x01;
public static final byte COLOR_RED = (byte)0x02;
public static final byte COLOR_CYAN = (byte)0x03;
public static final byte COLOR_VIOLET = (byte)0x04;
public static final byte COLOR_PURPLE = (byte)0x04;
public static final byte COLOR_GREEN = (byte)0x05;
public static final byte COLOR_BLUE = (byte)0x06;
public static final byte COLOR_YELLOW = (byte)0x07;
public static final byte COLOR_ORANGE = (byte)0x08;
public static final byte COLOR_BROWN = (byte)0x09;
public static final byte COLOR_LIGHTRED = (byte)0x0A;
public static final byte COLOR_GRAY1 = (byte)0x0B;
public static final byte COLOR_GRAY2 = (byte)0x0C;
public static final byte COLOR_LIGHTGREEN = (byte)0x0D;
public static final byte COLOR_LIGHTBLUE = (byte)0x0E;
public static final byte COLOR_GRAY3 = (byte)0x0F;
public static final byte CH_ULCORNER = (byte)0x8C;
public static final byte CH_URCORNER = (byte)0x8A;
public static final byte CH_LLCORNER = (byte)0x85;
public static final byte CH_LRCORNER = (byte)0x83;
public static final byte CH_HLINE = (byte)0x86;
public static final byte CH_VLINE = (byte)0x89;
public static final byte CH_TTEE = (byte)0x8E;
public static final byte CH_RTEE = (byte)0x89;
public static final byte CH_BTEE = (byte)0x8B;
public static final byte CH_LTEE = (byte)0x8D;
public static final byte CH_CROSS = (byte)0x8F;

// #if DEBUG
//@private String [][] labels = {
//@{ "REG_" + "REQPUT", "01" },
//@{ "REG_" + "REQEND", "02" },
//@{ "REG_" + "REQRES", "03" },
//@{ "REG_" + "REQPTRLO", "04" },
//@{ "REG_" + "REQPTRHI", "05" },
//@{ "REG_" + "ENABLE", "10" },
//@{ "REG_" + "FRMFPS", "11" },
//@{ "REG_" + "FRMDRAW", "12" },
//@{ "REG_" + "GKEY0", "13" },
//@{ "REG_" + "GKEY1", "14" },
//@{ "REG_" + "RANDOM", "17" },
//@{ "REG_" + "KEYBUF", "18" },
//@{ "REG_" + "CONCOLS", "20" },
//@{ "REG_" + "CONROWS", "21" },
//@{ "REG_" + "CONCX", "22" },
//@{ "REG_" + "CONCY", "23" },
//@{ "REG_" + "CONCCHR", "24" },
//@{ "REG_" + "CONCFG", "25" },
//@{ "REG_" + "CONCBG", "26" },
//@{ "REG_" + "CONVIDEO", "28" },
//@{ "REG_" + "CONROW0", "28" },
//@{ "REG_" + "CONROW1", "32" },
//@{ "REG_" + "CONROW2", "3C" },
//@{ "REG_" + "CONROW3", "46" },
//@{ "REG_" + "LID", "50" },
//@{ "REG_" + "LCTL", "51" },
//@{ "REG_" + "LX", "52" },
//@{ "REG_" + "LY", "53" },
//@{ "REG_" + "SFRAME", "54" },
//@{ "REG_" + "STRANSFM", "55" },
//@{ "REG_" + "SCWITH", "56" },
//@{ "REG_" + "TCOLLO", "57" },
//@{ "REG_" + "TCOLHI", "58" },
//@{ "REG_" + "TROWLO", "59" },
//@{ "REG_" + "TROWHI", "5A" },
//@{ "REG_" + "TCELL", "5B" },
//@{ "REG_" + "REQDAT", "60" },
//@{ "REQ_" + "NOREQ", "00" },
//@{ "REQ_" + "TIME", "02" },
//@{ "REQ_" + "LOADROM", "06" },
//@{ "REQ_" + "RSFORMAT", "08" },
//@{ "REQ_" + "RLOAD", "09" },
//@{ "REQ_" + "RSAVE", "0A" },
//@{ "REQ_" + "RDELETE", "0B" },
//@{ "REQ_" + "DPYINFO", "10" },
//@{ "REQ_" + "SETBGCOL", "11" },
//@{ "REQ_" + "SETPAL", "12" },
//@{ "REQ_" + "SETBGIMG", "13" },
//@{ "REQ_" + "IDESTROY", "14" },
//@{ "REQ_" + "IDIM", "15" },
//@{ "REQ_" + "IINFO", "16" },
//@{ "REQ_" + "ILOAD", "17" },
//@{ "REQ_" + "IDUMMY", "18" },
//@{ "REQ_" + "IPNGGEN", "19" },
//@{ "REQ_" + "IEMPTY", "1A" },
//@{ "REQ_" + "IMKIMMUT", "1B" },
//@{ "REQ_" + "IRAWRGBA", "1C" },
//@{ "REQ_" + "LMVIEW", "20" },
//@{ "REQ_" + "LMPOS", "21" },
//@{ "REQ_" + "LDESTROY", "22" },
//@{ "REQ_" + "LDIM", "23" },
//@{ "REQ_" + "LTILED", "24" },
//@{ "REQ_" + "LSPRITE", "25" },
//@{ "REQ_" + "LSETPOS", "26" },
//@{ "REQ_" + "LGETPOS", "27" },
//@{ "REQ_" + "LMOVE", "28" },
//@{ "REQ_" + "LSETPRI", "29" },
//@{ "REQ_" + "LGETPRI", "2A" },
//@{ "REQ_" + "LTLANIM", "2B" },
//@{ "REQ_" + "LTLFILL", "2C" },
//@{ "REQ_" + "LTLPUT", "2D" },
//@{ "REQ_" + "LTLSCRLL", "2E" },
//@{ "REQ_" + "LSPCOPY", "2F" },
//@{ "REQ_" + "LSPAPOS", "30" },
//@{ "REQ_" + "LSPREFPX", "31" },
//@{ "REQ_" + "LSPCLRCT", "32" },
//@{ "REQ_" + "GAMESET", "3C" },
//@{ "REQ_" + "FXTONE", "40" },
//@{ "REQ_" + "FXVIBRA", "41" },
//@{ "REQ_" + "FXFLASH", "42" },
//@{ "VAL_" + "ENABLE_BGCOL", "01" },
//@{ "VAL_" + "ENABLE_BGIMG", "02" },
//@{ "VAL_" + "ENABLE_CONSOLE", "04" },
//@{ "VAL_" + "ENABLE_LAYERS", "08" },
//@{ "VAL_" + "TIME_ABS", "01" },
//@{ "VAL_" + "TIME_RESET", "02" },
//@{ "VAL_" + "TIME_1", "01" },
//@{ "VAL_" + "TIME_10", "02" },
//@{ "VAL_" + "TIME_100", "03" },
//@{ "VAL_" + "TIME_1000", "04" },
//@{ "VAL_" + "DPYINFO_WIDTH", "00" },
//@{ "VAL_" + "DPYINFO_HEIGHT", "02" },
//@{ "VAL_" + "DPYINFO_COLORDEPTH", "04" },
//@{ "VAL_" + "DPYINFO_ALPHADEPTH", "05" },
//@{ "VAL_" + "DPYINFO_FLAGS", "06" },
//@{ "VAL_" + "DPYINFO_FLAGS_ISCOLOR", "80" },
//@{ "VAL_" + "DPYINFO_FLAGS_ISMIDP2", "40" },
//@{ "VAL_" + "DPYINFO_FLAGS_ISTOUCH", "20" },
//@{ "VAL_" + "IINFO_WIDTH", "00" },
//@{ "VAL_" + "IINFO_HEIGHT", "02" },
//@{ "VAL_" + "IINFO_FLAGS", "04" },
//@{ "VAL_" + "IINFO_FLAGS_ISMUTABLE", "80" },
//@{ "VAL_" + "IDUMMY_SIMPLE", "01" },
//@{ "VAL_" + "IDUMMY_SPRITE", "02" },
//@{ "VAL_" + "IDUMMY_TILES", "03" },
//@{ "VAL_" + "IPNGGEN_CT_GRAYSCALE", "00" },
//@{ "VAL_" + "IPNGGEN_CT_TRUECOLOR", "02" },
//@{ "VAL_" + "IPNGGEN_CT_INDEXED_COLOR", "03" },
//@{ "VAL_" + "IPNGGEN_CT_GRAYSCALE_ALPHA", "04" },
//@{ "VAL_" + "IPNGGEN_CT_TRUECOLOR_ALPHA", "06" },
//@{ "VAL_" + "IPNGGEN_FLAGS_IDX0TRANSP", "01" },
//@{ "VAL_" + "IPNGGEN_FLAGS_PALREF", "02" },
//@{ "VAL_" + "IPNGGEN_FLAGS_ZOOM0", "04" },
//@{ "VAL_" + "IPNGGEN_FLAGS_ZOOM1", "08" },
//@{ "VAL_" + "IRAWRGBA_FLAGS_ALPHA", "01" },
//@{ "VAL_" + "DATATYPE_U8", "01" },
//@{ "VAL_" + "DATATYPE_I8", "02" },
//@{ "VAL_" + "DATATYPE_U16", "03" },
//@{ "VAL_" + "DATATYPE_I16", "04" },
//@{ "VAL_" + "LCTL_SHIFTX0", "01" },
//@{ "VAL_" + "LCTL_SHIFTX1", "02" },
//@{ "VAL_" + "LCTL_SHIFTY0", "04" },
//@{ "VAL_" + "LCTL_SHIFTY1", "08" },
//@{ "VAL_" + "LCTL_PXLCOLL", "40" },
//@{ "VAL_" + "LCTL_ENABLE", "80" },
//@{ "VAL_" + "LCTL_SHIFTX_MASK", "03" },
//@{ "VAL_" + "LCTL_SHIFTY_MASK", "0C" },
//@{ "VAL_" + "LTRANSFM_NONE", "00" },
//@{ "VAL_" + "LTRANSFM_ROT90", "05" },
//@{ "VAL_" + "LTRANSFM_ROT180", "03" },
//@{ "VAL_" + "LTRANSFM_ROT270", "06" },
//@{ "VAL_" + "LTRANSFM_MIRROR", "02" },
//@{ "VAL_" + "LTRANSFM_MIRROR_ROT90", "07" },
//@{ "VAL_" + "LTRANSFM_MIRROR_ROT180", "01" },
//@{ "VAL_" + "LTRANSFM_MIRROR_ROT270", "04" },
//@{ "VAL_" + "GKEY0_UP", "02" },
//@{ "VAL_" + "GKEY0_LEFT", "04" },
//@{ "VAL_" + "GKEY0_RIGHT", "20" },
//@{ "VAL_" + "GKEY0_DOWN", "40" },
//@{ "VAL_" + "GKEY1_FIRE", "01" },
//@{ "VAL_" + "GKEY1_A", "02" },
//@{ "VAL_" + "GKEY1_B", "04" },
//@{ "VAL_" + "GKEY1_C", "08" },
//@{ "VAL_" + "GKEY1_D", "10" },
//@{ "VAL_" + "GAMESET_COLS", "00" },
//@{ "VAL_" + "GAMESET_ROWS", "02" },
//@{ "VAL_" + "TILESET_SILK", "FF" },
//@{ "VAL_" + "TILESET_FONT", "FE" },
//@	};
//@	
//@	String labelToString(String prefix, int value) {
//@		value = value & 0xFF;
//@		String s = Integer.toHexString(value).toUpperCase();
//@		if (s.length() == 1)
//@			s = "0" + s;
//@		for (int i = 0; i < labels.length; i++)
//@			if (labels[i][1].equals(s) && labels[i][0].startsWith(prefix))
//@				return labels[i][0];
//@		return prefix + "? (" + value + ")";
//@	}
// #endif

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
	
	private static final int STANDARD_PALETTE_MASK = 0xF;

	private static final int UNLOADED = 0;
	private static final int OPERATIONAL = 1;
	//private static final int ERROR = 2;

	private Display display;
	private Module as;

	private int vmStatus = IOSvc.VM_STATUS_RUNNING;
	private int ioStatus = UNLOADED;
	private boolean signalUserBreak;
	private boolean signalSuspendCPU;
	private boolean signalResumeCPU;

	/*************************************************************************
	 * Core
	 *************************************************************************/
	
	private static final long MAXRAND = 0xFFFFFFFFFFFFL;
	private static final int DEFAULT_IMAGE_DIM = 4;
	
	// #if ENABLE_GAME_CANVAS
	private Graphics g;
	// #endif
	private byte[] m = new byte[256];

	private byte[] req = new byte[256];
	private int reqlen;
	private int reqcur;

	// stream common variables
	private ByteArrayOutputStream strBAOS;
	private DataOutputStream strOS;
	private int strStatus;
	private int strNextSection;

	private long startTime;
	private long nextFrame;
	private int frameInterval;
	private boolean waitingForFrame;
	
	private long rndSeed0;
	private long rndSeed1;
	private int rndN;
	private long rndDivisor;
	
	private int[] palette;
	private int paletteMask;
	private int bgCol;

	// #ifdef ENABLE_IMAGE
	private Image bgImg;
	private Image[] images;
	// #endif

	private void coreReset() {
		startTime = System.currentTimeMillis();
		for (int i = 0; i < 256; i++)
			m[i] = 0;
		m[REG_ENABLE] = VAL_ENABLE_BGCOL;
		// #if ENABLE_CONSOLE
		m[REG_ENABLE] |= VAL_ENABLE_CONSOLE;
		// #endif
		doFrmFpsPut(40);
		rndSeed0 = System.currentTimeMillis() & MAXRAND;
		rndSeed1 = 0;
		doRandomPut(255);
		palette = standardPalette;
		paletteMask = STANDARD_PALETTE_MASK;
		bgCol = 0xFFFFFF;
		// #ifdef ENABLE_IMAGE
		bgImg = null;
		images = new Image[DEFAULT_IMAGE_DIM];
		// #endif
	}
	
	// FRMFPS is the desired # of frames per second * 4
	private void doFrmFpsPut(int value) {
		m[REG_FRMFPS] = (byte)value;
		if (value == 0) {
			frameInterval = 0;
			nextFrame = 0;
		} else {
			frameInterval = 4000 / value;
			nextFrame = System.currentTimeMillis() + frameInterval;
		}
	}

	private long rndNext() {
		rndSeed0 = (rndSeed0 * 0x5DEECE66DL + 0xBL) & MAXRAND;
		return rndSeed0;
	}
	
	private int doRandomGet() {
		long n;
		while (rndN <= (n = rndNext() / rndDivisor))
			;
		return (int)n;
	}
	
	private void doRandomPut(int max) {
		if (max == 0) {
			long t = rndSeed0;
			rndSeed0 = rndSeed1;
			rndSeed1 = t;
		} else {
			rndN = max + 1;
			rndDivisor = MAXRAND / rndN;
		}
	}
	
	private void doTime() {
		boolean isAbs = false;
		int fract = VAL_TIME_1000;
		switch  (reqlen) {
		case 1:
			break;
		case 3:
			fract = req[2];
		case 2:
			isAbs = req[1] == VAL_TIME_ABS;
			break;
		default:
			throw new RuntimeException();
		}
		reqcur = reqlen;
		long t = System.currentTimeMillis();
		if (!isAbs)
			t -= startTime;
		switch (fract) {
		case VAL_TIME_1:
			t /= 1000;
			break;
		case VAL_TIME_10:
			t /= 100;
			break;
		case VAL_TIME_100:
			t /= 10;
			break;
		}
		putLong(REG_REQDAT, t);
	}
	
	private void doLoadROM() throws Exception {
		int address = parseU16();
		String resName = parseString0();
		int offset = 0, size = 0;
		if (reqcur != reqlen) {
			offset = parseU16();
			size = parseU16();
		}
		InputStream is = null;
		try {
			int i;
			is = getClass().getResourceAsStream(resName);
			for (i = 0; i < offset; i++)
				if (is.read() == -1)
					throw new RuntimeException();
			for (i = 0; size == 0 || i < size; i++, address++) {
				int value = is.read();
				if (value == -1 && size == 0)
					break;
				if (value == -1 || address > 0xFF00)
					throw new RuntimeException();
				as.put(address, value);
			}
			putLong(REG_REQDAT, i);
		} finally {
			if (is != null)
				is.close();
		}
	}

	// #ifdef ENABLE_RECSTORE
	private void recOp(byte op, String name, int address, int size) throws Exception {
		RecordStore store = null;
		boolean found = false;
		int recId = 0;
		byte[] data = null;
		ByteArrayInputStream bais = null;
		DataInputStream is = null;
		int i;
		try {
			store = RecordStore.openRecordStore(Const.STORE_NAME, true);
			RecordEnumeration re = store.enumerateRecords(null, null, false);
			while (re.hasNextElement()) {
				recId = re.nextRecordId();
				data = store.getRecord(recId);
				bais = new ByteArrayInputStream(data);
				is = new DataInputStream(bais);
				String recName = is.readUTF();
				if (recName.length() == 0)
					continue;
				char type = recName.charAt(0);
				if (type != Const.RECORD_TYPE_DATA)
					continue;
				if (op == REQ_RSFORMAT) {
					store.deleteRecord(recId);
				} else if (recName.substring(1).equals(name)) {
					found = true;
					break;
				}
			}
			re.destroy();
			switch (op) {
			case REQ_RLOAD:
				if (!found)
					throw new RuntimeException();
				for (i = 0; size == 0 || i < size; i++, address++) {
					int value = is.read();
					if (value == -1 && size == 0)
						break;
					if (value == -1 || address > 0xFFFF)
						throw new RuntimeException();
					as.put(address, value);					
				}
				putLong(REG_REQDAT, i);
				break;
			case REQ_RSAVE:
				ByteArrayOutputStream baos = new ByteArrayOutputStream();
				DataOutputStream os = new DataOutputStream(baos);
				os.writeUTF(Const.RECORD_TYPE_DATA + name);
				for (i = 0; i < size; i++, address++) {
					if (address > 0xFF00)
						throw new RuntimeException();
					os.write(as.get(address));
				}
				os.flush();
				data = baos.toByteArray();
				if (found)
					store.setRecord(recId, data, 0, data.length);
				else
					store.addRecord(data, 0, data.length);
				break;
			case REQ_RDELETE:
				if (!found)
					throw new RuntimeException();
				store.deleteRecord(recId);
				break;
			}
		} finally {
			if (store != null)
				store.closeRecordStore();
		}
	}
	
	private void doRSFormat() throws Exception {
		if (parseU8() != 121 || parseU8() != 33 || reqcur != reqlen)
			throw new RuntimeException();
		recOp(REQ_RSFORMAT, null, 0, 0);
	}
	
	private void doRLoad() throws Exception {
		int address = parseU16();
		int size = parseU16();
		String name = parseString0();
		recOp(REQ_RLOAD, name, address, size);
	}
	
	private void doRSave() throws Exception {
		int address = parseU16();
		int size = parseU16();
		String name = parseString0();
		recOp(REQ_RSAVE, name, address, size);
	}

	private void doRDelete() throws Exception {
		String name = parseString0();
		recOp(REQ_RDELETE, name, 0, 0);
	}
	// #endif

	private void putLong(int address, long value) {
		for (int i = 0; i < 8; i++) {
			m[address + i] = (byte)value;
			value >>= 8;
		}
	}
	
	private void putInt(int address, int value) {
		for (int i = 0; i < 4; i++) {
			m[address + i] = (byte)value;
			value >>= 8;
		}
	}
	
	private void putShort(int address, int value) {
		m[address] = (byte)(value & 0xFF);
		m[address + 1] = (byte)((value & 0xFF00) >> 8);
	}
	
	private byte getDpyInfoDepth(int levels) {
		byte depth;
		int mask = 0x80000000;
		for (depth = 31; depth > 0; depth--) {
			if ((levels & mask) != 0)
				break;
			mask >>= 1;
		}
		return depth;
	}
	
	private void doDpyInfo() {
		putShort(REG_REQDAT + VAL_DPYINFO_WIDTH, getWidth());
		putShort(REG_REQDAT + VAL_DPYINFO_HEIGHT, getHeight());
		byte flags = 0;
		flags |= display.isColor() ? (byte)VAL_DPYINFO_FLAGS_ISCOLOR : 0;
		// #if JavaPlatform != "MIDP/1.0"
		flags |= (byte)VAL_DPYINFO_FLAGS_ISMIDP2;
		// #endif
		flags |= hasPointerEvents() ? (byte)VAL_DPYINFO_FLAGS_ISTOUCH : 0;
		m[REG_REQDAT + VAL_DPYINFO_FLAGS] = (byte)flags;
		// #if JavaPlatform != "MIDP/1.0"
		m[REG_REQDAT + VAL_DPYINFO_ALPHADEPTH] =
				getDpyInfoDepth(display.numAlphaLevels());
		// #else
//@		m[REG_REQDAT + VAL_DPYINFO_ALPHADEPTH] = 0;
		// #endif
		m[REG_REQDAT + VAL_DPYINFO_COLORDEPTH] =
				getDpyInfoDepth(display.numColors());
	}

	private int getPaletteColor(byte color) {
		try {
			return palette[color & paletteMask];
		} catch (ArrayIndexOutOfBoundsException e) {
			// This might happen during paint in MIDP1;
			// even if we could synchronize, the case is so rare that
			// we would be better off recovering like this anyway.
			int[] p = palette;
			return p[color % p.length];
		}
	}
	
	private int parseColor() {
		int col = 0xFF000000;
		int len = reqlen - reqcur;
		if (len > 4)
			len = 4;
		switch (len) {
		case 4:
			col = (req[reqcur++] & 0xFF) << 24;
		case 3:
			col |= (req[reqcur++] & 0xFF) << 16;
			col |= (req[reqcur++] & 0xFF) << 8;
			col |= (req[reqcur++] & 0xFF);
			break;
		case 1:
			col = getPaletteColor(req[reqcur++]);
			break;
		default:
			throw new RuntimeException();
		}
		return col;
	}
	
	private int parseU8() {
		return parseI8() & 0xFF;
	}
	
	private int parseI8() {
		if (reqlen - reqcur < 1)
			throw new RuntimeException();
		return req[reqcur++];
	}
	
	private int parseU16() {
		int value;
		if (reqlen - reqcur < 2)
			throw new RuntimeException();
		value = req[reqcur++] & 0xFF;
		value |= (req[reqcur++] & 0xFF) << 8;
		return value;
	}
	
	private int parseI16() {
		return (short)parseU16();
	}

	private int parseInt(int t) {
		switch (t) {
		case VAL_DATATYPE_U8:
			return parseU8();
		case VAL_DATATYPE_I8:
			return parseI8();
		case VAL_DATATYPE_U16:
			return parseU16();
		case VAL_DATATYPE_I16:
			return parseI16();
		default:
			throw new RuntimeException();
		}
	}

	private String parseString0() {
		StringBuffer buf = new StringBuffer();
		while (reqlen != reqcur) {
			char ch = (char)(req[reqcur++] & 0xFF);
			if (ch == 0)
				return buf.toString();
			buf.append(ch);
		}
		throw new RuntimeException();
	}
	
	private String parseString() {
		StringBuffer buf = new StringBuffer();
		while (reqlen != reqcur) {
			char ch = (char)(req[reqcur++] & 0xFF);
			buf.append(ch);
		}
		return buf.toString();
	}
	
	private void parseEnd() {
		if (reqcur != reqlen)
			throw new RuntimeException();
	}

	private void doSetPal(int pos, int value) throws Throwable {
		if (pos == 0) {
			strBAOS = new ByteArrayOutputStream();
		} else if (pos == -1) {
			try {
				if (strBAOS.size() == 0) {
					palette = standardPalette;
					paletteMask = STANDARD_PALETTE_MASK;
					return;
				}
				byte[] data = strBAOS.toByteArray();
				int in = data.length / 3;
				if (in > 256 || data.length % 3 != 0)
					throw new RuntimeException();
				int n;
				for (n = 128; n > 1 && n >= in; n >>= 1)
					;
				n <<= 1;
				int[] p = new int[n];
				for (int i = 0, j = 0; i < in; i++) {
					p[i] =
						((data[j] & 0xFF) << 16) | // red 
						((data[j + 1] & 0xFF) << 8) | // green 
						(data[j + 2] & 0xFF) // blue 
					;
					j += 3;
				}
				palette = p;
				paletteMask = n - 1;
			} finally {
				strBAOS = null;
			}
		} else {
			strBAOS.write(value);
		}
	}

	public static final int RAWRGBA_HEADER_SIZE = 7;
	public static final int RAWRGBA_HEADER_IMAGEID_OFFSET = 1;
	public static final int RAWRGBA_HEADER_WIDTH_OFFSET = 2;
	public static final int RAWRGBA_HEADER_HEIGHT_OFFSET = 4;
	public static final int RAWRGBA_HEADER_FLAGS_OFFSET = 6;
	
	// #if ENABLE_IMAGE

	private void doIDestroy() {
		int id = parseU8();
		images[id] = null;
	}

	private void doIDim() {
		int n = parseU8() + 1;
		Image[] newImages = new Image[n];
		if (images.length < n)
			n = images.length;
		System.arraycopy(images, 0, newImages, 0, n);
		images = newImages;
	}
	
	private void doIInfo() {
		Image image = images[parseU8()];
		putShort(REG_REQDAT + VAL_IINFO_WIDTH, image.getWidth());
		putShort(REG_REQDAT + VAL_IINFO_HEIGHT, image.getHeight());
		byte flags = 0;
		flags |= image.isMutable() ? (byte)VAL_IINFO_FLAGS_ISMUTABLE : 0;
		m[REG_REQDAT + VAL_IINFO_FLAGS] = (byte)flags;
	}

	private void doILoad() throws Exception {
		int id = parseU8();
		images[id] = Image.createImage(parseString0());
	}
	
	private void doIEmpty() throws Exception {
		int id = parseU8();
		int width = parseU16();
		int height = parseU16();
		images[id] = Image.createImage(width, height);
	}

	private void doIMkImmut() throws Exception {
		int id = parseU8();
		images[id] = Image.createImage(images[id]);
	}

	// #if JavaPlatform != "MIDP/1.0"

	private int[] rawRGBABuffer;

	private void doIRawRGBA(int pos, int value) throws Throwable {
		int width = 0, height = 0;
		if (pos == -1 || pos == RAWRGBA_HEADER_SIZE) {
			width = req[RAWRGBA_HEADER_WIDTH_OFFSET] & 0xFF;
			width |= (req[RAWRGBA_HEADER_WIDTH_OFFSET + 1] << 8) & 0xFF;
			height = req[RAWRGBA_HEADER_HEIGHT_OFFSET] & 0xFF;
			height |= (req[RAWRGBA_HEADER_HEIGHT_OFFSET + 1] << 8) & 0xFF;
		}
		if (pos == -1) {
			int imageId = req[RAWRGBA_HEADER_IMAGEID_OFFSET] & 0xFF;
			try {
				images[imageId] = Image.createRGBImage(rawRGBABuffer,
						width, height, (req[RAWRGBA_HEADER_FLAGS_OFFSET] &
								VAL_IRAWRGBA_FLAGS_ALPHA) != 0);
			} catch (Throwable e) {
				images[imageId] = null;
			} finally {
				rawRGBABuffer = null;
			}
		} else if (pos < RAWRGBA_HEADER_SIZE) {
			req[pos] = (byte)value;
		} else {
			if (pos == RAWRGBA_HEADER_SIZE)
				rawRGBABuffer = new int[width * height];
			pos -= RAWRGBA_HEADER_SIZE;
			int shifted = 0;
			switch (pos & 0x3) {
			case 0: // red
				shifted = value << 16;
				break;
			case 1: // green
				shifted = value << 8;
				break;
			case 2: // blue
				shifted = value;
				break;
			case 3: // alpha
				shifted = value << 24;
				break;
			}
			rawRGBABuffer[pos >> 2] |= shifted;
		}
	}

	// #endif

	// #endif

	private void doFrmDrawPut() {
		if (frameInterval == 0) {
			flush();
		} else {
			waitingForFrame = true;
			signalSuspendCPU = true;
		}
	}

	private synchronized void doKeyBufPut() {
		for (int i = 0; i < 7; i++)
			m[REG_KEYBUF + i] = m[REG_KEYBUF + i + 1];
		m[REG_KEYBUF + 7] = 0;
	}

	private synchronized void enqueKeyCode(int keyCode) {
		for (int i = 0; i < 8; i++)
			if (m[REG_KEYBUF + i] == 0) {
				m[REG_KEYBUF + i] = (byte)keyCode;
				return;
			}
	}

	/*************************************************************************
	 * Console
	 *************************************************************************/
	
	// #if ENABLE_CONSOLE
	
	private Font conFont;
	private int conFontWidth;
	private int conFontHeight;
	private byte[] conBuffer;

	private static final int conBufCols = 0;
	private static final int conBufRows = 1;
	private static final int conBufChrStart = 2;
	
	private byte[] newEmptyConBuffer(int cols, int rows) {
		int size = cols * rows;
		byte buf[] = new byte[2 + size * 3];; 
		buf[conBufCols] = (byte)cols;
		buf[conBufRows] = (byte)rows;
		for (int i = 0; i < size; i++)
			buf[conBufChrStart + i] = ' ';
		int fgIndex = conBufChrStart + size;
		int bgIndex = fgIndex + size;
		for (int i = 0; i < size; i++) {
			buf[fgIndex + i] = COLOR_BLACK;
			buf[bgIndex + i] = COLOR_WHITE;
		}
		return buf;
	}

	private void conReset() {
		// #if MICROEMULATOR
		conFont = Font.getFont(Font.FACE_MONOSPACE, Font.STYLE_BOLD,
				Font.SIZE_MEDIUM);
		// #else
//@		conFont = Font.getFont(Font.FACE_MONOSPACE, Font.STYLE_BOLD,
//@				Font.SIZE_SMALL);
		// #endif
		conFontWidth = conFont.charWidth('W');
		conFontHeight = conFont.getHeight();
		m[REG_CONCOLS] = 10;
		m[REG_CONROWS] = 4;
		conBuffer = newEmptyConBuffer(10, 4);
	}

	/*
	 * ...11...
	 * ...11...
	 * ...11...
	 * 222PX444
	 * 222XX444
	 * ...88...
	 * ...88...
	 * ...88...
	 */
	private void conDrawLineArt(Graphics g, int x, int y, int chr) {
		int px = (conFontWidth - 2) >> 1;
		int py = (conFontHeight - 2) >> 1;
		g.fillRect(x + px, y + py, 2, 2);
		if ((chr & 0x1) != 0)
			g.fillRect(x + px, y, 2, py);
		if ((chr & 0x2) != 0)
			g.fillRect(x, y + py, px, 2);
		if ((chr & 0x4) != 0)
			g.fillRect(x + px + 2, y + py, conFontWidth - px - 2, 2);
		if ((chr & 0x8) != 0)
			g.fillRect(x + px, y + py + 2, 2, conFontHeight - py - 2);
	}

	/*
	 * In MIDP 1.0 we cannot synchronize during flush, so to make the
	 * console resize atomic we use a buffer that is always consistent
	 * (its size always matches cols * rows) and we take a reference
	 * to it at the beginning of the conDraw.
	 */
	private void conDraw(Graphics g) {
		byte[] buf = conBuffer;
		int cols = buf[conBufCols] & 0xFF;
		int rows = buf[conBufRows] & 0xFF;
		int size = cols * rows;
		int ox = (getWidth() - cols * conFontWidth) >> 1;
		int oy = (getHeight() - rows * conFontHeight) >> 1;
		g.setFont(conFont);
		// #if DEBUG_MICROIO_BORDER
//@		g.setColor(0xFF0000);
//@		g.drawRect(
//@				(getWidth() >> 1) - 5 * conFontWidth - 4,
//@				(getHeight() >> 1) - 2 * conFontHeight - 4,
//@				10 * conFontWidth + 8,
//@				4 * conFontHeight + 8
//@		);
		// #endif
		int chrIndex = conBufChrStart;
		int fgIndex = conBufChrStart + size;
		int bgIndex = fgIndex + size;
		int x, y = oy;
		for (int r = 0; r < rows; r++) {
			x = ox;
			for (int c = 0; c < cols; c++) {
				int chr = buf[chrIndex++] & 0xFF;
				if (chr != 0) {
					g.setColor(getPaletteColor(buf[bgIndex++]));
					g.fillRect(x, y, conFontWidth, conFontHeight);
					g.setColor(getPaletteColor(buf[fgIndex++]));
					if ((chr & 0xF0) == 0x80)
						conDrawLineArt(g, x, y, chr);
					else
						g.drawChar((char)chr, x, y, 0);
				}
				x += conFontWidth;
			}
			y += conFontHeight;
		}
	}

	private void conResize() {
		int maxCols = getWidth() / conFontWidth;
		int maxRows = getHeight() / conFontHeight;
		// normalize REG_SCRCOLS
		int cols = m[REG_CONCOLS] & 0xFF;
		if (cols == 0 || cols > maxCols)
			cols = maxCols;
		m[REG_CONCOLS] = (byte)cols;
		// normalize REG_SCRROWS
		int rows = m[REG_CONROWS] & 0xFF;
		if (rows == 0 || rows > maxRows)
			rows = maxRows;
		m[REG_CONROWS] = (byte)rows;
		byte[] buf = conBuffer;
		int oldCols = buf[conBufCols] & 0xFF;
		int oldRows = buf[conBufRows] & 0xFF;
		// make sure update is necessary
		if (oldCols == cols && oldRows == rows)
			return;
		conBuffer = newEmptyConBuffer(cols, rows);
		// TODO copy valid window
	}

	private int doConsoleRW(boolean isWrite, int address, int value) {
		int x, y;
		switch (address) {
		case REG_CONCOLS:
		case REG_CONROWS:
			// isWrite == true
			m[address] = (byte)value;
			conResize();
			return 0;
		case REG_CONCX:
		case REG_CONCY:
			// isWrite == true
			m[address] = (byte)value;
			return 0;
		case REG_CONCCHR:
		case REG_CONCFG:
		case REG_CONCBG:
			x = m[REG_CONCX] & 0xFF;
			y = m[REG_CONCY] & 0xFF;
			break;
		default: // CONVIDEO
			int offset = address - REG_CONVIDEO;
			x = offset % 10;
			y = offset / 10;
			break;
		}
		byte[] buf = conBuffer;
		int cols = buf[conBufCols] & 0xFF;
		if (x >= cols)
			return 0;
		int rows = buf[conBufRows] & 0xFF;
		if (y >= rows)
			return 0;
		int size = rows * cols;
		int i = y * cols + x;
		switch (address) {
		case REG_CONCBG:
			i += size;
		case REG_CONCFG:
			i += size;
		default:
			i += conBufChrStart;
			break;
		}
		if (!isWrite)
			return buf[i] & 0xFF;
		buf[i] = (byte)value;
		return 0;
	}
	
	// #endif

	/*************************************************************************
	 * Dummy
	 *************************************************************************/
	
	// #if ENABLE_IMAGE_DUMMY

	private char dummyTileIdToChar(int id) {
		// printable ASCII character ('!' ... '~') shifted so id=0 => '0'
		return (char)('!' + (id + '0' - '!') % ('~' - '!' + 1));
	}
	
	private int dummyParseColor(boolean isForeground) {
		int col = parseU8();
		if (display.isColor())
			return getPaletteColor((byte)col);
		else
			return isForeground ? 0x000000 : 0xFFFFFF;
	}
	
	private void doIDummy() {
		int tileWidth, tileHeight, bg, fg, tileGroupLength = 0;
		int imageId = parseU8();
		int type = parseU8();
		String s = null;
		int tileCols = 0, tileRows = 0;
		switch (type) {
		case VAL_IDUMMY_SIMPLE:
			tileWidth = parseU16();
			tileHeight = parseU16();
			bg = dummyParseColor(false);
			fg = dummyParseColor(true);
			s = parseString();
			break;
		case VAL_IDUMMY_SPRITE:
			tileWidth = parseU8();
			tileHeight = parseU8();
			tileCols = parseU8();
			bg = dummyParseColor(false);
			fg = dummyParseColor(true);
			s = parseString();
			break;
		case VAL_IDUMMY_TILES:
			tileWidth = parseU8();
			tileHeight = parseU8();
			tileCols = parseU8();
			tileRows = parseU8();
			bg = dummyParseColor(false);
			fg = dummyParseColor(true);
			if (reqlen != reqcur)
				tileGroupLength = parseU8();
			else
				tileGroupLength = -1;
			break;
		default:
			throw new RuntimeException();
		}
		if (tileCols == 0)
			tileCols = 1;
		if (tileRows == 0)
			tileRows = 1;
		int width = tileWidth * tileCols;
		int height = tileHeight * tileRows;
		Image dummy = Image.createImage(width, height);
		Graphics g = dummy.getGraphics();
		Font font = Font.getFont(Font.FACE_SYSTEM, Font.STYLE_PLAIN,
				Font.SIZE_SMALL);
		g.setFont(font);
		int tileId = 0;
		int y = 0;
		for (int i = 0; i < tileRows; i++) {
			int x = 0;
			for (int j = 0; j < tileCols; j++) {
				g.setClip(x, y, tileWidth, tileHeight);
				g.setColor(bg);
				g.fillRect(x, y, tileWidth, tileHeight);
				g.setColor(fg);
				if (type != VAL_IDUMMY_TILES)
					g.drawRect(x, y, tileWidth - 1, tileHeight - 1);
				if (type == VAL_IDUMMY_SIMPLE) {
					if (s.length() == 0) {
						g.drawLine(0, 0, width, height);
						g.drawLine(width, 0, 0, height);
					} else {
						g.drawString(s,
								width >> 1, (height + font.getHeight()) >> 1,
								Graphics.BOTTOM | Graphics.HCENTER);
					}
				} else if (type == VAL_IDUMMY_SPRITE) {
					g.drawRect(x, y, tileWidth - 1, tileHeight - 1);
					g.drawString(s, x + 2, y + 2,
							Graphics.TOP | Graphics.LEFT);
					g.drawChar(dummyTileIdToChar(tileId),
							x + tileWidth - 2, tileHeight - 2,
							Graphics.BOTTOM | Graphics.RIGHT);
				} else if (type == VAL_IDUMMY_TILES) {
					char ch = dummyTileIdToChar(tileId + 1);
					g.drawChar(ch,
							x + (tileWidth - font.charWidth(ch)) / 2,
							y + (tileHeight - font.getHeight()) / 2,
							Graphics.TOP | Graphics.LEFT);
					tileGroupLength--;
					if (tileGroupLength == 0) {
						bg = dummyParseColor(false);
						fg = dummyParseColor(true);
						if (reqlen != reqcur)
							tileGroupLength = parseU8();
						else
							tileGroupLength = -1;
					}
				}
				x += tileWidth;
				tileId++;
			}
			y += tileHeight;
		}
		images[imageId] = Image.createImage(dummy);
	}

	// #endif

	/*************************************************************************
	 * PNG Generator
	 *************************************************************************/
	
	public static final int PNG_HEADER_SIZE = 9;
	public static final int PNG_HEADER_IMAGEID_OFFSET = 1;
	public static final int PNG_HEADER_WIDTH_OFFSET = 2;
	public static final int PNG_HEADER_HEIGHT_OFFSET = 4;
	public static final int PNG_HEADER_DEPTH_OFFSET = 6;
	public static final int PNG_HEADER_COLOR_OFFSET = 7;
	public static final int PNG_HEADER_FLAGS_OFFSET = 8;

	// #if ENABLE_IMAGE_PNGGEN
	
	private static final int PNG_STATUS_HEADER = 1;
	private static final int PNG_STATUS_PALETTE_SIZE = 2;
	private static final int PNG_STATUS_PALETTE_VALUE = 3;
	private static final int PNG_STATUS_BEGIN = 4;
	private static final int PNG_STATUS_DATA = 5;
	private static final int PNG_STATUS_END = 6;
	private static final int PNG_STATUS_ERROR = 7;
	
	private static final long PNG_SIGNATURE = 0x89504E470D0A1A0AL;
	private static final int PNG_CHUNK_IHDR = 0x49484452;
	private static final int PNG_CHUNK_PLTE = 0x504C5445;
	private static final int PNG_CHUNK_tRNS = 0x74524E53;
	private static final int PNG_CHUNK_IDAT = 0x49444154;
	private static final int PNG_CHUNK_IEND = 0x49454E44;
	private static final int ADLER32_BASE = 65521; 

	private int pngInDataSize;
	private int pngLastPos;
	private int pngRowWidth;
	private int pngDepth;
	private int pngZoom;
	private ByteArrayOutputStream pngBAOS;
	private Vector pngChunkOffsets;
	private Vector pngChunkLengths;
	private int[] pngCRCTable;
	private int pngOutDataSize;
	private int pngZPos;
	private int pngS1;
	private int pngS2;

	// TODO expose a way to release pngCRCTable
	private void pngInitCRCTable() {
		if (pngCRCTable != null)
			return;
		pngCRCTable = new int[256];
		for (int n = 0; n < 256; n++) {
			int c = n;
			for (int k = 0; k < 8; k++) {
				if ((c & 1) != 0)
					c = 0xEDB88320 ^ ((c >> 1) & 0x7FFFFFFF);
				else
					c = (c >> 1) & 0x7FFFFFFF;
			}
			pngCRCTable[n] = c;
		}
	}
	
	private void pngBegin() throws Throwable {
		strBAOS = new ByteArrayOutputStream();
		strOS = new DataOutputStream(strBAOS);
		pngChunkOffsets = new Vector();
		pngChunkLengths = new Vector();
		pngInitCRCTable();
		strOS.writeLong(PNG_SIGNATURE);
	}
	
	private void pngEnd() {
		byte[] data = strBAOS.toByteArray();
		for (int i = 0; i < pngChunkOffsets.size(); i++) {
			int offset = ((Integer)pngChunkOffsets.elementAt(i)).intValue();
			int length = ((Integer)pngChunkLengths.elementAt(i)).intValue();
			data[offset] = (byte)(length >> 24);
			data[offset + 1] = (byte)(length >> 16);
			data[offset + 2] = (byte)(length >> 8);
			data[offset + 3] = (byte)length;
			length += 4;
			int n = offset + 4;
			int c = -1;
			for (int j = 0; j < length; j++)
				c = pngCRCTable[(c ^ data[n++]) & 0xFF] ^ ((c >> 8) & 0x00FFFFFF);
			c = c ^ -1;
			data[n] = (byte)(c >> 24); 
			data[n + 1] = (byte)(c >> 16); 
			data[n + 2] = (byte)(c >> 8); 
			data[n + 3] = (byte)c; 
		}
		int imageId = req[PNG_HEADER_IMAGEID_OFFSET] & 0xFF;
		images[imageId] = Image.createImage(data, 0, data.length);
	}
	
	private void pngRelease() {
		strBAOS = null;
		strOS = null;
		pngBAOS = null;
		pngChunkOffsets = null;
		pngChunkLengths = null;
	}
	
	private void pngBeginChunk(int type) throws Throwable {
		strOS.flush();
		pngChunkOffsets.addElement(new Integer(strBAOS.size()));
		strOS.writeInt(0); // Length (edit later)
		strOS.writeInt(type);
	}
	
	private void pngEndChunk() throws Throwable {		
		strOS.flush();
		pngChunkLengths.addElement(new Integer(strBAOS.size()
				- ((Integer)pngChunkOffsets.elementAt(
						pngChunkOffsets.size() - 1)).intValue() - 8));
		strOS.writeInt(0); // CRC (edit later)
	}
	
	private void pngWriteZShort(int value) throws Throwable {
		strOS.writeByte(value & 0xFF);
		strOS.writeByte(value >> 8);
	}
	
	private void pngWriteZData(int value) throws Throwable {
		if ((pngZPos & 0x3FFF) == 0) {
			int len = pngOutDataSize - pngZPos;
			if (len <= 0x4000) {
				strOS.writeByte(0x01); // BFINAL=1, BTYPE=00
			} else {
				strOS.writeByte(0x00); // BFINAL=0, BTYPE=00
				len = 0x4000;
			}
			pngWriteZShort(len);
			pngWriteZShort(~len);
		}
		strOS.writeByte(value);
		pngZPos++;
		pngS1 += value;
		if (pngS1 > ADLER32_BASE)
			pngS1 -= ADLER32_BASE;
        pngS2 += pngS1;
		if (pngS2 > ADLER32_BASE)
			pngS2 -= ADLER32_BASE;
	}
	
	private void pngWriteMagnifiedRow() throws Throwable {
		byte[] buf = pngBAOS.toByteArray();
		for (int i = 0; i < pngZoom; i++)
			for (int j = 0; j < buf.length; j++)
				pngWriteZData(buf[j] & 0xFF);
	}
	
	private void doIPngGen(int pos, int value) throws Throwable {
		if (pos == 0) {
			pngBegin();
			strStatus = PNG_STATUS_HEADER;
		} else if (pos == -1) {
			try {
				if (strStatus != PNG_STATUS_END)
					throw new RuntimeException();
				pngEnd();
			} finally {
				pngRelease();
			}
			return;
		}
		switch (strStatus) {
		case PNG_STATUS_HEADER:
			req[pos] = (byte)value;
			if (pos == PNG_HEADER_SIZE - 1) {
				pngBeginChunk(PNG_CHUNK_IHDR);
				int width = req[PNG_HEADER_WIDTH_OFFSET] & 0xFF;
				width |= (req[PNG_HEADER_WIDTH_OFFSET + 1] << 8) & 0xFF;
				int height = req[PNG_HEADER_HEIGHT_OFFSET] & 0xFF;
				height |= (req[PNG_HEADER_HEIGHT_OFFSET + 1] << 8) & 0xFF00;
				pngDepth = req[PNG_HEADER_DEPTH_OFFSET] & 0xFF;
				int color = req[PNG_HEADER_COLOR_OFFSET] & 0xFF;
				if (color == VAL_IPNGGEN_CT_INDEXED_COLOR && (width & 0x3) == 0) 
					pngZoom = 1 + ((req[PNG_HEADER_FLAGS_OFFSET] >> 2) & 0x3);
				else
					pngZoom = 1;
				strOS.writeInt(width * pngZoom); // Width
				strOS.writeInt(height * pngZoom); // Height
				strOS.writeByte(pngDepth); // Bit depth
				strOS.writeByte(color); // Colour type
				strOS.writeByte(0); // Compression method
				strOS.writeByte(0); // Filter method
				strOS.writeByte(0); // Interlace method
				pngEndChunk();
				int bits;
				switch (color) {
				case VAL_IPNGGEN_CT_GRAYSCALE:
				case VAL_IPNGGEN_CT_INDEXED_COLOR:
					bits = pngDepth;
					break;
				case VAL_IPNGGEN_CT_GRAYSCALE_ALPHA:
					bits = pngDepth * 2;
					break;
				case VAL_IPNGGEN_CT_TRUECOLOR:
					bits = pngDepth * 3;
					break;
				case VAL_IPNGGEN_CT_TRUECOLOR_ALPHA:
					bits = pngDepth * 4;
					break;
				default:
					strStatus = PNG_STATUS_ERROR;
					return;
				}
				pngRowWidth = ((width * bits) + 7) >> 3;
				pngInDataSize = height * pngRowWidth;
				pngOutDataSize = (pngInDataSize * pngZoom + height)
						* pngZoom; // + Filter type
				if (color == VAL_IPNGGEN_CT_INDEXED_COLOR)
					strStatus = PNG_STATUS_PALETTE_SIZE;
				else
					strStatus = PNG_STATUS_BEGIN;
			}
			break;
		case PNG_STATUS_PALETTE_SIZE:
			pngBeginChunk(PNG_CHUNK_PLTE);
			value++;
			if ((req[PNG_HEADER_FLAGS_OFFSET] & VAL_IPNGGEN_FLAGS_PALREF)
					!= 0) {
				strNextSection = pos + 1 + value;				
			} else {
				strNextSection = pos + 1 + value * 3;
			}
			strStatus = PNG_STATUS_PALETTE_VALUE;
			break;
		case PNG_STATUS_PALETTE_VALUE:
			if (pos != strNextSection) {
				if ((req[PNG_HEADER_FLAGS_OFFSET] & VAL_IPNGGEN_FLAGS_PALREF)
						!= 0) {
					int col = getPaletteColor((byte)value);
					strOS.writeByte((col >> 16) & 0xFF);
					strOS.writeByte((col >> 8) & 0xFF);
					strOS.writeByte(col & 0xFF);
				} else {
					strOS.writeByte(value);
				}
				break;
			}
			pngEndChunk();
			if ((req[PNG_HEADER_FLAGS_OFFSET] & VAL_IPNGGEN_FLAGS_IDX0TRANSP)
					!= 0) {
				pngBeginChunk(PNG_CHUNK_tRNS);
				strOS.writeByte(0);
				pngEndChunk();
			}
			strStatus = PNG_STATUS_BEGIN;
			// fall through
		case PNG_STATUS_BEGIN:
			// TODO reserve space in strBAOS?
			pngBeginChunk(PNG_CHUNK_IDAT);
			strOS.writeByte(0x08); // CM=8, CINFO=0 (window=256)
			strOS.writeByte(0x1D); // FDICT=0, FLEVEL=0
			pngZPos = 0;
			pngS1 = 1;
			pngS2 = 0;
			strNextSection = pos;
			pngLastPos = pos + pngInDataSize - 1;
			strStatus = PNG_STATUS_DATA;
			// fall through
		case PNG_STATUS_DATA:
			if (pos == strNextSection) {
				// Filter type = None
				if (pngZoom == 1) {
					pngWriteZData(0);
				} else if (pngBAOS == null) {
					pngBAOS = new ByteArrayOutputStream(pngRowWidth * pngZoom
							+ 1);
					pngBAOS.write(0);
				} else {
					pngWriteMagnifiedRow();
					pngBAOS.reset();		
					pngBAOS.write(0);
				}
				strNextSection += pngRowWidth;
			}
			if (pngZoom == 1) {
				pngWriteZData(value & 0xFF);
			} else {
				int mask = 0x80;
				int bit = 0;
				int accumulator = 0;
				while (mask != 0) {
					for (int z = 0; z < pngZoom; z++) {
						int savedMask = mask;
						for (int i = 0; i < pngDepth; i++) {
							accumulator <<= 1;
							if ((value & mask) != 0)
								accumulator |= 1;
							bit++;
							if (bit == 8) {
								pngBAOS.write(accumulator);
								accumulator = 0;
								bit = 0;
							}
							mask >>= 1;
						}
						mask = savedMask;
					}
					mask >>= pngDepth;
				}
			}
			if (pos == pngLastPos) {
				if (pngZoom != 1)
					pngWriteMagnifiedRow();
				strOS.writeInt((pngS2 << 16) + pngS1); // ADLER32
				pngEndChunk();
				pngBeginChunk(PNG_CHUNK_IEND);
				pngEndChunk();
				strStatus = PNG_STATUS_END;
			}
			break;
		case PNG_STATUS_END:
			// unexpected byte
			strStatus = PNG_STATUS_ERROR;
			break;
		}
	}
	
	// #endif

	/*************************************************************************
	 * Game
	 *************************************************************************/
	
	public static final int LTLPUT_HEADER_SIZE = 8;
	public static final int LTLPUT_HEADER_LAYERID_OFFSET = 1;
	public static final int LTLPUT_HEADER_OX_OFFSET = 2;
	public static final int LTLPUT_HEADER_OY_OFFSET = 4;
	public static final int LTLPUT_HEADER_COLS_OFFSET = 6;
	
	// #ifdef ENABLE_GAMEAPI

	private static final int DEFAULT_LAYER_DIM = 16;
	private static final int LAYER_MAX_ANIM_TYLES = 127;

	private LayerManager layerManager;
	private int layerManagerOX;
	private int layerManagerOY;
	private Layer layerObj[];
	private byte layerCtl[];
	private int layerOX[];
	private int layerOY[];
	private int layerPri[];
	private byte layerExtra[]; // # of anim t. for TL, transf for Spr. 

	private void layerReset() {
		layerManager = new LayerManager();
		layerManagerOX = 0;
		layerManagerOY = 0;
		layerObj = new Layer[DEFAULT_LAYER_DIM];
		layerCtl = new byte[DEFAULT_LAYER_DIM];
		layerOX = new int[DEFAULT_LAYER_DIM];
		layerOY = new int[DEFAULT_LAYER_DIM];
		layerPri = new int[DEFAULT_LAYER_DIM];
		layerExtra = new byte[DEFAULT_LAYER_DIM];
	}
	
	private void layerReorder(int id) {
		Layer l = layerObj[id];
		if (l == null)
			return;
		int pri = layerPri[id];
		layerManager.remove(l);
		int precedingLayers = 0;
		for (int i = 0; i < layerObj.length; i++) {
			if (i != id && layerObj[i] != null && layerPri[i] < pri)
				precedingLayers++;
		}
		layerManager.insert(l, precedingLayers);
	}
	
	private void layerCreateSprite(int id, Sprite l) {
		l.setVisible(false);
		layerObj[id] = l; 
		layerCtl[id] = 0; 
		layerOX[id] = 0; 
		layerOY[id] = 0; 
		layerPri[id] = id; 
		layerExtra[id] = Sprite.TRANS_NONE;
		layerReorder(id);
	}

	// with datatype to allow neg./large values
	private void doLMView() {
		if (reqlen - reqcur == 1) {
			Layer l = layerObj[parseU8()];
			layerManager.setViewWindow(0, 0, l.getWidth(), l.getHeight());
			layerManagerOX = (getWidth() - l.getWidth()) >> 1;
			layerManagerOY = (getHeight() - l.getHeight()) >> 1;
		} else {
			int t = parseU8();
			int ox = parseInt(t);
			int oy = parseInt(t);
			int width;
			int height;
			if (reqcur == reqlen) {
				width = ox;
				height = oy;
				ox = 0;
				oy = 0;
				layerManagerOX = (getWidth() - width) >> 1;
				layerManagerOY = (getHeight() - height) >> 1;
			} else {
				width = parseInt(t);
				height = parseInt(t);
			}
			layerManager.setViewWindow(ox, oy, width, height);
		}
	}
	
	// with datatype to allow neg./large values
	private void doLMPos() {
		int t = parseU8();
		layerManagerOX = parseInt(t);
		layerManagerOY = parseInt(t);
	}
	
	private void doLDestroy() {
		int id = parseU8();
		if (layerObj[id] != null) {
			layerManager.remove(layerObj[id]);
			layerObj[id] = null;
		}
	}
	
	private void doLDimImpl(int n) {
		Layer[] newLayerObj = new Layer[n];
		byte[] newLayerCtl = new byte[n];
		int[] newLayerOX = new int[n];
		int[] newLayerOY = new int[n];
		int[] newLayerPri = new int[n];
		byte[] newLayerExtra = new byte[n];
		for (int i = n; i < layerObj.length; i++) {
			if (layerObj[i] != null)
				layerManager.remove(layerObj[i]);
		}
		if (layerObj.length < n)
			n = layerObj.length;
		System.arraycopy(layerObj, 0, newLayerObj, 0, n);
		System.arraycopy(layerCtl, 0, newLayerCtl, 0, n);
		System.arraycopy(layerOX, 0, newLayerOX, 0, n);
		System.arraycopy(layerOY, 0, newLayerOY, 0, n);
		System.arraycopy(layerPri, 0, newLayerPri, 0, n);
		System.arraycopy(layerExtra, 0, newLayerExtra, 0, n);
		layerObj = newLayerObj;
		layerCtl = newLayerCtl;
		layerOX = newLayerOX;
		layerOY = newLayerOY;
		layerPri = newLayerPri;
		layerExtra = newLayerExtra;
	}

	private void doLDim() {
		doLDimImpl(parseU8() + 1);
	}
	
	private void doLTiled() {
		int id = parseU8();
		if (layerObj[id] != null)
			layerManager.remove(layerObj[id]);
		Image image = images[parseU8()];
		int tileWidth = parseU8();
		int tileHeight = parseU8();
		int animTiles = parseU8();
		if (animTiles > LAYER_MAX_ANIM_TYLES)
			throw new RuntimeException();
		int t = parseU8();
		int cols = parseInt(t);
		int rows = parseInt(t);
		TiledLayer l = new TiledLayer(cols, rows, image, tileWidth, tileHeight);
		l.setVisible(false);
		for (int i = 0; i < animTiles; i++)
			l.createAnimatedTile(0);
		layerObj[id] = l; 
		layerCtl[id] = 0; 
		layerOX[id] = 0; 
		layerOY[id] = 0; 
		layerPri[id] = id; 
		layerExtra[id] = (byte)animTiles;
		layerReorder(id);
	}
	
	private void doLSprite() {
		int id = parseU8();
		if (layerObj[id] != null)
			layerManager.remove(layerObj[id]);
		Image image = images[parseU8()];
		Sprite l;
		if (reqcur != reqlen) {
			int frameWidth = parseU8(); 
			int frameHeight = parseU8();
			l = new Sprite(image, frameWidth, frameHeight);
		} else {
			l = new Sprite(image);
		}
		layerCreateSprite(id, l);
	}
	
	// with datatype to allow neg./large values
	private void doLSetPos() {
		int id = parseU8();
		Layer l = (Layer)layerObj[id];
		int t = parseU8();
		int x = parseInt(t);
		int y = parseInt(t);
		l.move(x - layerOX[id], y - layerOY[id]);
		layerOX[id] = x;
		layerOY[id] = y;
	}
	
	private void doLGetPos() {
		int id = parseU8();
		if (layerObj[id] == null)
			throw new RuntimeException();
		putInt(REG_REQDAT, layerOX[id]);
		putInt(REG_REQDAT + 4, layerOY[id]);
	}
	
	// with datatype to allow neg./large values
	private void doLMove() {
		int id = parseU8();
		Layer l = (Layer)layerObj[id];
		int t = parseU8();
		int dx = parseInt(t);
		int dy = parseInt(t);
		l.move(dx, dy);
		layerOX[id] += dx;
		layerOY[id] += dy;
	}
	
	// with datatype to allow neg./large values
	private void doLSetPri() {
		int id = parseU8();
		int t = parseU8();
		layerPri[id] = parseInt(t);
		layerReorder(id);
	}
	private void doLGetPri() {
		int id = parseU8();
		if (layerObj[id] == null)
			throw new RuntimeException();
		putInt(REG_REQDAT, layerPri[id]);
	}	
	
	private void doLTlAnim() {
		int id = parseU8();
		TiledLayer l = (TiledLayer)layerObj[id];
		int animTile = parseI8();
		int tile = parseU8();
		l.setAnimatedTile(animTile, tile);
	}
	
	private void doLTlFill() {
		int id = parseU8();
		TiledLayer l = (TiledLayer)layerObj[id];
		int animTiles = layerExtra[id];
		int index = parseU8();
		int col, row, numCols, numRows;
		if (reqcur == reqlen) {
			col = 0;
			row = 0;
			numCols = l.getColumns();
			numRows = l.getRows();
		} else {
			int t = parseU8();
			col = parseInt(t);
			row = parseInt(t);
			numCols = parseInt(t);
			numRows = parseInt(t);
		}
		if (index > 255 - animTiles)
			index = (byte)index;
		l.fillCells(col, row, numCols, numRows, index);
	}
	
	// TODO test more cases; rewrite smarter/shorter?
	private void doTlScrll() {
		int id = parseU8();
		TiledLayer l = (TiledLayer)layerObj[id];
		if (parseU8() != 0)
			throw new RuntimeException();
		int t = parseU8();
		int x1 = parseInt(t);
		int y1 = parseInt(t);
		int x2 = parseInt(t) + x1 - 1;
		int y2 = parseInt(t) + y1 - 1;
		if (x1 > x2 || y1 > y2)
			return;
		int dx = parseInt(t);
		int dy = parseInt(t);
		if (dy > 0) {
			for (int y = y2; y >= y1; y--) {
				int sy = y - dy;
				if (dx > 0) {
					for (int x = x2; x >= x1; x--) {
						int sx = x - dx;
						if (sx < x1 || sx > x2 || sy < y1 || sy > y2)
							l.setCell(x, y, 0);
						else
							l.setCell(x, y, l.getCell(sx, sy));
					}
				} else {
					for (int x = x1; x <= x2; x++) {
						int sx = x - dx;
						if (sx < x1 || sx > x2 || sy < y1 || sy > y2)
							l.setCell(x, y, 0);
						else
							l.setCell(x, y, l.getCell(sx, sy));
					}
				}
			}
		} else {
			for (int y = y1; y <= y2; y++) {
				int sy = y - dy;
				if (dx > 0) {
					for (int x = x2; x >= x1; x--) {
						int sx = x - dx;
						if (sx < x1 || sx > x2 || sy < y1 || sy > y2)
							l.setCell(x, y, 0);
						else
							l.setCell(x, y, l.getCell(sx, sy));
					}
				} else {
					for (int x = x1; x <= x2; x++) {
						int sx = x - dx;
						if (sx < x1 || sx > x2 || sy < y1 || sy > y2)
							l.setCell(x, y, 0);
						else
							l.setCell(x, y, l.getCell(sx, sy));
					}
				}
			}
		}
	}
	
	private void doLSpCopy() {
		int id = parseU8();
		if (layerObj[id] != null)
			layerManager.remove(layerObj[id]);
		int template = parseU8();
		Sprite l = new Sprite((Sprite)layerObj[template]);
		layerCreateSprite(id, l);
	}
	
	private void doLSpAPos() {
		int id = parseU8();
		Sprite l = (Sprite)layerObj[id];
		int t = parseU8();
		int x = parseInt(t);
		int y = parseInt(t);
		int xoffset = l.getX() - layerOX[id];
		int yoffset = l.getY() - layerOY[id];
		l.setRefPixelPosition(x, y);
		layerOX[id] = l.getX() - xoffset;
		layerOY[id] = l.getY() - yoffset;
	}
	
	// with datatype to allow ref pixel to be outside the sprite
	private void doLSpRefPx() {
		int id = parseU8();
		Sprite l = (Sprite)layerObj[id];
		int t = parseU8();
		int ox = parseInt(t);
		int oy = parseInt(t);
		l.defineReferencePixel(ox, oy);
	}
	
	// with datatype to allow bbox to be bigger than the sprite
	private void doLSpClRct() {
		int id = parseU8();
		Sprite l = (Sprite)layerObj[id];
		int t = parseU8();
		int x = parseInt(t);
		int y = parseInt(t);
		int width = parseInt(t);
		int height = parseInt(t);
		l.defineCollisionRectangle(x, y, width, height);
	}

	private static final int GAMESET_LID = 1;

	private void doGameSet() throws Throwable {
		int imageId = VAL_TILESET_SILK, cols = 0, rows = 0, tileWidth = 0, tileHeight = 0;
		int defaultTileWidth = 8, defaultTileHeight = 8, layerId = GAMESET_LID;
		if (reqlen > reqcur)
			imageId = parseI8();
		if (reqlen > reqcur) {
			cols = parseU8();
			rows = parseU8();
		}
		if (reqlen > reqcur)
			layerId = parseU8();
		if (reqlen > reqcur) {
			tileWidth = parseU8();
			tileHeight = parseU8();
		}
		if (reqlen != reqcur)
			throw new RuntimeException();
		if (layerObj.length < layerId + 1)
			doLDimImpl(layerId + 1);
		Image image;
		if (imageId == VAL_TILESET_SILK) {
			image = Image.createImage("/silk.png");
			defaultTileWidth = 16;
			defaultTileHeight = 16;
		} else if (imageId == VAL_TILESET_FONT) {
			image = Image.createImage("/font.png");
		} else {
			image = images[imageId];
		}
		if (tileWidth == 0)
			tileWidth = defaultTileWidth;
		if (tileHeight == 0)
			tileHeight = defaultTileHeight;
		if (cols == 0)
			cols = getWidth() / tileWidth;
		if (rows == 0)
			rows = getHeight() / tileHeight;
		if (layerObj[layerId] != null)
			layerManager.remove(layerObj[layerId]);
		TiledLayer l = new TiledLayer(cols, rows, image, tileWidth, tileHeight);
		l.setVisible(true);
		layerManager.setViewWindow(0, 0, l.getWidth(), l.getHeight());
		layerManagerOX = (getWidth() - l.getWidth()) >> 1;
		layerManagerOY = (getHeight() - l.getHeight()) >> 1;
		layerObj[layerId] = l;
		layerCtl[layerId] = VAL_LCTL_ENABLE;
		layerOX[layerId] = 0;
		layerOY[layerId] = 0;
		layerPri[layerId] = GAMESET_LID;
		layerExtra[layerId] = 0;
		layerReorder(layerId);
		m[REG_ENABLE] = VAL_ENABLE_BGCOL | VAL_ENABLE_LAYERS;
		m[REG_LID] = (byte)layerId;
		putShort(REG_REQDAT + VAL_GAMESET_COLS, cols);
		putShort(REG_REQDAT + VAL_GAMESET_ROWS, rows);
	}

	private synchronized void doGKey0Put() {
		int keyStates = getKeyStates();
		m[REG_GKEY0] = (byte)keyStates;
		m[REG_GKEY1] = (byte)(keyStates >> 8);
	}

	private int doLayerRW(boolean isWrite, int address, int value) {
		if (isWrite)
			m[address] = (byte)value;
		try {
			int id = m[REG_LID] & 0xFF;
			Layer l = layerObj[id];
			switch (address) {
			case REG_LCTL:
				if (isWrite) {
					l.setVisible((value & VAL_LCTL_ENABLE) != 0);
					layerCtl[id] = (byte)value;
				} else
					return layerCtl[id] & 0xFF;
				break;
			case REG_LX: {
				int shift = layerCtl[id] & VAL_LCTL_SHIFTX_MASK;
				if (shift == 0) {
					if (isWrite)
						l.setPosition(layerOX[id] + value, l.getY());
					else
						return (l.getX() - layerOX[id]) & 0xFF;
				} else {
					if (isWrite)
						l.setPosition(layerOX[id] + (value << shift), l.getY());
					else
						return ((l.getX() - layerOX[id]) >> shift) & 0xFF;
				}
				break; }
			case REG_LY: {
				int shift = layerCtl[id] & VAL_LCTL_SHIFTY_MASK;
				if (shift == 0) {
					if (isWrite)
						l.setPosition(l.getX(), layerOY[id] + value);
					else
						return (l.getY() - layerOY[id]) & 0xFF;
				} else {
					shift >>= 2; 
					if (isWrite)
						l.setPosition(l.getX(), layerOY[id] + (value << shift));
					else
						return ((l.getY() - layerOY[id]) >> shift) & 0xFF;
				}
				break; }
			case REG_SFRAME:
				if (isWrite)
					((Sprite)l).setFrame(value);
				else
					return ((Sprite)l).getFrame();
				break;
			case REG_STRANSFM:
				if (isWrite) {
					((Sprite)l).setTransform(value);
					layerExtra[id] = (byte)value;
				} else
					return layerExtra[id] & 0xFF;
				break;
			case REG_SCWITH:
				if (isWrite) {
					Layer with = layerObj[value];
					if (with instanceof TiledLayer)
						m[address] = ((Sprite)l).collidesWith(
								(TiledLayer)with,
								(layerCtl[id] & VAL_LCTL_PXLCOLL) != 0)
								? (byte)-1 : (byte)0;
					else
						m[address] = ((Sprite)l).collidesWith(
								(Sprite)with,
								(layerCtl[id] & VAL_LCTL_PXLCOLL) != 0)
								? (byte)-1 : (byte)0;
				}
				break;
			case REG_TCELL: {
				int x = m[REG_TCOLLO] & 0xFF;
				x |=  (m[REG_TCOLHI] & 0xFF) << 8;
				int y = m[REG_TROWLO] & 0xFF;
				y |=  (m[REG_TROWHI] & 0xFF) << 8;
				if (isWrite) {
					if (value > 255 - layerExtra[id])
						((TiledLayer)l).setCell(x, y, (byte)value);
					else
						((TiledLayer)l).setCell(x, y, value);
				} else {
					return ((TiledLayer)l).getCell(x, y) & 0xFF;
				}
				break; }
			default:
				;
			}
		} catch (Throwable e) {
		}
		if (isWrite)
			return 0;
		else
			return m[address];
	}

	private int layerPutOX;
	private int layerPutY;
	private int layerPutCols;
	
	// without datatype to simplify code/other modules
	private void doLTlPut(int pos, int value) throws Throwable {
		if (pos == -1) {
			// nothing to do
		} else if (pos < LTLPUT_HEADER_SIZE) {
			req[pos] = (byte)value;
			strNextSection = LTLPUT_HEADER_SIZE;
		} else {
			if (pos == LTLPUT_HEADER_SIZE) {
				layerPutOX = req[LTLPUT_HEADER_OX_OFFSET] & 0xFF;
				layerPutOX |= (req[LTLPUT_HEADER_OX_OFFSET + 1] << 8) & 0xFF;
				layerPutY = req[LTLPUT_HEADER_OY_OFFSET] & 0xFF;
				layerPutY |= (req[LTLPUT_HEADER_OY_OFFSET + 1] << 8) & 0xFF;
				layerPutCols = req[LTLPUT_HEADER_COLS_OFFSET] & 0xFF;
				layerPutCols |= (req[LTLPUT_HEADER_COLS_OFFSET + 1] << 8) & 0xFF;
			}
			int id = req[LTLPUT_HEADER_LAYERID_OFFSET];
			int x = pos - strNextSection;
			if (x == layerPutCols) {
				strNextSection = pos;
				layerPutY++;
				x = 0;
			}
			TiledLayer l = (TiledLayer)layerObj[id];
			if (value > 255 - layerExtra[id])
				l.setCell(x + layerPutOX, layerPutY, (byte)value);
			else
				l.setCell(x + layerPutOX, layerPutY, value);
		}
	}

	private void layerDraw(Graphics g) {
		layerManager.paint(g, layerManagerOX, layerManagerOY);
	}

	// #endif

	/*************************************************************************
	 * Effects
	 *************************************************************************/
	
	// #if ENABLE_EFFECTS
	
	private void doFXTone() {
		int t = parseU8() * 10;
		int tone = parseU8();
		int vol = parseU8();
		try {
			Manager.playTone(tone, t, vol);
		} catch (Exception e) {
			throw new RuntimeException();
		}
	}
	
	private void doFXVibra() {
		int t = parseU8() * 10;
		boolean supported = display.vibrate(t);
		m[REG_REQDAT] = (byte)(supported ? -1 : 0);
	}
	
	private void doFXFlash() {
		int t = parseU8() * 10;
		boolean supported = display.flashBacklight(t);
		m[REG_REQDAT] = (byte)(supported ? -1 : 0);
	}
	
	// #endif

	/*************************************************************************
	 * Shell
	 *************************************************************************/
	
	public IO() {
		// #if ENABLE_GAME_CANVAS
		super(false);
		// #endif
		// #if JavaPlatform != "MIDP/1.0"
		setFullScreenMode(true);
		// #endif
		// #if ENABLE_GAME_CANVAS
		g = getGraphics();
		// #endif
	}
	
	private int reset() {
		coreReset();
		// #if ENABLE_CONSOLE
		conReset();
		// #endif
		// #ifdef ENABLE_GAMEAPI
		layerReset();
		// #endif
		flush();
		return 0;
	}

	// #if !ENABLE_GAME_CANVAS
//@	private void flush() {
//@		repaint();
//@		serviceRepaints();
//@	}
//@	
//@	protected void paint(Graphics g) {
	// #else
	private void flush() {
	// #endif
		int enable = m[REG_ENABLE];
		int w = getWidth(), h = getHeight();
		if ((enable & VAL_ENABLE_BGCOL) != 0) {
			g.setColor(bgCol);
			g.fillRect(0, 0, w, h);
		}
		// #if ENABLE_IMAGE
		if ((enable & VAL_ENABLE_BGIMG) != 0) {
			Image image = bgImg;
			if (image != null)
				g.drawImage(image, w >> 1, h >> 1,
						Graphics.HCENTER | Graphics.VCENTER);
		}
		// #endif
		// #if ENABLE_CONSOLE
		if ((enable & VAL_ENABLE_CONSOLE) != 0)
			conDraw(g);
		// #endif
		// #if ENABLE_GAMEAPI
		if ((enable & VAL_ENABLE_LAYERS) != 0)
			layerDraw(g);
		// #endif
		touchDraw(false, g);
		// #if ENABLE_GAME_CANVAS
		flushGraphics();
		// #endif
	}
	
	private boolean isStreamRequest() {
		switch (req[0]) {
		case REQ_SETPAL:
		// #if ENABLE_IMAGE
		case REQ_IRAWRGBA:
		// #endif
		// #if ENABLE_IMAGE_PNGGEN
		case REQ_IPNGGEN:
		// #endif
		// #if ENABLE_GAMEAPI
		case REQ_LTLPUT:
		// #endif
			return true;
		}
		return false;
	}
	
	private boolean handleStreamRequest(int pos, int value) throws Throwable {
		switch (req[0]) {
		case REQ_SETPAL:
			doSetPal(pos, value);
			return true;
		// #if ENABLE_IMAGE && 	JavaPlatform != "MIDP/1.0"
		case REQ_IRAWRGBA:
			doIRawRGBA(pos, value);
			return true;
		// #endif
		// #if ENABLE_IMAGE_PNGGEN
		case REQ_IPNGGEN:
			doIPngGen(pos, value);
			return true;
		// #endif
		// #if ENABLE_GAMEAPI
		case REQ_LTLPUT:
			doLTlPut(pos, value);
			return true;
		// #endif
		default:
			return false;
		}
	}
	
	private void doRequestPut(int value) {
		if (reqlen == 0)
			req[0] = (byte)value;
		if (isStreamRequest()) {
			try {
				handleStreamRequest(reqlen++, value);
			} catch (Throwable e) {
			}
			return;
		}
		if (reqlen >= 255) {
			reqlen++;
			return;
		}
		req[reqlen++] = (byte)value;
	}
	
	private void doRequestEnd() {
		boolean ok = true;
		try {
			if (reqlen == 0 || (reqlen > 255 && !isStreamRequest()))
				throw new RuntimeException();
			// #if DEBUG
//@			System.out.print(labelToString("REQ", req[0]));
//@			for (int i = 1; i < reqlen; i++)
//@				System.out.print(" " + (req[i] & 0xFF));
//@			System.out.println();
			// #endif
			reqcur = 0;
			byte type = req[reqcur++];
			switch (type) {
			case REQ_TIME:
				doTime();
				break;
			case REQ_LOADROM:
				doLoadROM();
				break;
			// #ifdef ENABLE_RECSTORE
			case REQ_RSFORMAT:
				doRSFormat();
				break;
			case REQ_RLOAD:
				doRLoad();
				break;
			case REQ_RSAVE:
				doRSave();
				break;
			case REQ_RDELETE:
				doRDelete();
				break;
			// #endif
			case REQ_DPYINFO:
				doDpyInfo();
				break;
			case REQ_SETBGCOL:
				bgCol = parseColor();
				break;
			// #if ENABLE_IMAGE
			case REQ_SETBGIMG:
				bgImg = images[parseU8()];
				break;
			case REQ_IDESTROY:
				doIDestroy();
				break;
			case REQ_IDIM:
				doIDim();
				break;
			case REQ_IINFO:
				doIInfo();
				break;
			case REQ_ILOAD:
				doILoad();
				break;
			case REQ_IEMPTY:
				doIEmpty();
				break;
			case REQ_IMKIMMUT:
				doIMkImmut();
				break;
			// #endif
			// #if ENABLE_IMAGE_DUMMY
			case REQ_IDUMMY:
				doIDummy();
				break;
			// #endif
			// #if ENABLE_GAMEAPI
			case REQ_LMVIEW:
				doLMView();
				break;
			case REQ_LMPOS:
				doLMPos();
				break;
			case REQ_LDESTROY:
				doLDestroy();
				break;
			case REQ_LDIM:
				doLDim();
				break;
			case REQ_LTILED:
				doLTiled();
				break;
			case REQ_LSPRITE:
				doLSprite();
				break;
			case REQ_LSETPOS:
				doLSetPos();
				break;
			case REQ_LGETPOS:
				doLGetPos();
				break;
			case REQ_LMOVE:
				doLMove();
				break;
			case REQ_LSETPRI:
				doLSetPri();
				break;
			case REQ_LGETPRI:
				doLGetPri();
				break;
			case REQ_LTLANIM:
				doLTlAnim();
				break;
			case REQ_LTLFILL:
				doLTlFill();
				break;
			case REQ_LTLSCRLL:
				doTlScrll();
				break;
			case REQ_LSPCOPY:
				doLSpCopy();
				break;
			case REQ_LSPAPOS:
				doLSpAPos();
				break;
			case REQ_LSPREFPX:
				doLSpRefPx();
				break;
			case REQ_LSPCLRCT:
				doLSpClRct();
				break;
			case REQ_GAMESET:
				doGameSet();
				break;
			// #endif
			// #if ENABLE_EFFECTS
			case REQ_FXTONE:
				doFXTone();
				break;
			case REQ_FXVIBRA:
				doFXVibra();
				break;
			case REQ_FXFLASH:
				doFXFlash();
				break;
			// #endif
			default:
				ok = handleStreamRequest(-1, (byte)0);
			}
			if (!isStreamRequest())
				parseEnd();
			if (!ok)
				throw new RuntimeException();
			m[REG_REQRES] = 0;
		} catch (Throwable e) {
			// #if DEBUG
//@			System.out.println("REQ Failed!!!");
			// #endif
			m[REG_REQRES] = -1;
		} finally {
			reqlen = 0;
		} 
	}
	
	private void doRequestPtr(int value) {
		m[REG_REQPTRLO] = (byte)value;
		int ptr = ((m[REG_REQPTRHI] << 8) & 0xFF00) | value;
		try {
			if (ptr > 0xFFFE)
				throw new RuntimeException();
			int len = as.get(ptr++);
			len |= as.get(ptr++) << 8;
			int i;
			for (i = 0; i < len & ptr <= 0xFFFF; i++)
				doRequestPut(as.get(ptr++));
			if (i < len)
				throw new RuntimeException();
		} catch (Throwable e) {
			m[REG_REQRES] = -1;
			return;
		}
		doRequestEnd();
	}
	
	protected void keyPressed(int keyCode) {
		if (vmStatus == IOSvc.VM_STATUS_MONITOR) {
			signalUserBreak = true;
			return;
		} else if (keyCode > 0) {
			enqueKeyCode(keyCode);
		} else  {
			// The following is a very hard choice.
			// skipping every game key might loose the ability to
			// break the program on some phones.
			// We choose to skip only the most important ones,
			// hoping for the best.
			// #if ENABLE_GAMEAPI
			try {
				switch (getGameAction(keyCode)) {
				case FIRE:
				case UP:
				case DOWN:
				case LEFT:
				case RIGHT:
					return;
				}
			} catch (Throwable e) {
			}
			// #endif
			signalUserBreak = true;
		}
	}
	
	public void commandAction(Command c, Displayable d) {
	}

	private int align(int vmStatus) {
		this.vmStatus = vmStatus;
		if (signalUserBreak) {
			signalUserBreak = false;
			return IOSvc.ALIGN_USER_BREAK;
		}
		if (signalSuspendCPU) {
			signalSuspendCPU = false;
			return IOSvc.ALIGN_SUSPEND_CPU;
		}
		if (signalResumeCPU) {
			signalResumeCPU = false;
			return IOSvc.ALIGN_RESUME_CPU;
		}
		switch (ioStatus) {
		case UNLOADED:
		case OPERATIONAL:
			return IOSvc.ALIGN_ACTIVE;
		default:
			return IOSvc.ALIGN_FAILED;
		}
	}
	
	private static final int FRAME_MIN_WAIT = 10;
	
	private int doSomeWork() {
		int wait = -1;
		long now = System.currentTimeMillis();
		if (nextFrame != 0 && now >= nextFrame - FRAME_MIN_WAIT) {
			do
				nextFrame += frameInterval;
			while (nextFrame < now);
			flush();
			if (waitingForFrame) {
				waitingForFrame = false;
				signalResumeCPU = true;
			}
			wait = 0;
		}
		if (waitingForFrame) {
			wait = (int)(nextFrame - now - (FRAME_MIN_WAIT >> 1));
		}
		return wait;
	}

	private Object getMetadata(int iArg) {
		switch (iArg) {
		case METADATA_ID_LABEL:
			return "IO Chip";
		case METADATA_ID_TYPE:
			return new Integer(TYPE_VM_PLUGIN);
		case METADATA_ID_SERVICES:
			return new String[] { IOSvc.TAG };
		default:
			return null;
		}
	}

	private int init(Object oArg) {
		display = Display.getDisplay((MIDlet)oArg);
		// #if !JBIT_RUNTIME
		touch = (Module)((Module)oArg).opO(JBitSvc.OP_FIND_SERVICE, 0, TouchSvc.TAG);
		// #else
//@		try {
//@			Class cls = Class.forName("Touch");
//@			touch = (Module)cls.newInstance();
//@			touch.opI(Module.OP_INIT, 0, null);
//@		} catch (Throwable e) {
//@			touch = null;
//@		}
		// #endif
		return 0;
	}

	private int activate() {
		return 0;
	}

	private int deactivate() {
		return 0;
	}

	private static final Integer touchContext = new Integer(TouchSvc.CONTEXT_EMULATOR);
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
			return init(oArg);
		case OP_ACTIVATE:
			return activate();
		case OP_DEACTIVATE:
			return deactivate();
		case IOSvc.OP_RESET:
			return reset();
		case IOSvc.OP_FLUSH:
			if (frameInterval != 0)
				flush();
			return 0;
		case IOSvc.OP_ALIGN:
			return align(iArg);
		case IOSvc.OP_DO_SOME_WORK:
			return doSomeWork();
		case IOSvc.OP_SET_ADDRESS_SPACE:
			as = (Module)oArg;
			return 0;
		}
		return -1;
	}

	public int get(int address) {
		switch (address) {
		case REG_RANDOM:
			return doRandomGet();
		// #if ENABLE_CONSOLE
		case REG_CONCCHR:
		case REG_CONCFG:
		case REG_CONCBG:
			return doConsoleRW(false, address, 0);
		// #endif
		default:
			if (false)
				;
			// #if ENABLE_GAMEAPI
			else if (address >= REG_LID && address <= REG_TCELL)
				return doLayerRW(false, address, 0);
			// #endif
			// #if ENABLE_CONSOLE
			else if (address >= REG_CONVIDEO && address < REG_CONVIDEO + 40)
				return doConsoleRW(false, address, 0);
			// #endif
			else
				break;
		}			
		return m[address] & 0xFF;
	}

	public int put(int address, int value) {
		switch (address) {
		case REG_REQPUT:
			doRequestPut(value);
			return 0;
		case REG_REQEND:
			doRequestEnd();
			return 0;
		case REG_REQPTRLO:
			doRequestPtr(value);
			return 0;
		case REG_FRMFPS:
			doFrmFpsPut(value);
			return 0;
		case REG_FRMDRAW:
			doFrmDrawPut();
			return 0;
		case REG_RANDOM:
			doRandomPut(value);
			return 0;
		case REG_CONCOLS:
		case REG_CONROWS:
		case REG_CONCX:
		case REG_CONCY:
		case REG_CONCCHR:
		case REG_CONCFG:
		case REG_CONCBG:
			// #if ENABLE_CONSOLE
			doConsoleRW(true, address, value);
			// #endif
			return 0;
		case REG_KEYBUF:
			doKeyBufPut();
			return 0;
		// #if ENABLE_GAMEAPI
		case REG_GKEY0:
			doGKey0Put();
			return 0;
		// #endif
		case REG_ENABLE:
		case REG_REQPTRHI:
			break; // OK
		default:
			if (address >= REG_REQDAT && address < REG_REQDAT + 32)
				; // OK
			else if (address >= REG_LID && address <= REG_TCELL) {
				// #if ENABLE_GAMEAPI
				doLayerRW(true, address, value);
				return 0;
				// #else
//@				break;
				// #endif
			}
			else if (address >= REG_CONVIDEO && address < REG_CONVIDEO + 40) {
				// #if ENABLE_CONSOLE
				doConsoleRW(true, address, value);
				return 0;
				// #else
//@				break;
				// #endif
			}
			else
				return 0; // ignore them TODO -1 or 0?
		}			
		m[address] = (byte)value;
		return 0;
	}

}
