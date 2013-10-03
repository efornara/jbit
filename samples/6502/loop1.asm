; Keep updating a character on the screen (infinite loop). \
; To stop the program press a soft key.
; CPU Instructions: INC (INCrement memory), NOP (No OPeration) \
; and JMP (JuMP).
; IO: Writing into 2:18 suspends the CPU until the screen \
; has been redrawn (refresh rate is 10 frames per second).

	.device "microio"

.code

l1:	inc	2:40
	sta	2:18
	nop
	jmp	l1
