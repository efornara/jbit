# this would do for now

	.device "microio"

.define STEP 1

.define MSGLO 2
.define MSGHI 3

.code

	lda #0
	sta STEP
next_step:
	jsr goto_step
	jsr handle_nav
	jmp next_step

goto_step:
	lda STEP
	asl
	tax
	lda steps,x
	sta jmp_op1
	inx
	lda steps,x
	sta jmp_op2
	bne jump
	jmp exit
jump:
	76
jmp_op1:
	0
jmp_op2:
	0

handle_nav:
	jsr wait_key
	cmp #'6'
	beq handle_next
	cmp #'4'
	beq handle_prev
	cmp #'#'
	bne handle_nav
	jmp exit
handle_next:
	inc STEP
	rts
handle_prev:
	lda STEP
	beq handle_nav
	dec STEP
	rts

wait_key:
	sta FRMDRAW
	lda KEYBUF
	beq wait_key
	sta KEYBUF
	rts

clear_screen:
	lda #' '
	ldy #40
next2:
	sta 2:39,y
	dey
	bne next2
	rts

exit:
	jsr clear_screen
	brk

step1:
	lda #<step1_msg
	sta MSGLO
	lda #>step1_msg
	sta MSGHI
	jmp print_msg

step2:
	lda #<step2_msg
	sta MSGLO
	lda #>step2_msg
	sta MSGHI
	jmp print_msg

print_msg:
	jsr clear_screen
next:
	lda (MSGLO),y
	beq done
	sta CONVIDEO,y
	iny
	bne next
done:
	rts

.data

steps:
	step1
	step2
	0:0

step1_msg:
	"6: Next   "
	"4: Prev   "
	"#: Exit"
	0

step2_msg:
	"Nothing to"
	"see here. "
	"Come back "
	"later..."
	0
