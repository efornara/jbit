; Random numbers (press any key except * to roll two dice, \
; use * to exit).
; CPU: CLC (CLear Carry) and ADC (ADd with Carry).
; IO: Read from 2:23 to get a random number <= maxrand. \
; Write into 2:23 to set maxrand (default 255).


	.device "microio"

.code

	lda	#5
	sta	2:23
next:	lda	2:23
	clc
	adc	#'1'
	sta	2:53 ; 2nd row (2:50), 4th column (+3)
	lda	2:23
	clc
	adc	#'1'
	sta	2:56 ; 2nd row (2:50), 7th column (+6)
wait:	sta	2:18
	lda	2:24
	beq	wait
	ldx	#1
	stx	2:24
	cmp	#'*'
	bne	next
	brk
