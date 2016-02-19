; test tty

	.device "xv65"

.define RAW_AND_WAKEUP 35 ; TTY_RAW|TTY_WAKEUP

	lda #>sleep
	sta REQPTRHI
	jsr refresh
wait_key:
	sta FRMDRAW
	lda #<sleep
	sta REQPTRLO
	bit TTYCTL
	bvc clean
	jsr refresh
clean:
	lda KEYBUF
	beq wait_key
	sta KEYBUF
	cmp #12 ; ctrl-l
	bne std_key
	jsr refresh
	jmp clean
std_key:
	sta PUTUINT8
	lda #' '
	sta PUTCHAR
	bne clean

refresh:
	lda #RAW_AND_WAKEUP
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

sleep: .req
	REQ_SLEEP 10
.endreq

msg:
	$1b "[2J" $1b "[;H"
	"Press:\n"
	" - ctrl-c to exit\n"
	" - ctrl-z to suspend the job\n"
	" - ctrl-l to clear the screen\n"
	"\n"
	"Showing key codes...\n"
	0
