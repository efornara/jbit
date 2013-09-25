; Same as fill1, but faster.
; Note that X ranges from 40 to 1 inside the loop \
; (in fill1 the range was 0 to 39).
; CPU Instructions: DEX (DEcrement X register).
; Puzzle: Not Equal in BNE really means "Not Zero" \
; and CPX really means "subtract discarding the result".

	.include "jbit.inc"

.code

	ldx	#40
	lda	#'X'
L1:	sta	CONVIDEO-1,x
	dex
	bne	L1
	brk
