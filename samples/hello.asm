; Hello World in jbit

	.device "stdout"

	ldx	#0
l1:	lda	msg,X
	beq	exit
	sta	PUTCHAR
	inx
	bne	l1
exit:	brk

.data

msg: 
	"Hello, World!\n" 0
