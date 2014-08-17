; blink led attatched to digital pin #13

	.device "primo"

	lda #13
	sta DIGID
	lda #DIGWCFG_OUTPUT
	sta DIGWCFG

l1:	lda #DIGVAL_LOW
	sta DIGVAL
	jsr pause
	lda #DIGVAL_HIGH
	sta DIGVAL
	jsr pause
	jmp l1

pause:
	lda #REQ_DELAY
	sta REQPUT
	lda #232
	sta REQPUT
	lda #3 ; 3*256+232 = 1000ms
	sta REQPUT
	sta REQEND
	rts
