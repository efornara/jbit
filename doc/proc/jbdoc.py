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
import re

PARLEN = 77

fallback_resources = {}

fallback_resources['html_header.txt'] = """
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en">
<head>
<title>JBDoc - __TITLE__</title>
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1" />
<meta name="viewport" content="width=device-width" />
<meta name="HandheldFriendly" content="true" />
<style type="text/css">
body { font-family: sans-serif; }
.nav { font-size: 70%; }
#links a { margin: 0em 0.5em; }
</style>
</head>
<body>
<p align="right"><a class="nav" href="#content">skip nav</a></p>
"""

fallback_resources['html_footer.txt'] = """
<p><a class="nav" href="#top">top</a></p>
</body>
</html>
"""

def resource(res):
	return fallback_resources[res][1:]

def template_resource(res, subs):
	s = resource(res)
	for k, v in subs.iteritems():
		s = s.replace('__' + k + '__', v)
	return s

Section = collections.namedtuple('Section', 'kind content indexed')

class JBDoc:
	basename = ''
	title = 'Untitled'
	sections = []
	def __init__(self, filename):
		def section(kind, content, indexed):
			self.sections.append(Section(kind, content, indexed))
		def parse_section(s, lineno):
			if s.startswith('# '):
				if s.endswith(' #\n'):
					section('a', s[2:-3], True)
				else:
					section('a', s[2:-1], False)
			elif s.startswith('## '):
				if s.endswith(' ##\n'):
					section('b', s[3:-4], True)
				else:
					section('b', s[3:-1], False)
			elif s.startswith('### '):
				section('i', s[4:-1], False)
			elif s.startswith('\t'):
				lines = s.splitlines()
				s = ''
				for l in lines:
					s += l[1:] + '\n'
				section('#', s, False)
			else:
				section('p', s[:-1].replace('\n', ' '), False)
		def parse_file(f):
			lineno = 0
			s = ''
			for line in f:
				lineno += 1
				line = line.rstrip()
				if len(line) == 0:
					parse_section(s, lineno)
					s = ''
				else:
					s += line + '\n'
			if len(s) != 0:
				parse_section(s)
		with open(filename, 'r') as f:
			parse_file(f)
		if len(self.sections) > 0:
			s = self.sections[0]
			if s.kind == 'a':
				self.title = s.content
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
			if kind == 'a':
				self.outln(ruler(content))
			self.outln(content)
			self.outln(ruler(content))
		def out_section(kind, content):
			if kind == '#':
				self.out(content)
			elif kind == 'a' or kind == 'b':
				out_header(kind, content)
			elif kind == 'p':
				self.outln(textwrap.fill(content, PARLEN))
			elif kind == 'i':
				self.out('* ')
				self.outln(content)
			self.outln('')
		for s in self.doc.sections:
			out_section(s.kind, s.content)

class DATPageConverter(PageConverter):
	def convert(self):
		def out_section(kind, content, indexed):
			if indexed:
				self.out(kind.upper())
			else:
				self.out(kind)
			self.out(content)
			self.out(chr(0))
		for s in self.doc.sections:
			out_section(s.kind, s.content, s.indexed)

class HTMLPageConverter(PageConverter):
	def convert(self):
		def escape(s):
			s = re.sub('&', '&amp;', s)
			s = re.sub('<', '&lt;', s)
			s = re.sub('>', '&gt;', s)
			return s
		def out_toc(kind, content, indexed):
			if not indexed:
				return
			self.out('<a class="nav" href="#id' + str(id[0]) + '">')
			self.out(escape(content))
			self.outln('</a><br />')
			id[0] += 1
		def out_header(kind, content, indexed):
			c = escape(content)
			if kind == 'a':
				el = 'h1'
			else:
				el = 'h2'
			attr = ''
			if indexed:
				if id[0] != 1:
					self.outln('<p><a class="nav" href="#top">top</a></p>')
				self.outln('<hr />')
				attr = ' id="id' + str(id[0]) + '"'
				id[0] += 1
			self.outln('<' + el + attr + '>' + c + '</' + el + '>');
		def out_section(kind, content, indexed):
			if kind == '#':
				self.outln('<pre>')
				self.out(escape(content))
				self.outln('</pre>')
			elif kind == 'a' or kind == 'b':
				out_header(type, content, indexed)
			elif kind == 'p':
				self.outln('<p>')
				self.outln(escape(textwrap.fill(content, PARLEN)))
				self.outln('</p>')
			elif kind == 'i':
				self.out('<h3>')
				self.out(escape(content))
				self.outln('</h3>')
		self.out(template_resource('html_header.txt',
			{ 'TITLE': escape(self.doc.title) }))
		self.outln('<p id="top">')
		id = [1]
		for s in self.doc.sections:
			out_toc(s.kind, s.content, s.indexed)
		self.outln('</p>');
		self.outln('<p id="content">')
		id = [1]
		for s in self.doc.sections:
			out_section(s.kind, s.content, s.indexed)
		self.out(resource('html_footer.txt'))

def convert(in_file_name, fmt, out_file_name):
	if not fmt:
		if out_file_name.endswith('.txt'):
			fmt = 'txt'
		elif out_file_name.endswith('.dat'):
			fmt = 'dat'
		elif out_file_name.endswith('.html') or out_file_name.endswith('.htm'):
			fmt = 'xhtml1'
		else:
			raise StandardError('unrecognized output extension')
	if fmt == 'txt':
		converter = TextPageConverter()
	elif fmt == 'dat':
		converter = DATPageConverter()
	elif fmt == 'xhtml1':
		converter = HTMLPageConverter()
	else:
		raise StandardError('unrecognized format')
	converter.doc = JBDoc(in_file_name)
	converter.f = open(out_file_name, 'w')
	converter.convert()

def usage():
	print """usage: jbdoc.py input [-f fmt] output

where fmt is:
  txt          preformatted ascii text file
  dat          binary format for the JBDoc midlet
  xhtml1       self-contained xhtml 1.0 file (no external resources)

"""
	sys.exit(1)

if len(sys.argv) == 3:
	IN = sys.argv[1]
	FMT = None
	OUT = sys.argv[2]
elif len(sys.argv) == 5 and sys.argv[2] == '-f':
	IN = sys.argv[1]
	FMT = sys.argv[3]
	OUT = sys.argv[4]
else:
	usage()
convert(IN, FMT, OUT)
