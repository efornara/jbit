; test delay (and trace)

	.device "primo"

	; setup
	lda #>_data_start_
	sta VMTRCHI
	sta REQPTRHI
l1:
	lda #<delay
	sta REQPTRLO
	lda #<msg
	sta VMTRCLO
	jmp l1

	.data
_data_start_:

delay: .req
	; wait 2000 ms
	REQ_DELAY 7:208
.endreq

msg:
	"Tick!" 0
