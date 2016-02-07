; test tty

	.device "xv65"

	ldx #0
l1:
	lda msg,x
	beq done
	sta PUTCHAR
	inx
	bne l1
done:
	ldx #' '
	lda	#TTY_RAW
	sta TTYCTL
next:
	sta FRMDRAW
	lda KEYBUF
	beq next
	sta KEYBUF
	sta PUTUINT8
	stx PUTCHAR
	bne next

.data

msg:
	"Showing key codes (press ctrl-c to exit)...\n" 0

