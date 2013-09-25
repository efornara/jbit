; Show the word "CIAO".
; CPU Instructions: LDA (LoaD Accumulator), STA (STore Accumulator) \
; and BRK (BReaK).
; CPU Addressing Modes: Immediate [#n], absolute [n:n] and implied [].
; IO: Video memory is at 40-79 in page 2; 1st row starts at 2:40, \
; 2nd row starts at 2:50 and so on for a 10x4 matrix of Latin1 \
; (extended ASCII) characters.

	.device "microio"

.define START0 2:53 ; CONVIDEO + 10*1 + 3
.define START1 2:54
.define START2 2:55
.define START3 2:56

.code

	lda	#'C'
	sta	START0
	lda	#'I'
	sta	START1
	lda	#'A'
	sta	START2
	lda	#'O'
	sta	START3
	brk
