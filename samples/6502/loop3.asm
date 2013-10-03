; Bouncing letters (finite loop A-F). \
; More of the same.

	.device "microio"

.code

	ldy	#'A'
l3:	ldx	#0
l1:	lda	#' '
	sta	2:40,x
	inx
	tya
	sta	2:40,x
	sta	2:18
	cpx	#9
	bne	l1
l2:	lda	#' '
	sta	2:40,x
	dex
	tya
	sta	2:40,x
	sta	2:18
	cpx	#0
	bne	l2
	iny
	cpy	#'G'
	bne	l3
	lda	#' '
	sta	2:40,x
	brk
