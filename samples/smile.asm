; Show a simple image

	.device "io2"

.define ENABLE_IMAGE 3 ; ENABLE_BGCOL|ENABLE_BGIMG
.define PNG_TRANSP_ZOOMED 5 ; IPNGGEN_FLAGS_IDX0TRANSP | IPNGGEN_FLAGS_ZOOM0

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
	0:16 ; width
	0:16 ; height
	4 ; depth
	IPNGGEN_CT_INDEXED_COLOR
	PNG_TRANSP_ZOOMED
	; 16x16, 16 colors - converted by xpm2req
	15
	  0   0   0 ; UNUSED
	246 183   0 ; .
	246 222 119 ; +
	248 230 170 ; @
	245 207 110 ; #
	239 198  57 ; $
	247 225 156 ; %
	247 221 105 ; &
	245 218 137 ; *
	242 209  84 ; =
	228 160   0 ; -
	182 143  84 ; ;
	177 133  34 ; >
	218 175  70 ; ,
	217 173  42 ; '
	213 143   0 ; )
	.lookup " .+@#$%&*=-;>,')"
	.bits "                "
	.bits "      ....      "
	.bits "    .+@@@@#.    "
	.bits "   $%%+&&+**$   "
	.bits "  .@*+&&===+%-  "
	.bits " .+%+&;==;$$#*- "
	.bits " .@+&&&===$$=@- "
	.bits " .@+&&===$$$$%- "
	.bits " .@+>,==$$'>$%) "
	.bits " .@+=>,$$'>'$%) "
	.bits " .#*==>>>>'$#*) "
	.bits "  -@+=$$$$$=%)  "
	.bits "   '%*=$$$#%,   "
	.bits "    -#@@%%*)    "
	.bits "     --))))     "
	.bits "                "
.endreq
; image credit: http://www.famfamfam.com/lab/icons/silk (color-reduced)

setbgcol: .req
	REQ_SETBGCOL COLOR_GREEN
.endreq

setbgimg: .req
	REQ_SETBGIMG IMAGE_ID
.endreq
