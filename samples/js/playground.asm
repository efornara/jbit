; This is the playground: a slot where you can edit your own programs.
;
; You can switch to the read-only demos by clicking on the list on
; the left side of the editor. When you switch back to the playground, you
; will find your program just as you left it.
;
; HOWEVER, when you leave this site your program will be lost.
; If you need to keep a copy of your program, select all the text (Ctrl-A)
; and copy and paste it somewhere.
;
; To run a program without the editor losing focus, press Ctrl-M
; (or Command-M on Macs).
;

	.device "microio"

	lda	#'X'
	ldx	#39
l1:	sta	CONVIDEO,x
	dex
	bpl	l1
