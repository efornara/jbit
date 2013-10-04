; Show the word "CIAO".
; CPU Instructions: LDA (LoaD Accumulator), STA (STore Accumulator) \
; and BRK (BReaK).
; CPU Addressing Modes: Immediate [#n], absolute [n:n] and implied [].
; IO: Video memory is at 40-79 in page 2; 1st row starts at 2:40, \
; 2nd row starts at 2:50 and so on for a 10x4 matrix of Latin1 \
; (extended ASCII) characters.

	.device "microio"

.code

	LDA	#67 ; ASCII CODE: A IS 65, B IS 66, ETC...
	STA	2:53 ; 2ND ROW (2:50), 4TH COLUMN (+3)
	LDA	#73
	STA	2:54 ; 5TH COLUMN (+4)
	LDA	#65
	STA	2:55 ; 6TH COLUMN (+5)
	LDA	#79
	STA	2:56 ; 7TH COLUMN (+6)
	BRK
