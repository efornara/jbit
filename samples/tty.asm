; test tty

	.device "xv65"

	ldy #' '
	jsr refresh
next:
	sta FRMDRAW
	bit TTYCTL
	bvc clean
	jsr refresh
clean:
	lda KEYBUF
	beq next
	sta KEYBUF
	cmp #12 ; ctrl-l
	bne std_key
	jsr refresh
	jmp next
std_key:
	sta PUTUINT8
	sty PUTCHAR
	bne next

refresh:
	lda #TTY_RAW
	sta TTYCTL ; on next calls, also clear ISDIRTY
	ldx #0
l1:
	lda msg,x
	beq done
	sta PUTCHAR
	inx
	bne l1
done:
	rts

.data

msg:
	$1b "[2J" $1b "[;H"
	"Press:\n"
	" - ctrl-c to exit\n"
	" - ctrl-z to suspend the job\n"
	" - ctrl-l to clear the screen\n"
	"\n"
	"Showing key codes...\n"
	0
