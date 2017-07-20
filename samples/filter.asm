; Copy stdin to stdout, replacing each digit with a random one.

	.device "std"

	lda	#9
	sta	RANDOM
next:
	sta	GETCHAR
	lda	GETEOF
	bne	exit
	lda	GETCHAR
	cmp	#'0'
	bmi	output
	cmp	#':'
	bpl	output
	lda	RANDOM
	clc
	adc	#'0'
output:
	sta	PUTCHAR
	jmp	next
exit:
	brk
