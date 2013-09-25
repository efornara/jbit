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

	ldx	#0
	lda	#'X'
L1:	sta	CONVIDEO,x
	inx
	cpx	#40
	bne	L1
	brk
