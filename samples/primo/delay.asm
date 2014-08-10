; test delay (and trace)

	.device "primo"

	; setup trace
	lda #>msg
	sta VMTRCHI
l1:
	; wait 2000 ms
	lda #REQ_DELAY
	sta REQPUT
	lda #208
	sta REQPUT
	lda #7
	sta REQPUT
	sta REQEND
	; trace
	lda #<msg
	sta VMTRCLO
	jmp l1

	.data

msg:
	"CIAO" 0
