# IO Programming (1) #

In this tutorial I assume that you have read the Beginner's Tutorial, spent a
bit of time studying the 6502 examples and written some snippets of code
targeting the MicroIO chip.

## Tiles

The MIDP2 IO chip (called simply IO2 from now on) provides essentially
the same level of control that professional game programmers have when
writing 2D action games. This flexibility comes at a price: complexity.

Most of this complexity is usually at the initial stage of the program.
How many objects are there? What is their shape and color? What about
the playfield? How big is it? Does it scroll horizontally? Vertically?
Not at all? All of these questions must be answered, answered of course
using bytes... You can imagine that it is not going to be easy.

A short sequence of register settings, GAMESET, provides a set of
answers:

	LDA #60
	STA 2:1
	STA 2:2

The answers that GAMESET provides are designed to setup a playfield that
will feel like the MicroIO display from the inside, but that will look
prettier from the outside. Instead of displaying letters, symbols and
numbers, you can display painted tiles chosen from a set of 56 tiles:

	+---------+
	|         |
	|   56    |
	|  tiles  |
	| omitted |
	|         |
	+---------+

Thanks to:

	http://www.famfamfam.com/lab/icons/silk/

Here is the description of the first 7 tiles:

	 1 shape_square
	 2 lightbulb_off
	 3 lightbulb
	 4 lock_open
	 5 lock
	 6 emoticon_unhappy
	 7 emoticon_smile

So, is it just a matter of writing 3 into 2:42 to make a bulb appear?
Unfortunately not. A direct link of cells to tiles is fine for a small
grid, but with IO2 you can define grids of thousands of tiles. Nothing
prevents you from defining a grid of, say, 500x500 tiles (i.e. 25000
tiles) and show only a portion of them.

Having said that, GAMESET actually configures as many tiles as possible
to fit on the display of your phone. For example, on a phone with a
resolution of 176x220 pixels, 11 columns of 13 rows of tiles are
available (each tile as configured by GAMESET is 16x16 pixels big).

So, how can you access a potentially large number of tiles with the IO chip?
One at a time. The IO chip has the notion of the Current Tile. You can use
the following cells:

	Reg    Content
	----   -----------------------------------
	2:87   Column of the Current Tile
	       (initially 0, the leftmost column).
	2:89   Row of the Current Tile
	       (initially 0, the topmost row).
	2:91   The tile itself
	       (initially all 0, empty cells).

After all this theory, let's do some practice. Create a new program and,
starting from 3:0, type (GAMESET):

	LDA #60
	STA 2:1
	STA 2:2

Then (from 3:8):

	LDA #2
	STA 2:87
	LDX #3
	STX 2:91
	INC 2:89
	STX 2:91
	INC 2:89
	STA 2:91

## Animation (1)

Animation is essentially about timing and loops.

Programs so far have been "linear", i.e. after a few instructions, the
program would reach a BRK and terminate. To be able to write animations, we
need to be able to repeat a sequence of instructions over and over again. The
easiest way to do it is to use JMP, for JuMP. It allows us to change the PC
of the CPU, thus changing the address of the next instruction to execute.

With this knowledge, you might think of writing the following program
(starting from 3:8, I assume that you have typed the GAMESET sequence
already):

	INC 2:91
	DEC 2:91
	JMP 3:8

Unfortunatelly, this program does not work. Well, it might work, but it is so
unpredictable and inefficient that you really do not want to run it.

It is unpredictable, because JBit tries to run the program as fast as it can,
and the actual speed varies a lot from phone to phone. My tests and the
feedback that I have received suggest that JBit is able to simulate from
around 10,000 instructions per second on slow phones to around 500.000
instructions per second on fast ones.

It is inefficient, because it would try to change the tile too often. Even on
a slow phone, the tile would change thousands of times per second.

Actually, all this work would be wasted, because IO2 would not even
bother to keep up. IO2 would update the tile eventually, but you have
no idea how often. It might even be the case that IO2 would use the
same value more than once. What you would see is effectively a sampling of a
very fast variable. The odds are not even 50%, as the value 0 is more likely.

There is a better way. When you are finished arranging the tiles, you can
tell IO2 that now is the time to update the display. You do so by
writing any value into 2:18. This has the nice side effect that JBit will
suspend its CPU for a while and tell your phone that JBit is idle. Some
phones might not care, but some might be able to switch their CPU into
low-power mode in turn, draining less power from the battery.

So, this is a predictable and efficient version of the program above:

	INC 2:91
	STA 2:18
	DEC 2:91
	STA 2:18
	JMP 3:8

## Animation (2)

Usually, we don't want to repeat a sequence of instructions forever, but just
for a little while. We can do this by using BNE, for Branch if Not Equal.
Branch instructions change the PC only if a specific condition is met. In the
case of BNE, Not Equal is a bit misleading, as the condition really is Not
Zero and it refers to the result of the instruction before the branch.

For example, the following sequence:

	...
	LDA #3
	BNE 3:20
	...

Will continue to 3:20, while the following:

	...
	LDA #0
	BNE 3:20
	...

Will continue to the next instruction after BNE.

There is a slight complication here. If you try to type in BNE, you will
notice that the format is BNE r, not BNE n:n as you might expect.

The reason is that branches usually point to an instruction nearby and two
bytes would often be wasted. The single byte operand is used as the number of
bytes to skip forward (if between 0 and 127) or backward (if between 128 and
255, where 255 means 1, 254 means 2, etc...). Fortunately, there is no need
for you to count them.

We will use the following code as an example:

	LDX #30
	STA 2:18    [1]
	DEX
	BNE <<1>>

We want the BNE instruction to point to STA 2:18, thus writing into 2:18
thirty times and causing the VM to run for about 3 seconds (plus the time
spent for initialization).

Type in the code starting from 3:0, leaving the operand of BNE to 0.
Then switch to NAV ASM mode. The listing should look like this:

	LDX #30
	STA 2:18
	DEX
	BNE !!!!

The BNE is pointless as it is (no matter the result of DEX, no branching
takes place) and !!!! is a reminder that you probably meant to change it
later on.

Here are the steps:

1. Make sure that the cursor is on BNE !!!!

2. Press 7. This sets an invisible marker on this address (3:6).

3. Move the cursor to STA 2:18 (i.e., 3:2)

4. Press 0. This swaps the cursor and the marker. The cursor is now on the
BNE !!!! instruction and the invisible marker is now on the STA instruction.

Select PutMark from the menu.

The listing should now look like this:

	LDX #30
	STA 2:18
	DEX
	BNE 3:2

With the cursor still on the BNE instruction, press 9. This moves the cursor
to the address pointed by the operand and can be handy to check if the
operands are correct.

If you run the program, it should last for a bit more than 3 seconds and then
terminate.

As an exercise, type in the GAMESET sequence, followed by:

	LDY #10
	LDX #8      [1]
	LDA #2
	STA 2:91
	STA 2:18    [2]
	DEX
	BNE <<2>>
	LDX #2
	LDA #3
	STA 2:91
	STA 2:18    [3]
	DEX
	BNE <<3>>
	DEY
	BNE <<1>>

## Key presses (OBSOLETE)

When you are confident in writing animations, you can start making programs
that react to the user (i.e. interactive programs).

Empty includes a handy subroutine. It takes care of updating the display and
saving in the Accumulator the code of the key that the user has pressed.

What is a subroutine? It is a piece of code that can be used again and again.
You don't even have to understand how the piece of code works to be able to
use. You "call" the subroutine (using JSR), and, when the subroutine has
finished to do its job, the control returns to your code.

You can call the subroutine above with:

	JSR 3:3

You can then check if a key has been pressed (BNE) or not (BEQ). For example,
to wait for the user to press two keys, you could write something like this:

	JSR 3:3     [1]
	BEQ <<1>>
	JSR 3:3     [2]
	BEQ <<2>>

If the user has pressed a key, you can inspect the Accumulator to find out
which key it was. The key codes are the same as the ones you use to write
into 2:40 - 2:79. As a reminder, on a standard keypads they are:

	key|value
	---+-----
	 # | 35
	 * | 42
	 0 | 48
	 1 | 49
	 . . ..
	 . . ..
	 9 | 57

On a QWERTY, you might receive other codes too (e.g. 65 or 97 if the user
presses the 'A' key).

Here is a more complex example. Press '1' or '3' to change a tile on the
screen, and '*' to terminate the program.

	JSR 3:3     [1]
	BEQ <<1>>
	CMP #49
	BNE <<2>>
	DEC 2:91
	JMP <<1>>
	CMP #51     [2]
	BNE <<3>>
	INC 2:91
	JMP <<1>>
	CMP #42     [3]
	BNE <<1>>

If you have problem understanding it at first, you can set a break point on
the instruction CMP #49 (i.e. from within the editor, go to 4:5 and select
SetBrkPt before running the program). Once you press a key, the program is
stopped and you can start pressing '1' to see what happens. If you are lost,
you can press '*' to resume the program and try again with another key.

