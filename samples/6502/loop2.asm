; Changing letters (infinite loop).
; CPU Instructions: DEC (DECrement memory), LDY, STY and TAX \
; (Transfer/copy Accumulator to X register).
; IO: Write the desired FPS * 4 into 2:17 (e.g. 4 = 1 FPS).

	.device "microio"

.code

	LDA	#4
	STA	2:17
	LDA	#65 ; A
	STA	2:40
	STA	2:18
	LDA	#25
L3:	TAX
L1:	INC	2:40
	STA	2:18
	DEX
	BNE	L1
	TAX
L2:	DEC	2:40
	STA	2:18
	DEX
	BNE	L2
	JMP	L3
