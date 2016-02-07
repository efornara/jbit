; Firing characters (press any key except * to fire a character, \
; use * to stop firing).
; CPU: BEQ (Banch if EQual) and CMP (CoMPare accumulator).
; IO: Read from 2:24; if the value is 0 no key has been pressed; \
; a value different than 0 is the ASCII code of the key \
; that has been pressed (e.g. '0' is 48). \
; Write 1 into 2:24 to acknowledge the key and remove it from 2:24. \
; At most 8 unacknowledged keys are kept, the others are lost.


	.device "microio"

.code

START:	LDA	#0
NEXT:	STA	2:18
	LDA	2:24
	BEQ	NEXT
	CMP	#42 ; *
	BEQ	QUIT
	LDX	#0
L1:	STA	2:40,X
	TAY
	LDA	#0
	STA	2:18
	LDA	#32 ; SPACE
	STA	2:40,X
	TYA
	INX
	CPX	#10
	BNE	L1
	LDA	#1
	STA	2:24
	JMP	START
QUIT:	BRK
