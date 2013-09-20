; Show a simple image

	.device "io2"

.define ENABLE_IMAGE 3 ; ENABLE_BGCOL|ENABLE_BGIMG
.define PNG_TRANSP_PALREF_ZOOMED 7 ; IPNGGEN_FLAGS_PALREF | IPNGGEN_FLAGS_IDX0TRANSP | IPNGGEN_FLAGS_ZOOM0

	lda	#ENABLE_IMAGE
	sta	ENABLE
	lda	#>_data_start_
	sta	REQPTRHI
	lda	#<ipnggen
	sta	REQPTRLO
	lda	#<setbgcol
	sta	REQPTRLO
	lda	#<setbgimg
	sta	REQPTRLO

loop:
	sta	FRMDRAW
	lda	KEYBUF
	beq	loop

.data
_data_start_:

.define IMAGE_ID 1

ipnggen: .req
	REQ_IPNGGEN IMAGE_ID
	0:8 ; width
	0:8 ; height
	1 ; depth
	IPNGGEN_CT_INDEXED_COLOR
	PNG_TRANSP_PALREF_ZOOMED
	1 ; palette 0..1 x (1 if PALREF is set, 3 (RGB) if not)
	COLOR_WHITE  ; ref 0 - unused because IDX0TRANSP is set
	COLOR_YELLOW ; ref 1
	%01111110
	%11111111
	%11011011
	%11111111
	%11000011
	%11100111
	%11111111
	%01111110
.endreq

setbgcol: .req
	REQ_SETBGCOL COLOR_GREEN
.endreq

setbgimg: .req
	REQ_SETBGIMG IMAGE_ID
.endreq
