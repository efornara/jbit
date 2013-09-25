; Browse the charset (2=Up, 8=Down and 0=Exit).
; CPU: SEC (SEt Carry), \
; SBC (SuBtract with Carry), BCS (Branch on Carry Set), \
; BCC (Branch on Carry Clear).
; Style: This is "spaghetti" code; note how difficult it is \
; to understand how it works. A few tricks have also been \
; used to keep it in 64 bytes (BCC/BCS instead of JMP).

	.device "microio"

.code

	lda	#0
show:	tax
	ldy	#0
L1:	sta	CONVIDEO,y
	clc
	adc	#1
	iny
	cpy	#40
	bne	L1
	txa
wait:	sta	FRMDRAW
	ldy	KEYBUF
	beq	wait
	cpy	#'2'
	beq	up
	cpy	#'8'
	beq	down
	cpy	#'0'
	beq	quit
next:	ldy	#1
	sty	KEYBUF
	bne	show
up:	cmp	#0
	beq	next
	sec
	sbc	#10
	bcs	next
down:	cmp	#220
	beq	next
	clc
	adc	#10
	bcc	next
quit:	brk
