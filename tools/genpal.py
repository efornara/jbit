#! /usr/bin/python3

standardPalette = [
	0x000000, # Black
	0xffffff, # White
	0xbe1a24, # Red
	0x30e6c6, # Cyan
	0xb41ae2, # Purple
	0x1fd21e, # Green
	0x211bae, # Blue
	0xdff60a, # Yellow
	0xb84104, # Orange
	0x6a3304, # Brown
	0xfe4a57, # Light Red
	0x424540, # Dark Gray
	0x70746f, # Medium Gray
	0x59fe59, # Light Green
	0x5f53fe, # Light Blue
	0xa4a7a2, # Light Gray
]

# see: http://pixeljoint.com/forum/forum_posts.asp?TID=12795 (DawnBringer)
db16Palette = [
	0x140c1c,
	0x442434,
	0x30346d,
	0x4e4a4e,
	0x854c30,
	0x346524,
	0xd04648,
	0x757161,
	0x597dce,
	0xd27d2c,
	0x8595a1,
	0x6daa2c,
	0xd2aa99,
	0x6dc2ca,
	0xdad45e,
	0xdeeed6,
]

microioPalette = [
	0x000000, # Foreground
	0x78c8b4, # Background
	0x90e0c0, # Border
	0x000000, # Unused
]

websafePalette = []
levels = [ 0x00, 0x33, 0x66, 0x99, 0xcc, 0xff ]
for r in levels:
	for g in levels:
		for b in levels:
			websafePalette.append((((r << 8) | g) << 8) | b)

def merge_palette(accum, palette):
	for c in palette:
		if not c in accum:
			accum.append(c)

palette = []
merge_palette(palette, standardPalette)
merge_palette(palette, db16Palette)
merge_palette(palette, microioPalette)
merge_palette(palette, websafePalette)

for c in palette:
	print("\t0x%02X, 0x%02X, 0x%02X," % (c >> 16, (c >> 8) & 0xff, c & 0xff))
