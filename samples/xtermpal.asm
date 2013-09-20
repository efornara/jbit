; print 256 color palette (xterm only)

	.device "xv65"

.define STRPTRLO 4
.define STRPTRHI 5

setup_pointers:
	lda	#>_data_start_
	sta	REQPTRHI
	sta	STRPTRHI

check_env:
	lda	#<env
	sta	REQPTRLO
	ldy	#5
l1:	lda	buf,y
	cmp	xterm,y
	bne	print_error
	dey
	bpl	l1

print_palette:
	ldx	#0
l2:	lda	#<prefix
	sta	STRPTRLO
	jsr	put_string
	stx	PUTUINT8
	lda	#<postfix
	sta	STRPTRLO
	jsr	put_string
	txa
	and	#$0f
	cmp	#$0f
	bne	j1
	lda	#10
	sta	PUTCHAR
j1:	inx
	bne	l2
	brk

print_error:
	lda	#<error_msg
	sta	STRPTRLO
	jsr	put_string
	brk

put_string:
	ldy	#0
l3:	lda	(STRPTRLO),y
	beq	r1
	sta	PUTCHAR
	iny
	bne	l3
r1:	rts

.data
_data_start_:

env: .req
	REQ_ENV
	buf
	0:6
	"TERM" 0
.endreq

error_msg:
	"xterm not detected.\n" 0

xterm:
	"xterm" 0

prefix:
	"\e[48;5;" 0

postfix:
	"m  \e[0m" 0

buf:
