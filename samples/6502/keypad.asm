; Firing characters (press any key except * to fire a character, \
; use * to stop firing).
; CPU: BEQ (Banch if EQual) and CMP (CoMPare accumulator).
; IO: Read from 2:24; if the value is 0 no key has been pressed; \
; a value different than 0 is the ASCII code of the key \
; that has been pressed (e.g. '0' is 48). \
; Write 1 into 2:24 to acknowledge the key and remove it from 2:24. \
; At most 8 unacknowledged keys are kept, the others are lost.


	.device "microio"

.code

start:	lda	#0
next:	sta	2:40
	lda	2:24
	beq	next
	cmp	#'*'
	beq	quit
	ldx	#0
l1:	sta	2:40,x
	tay
	lda	#0
	sta	2:18
	lda	#' '
	sta	2:40,x
	tya
	inx
	cpx	#10
	bne	l1
	lda	#1
	sta	2:24
	jmp	start
quit:	brk
