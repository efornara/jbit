; Browse the charset (2=Up, 8=Down and 0=Exit).
; CPU: SEC (SEt Carry), \
; SBC (SuBtract with Carry), BCS (Branch on Carry Set), \
; BCC (Branch on Carry Clear).
; Style: This is "spaghetti" code; note how difficult it is \
; to understand how it works. A few tricks have also been \
; used to keep it in 64 bytes (BCC/BCS instead of JMP).

	.device "microio"

.code

	LDA	#0
SHOW:	TAX
	LDY	#0
L1:	STA	2:40,Y
	CLC
	ADC	#1
	INY
	CPY	#40
	BNE	L1
	TXA
WAIT:	STA	2:18
	LDY	2:24
	BEQ	WAIT
	CPY	#50 ; 2
	BEQ	UP
	CPY	#56 ; 8
	BEQ	DOWN
	CPY	#48 ; 0
	BEQ	QUIT
NEXT:	LDY	#1
	STY	2:24
	BNE	SHOW
UP:	CMP	#0
	BEQ	NEXT
	SEC
	SBC	#10
	BCS	NEXT
DOWN:	CMP	#220
	BEQ	NEXT
	CLC
	ADC	#10
	BCC	NEXT
QUIT:	BRK
