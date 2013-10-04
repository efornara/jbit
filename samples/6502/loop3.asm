; Bouncing letters (finite loop A-F). \
; More of the same.

	.device "microio"

.code

	LDY	#65 ; A
L3:	LDX	#0
L1:	LDA	#32 ; SPACE
	STA	2:40,X
	INX
	TYA
	STA	2:40,X
	STA	2:18
	CPX	#9
	BNE	L1
L2:	LDA	#32 ; SPACE
	STA	2:40,X
	DEX
	TYA
	STA	2:40,X
	STA	2:18
	CPX	#0
	BNE	L2
	INY
	CPY	#71 ; G (LETTER AFTER F)
	BNE	L3
	LDA	#32 ; SPACE
	STA	2:40,X
	BRK
