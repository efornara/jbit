# Beginner's Tutorial

## Introduction ##

JBit is a complete programming environment and is more complex than a typical
mobile phone application. This tutorial will help you to get started.

The first thing to understand is that, when you are writing a program using
JBit, you are not writing a program for your mobile phone, but for the JBit
Virtual Machine (MicroIO Edition), or VM.

Here there is a picture of the VM:

	+--------------+
	|  ..........  |
	|  ..........  |
	|  ..........  |
	|  ..........  |
	|              |
	|   1  2  3    |
	|   4  5  6    |
	|   7  8  9    |
	|   *  0  #    |
	|              |
	+--------------+

As you can see, IT looks like an ugly mobile phone. Now, the VM might be
ugly, but it is definitely not a mobile phone. For a start, you cannot make
or receive calls, nor send or receive text messages with it. Furthermore, the
display can only show a matrix of 10x4 characters and the keypad has only the
standard keys (i.e. 0-9, * and #).

Since the VM cannot function as a phone, what can it be useful for? Well, the
VM is pretty much useless. However, learning how to program it can help you
to understand how computers work. Computers are complex devices and their
inner core is hidden by several layers of abstraction, but they do have a
core and their core is not much more complex than the VM.

Another thing to understand is the difference between the VM and JBit. JBit
includes the VM, but it is not only that. It also includes tools to help you
to edit and manage programs for the VM.

Actually, you cannot reach the VM until you make a program for it. The VM
itself is empty and has no such a thing such a Desktop that you can play
with. Therefore, our first task will be to create an empty program, so that
we can have a look at the VM.

Here is how JBit looks like when you start it (you might have more options in
your version of JBit; just ignore them for now):

	+-JBit---------+
	| Store        |
	|>Editor       |
	| About        |
	|              |
	|Select    Exit|
	+--------------+

Select Editor from the list.

The Editor needs to know how big the program is. At this stage we do not
really care and so we can just confirm 4 Code Pages and 3 Data Pages and
select OK.

	+-Size (New)---+
	|Code Pages    |
	|[4]           |
	|Data Pages    |
	|[3]           |
	|Options Cancel|
	+--------------+

On some phones, OK is directly mapped to a soft key, while on other phones,
OK is available using a menu (e.g. Options or Menu). If you do not see the
Size (New) screen, it probably means that you already have one program loaded
and should restart JBit.

You should now see a matrix of zeros:

	+--------------+
	|C 3:0  NAV MEM|
	|  <0>  0   0  |
	|   0   0   0  |
	|   0   0   0  |
	|Options   Back|
	+--------------+

This is the program. We will come back to it later. For now, we only want to
quickly get to the VM.

Select Debug.

## Inside the VM

Here is the VM:

	+--------------+
	|PC 3:0 R   CPU|
	| BRK          |
	| A 0 X 0 Y 0  |
	| --------     |
	|Options   Edit|
	+--------------+

It does not look anything like the picture above. This is because you are
actually looking inside the VM!

Press # to switch to the outside view.

	+--------------+
	|              |
	|              |
	|              |
	|              |
	|              |
	+--------------+

Now you are looking at the display of the VM. There is no point in showing
you the keypad of the VM, since the keypad of your phone can act as the
keypad of the VM.

There is a minor complication here. If we started the VM properly, the VM
would have halted immediately, because the program is empty. Instead, we
started the VM in a frozen mode that allows us to inspect it. In this mode of
operation, it is far more common to look at the display than to use the
keypad, so, right now, the keypad of your phone is not acting as the keypad
of the VM. Instead, the keypad of your phone is used to switch back to the
inside view.

Press # again to switch back to the inside view.

	+--------------+
	|PC 3:0 R   CPU|
	| BRK          |
	| A 0 X 0 Y 0  |
	| --------     |
	|Options   Edit|
	+--------------+

Let us have a look at what is inside the VM.

You have probably already heard of the term CPU (Central Process Unit), as it
is one of the criteria of selection when shopping for a computer. The CPU
used by the VM is essentially a 6502, a CPU popular during the 70s and the
80s. It is smaller and slower, but, from a theoretical point of view, no less
powerful than a modern CPU.

Gaining an understanding of the CPU is the whole point of JBit and will take
a while. We can start with this: the CPU is an agent acting on a virtual
world on our behalf. Before letting the CPU act on our behalf, we will play a
bit with this virtual world ourselves.

## Memory

Press 0 to switch to the MEMory view.

	+--------------+
	|3:0        MEM|
	|   0   0   0  |
	|   0   0   0  |
	|  <0>  0   0  |
	|Options   Edit|
	+--------------+

Unlike in the real world, where we are free to place objects as we like, in
this virtual world, objects are placed in cells and cells are disposed in a
single row.

Also, unlike in the real world, where there is a variety of objects, in this
virtual world, there is only one kind of object: the byte, a small integer
ranging from 0 to 255.

Finally, unlike the real world, this virtual world is finite. There are
exactly 65536 cells. To better manage this long sequence of cells , we
partition it in 256 segments (called pages) of 256 cells each. This has the
nice side effect of allowing us to refer to the location of a cell using two
bytes: the number of the page (using 0 for the first page and 255 for the
last) and the position (or offset) within that page (using 0 for the first
position and 255 for the last). This pair of bytes is called the address of a
cell and is written as two numbers separated by a colon. For example, the
first cell is at address 0:0, the last one is at address 255:255 and the
260th one is at address 1:3 (i.e. the 4th cell of the 2nd page). We will
often just write the cell Page:Offset to refer to the cell at address
Page:Offset.

The MEMory view is a view on this virtual world. To make the most of the
small display of a typical mobile phone, cells are shown on a matrix, but you
should always keep in mind that they are in fact disposed in a single row.

The current cell (i.e. the cell we are currently inspecting) is marked by the
cursor. The cursor can be moved by using 4 (or left) to go to the previous
cell and 6 (or right) to go to the next one. You can also use 2 (or up) and 8
(or down).

Reach the cell 3:25 and then select Edit.

	+-Edit---------+
	|Value         |
	|[...]         |
	|Offset        |
	|[25]          |
	|Options Cancel|
	+--------------+

This screen allows you to change the content of a cell.

Input 56 into the Value field and then select OK.

	+--------------+
	|3:25       MEM|
	|   0 <56>  0  |
	|   0   0   0  |
	|   0   0   0  |
	|Options   Edit|
	+--------------+

The content of the cell has changed. Move the cursor further down until the
cell 3:25 is not visible anymore (e.g. until you reach 3:50) and then back
until the cell 3:25 is visible again. You can see that the cell has kept the
value you have put into it. The cell 3:25, like most of the cells, is a
memory cell, that is, a cell that just keeps the value that is put into it.

## IO chip

Select GoTo.

	+-GoTo---------+
	|Page          |
	|[...]         |
	|Offset        |
	|[...]         |
	|Options Cancel|
	+--------------+

This screen allows you to quickly move the cursor to a specific cell.

Input 2 into the Page field, input 40 into the Offset field and then select
OK.

	+--------------+
	|2:40       MEM|
	| <32> 32  32  |
	|  32  32  32  |
	|  32  32  32  |
	|Options   Edit|
	+--------------+

Cells on page 2 are not memory cells. Instead, they are links to a component
called the IO chip. In particular, cells from 2:40 to 2:79 are connected to
the display, in this way:

	   0 1 2 3 4 5 6 7 8 9
	40 . . . . . . . . . .
	50 . . . . . . . . . .
	60 . . . . . . . . . .
	70 . . . . . . . . . .

Putting a byte from 32 to 126 into one of the cells above will cause a
character to appear on the display, according to this table:

	    0 1 2 3 4 5 6 7 8 9
	 30       ! " # $ % & '
	 40 ( ) * + , - . / 0 1
	 50 2 3 4 5 6 7 8 9 : ;
	 60 < = > ? @ A B C D E
	 70 F G H I J K L M N O
	 80 P Q R S T U V W X Y
	 90 Z [ \ ] ^ _ ` a b c
	100 d e f g h i j k l m
	110 n o p q r s t u v w
	120 x y z { | } ~

Select Edit, input 88 into the Value field and then select OK.

	+--------------+
	|2:40       MEM|
	| <88> 32  32  |
	|  32  32  32  |
	|  32  32  32  |
	|Options   Edit|
	+--------------+

Press # to switch to the outside view.

	+--------------+
	| X            |
	|              |
	|              |
	|              |
	|              |
	+--------------+

You can see that the character X is visible on the top left corner of the
display. On phones with large displays, the character will likely be more
toward the center, since the display of the VM is far smaller than the
display of the phone.

Now that we have a better understanding of what "acting on a virtual world"
means, we are ready to take another look at the CPU.

## CPU

Press # again to switch back to the inside view and then 0 to switch to the
CPU view.

	+--------------+
	|PC 3:0 R   CPU|
	| BRK          |
	| A 0 X 0 Y 0  |
	| --------     |
	|Options   Edit|
	+--------------+

The CPU can perform a limited number of elementary actions, called
instructions or operations. For example: reading a byte from a cell, writing
a byte into a cell, adding two bytes together, etc...

A program is essentially a script detailing the sequence of operations that
the CPU must perform to produce the desired behaviour.

The CPU retrieves the operations to perform from the cells, starting from the
cell 3:0. What for us is just a number like any other, for the CPU has a
specific meaning. For example, 0 means BRK (abbreviation of BReaK) for the
CPU. The name BRK comes from the fact that, on a real 6502, BRK could be used
to suspend the program. This is one of the few differences between a real
6502 and the CPU of the VM. On the CPU of the VM, BRK causes the program to
terminate.

Places in the CPU are called registers instead of cells. We will examine only
a few of them now. First, there is the PC (or Program Counter). It has room
for two bytes and contains the address of the cell containing the next
operation to perform. Then there are three registers that have room for a
single byte each. They are: the Accumulator (abbreviated by A), X and Y.

Press 0 to switch to the MEMory view.

Write 232 into the cell 3:0. That is: select GoTo, input 3 into the Page
field, input 0 into the Offset field, select OK, select Edit, input 232 into
the Value field and then select OK.

	+--------------+
	|3:0        MEM|
	|   0   0   0  |
	|   0   0   0  |
	|<232>  0   0  |
	|Options   Edit|
	+--------------+

Press 0 to switch back to the CPU view.

	+--------------+
	|PC 3:0 R   CPU|
	| INX          |
	| A 0 X 0 Y 0  |
	| --------     |
	|Options   Edit|
	+--------------+

232 means INX (abbreviation of INcrement X) for the CPU. INX causes the X
register to be incremented by 1. If the X register already contains 255, it
is reset to 0.

Press 1 to advance one step (i.e. let the CPU perform one operation).

	+--------------+
	|PC 3:1 R   CPU|
	| BRK          |
	| A 0 X 1 Y 0  |
	| --------     |
	|Options   Edit|
	+--------------+

You can see that the operation has been carried out: the register X now
contains 1 and the PC now points to the next cell (i.e. the cell 3:1,
containing the next operation to perform).

We could change the content of the cell 3:1 and repeat the process again, but
this is not the best way to do it. Writing sequences of instructions is
better done using the Editor.

Press 1 to advance one step and terminate the program.

	+--------------+
	|HALTED        |
	|3:1           |
	|              |
	|              |
	|End      Video|
	+--------------+

You are given the last chance to have a look at the display, before powering
off the VM and returning to the Editor.

Select End.

## Editing programs

The Editor is a modal editor, i.e. the effects of pressing a key depend on
the mode of operation of the editor. There are two major modes of operation
(NAVigation and EDiTing) combined with two minor modes of operation (MEMory
and ASseMbly).

	+--------------+
	|C 3:0  NAV MEM|
	|  <0>  0   0  |
	|   0   0   0  |
	|   0   0   0  |
	|Options   Back|
	+--------------+

The NAV MEM mode behaves like the MEMory view described before. Just like
before, you can move the cursor using 4 (or left), 6 (or right), 2 (or up)
and 8 (or down). The main difference is that you cannot reach the cells
beyond the limit of the program. This is because, when writing a program, you
are not editing the memory of the VM, but merely writing a template of bytes
that will be later used to initialize it.

Make sure that the cursor is at 3:0 and press 5 (or fire, on some phones).

The EDT MEM mode allows typing the bytes of the program "in place" (i.e.
without opening a new screen).

Press 2, *, 2, 3, 2 and 2.

	+--------------+
	|C 3:2  EDT MEM|
	|   2 232  <2> |
	|   0   0   0  |
	|   0   0   0  |
	|OK      Cancel|
	+--------------+

Note the following:

1. You are not limited to typing one byte, but you can type a whole sequence
of bytes.

2. For bytes with fewer than three digits, you can move to the next byte by
pressing *.

3. For bytes with exactly three digits, moving to the next byte is automatic.

If you make a mistake typing the sequence, you can select Cancel to clear the
current byte. If the current byte is already cleared, selecting Cancel will
move to the previous byte. Finally, if you reach the first byte and select
Cancel, you will return to the NAV MEM mode without making any changes to the
program.

Press OK to confirm the changes and return to the NAV MEM mode.

	+--------------+
	|C 3:2  NAV MEM|
	|   2 232  <2> |
	|   0   0   0  |
	|   0   0   0  |
	|Options   Back|
	+--------------+

The first three bytes of the program should now be: 2, 232 and 2. You might
recognize 232 as meaning INX, but what about 2?

Press #.

	+--------------+
	|C 3:2  NAV ASM|
	| ???          |
	| INX          |
	|>???          |
	|Options   Back|
	+--------------+

The NAV ASM mode allows you see the program as it would be interpreted by the
CPU. You can move the cursor using 2 (or up) and 8 (or down) to scroll the
listing, but you cannot press 5 to edit it in place.

So, 2 means ??? for the CPU. Well, that was not very helpful. Let us start
the VM (properly this time) to see what happens.

Press *.

	+--------------+
	|INV.OP.       |
	|3:0           |
	|Opcode 2      |
	|              |
	|End      Video|
	+--------------+

Here is what happened:

1. The VM was powered on.

2. The program was copied to the memory cells of the VM.

3. The CPU read the cell 3:0 to get an operation to perform.

4. Since 2 has no meaning for the CPU, the VM was halted with an error.

INV.OP. is the abbreviation of "invalid opcode". Opcode is the contraction of
"operation code" and is a byte identifying what kind of operation to perform.
Some opcodes (e.g. 232) make sense for the CPU, while some others (e.g. 2) do
not.

Select End to return to the Editor and then press # to switch to the NAV MEM
mode.

	+--------------+
	|C 3:2  NAV MEM|
	|   2 232  <2> |
	|   0   0   0  |
	|   0   0   0  |
	|Options   Back|
	+--------------+

## Opcodes and Addr. Modes

Before going any further, let me rephrase the sentence:

232 means INX for the CPU.

Without using the misleading word "means" ("means" might suggest that the CPU
understands something):

When the CPU reads the opcode 232 from a cell, it increments the X register.

The three-letter label INX is called mnemonic and its only purpose is to
remind the programmer of what the CPU does.

Let us have a look at two new opcodes: 169 and 173.

169. When the CPU reads the opcode 169 from a cell, it replaces the content
of the Accumulator with the content of the cell after the cell containing the
opcode.

173. When the CPU reads the opcode 173 from a cell, it replaces the content
of the Accumulator with the content of a specific cell. The address of that
cell is computed using, as the offset, the content of the cell after the cell
containing the opcode, and, as the page, the content of the cell after the
cell providing the offset

The wording is a bit convoluted, but I hope it will become clear with the
examples below. What is important right now is to see a similarity between
the two opcodes: in both cases, the content of the Accumulator is replaced.
The mnemonic for that is LDA (abbreviation of LoaD Accumulator).

Here is an example for the opcode 169:

	169 65

Note that, unlike BRK and INX, one byte is not enough to specify the
behaviour of the CPU. If we want the CPU to replace the content of the
Accumulator, we must specify the new content (65 in this case). In other
words: the operation beginning with the opcode 169 is two-bytes long and the
second byte (a.k.a. the operand) specifies the new content of the Accumulator.

And here is an example for the opcode 173:

	173 40 2

In this case the operation is three-bytes long, and the last two bytes (i.e.
the operand) specify the address of a cell where the new value of the
Accumulator can be found. In this case, that would be cell 2:40. Yes, the
order is inverted and this might be confusing at first, but you will quickly
get used to it.

If you look at the two sequences of bytes above, you see that it is not clear
that they are doing something similar (i.e. replacing the content of the
Accumulator). Furthermore, on a long program composed of several operations,
it is difficult to spot where the operations begin and to check if you
provided the right number of bytes as operands.

Consider this sequence:

	169 40 2

Maybe you meant to load the Accumulator from the cell 2:40. But, of course,
the CPU cannot guess what you meant; it would load the Accumulator with 40
and then halt the VM because 2 is not a valid opcode.

## Assembly

The assembly language is an effective method to present the bytes of a
program. Every operation is clearly shown in its own line, no matter if it is
1, 2 or 3 bytes long. Its behaviour is unambiguously specified using its
mnemonic and, if needed, its operand. The operand itself is formatted using a
rigorous pattern. This pattern is a compact way to express the second part of
the two sentences above, or, in 6502 jargon, the addressing mode.

	Memory view   Assembly view   Explanation
	-----------   -------------   ----------------------------------
	169 65        LDA #65         LoaD the Accumulator [LDA] with
	                              the constant [#n] 65
	173 40 2      LDA 2:40        LoaD the Accumulator [LDA] with
	                              the content of the cell [n:n] 2:40

The addressing modes here are #n and n:n, where n is the abbreviation of
number.

The "qsref" book contains the full list of the addressing modes of the CPU.
You are not expected to understand all of them by the brief descriptions
contained in that book, but, even if you do not understand them, you can
still get some information from the way they look: the number of lower-case
characters of an addressing mode is the number of extra bytes an operation
needs. For example, the addressing mode n:n,X requires two extra bytes beside
the opcode.

Assembly, like every programming language and unlike natural languages, is a
formal language and every small detail matters. For example, in English, you
might make a punctuation mistake, but you can still be understood. In
Assembly, even punctuation is critical. The operations LDA #65 (i.e. 169 65)
and LDA 65 (i.e. 165 65) are completely different (the second one, if we
ignore the fact that is shorter and faster, is equivalent to LDA 0:65).

## Your First Program

Place the cursor at 3:0, press 5 to switch to the EDT MEM mode and then press
# to switch to the EDT ASM mode.

	+--------------+
	|C 3:0  EDT ASM|
	|  <> 232   2  |
	|   0   0   0  |
	|   0   0   0  |
	|Options   Back|
	+--------------+

The EDT ASM mode allows you to lookup a valid opcode.

Press 5 (jkL), 3 (Def) and 2 (Abc).

	+-Matchings----+
	| LDA (n,X)    |
	| LDA n        |
	|>LDA #n       |
	| LDA n:n      |
	|OK      Cancel|
	+--------------+

Select LDA #n from the list. 169 is inserted, the cursor is moved to the next
cell and the mode is switched back to EDT MEM.

	+--------------+
	|C 3:2  EDT ASM|
	| 169  65  <>  |
	|   0   0   0  |
	|   0   0   0  |
	|Options   Back|
	+--------------+

Press 6, 5 and * to insert the operand.

Press # to switch to the EDT ASM mode.

Press 7 (pqrS), 8 (Tuv) and 2 (Abc).

	+-Matchings----+
	| STA (n,X)    |
	| STA n        |
	|>STA n:n      |
	| STA (n),Y    |
	|OK      Cancel|
	+--------------+

STA is the mnemonic of STore Accumulator and is in a way the opposite of LDA:
it replaces the content of a cell with the content of the Accumulator.

The "qsref" book contains the full list of the mnemonics of the CPU. Again,
you are not expected to understand all of them by the brief descriptions
contained in that book, but, even without looking at the list, you should
already be able to guess what a few other valid mnemonics (LDX, STX, LDY,
STY, INY, DEX and DEY) stand for.

Select STA n:n from the list. 141 is inserted, the cursor is moved to the
next cell and the mode is switched back to EDT MEM.

	+--------------+
	|C 3:4  NAV MEM|
	| 169  65 141  |
	|  40  <2>  0  |
	|   0   0   0  |
	|Options   Back|
	+--------------+

Press 4, 0 and * to insert the first byte of the operand (i.e. the offset).

Press 2 and select OK to insert the second byte of the operand (i.e. the
page).

Press # to switch to the NAV ASM mode.

	+--------------+
	|C 3:2  NAV ASM|
	| LDA #65      |
	|>STA 2:40     |
	| BRK          |
	|Options   Back|
	+--------------+

Here is the Assembly view of the complete program:

	LDA #65
	STA 2:40
	BRK

To sum up:

1. The first operation loads the Accumulator with the value 65.

2. The second operation stores the Accumulator (i.e. 65) to the cell 2:40,
causing the character A (see the table above) to appear at the top left
corner of the display.

3. The third operation halts the VM.

Select Debug to test the program.

	+--------------+
	|PC 3:0 R   CPU|
	| LDA #65      |
	| A 0 X 0 Y 0  |
	| --------     |
	|Options   Edit|
	+--------------+

Press 1 to advance one step. The Accumulator now contains 65 and the Program
Counter 3:2.

Press 1 to advance another step. The Program Counter now contains 3:5. You
can check that the character A is visible on the display of the VM by
pressing # and then go back to the CPU view by pressing # again.

Select Abort to terminate the program and go back to the Editor.

	+-abc----------+
	|              |
	|Name (Save)   |
	|[Tutorial    ]|
	|              |
	|Options  Clear|
	+--------------+

At the beginning, you are likely to write very short programs and saving your
work should not be a priority. In fact, starting from scratch each time might
help you to consolidate what you have learned. If in the future you begin
writing larger programs, here is how you can save them:

Select Save, type Tutorial and select OK.

	+-JBit---------+
	|>Store        |
	| Editor       |
	| About        |
	|              |
	|Select    Exit|
	+--------------+

Select Back followed by Exit to terminate JBit, and then start JBit again.

## Modules and Versions

JBit is a modular system and can be configured to include more or fewer
tools, depending on the limitations of your phone. The main menu lists which
tools have been included.

In this tutorial I am using JBit1M, a version of JBit targeting old phones
that comes with only two tools: the Store and the Editor. We have already
used the Editor and below we are going to use the Store, but before that, let
me introduce a couple of tools that you probably have in your version of JBit.

### Demos

This tool provides a few ready-made programs that you can study. In
particular, the 6502 demos are especially designed for the beginning
programmer. If you have problem installing a version of JBit including the
Demos tool, you can type the 6502 demos manually using the JBit-E0.pdf sheet
as a reference.

### Paint

This tool allows you to create and edit simple images with JBit.

An important thing to understand is that the tools operate on the current
program. Unlike most computer applications, JBit can only work on one
"document" at a time; if you run a demo or load a program, the program you
are working on will be silently replaced, even if it has not been saved.

You might wonder why there is a Paint tool, if the display of the VM cannot
show images. The reason is that you can equip the VM with different versions
of the IO chip:

### MicroIO

The MicroIO version provides only the most basic facilities to write
interactive programs.

### MIDP1

The MIDP1 version adds various functionalities, but, most importantly, adds
the ability to display images.

### MIDP2

The MIDP2 version adds the ability to write 2D action games.

The MicroIO version should not be dismissed. You can learn it quickly and it
so simple that you do not need to consult a reference to use it. Even if you
have a better version available, targeting the MicroIO version when you write
your programs is a very good idea.

## Store

Select Store.

	+-Store (1% us-+
	|>Tutorial     |
	|              |
	|              |
	|              |
	|Options   Back|
	+--------------+

The Store tool gives you a container where you can keep your programs for
future editing. You have already used the Store tool without realizing it,
when you saved the program from within the Editor, but accessing it manually
gives you more options. The list of the saved programs is presented to you
and you can use a few commands to manage them.

Load&Edit (the default command) loads a program and starts the Editor.
Load&Run loads a program and starts the VM. Load just loads a program.

Save asks for the name of the current program and saves it. You cannot
overwrite an existing program using Save; for that you have to use Overwrite.

Info, Copy, Rename and Delete do pretty much what you expect them to do. They
do not change the current program.

When you upgrade JBit to a new version, your phone should ask you if you want
to keep its data and it is important to reply appropriatelly to keep your
saved programs. The exact wording of the question varies from phone to phone.

Select Tutorial.

	+--------------+
	|C 3:3  NAV MEM|
	| 162  88 142  |
	| <40>  2   0  |
	|   0   0   0  |
	|Options   Back|
	+--------------+

## Editing session

Here is a quick editing session, as a review:

1. Press 5 to start editing.

2. Press # to lookup the first opcode.

3. Type 539 (for LDX).

4. Select LDX #n from the list.

5. Type 88* to enter the operator.

6. Press # to lookup the second opcode.

7. Type 789 (for STX).

8. Select STX n:n from the list.

9. Select OK to confirm the changes (the result should be: 162 88 142 40 2).

10. Press * to run the program.

11. Select Video to have a look at the display.

12. Press any key (e.g. the soft key you have used for selecting Video, even
if it does not have a label attached to it anymore) to go back to the HALTED
screen.

13. Select End to go back to the Editor.

14. Select Save to save the new version of the program to the Store.

Select Back and then Exit to terminate JBit.

