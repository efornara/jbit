; Show the word "CIAO".
; CPU Instructions: LDA (LoaD Accumulator), STA (STore Accumulator) \
; and BRK (BReaK).
; CPU Addressing Modes: Immediate [#n], absolute [n:n] and implied [].
; IO: Video memory is at 40-79 in page 2; 1st row starts at 2:40, \
; 2nd row starts at 2:50 and so on for a 10x4 matrix of Latin1 \
; (extended ASCII) characters.

	.device "microio"

.code

	lda	#67 ; ASCII code: A is 65, B is 66, etc...
	sta	2:53 ; 2nd row (2:50), 4th column (+3)
	lda	#'I' ; you can use this notation
	sta	2:54 ; 5th column (+4)
	lda	#'A'
	sta	2:55 ; 6th column (+5)
	lda	#'O'
	sta	2:56 ; 7th column (+6)
	brk
