; Random numbers (press any key except * to roll two dice, \
; use * to exit).
; CPU: CLC (CLear Carry) and ADC (ADd with Carry).
; IO: Read from 2:23 to get a random number <= maxrand. \
; Write into 2:23 to set maxrand (default 255).


	.device "microio"

.code

	LDA	#5
	STA	2:23
NEXT:	LDA	2:23
	CLC
	ADC	#49 ; 1
	STA	2:53 ; 2ND ROW (2:50), 4TH COLUMN (+3)
	LDA	2:23
	CLC
	ADC	#49 ; 1
	STA	2:56 ; 2ND ROW (2:50), 7TH COLUMN (+6)
WAIT:	STA	2:18
	LDA	2:24
	BEQ	WAIT
	LDX	#1
	STX	2:24
	CMP	#42 ; *
	BNE	NEXT
	BRK
