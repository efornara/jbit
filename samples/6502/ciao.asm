; Show the word "CIAO".
; CPU Instructions: LDA (LoaD Accumulator), STA (STore Accumulator) \
; and BRK (BReaK).
; CPU Addressing Modes: Immediate [#n], absolute [n:n] and implied [].
; IO: Video memory is at 40-79 in page 2; 1st row starts at 2:40, \
; 2nd row starts at 2:50 and so on for a 10x4 matrix of Latin1 \
; (extended ASCII) characters.

	.include "jbit.inc"

START = CONVIDEO + 10*1 + 3

.code

	lda	#'C'
	sta	START
	lda	#'I'
	sta	START+1
	lda	#'A'
	sta	START+2
	lda	#'O'
	sta	START+3
	brk
