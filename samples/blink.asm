; blink led attatched to digital pin #13

	.device "primo"

	lda #13
	sta DIGID
	lda #1
	sta DIGCFG

l1:	lda #0
	sta DIGVAL
	jsr pause
	lda #1
	sta DIGVAL
	jsr pause
	jmp l1

pause:
	ldx #0
l2: nop
	dex
	bne l2
