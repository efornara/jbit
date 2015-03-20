#! /usr/bin/python2

#
# Copyright (C) 2015  Emanuele Fornara
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

import collections
import os
import sys
import textwrap

PARLEN = 77

Section = collections.namedtuple("Section", "kind content indexed")

class JBDoc:
	basename = ""
	sections = []
	def __init__(self, filename):
		def section(kind, content, indexed):
			self.sections.append(Section(kind, content, indexed))
		def parse_section(s, lineno):
			if s.startswith("# "):
				if s.endswith(" #\n"):
					section("a", s[2:-3], True)
				else:
					section("a", s[2:-1], False)
			elif s.startswith("## "):
				if s.endswith(" ##\n"):
					section("b", s[3:-4], True)
				else:
					section("b", s[3:-1], False)
			elif s.startswith("### "):
				section("i", s[4:-1], False)
			elif s.startswith("    "):
				lines = s.splitlines()
				s = ""
				for l in lines:
					s += l[4:] + "\n"
				section("#", s, False)
			else:
				section("p", s[:-1].replace("\n", " "), False)
		def parse_file(f):
			lineno = 0
			s = ""
			for line in f:
				lineno += 1
				line = line.rstrip()
				if len(line) == 0:
					parse_section(s, lineno)
					s = ""
				else:
					s += line + "\n"
			if len(s) != 0:
				parse_section(s)
		with open(filename, "r") as f:
			parse_file(f)
		self.basename = os.path.basename(filename)

class PageConverter:
	doc = None
	f = None
	def out(self, s):
		self.f.write(s)
	def outln(self, s):
		self.f.write(s)
		self.f.write('\n')
	def convert(self):
		pass

class TextPageConverter(PageConverter):
	def convert(self):
		def ruler(content):
			return '=' * len(content)
		def out_header(kind, content):
			if (kind == 'a'):
				self.outln(ruler(content))
			self.outln(content)
			self.outln(ruler(content))
		def out_section(kind, content):
			if (kind == '#'):
				self.out(content)
			elif (kind == 'a' or kind == 'b'):
				out_header(kind, content)
			elif (kind == 'p'):
				self.outln(textwrap.fill(content, PARLEN))
			elif (kind == 'i'):
				self.out('* ')
				self.outln(content)
			self.outln('')
		for s in self.doc.sections:
			out_section(s.kind, s.content)

def convert(in_file_name, out_file_name):
	converter = TextPageConverter()
	converter.doc = JBDoc(in_file_name)
	converter.f = open(out_file_name, 'w')
	converter.convert()

def usage():
	print "usage: jbdoc.py input output"
	sys.exit(1)

if len(sys.argv) != 3:
	usage()
convert(sys.argv[1], sys.argv[2])
