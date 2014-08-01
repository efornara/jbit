; blink led attatched to digital pin #13

	.device "primo"

	lda #13
	sta DIGID
	lda #DIGCFG_OUTPUT
	sta DIGCFG

l1:	lda #DIGVAL_LOW
	sta DIGVAL
	jsr pause
	lda #DIGVAL_HIGH
	sta DIGVAL
	jsr pause
	jmp l1

pause:
	ldx #0
l2: nop
	dex
	bne l2
	rts
