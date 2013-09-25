; Bouncing letters (finite loop A-F). \
; More of the same.

	.device "microio"

.code

	ldy	#'A'
L3:	ldx	#0
L1:	lda	#' '
	sta	CONVIDEO,x
	inx
	tya
	sta	CONVIDEO,x
	sta	FRMDRAW
	cpx	#9
	bne	L1
L2:	lda	#' '
	sta	CONVIDEO,x
	dex
	tya
	sta	CONVIDEO,x
	sta	FRMDRAW
	cpx	#0
	bne	L2
	iny
	cpy	#'G'
	bne	L3
	lda	#' '
	sta	CONVIDEO,x
	brk
