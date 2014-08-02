; analog pin A0 to pwm pin 11

	.device "primo"

	lda #11
	sta DIGID
	lda #DIGWCFG_OUTPUT
	sta DIGWCFG

l1:	lda #0
	sta ANLGHI
	lda ANLGHI
	sta DIGPWM
	jmp l1
