#! /usr/bin/perl

while (<>) {
	chop;
	if (/^(\S*?)\s*?=\s*\$(.*?)$/) { print "\tcase 0x$2: return \"$1\";\n"; }
	elsif (/^(\S*?)\s*?=\s*(.*?)$/) { print "\tcase $2: return \"$1\";\n"; }
}
