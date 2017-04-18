# this would do for now

	.device "microio"

.code

	ldx #0
next:
	lda msg,x
	beq exit
	sta CONVIDEO,x
	inx
	bne next
exit:

.data

msg:
	"Load a .jb"
	"file from:"
	"samples/  "
	"6502"
	0
