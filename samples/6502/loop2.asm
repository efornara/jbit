; Changing letters (infinite loop).
; CPU Instructions: DEC (DECrement memory), LDY, STY and TAX \
; (Transfer/copy Accumulator to X register).
; IO: Write the desired FPS * 4 into 2:17 (e.g. 4 = 1 FPS).

	.include "jbit.inc"

.code

	lda	#4
	sta	FRMFPS
	lda	#'A'
	sta	CONVIDEO
	sta	FRMDRAW
	lda	#25
L3:	tax
L1:	inc	CONVIDEO
	sta	FRMDRAW
	dex
	bne	L1
	tax
L2:	dec	CONVIDEO
	sta	FRMDRAW
	dex
	bne	L2
	jmp	L3
