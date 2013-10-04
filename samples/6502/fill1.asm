; Fill the display with Xs.
; CPU Instructions: LDX (LoaD X register), INX (INcrement X register), \
; CPX (ComPare X register) and BNE (Branch on Not Equal)
; CPU Addressing Modes: Absolute indexed [n:n,X] and relative [r]; \
; relative mode is used in branches and looks like absolute mode \
; in the monitor.
; Style: Even if in JBit the registers are always 0 at the beginning, \
; it is suggested to clear them anyway.

	.device "microio"

.code

	LDX	#0
	LDA	#88
L1:	STA	2:40,X
	INX
	CPX	#40
	BNE	L1
	BRK
