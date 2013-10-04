; QUICK START
 
; A byte is an integer ranging from 0 to 255.

; A cell is place where you can write and read a byte.
; JBit has 65536 cells, organized in 256 pages of 256 cells each.
; The address of a cell is a pair of bytes separated by : (e.g. 2:40).
; The first byte is the page of the cell.
; The second byte is the offset of the cell within the page.
; EXAMPLES:
; - The first cell is at address 0:0
; - The last cell is at address 255:255
; - The 260th cell is at address 1:3 (i.e. the 4th cell of the 2nd page).

; A program is a list of bytes. For example, this is a program:

  238 40 2 238 79 2 238 79 2 0

; If you press RUN, these 10 bytes are written starting from the beginning
; of the 4th page (address 3:0).

; The CPU start reading from the first byte of the program (here, 238),
; and does the operation encoded in that byte.
; 238 means INCrement the byte of a cell; the address of the cell to
; increment follows: first the offset, then the page. 
; These three bytes together are called an "instruction".
; When the CPU has finished executing an instruction, it carries on
; executing the next.
; 0 means BReaK and it is used to end the program.

; In JBit, most of the cells are memory cells (i.e. you write something
; into it and you can later read it back), but cells on page 2 are special.
; In particular, cells at addresses from 2:40 to 2:79 are connected
; to the display. Initially, they all contains byte 32 (ASCII code for SPACE).

; Here are some more ASCII codes:
;     +---+---+---+    +---+---+---+    +---+---+---+
;  .. |   | ! | " | .. | 0 | 1 | 2 | .. | A | B | C | ..
;     +---+---+---+    +---+---+---+    +---+---+---+
;       32  33  34       48  49  50       65  66  67

; The program above could have been written like this:

  INC 2:40
  INC 2:79
  INC 2:79
  BRK
