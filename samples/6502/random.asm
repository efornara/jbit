; Random numbers (press any key except * to roll two dice, \
; use * to exit).
; CPU: CLC (CLear Carry) and ADC (ADd with Carry).
; IO: Read from 2:23 to get a random number <= maxrand. \
; Write into 2:23 to set maxrand (default 255).


	.device "microio"

.define DIE1POS 2:53 ; CONROW1 + 0 + 3
.define DIE2POS 2:56 ; CONROW1 + 9 - 3

.code

	lda	#5
	sta	RANDOM
next:	lda	RANDOM
	clc
	adc	#'1'
	sta	DIE1POS
	lda	RANDOM
	clc
	adc	#'1'
	sta	DIE2POS
wait:	sta	FRMDRAW
	lda	KEYBUF
	beq	wait
	ldx	#1
	stx	KEYBUF
	cmp	#'*'
	bne	next
	brk