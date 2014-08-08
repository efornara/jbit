all: d_xv65.h d_io2.h d_microio.h d_primo.h d_jbvm.h

d_xv65.h: xv65.inc
	perl ../tools/inc2sym.pl <xv65.inc >d_xv65.h
	perl ../tools/inc2h.pl <xv65.inc >../native/_xv65.h

d_io2.h: io2.inc
	perl ../tools/inc2sym.pl <io2.inc >d_io2.h

d_microio.h: microio.inc
	perl ../tools/inc2sym.pl <microio.inc >d_microio.h

d_primo.h: primo.inc
	perl ../tools/inc2sym.pl <primo.inc >d_primo.h
	perl ../tools/inc2h.pl <primo.inc >../embd/_primo.h

d_jbvm.h: jbvm.inc
	perl ../tools/inc2sym.pl <jbvm.inc >d_jbvm.h
	perl ../tools/inc2h.pl <jbvm.inc >../embd/_jbvm.h
