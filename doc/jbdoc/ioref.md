# IO Reference #

# Registers

### REQPUT (1) [W]

### REQEND (2) [W]

Send a request to the IO chip byte by byte. Each byte of the request is sent
by writing it into REQPUT. The request is processed when any value is written
into REQEND.

### REQRES (3) [R]

Result of the last issued request. 0 on success, != 0 otherwise. To simplify
the code, it is common not to test it.

### REQPTRLO (4) [W]

### REQPTRHI (5) [RW]

Send a request to the IO chip using DMA (Direct Memory Access). When a value
is written into REQPTRLO, the two registers (LO for the offset, HI for the
page) are used to point (PTR, for PoinTeR) to an area in memory where the
request is stored. The area begins with two bytes (least significant byte
first) storing the length of the request.

### ENABLE (16) [RW]

Enable the drawing of: the background color, the background image, the
console, and the layers. Default is 5 (1+4, BGCOL+CONSOLE).

	ENABLE
	-+-------
	1|BGCOL
	2|BGIMG
	4|CONSOLE
	8|LAYERS

### FRMFPS (17) [RW]

Number of frames per seconds multiplied by 4 (e.g. 40, the initial value, is
10 fps).

### FRMDRAW (18) [W]

 Writing into FRMDRAW causes the CPU to be suspended until the current frame
has been drawn.

### GKEY0 (19) [RW]

### GKEY1 (20) [R]

First write into GKEY0 to update the game keys latches, then read GKEY0/GKEY1
to find out the status of the gamekeys.

	GKEY0
	--+-----
	 2|UP
	 4|LEFT
	32|RIGHT
	64|DOWN

	GKEY1
	--+----
	 1|FIRE
	 2|A
	 4|B
	 8|C
	16|D

### RANDOM (23) [RW]

On read: get a random number <= n (255 by default). On write: set n (if > 0),
or swap the number generator (if == 0). There are two number generators: one
(used by default) is initialized using the time at the start of the program,
the other is initialized with a constant.

### KEYBUF (24, 8 bytes) [RW]

The standard KeyPresses (the ones that can be represented by ASCII codes;
usually only 0-9, # and *) are enqueued starting from KEYBUF; the rest of the
buffer is filled with 0s. Write into KEYBUF to consume a KeyPress. If the
buffer is full when a new key is pressed, that KeyPress is lost.

### CONCOLS (32) [RW]

### CONROWS (33) [RW]

On read: get the dimension of the console (default: 10x4). On write: resize
the console (0 = max size).

### CONCX (34) [RW]

### CONCY (35) [RW]

The console cursor (current console cell). CX < CONCOLS and CY < CONROWS.
CONCCHR, CONCFG and CONCBG are relative to this cell.

### CONCCHR (36) [RW]

The character code of the current console cell. Codes 32-126 are standard
ASCII codes. Codes 161-255 are standard Latin1 codes. Codes 128-143 are
non-standard line art codes (see bitmask below).

	    0 1 2 3 4 5 6 7 8 9
	 30       ! " # $ % & '
	 40 ( ) * + , - . / 0 1
	 50 2 3 4 5 6 7 8 9 : ;
	 60 < = > ? @ A B C D E
	 70 F G H I J K L M N O
	 80 P Q R S T U V W X Y
	 90 Z [ \ ] ^ _ ` a b c
	100 d e f g h i j k l m
	110 n o p q r s t u v w
	120 x y z { | } ~

	   T    +-+-+-+-+-+-+-+-+
	   T    |1|0|0|0|B|R|L|T|
	LLLXRRR +-+-+-+-+-+-+-+-+
	   B     1 6 3 1 8 4 2 1
	   B     2 4 2 6
	         8

### CONCFG (37) [RW]

### CONCBG (38) [RW]

The foreground and background color of the current console cell. Here are the
standard color codes:

	 #|RED|GRN|BLU|NAME
	--+---+---+---+----------
	 0|  0|  0|  0|BLACK
	 1|255|255|255|WHITE
	 2|190| 26| 36|RED
	 3| 48|230|198|CYAN
	 4|180| 26|226|PURPLE
	 5| 31|210| 30|GREEN
	 6| 33| 27|174|BLUE
	 7|223|246| 10|YELLOW
	 8|184| 65|  4|ORANGE
	 9|106| 51|  4|BROWN
	10|254| 74| 87|LIGHTRED
	11| 66| 69| 64|GRAY1
	12|112|116|111|GRAY2
	13| 89|254| 89|LIGHTGREEN
	14| 95| 83|254|LIGHTBLUE
	15|164|167|162|GRAY3

### CONVIDEO (40, 40 bytes) [RW]

Quick access to the character codes of the topmost/leftmost 10x4 cells of the
console.

### LID (80) [RW]

The ID of the current layer.

### LCTL (81) [RW]

Control register of the current layer. SHIFTX0 (1) and SHIFTX1 (2) are used
to shift its X offset (see LX). SHIFTY0 (4) and SHIFTY1 (8) are used to shift
its Y offset (see LY). PXLCOLL (16) is set if collision detection should be
pixel perfect (more accurate, but slower); if unset, its collision rectangle
is used (approximate, but faster). ENABLE (128) is set if the layer is
visible.

### LX (82) [RW]

### LY (82) [RW]

The offset added to the position of the current layer, shifted by LCTL:SHIFTX
and LCTL:SHIFTY bits to the left. For example, if LX is 3 and SHIFTX is 2
(i.e. SHIFTX0 is 0 and SHIFTX1 is 1) the offset is 3 << 2 = 12.

### SFRAME (84) [RW]

If the current layer is a sprite, its active frame.

### STRANSFM (85) [RW]

If the current layer is a sprite, its current transformation relative to its
reference point. Valid values are: NONE (0), ROT90 (5), ROT180 (3), ROT270
(6), MIRROR (2), MROT90 (7), MROT180 (1), MROT270 (4).

### SCWITH (86) [RW]

Write the ID of a layer to check if the current layer collides with it; then
read the result (0 means no collision).

### TCOLLO (87) [RW]

### TCOLHI (88) [RW]

### TROWLO (89) [RW]

### TROWHI (90) [RW]

If the current layer is a tiled layer, the current tile cell (HI*256+LO).

### TCELL (91)

The tile id of the current tile cell.

### REQDAT (96, 32 bytes)

Return values for some IO requests.

# Requests

Here below you can find the syntax of the requests (and the corresponding
results, when applicable). See the bgcol1 and bgcol2 demos for examples of
how to send a request to the IO chip. After the request has been sent, REQRES
contains 0 on success and 255 on failure (usually not tested). Results are
available starting from REQDAT. Streaming requests (identified by a >) are
not bounded. The other requests are bounded (255 bytes).

Optional values are delimited by [ and ]. ( and ) are used for grouping. *
means repeat 0 or more times. + means repeat at least once. # means repeat
with constraints. | means choice (priority is low). Datatype is U8 unless
stated otherwise (by a tag preceded by :). For datatypes larger than 8 bits,
the least significat byte comes first. Enumerated values are identified by C
(for choice). Bitmasks are identified by O (for OR). Strings can be delimited
by 0 (S0) or by the end of the request (S). When an argument has datatype T,
the actual datatype is chosen by the user with DType: U8 (1), I8 (2), U16 (3)
and I16 (4).

For the semantic of the requests, take a look at the demos or simply
experiment using names as hints. Notes for the IPNGGEN request: using INDEXED
COLOR causes a PLTE chunk to be generated (a pallette must be provided) and
setting IDX0TRANSP causes a tRNS chunk to be generated. For more information
see the PNG specification.

## System

### TIME(2)

Get the elapsed time either since the epoch (1 Jan 1970) or the time the IO
chip was reset. Resolution ranges from milliseconds to seconds.

Syntax: TIME(2) [RefTime=ABS [Fract=1000]] ; RefTime(C): ABS(1), RESET(2) ;
Fract(C): 1(1), 10(2), 10(3), 1000(4) ; Result: Time:U64

## Display and Imaging

### DPYINFO(16)

Get information about the display.

Syntax: DPYINFO(16) ; Result: Width:U16 Height:U16 ColorDepth AlphaDepth
Flags ; Flags(O): ISCOLOR(128), ISMIDP2(64)

### SETBGCOL(17)

Set the background color. Only used if BGCOL is set in the ENABLE register.

Syntax: SETBGCOL(17) PaletteEntry | Red Green Blue

### SETPAL(18)

Set the current palette. An empty palette resets the standard one.

Syntax: SETPAL(18)> (Red Green Blue)*

### SETBGIMG(19)

Set the background image. Only used if BGIMG is set in the ENABLE register.
Once the request returns, the source image slot can be released / reused.

Syntax: SETBGIMG(19) ImageId

### IDESTROY(20)

Release a specific image slot.

Syntax: IDESTROY(20) ImageId

### IDIM(21)

Define which image slots are available (0..MaxImageId, initially 0..3).

Syntax: IDIM(21) MaxImageId

### IINFO(22)

Get information about an image.

Syntax IINFO(22) ImageId ; Result: Width:U16 Height:U16 Flags ; Flags(O):
ISMUTABLE(128)

### ILOAD(23)

Load an image from a resource included in the jar.

Syntax: ILOAD(23) ImageId ResName:S0

### IDUMMY(24)

Create a dummy image with a user defined pattern. Might be useful for
testing. Bg is the background color, Fg is the foreground color, Title is a
short text used as a label. Width and Height refer to the size of a single
Frame/Tile. Tiles can be spans of different colors.

Syntax(1): IDUMMY(24) ImageId Type=SIMPLE Width:U16 Height:U16 Bg Fg
[Title:S] ; Syntax(2): IDUMMY(24) ImageId Type=SPRITE Width Height Frames Bg
Fg [Title:S] ; Syntax(3): IDUMMY(24) ImageId Type=TILES Width Height Cols
Rows Bg Fg (N Bg Fg)* ; Type(C): SIMPLE(1), SPRITE(2), TILES(3)

### IPNGGEN(25)

Generate an image using a packed representation. Data is a sequence of rows,
each row is a sequence of pixels and each pixel is a sequence of bits. Each
row is padded to a whole byte. If INDEXED COLOR is used, pixels are pointers
to a palette preceding Data. If PALREF is set, palette entries are references
to the current palette defined by the request SETPAL (or the standard
palette). If IDX0TRANSP is set, ColorType must be INDEXED COLOR and pixels 0
are treated as transparent and the first palette entry is ignored.

Syntax: IPNGGEN(25)> ImageId Width:U16 Height:U16 Depth ColorType Flag
[MaxPaletteEntry (PaletteEntry | Red Green Blue)#] Data# ; ColorType(C):
GRAYSCALE(0), TRUECOLOR(2), INDEXED COLOR(3), GRAYSCALE ALPHA(4), TRUECOLOR
ALPHA(6) ; Flags(O): IDX0TRANSP(1), PALREF(2), ZOOM0(4), ZOOM1(8)

### IEMPTY(26)

Create an empty mutable image. Not useful at the moment.

Syntax: IEMPTY(26) ImageId Width:U16 Height:U16

### IMKIMMUT(27)

Make an image immutable. Not useful at the moment.

Syntax: IMKIMMUT(27) ImageId

### IRAWRGBA(28)

Generate an image using data in RGBA8 format. If ALPHA is set, the alpha
component is meaningful. Might be faster than IPNGGEN. Only available on
MIDP2 phones.

Syntax: IRAWRGBA(28)> ImageId Width:U16 Height:U16 Flags (Red Green Blue
Alpha)# ; Flags(O): ALPHA(128)

## Layers (Game API)

### LMVIEW(32)

Setup the layer manager so that: 1) a tiled layer is centered; or 2) a window
is displayed

Syntax: LMVIEW(32) TiledLayerId | DType [X:T Y:T] Width:T Height:T

### LMPOS(33)

Set the position of the layer manager on the screen

Syntax: LMPOS(33) DType OX:T OY:T

### LDESTROY(34)

Release a specific layer slot.

Syntax: IDESTROY(34) LayerId

### LDIM(35)

Define which layer slots are available (0..MaxLayerId, initially 0..15).

Syntax: LDIM(35) MaxLayerId

### LTILED(36)

Create a (tiled) layer. Each tile is TWidth x THeight pixels. Pixels for the
tiles come from the source image. After the layer is created, the image can
be destroyed. The layer is composed of Cols x Rows tiles. The last NAnimTiles
ids (e.g. 254 and 255, NAnimTiles is 2) are dynamic and you can change the
static id they point to using the LTLANIM request.

Syntax: LTILED(36) TiledLayerId ImageId TWidth THeight NAnimTiles DType
Cols:T Rows:T

### LSPRITE(37)

Create a (sprite) layer. Pixels for the sprite come from the source image.
After the layer is created, the image can be destroyed. if Width x Height is
provided, the sprite is composed of multiple frames. Use the SFRAME register
to select the active one (initially 0).

Syntax: LSPRITE(37) SpriteId ImageId [Width Height]

### LSETPOS(38)

Set the position of a layer.

Syntax: LSETPOS(38) LayerId DType X:T Y:T

### LGETPOS(39)

Get the position of a layer.

Syntax: LGETPOS(39) LayerId ; Result: X:I32 Y:I32

### LMOVE(40)

Add a relative value to the position of a layer.

Syntax: LMOVE(40) LayerId DType DX:T DY:T

### LSETPRI(41)

Set the priority of a layer (initially equal to its ID). This can be a fairly
slow request, and should be avoided during the main loop.

Syntax: LSETPRI(41) LayerId DType Priority:T

### LGETPRI(42)

Get the priority of a layer.

Syntax: LGETPRI(42) LayerId ; Result: Priority:I32

### LTLANIM(43)

Change the id of the dynamic tile (initially 0). This is a fairly fast
request, and can be safetly used during the main loop.

System: LTLANIM(43): TiledLayerId AnimTile StaticTile

### LTLFILL(44)

Fill (part of) a tiled layer with a specific tile. When created, a tiled
layer is filled with 0 (empty tile).

Syntax: LTLFILL(44) TiledLayerId Tile [DType Col:T Row:T NumCols:T NumRows:T]

### LTLPUT(45)

Fill part of a tiled layer with tiles provided at the end of the request.

Syntax: LTLPUT(45)> TiledLayerId Col:U16 Row:U16 NumCols:U16 Tile*

### LTLSCRLL(46)

Scroll the tiles within a rectangular region of a tiled layer. New tiles are
filled with 0 (empty tile).

Syntax: LTLSCRLL(46) TiledLayerId ScrollType=0 DType Col:T Row:T NumCols:T
NumRows:T DX:T DY:T

### LSPCOPY(47)

Create a (sprite) layer using an existing sprite as a template. On some
phones, this might be more memory efficient than creating an idential sprite.

Syntax: LSPCOPY(47) SpriteId TemplateSpriteId

### LSPAPOS(48)

Syntax: LSPAPOS(48) SpriteId DType AbsX:T AbsY:T

### LSPREFPX(49)

Set the reference point of a sprite. Used as a pivot point for transformation
and when setting the sprite position with LSPAPOS.

Syntax: LSPREFPX(49) SpriteId DType RefPixelX:T RefPixelY:T

### LSPCLRCT(50)

Set the bounding box of a sprite. Used for approximate collision detection.

Syntax: LSPCLRCT(50) SpriteId DType CollRctOX:T CollRctOY:T CollRctWidth:T
CollRctHeight:T

## Effects

WARNING: The effect API is poorly supported on most phones, and in some cases
it might cause JBit to freeze. Experiment only after you have saved your work!

### FXTONE(64)

Play a tone for Duration * 10 milliseconds. Volume: 0-100.

Syntax: FXTONE(64) Duration Frequency Volume

### FXVIBRA(65)

Enable the vibra function for Duration * 10 milliseconds.

Syntax: FXVIBRA(65) Duration ; Result: Supported

### FXFLASH(66)

Enable the flashlight function for Duration * 10 milliseconds.

Syntax: FXFLASH(66) Duration ; Result: Supported

