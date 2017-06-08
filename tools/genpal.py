#! /usr/bin/python3

standardPalette = [
	0x000000, # Black
	0xFFFFFF, # White
	0xBE1A24, # Red
	0x30E6C6, # Cyan
	0xB41AE2, # Purple
	0x1FD21E, # Green
	0x211BAE, # Blue
	0xDFF60A, # Yellow
	0xB84104, # Orange
	0x6A3304, # Brown
	0xFE4A57, # Light Red
	0x424540, # Dark Gray
	0x70746F, # Medium Gray
	0x59FE59, # Light Green
	0x5F53FE, # Light Blue
	0xA4A7A2, # Light Gray
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
merge_palette(palette, microioPalette)
merge_palette(palette, websafePalette)

for c in palette:
	print("\t0x%02X, 0x%02X, 0x%02X," % (c >> 16, (c >> 8) & 0xff, c & 0xff))
