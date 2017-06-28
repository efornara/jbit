#! /usr/bin/python3

import sys
if len(sys.argv) != 3:
	raise Exception('usage: ipnggen.py <input image> <output jbr>')

from PIL import Image

# get a palette-based image
im = Image.open(sys.argv[1])
w, h = im.size
alpha = im.convert('RGBA').split()[3]
if im.mode != 'P':
	im = im.convert('P')
p = im.palette.getdata()[1]

# find the transparent index (if any)...
pal = list(range(len(p) // 3))
transp = None
for y in range(h):
	for x in range(w):
		if alpha.getpixel((x, y)) < 128:
			transp = im.getpixel((x, y))
# ...and make it the first one
if transp:
	pal[0], pal[transp] = pal[transp], pal[0]

# calculate best depth
if len(pal) > 256:
	raise Exception('too many colors')
if len(pal) > 16:
	depth = 8
elif len(pal) > 4:
	depth = 4
elif len(pal) > 2:
	depth = 2
else:
	depth = 1

class JBReqGen:
	binary = bytearray()
	bitbuf = 0
	bitlen = 0
	def u8(self, value, comment=None):
		self.binary.append(value)
	def u16(self, value, comment=None):
		self.binary.append(value & 0xff)
		self.binary.append(value >> 8)
	def bits(self, value, depth):
		mask = 1 << (depth - 1)
		while mask != 0:
			self.bitbuf <<= 1
			if value & mask != 0:
				self.bitbuf |= 1
			mask >>= 1
			self.bitlen += 1
			if self.bitlen == 8:
				self.u8(self.bitbuf)
				self.bitbuf = 0
				self.bitlen = 0
	def padbits(self):
		if self.bitlen == 0:
			return
		for i in range(8 - self.bitlen, 8):
			self.bitbuf <<= 1
		self.u8(self.bitbuf)
		self.bitbuf = 0
		self.bitlen = 0
	def seq(self, value, comment=None):
		for v in value:
			self.binary.append(v)
	def write(self, filename):
		size = len(self.binary) - 8
		if size < 256 * 256:
			self.binary[6] = size & 0xff
			self.binary[7] = size >> 8
		with open(filename, 'wb') as f:
			f.write(self.binary)
	def __init__(self, comment=None):
		self.seq(b'JBRQ')
		self.u16(0) # format version
		self.u16(0) # request size

# create request
req = JBReqGen()
req.u8(25, 'IPNGGEN')
req.u8(0, 'ImageId')
req.u16(w, 'Width')
req.u16(h, 'Height')
req.u8(depth, 'Depth')
req.u8(3, 'INDEXED_COLOR')
if transp is not None:
	req.u8(1, 'IDX0TRANSP')
else:
	req.u8(0)
req.u8(len(pal) - 1, 'MaxPaletteEntry')
for i in pal:
	rgb = p[i*3:i*3+3]
	req.seq(rgb)
for y in range(h):
	for x in range(w):
		c = pal[im.getpixel((x, y))]
		req.bits(c, depth)
	req.padbits()
req.write(sys.argv[2])
