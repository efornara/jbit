; Keep updating a character on the screen (infinite loop). \
; To stop the program press a soft key.
; CPU Instructions: INC (INCrement memory), NOP (No OPeration) \
; and JMP (JuMP).
; IO: Writing into 2:18 suspends the CPU until the screen \
; has been redrawn (refresh rate is 10 frames per second).

	.device "microio"

.code

L1:	INC	2:40
	STA	2:18
	NOP
	JMP	L1
