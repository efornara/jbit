; Changing letters (infinite loop).
; CPU Instructions: DEC (DECrement memory), LDY, STY and TAX \
; (Transfer/copy Accumulator to X register).
; IO: Write the desired FPS * 4 into 2:17 (e.g. 4 = 1 FPS).

	.device "microio"

.code

	lda	#4
	sta	2:17
	lda	#'A'
	sta	2:40
	sta	2:18
	lda	#25
l3:	tax
l1:	inc	2:40
	sta	2:18
	dex
	bne	l1
	tax
l2:	dec	2:40
	sta	2:18
	dex
	bne	l2
	jmp	l3
