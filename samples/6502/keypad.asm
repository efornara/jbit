; Firing characters (press any key except * to fire a character, \
; use * to stop firing).
; CPU: BEQ (Banch if EQual) and CMP (CoMPare accumulator).
; IO: Read from 2:24; if the value is 0 no key has been pressed; \
; a value different than 0 is the ASCII code of the key \
; that has been pressed (e.g. '0' is 48). \
; Write 1 into 2:24 to acknowledge the key and remove it from 2:24. \
; At most 8 unacknowledged keys are kept, the others are lost.


	.include "jbit.inc"

.code

start:	lda	#0
next:	sta	FRMDRAW
	lda	KEYBUF
	beq	next
	cmp	#'*'
	beq	quit
	ldx	#0
L1:	sta	CONVIDEO,x
	tay
	lda	#0
	sta	FRMDRAW
	lda	#' '
	sta	CONVIDEO,x
	tya
	inx
	cpx	#10
	bne	L1
	lda	#1
	sta	KEYBUF
	jmp	start
quit:	brk
