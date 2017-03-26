%include "nasmhead.inc"

section .data align=64

	extern MD_Screen
	extern MD_Screen32
	extern CRam
	extern Palette
	extern Palette32
	extern CDD.Control
	extern CDD.Rcv_Status
	extern CDD.Status
	extern CDD.Minute
	extern CDD.Seconde
	extern CDD.Frame
	extern CDD.Ext
	extern _Bits32


	DECL Small_Font_Data
	Small_Police:
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 32   
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000000, 0x00000700, 0x00000000			; 33	!
		dd 0x00000000, 0x00040002, 0x00050003, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 34	"
		dd 0x00000000, 0x00040002, 0x00050403, 0x00060004, 0x00070605, 0x00080006, 0x00000000			; 35	#
		dd 0x00000000, 0x00040300, 0x00000403, 0x00000500, 0x00070600, 0x00000706, 0x00000000			; 36	$
		dd 0x00000000, 0x00000002, 0x00050000, 0x00000500, 0x00000005, 0x00080000, 0x00000000			; 37	%
		dd 0x00000000, 0x00000300, 0x00050003, 0x00000500, 0x00070005, 0x00080700, 0x00000000			; 38	&
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 39	'
		dd 0x00000000, 0x00000300, 0x00000003, 0x00000004, 0x00000005, 0x00000700, 0x00000000			; 40	(
		dd 0x00000000, 0x00000300, 0x00050000, 0x00060000, 0x00070000, 0x00000700, 0x00000000			; 41	)
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060504, 0x00000600, 0x00080006, 0x00000000			; 42	*
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060504, 0x00000600, 0x00000000, 0x00000000			; 43	+
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000600, 0x00000700, 0x00000007			; 44	,
		dd 0x00000000, 0x00000000, 0x00000000, 0x00060504, 0x00000000, 0x00000000, 0x00000000			; 45	-
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000700, 0x00000000			; 46	.
		dd 0x00030000, 0x00040000, 0x00000400, 0x00000500, 0x00000005, 0x00000006, 0x00000000			; 47	/
		dd 0x00000000, 0x00000300, 0x00050003, 0x00060004, 0x00070005, 0x00000700, 0x00000000			; 48	0
		dd 0x00000000, 0x00000300, 0x00000403, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 49	1
		dd 0x00000000, 0x00000302, 0x00050000, 0x00000500, 0x00000005, 0x00080706, 0x00000000			; 50	2
		dd 0x00000000, 0x00000302, 0x00050000, 0x00000504, 0x00070000, 0x00000706, 0x00000000			; 51	3
		dd 0x00000000, 0x00000300, 0x00000003, 0x00060004, 0x00070605, 0x00080000, 0x00000000			; 52	4
		dd 0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00070000, 0x00000706, 0x00000000			; 53	5
		dd 0x00000000, 0x00000300, 0x00000003, 0x00000504, 0x00070005, 0x00000700, 0x00000000			; 54	6
		dd 0x00000000, 0x00040302, 0x00050000, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 55	7
		dd 0x00000000, 0x00000300, 0x00050003, 0x00000500, 0x00070005, 0x00000700, 0x00000000			; 56	8
		dd 0x00000000, 0x00000300, 0x00050003, 0x00060500, 0x00070000, 0x00000700, 0x00000000			; 57	9
		dd 0x00000000, 0x00000000, 0x00000400, 0x00000000, 0x00000000, 0x00000700, 0x00000000			; 58	:
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000500, 0x00000000, 0x00000700, 0x00000007			; 59	;
		dd 0x00000000, 0x00040000, 0x00000400, 0x00000004, 0x00000600, 0x00080000, 0x00000000			; 60	<
		dd 0x00000000, 0x00000000, 0x00050403, 0x00000000, 0x00070605, 0x00000000, 0x00000000			; 61	=
		dd 0x00000000, 0x00000002, 0x00000400, 0x00060000, 0x00000600, 0x00000006, 0x00000000			; 62	>
		dd 0x00000000, 0x00000302, 0x00050000, 0x00000500, 0x00000000, 0x00000700, 0x00000000			; 63	?
		dd 0x00000000, 0x00000300, 0x00050400, 0x00060004, 0x00070600, 0x00000000, 0x00000000			; 64	@
		dd 0x00000000, 0x00000300, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000			; 65	A
		dd 0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00070005, 0x00000706, 0x00000000			; 66	B
		dd 0x00000000, 0x00040300, 0x00000003, 0x00000004, 0x00000005, 0x00080700, 0x00000000			; 67 	C
		dd 0x00000000, 0x00000302, 0x00050003, 0x00060004, 0x00070005, 0x00000706, 0x00000000			; 68	D
		dd 0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00000005, 0x00080706, 0x00000000			; 69	E
		dd 0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00000005, 0x00000006, 0x00000000			; 70	F
		dd 0x00000000, 0x00040300, 0x00000003, 0x00060004, 0x00070005, 0x00080700, 0x00000000			; 71	G
		dd 0x00000000, 0x00040002, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000			; 72	H
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 73	I
		dd 0x00000000, 0x00040000, 0x00050000, 0x00060000, 0x00070005, 0x00000700, 0x00000000			; 74	J
		dd 0x00000000, 0x00040002, 0x00050003, 0x00000504, 0x00070005, 0x00080006, 0x00000000			; 75	K
		dd 0x00000000, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00080706, 0x00000000			; 76	l
		dd 0x00000000, 0x00040002, 0x00050403, 0x00060004, 0x00070005, 0x00080006, 0x00000000			; 77	M
		dd 0x00000000, 0x00000302, 0x00050003, 0x00060004, 0x00070005, 0x00080006, 0x00000000			; 78	N
		dd 0x00000000, 0x00040302, 0x00050003, 0x00060004, 0x00070005, 0x00080706, 0x00000000			; 79	O
		dd 0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00000005, 0x00000006, 0x00000000			; 80	P
		dd 0x00000000, 0x00040302, 0x00050003, 0x00060004, 0x00070005, 0x00080706, 0x00090000			; 81	Q
		dd 0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00070005, 0x00080006, 0x00000000			; 82	R
		dd 0x00000000, 0x00040300, 0x00000003, 0x00000500, 0x00070000, 0x00000706, 0x00000000			; 83 	S
		dd 0x00000000, 0x00040302, 0x00000400, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 84	T
		dd 0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00070005, 0x00080706, 0x00000000			; 85	U
		dd 0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00000600, 0x00000700, 0x00000000			; 86	V
		dd 0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00070605, 0x00080006, 0x00000000			; 87	W
		dd 0x00000000, 0x00040002, 0x00050003, 0x00000500, 0x00070005, 0x00080006, 0x00000000			; 88	X
		dd 0x00000000, 0x00040002, 0x00050003, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 89	Y
		dd 0x00000000, 0x00040302, 0x00050000, 0x00000500, 0x00000005, 0x00080706, 0x00000000			; 90	Z
		dd 0x00000000, 0x00040300, 0x00000400, 0x00000500, 0x00000600, 0x00080700, 0x00000000			; 91	[
		dd 0x00000000, 0x00000002, 0x00000400, 0x00000500, 0x00070000, 0x00080000, 0x00000000			; 92	\
		dd 0x00000000, 0x00000302, 0x00000400, 0x00000500, 0x00000600, 0x00000706, 0x00000000			; 93	]
		dd 0x00000000, 0x00000300, 0x00050003, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 94	^
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00080706, 0x00000000			; 95	_
		dd 0x00000000, 0x00000002, 0x00000400, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 96	`
		dd 0x00000000, 0x00000000, 0x00050400, 0x00060004, 0x00070005, 0x00080700, 0x00000000			; 97	a
		dd 0x00000000, 0x00000002, 0x00000003, 0x00000504, 0x00070005, 0x00000706, 0x00000000			; 98	b
		dd 0x00000000, 0x00000000, 0x00050400, 0x00000004, 0x00000005, 0x00080700, 0x00000000			; 99	c
		dd 0x00000000, 0x00040000, 0x00050000, 0x00060500, 0x00070005, 0x00080700, 0x00000000			; 100	d
		dd 0x00000000, 0x00000000, 0x00050400, 0x00060504, 0x00000005, 0x00080700, 0x00000000			; 101	e
		dd 0x00000000, 0x00040300, 0x00000003, 0x00000504, 0x00000005, 0x00000006, 0x00000000			; 102	f
		dd 0x00000000, 0x00000000, 0x00050400, 0x00060004, 0x00070600, 0x00080000, 0x00000807			; 103	g
		dd 0x00000000, 0x00000002, 0x00000003, 0x00000504, 0x00070005, 0x00080006, 0x00000000			; 104	h
		dd 0x00000000, 0x00000300, 0x00000000, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 105	i
		dd 0x00000000, 0x00000300, 0x00000000, 0x00000500, 0x00000600, 0x00000700, 0x00000007			; 106	j
		dd 0x00000000, 0x00000002, 0x00000003, 0x00060004, 0x00000605, 0x00080006, 0x00000000			; 107	k
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000600, 0x00080000, 0x00000000			; 108	l
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000			; 109	m
		dd 0x00000000, 0x00000000, 0x00000403, 0x00060004, 0x00070005, 0x00080006, 0x00000000			; 110	n
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070005, 0x00000700, 0x00000000			; 111	o
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00000605, 0x00000006, 0x00000007			; 112	p
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070600, 0x00080000, 0x00090000			; 113	q
		dd 0x00000000, 0x00000000, 0x00050003, 0x00000504, 0x00000005, 0x00000006, 0x00000000			; 114	r
		dd 0x00000000, 0x00000000, 0x00050400, 0x00000004, 0x00070600, 0x00000706, 0x00000000			; 115	s
		dd 0x00000000, 0x00000300, 0x00050403, 0x00000500, 0x00000600, 0x00080000, 0x00000000			; 116	t
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070005, 0x00080700, 0x00000000			; 117	u
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070005, 0x00000700, 0x00000000			; 118	v
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070605, 0x00080006, 0x00000000			; 119	w
		dd 0x00000000, 0x00000000, 0x00050003, 0x00000500, 0x00070005, 0x00080006, 0x00000000			; 120	x
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00000600, 0x00000700, 0x00000007			; 121	y
		dd 0x00000000, 0x00000000, 0x00050403, 0x00000500, 0x00000005, 0x00080706, 0x00000000			; 122	z
		dd 0x00000000, 0x00040300, 0x00000400, 0x00000504, 0x00000600, 0x00080700, 0x00000000			; 123	{
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000000, 0x00000600, 0x00000700, 0x00000000			; 124	|
		dd 0x00000000, 0x00000302, 0x00000400, 0x00060500, 0x00000600, 0x00000706, 0x00000000			; 125	}
		dd 0x00000000, 0x00000302, 0x00050000, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 126	~
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070605, 0x00000000, 0x00000000			; 127 	
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000

	Palette_Blanc_15:

	%assign i 16
	%rep 16
		dw (i * 1024 + i * 32 + i)
	%assign i i+1
	%endrep

	Palette_Bleu_15:

	%assign i 16
	%rep 16
		dw i
	%assign i i+1
	%endrep

	Palette_Vert_15:

	%assign i 16
	%rep 16
		dw (i * 32)
	%assign i i+1
	%endrep

	Palette_Rouge_15:

	%assign i 16
	%rep 16
		dw (i * 1024)
	%assign i i+1
	%endrep

	Palette_Blanc_16:

	%assign i 16
	%rep 16
		dw (i * 2048 + i * 64 + i)
	%assign i i+1
	%endrep

	Palette_Bleu_16:

	%assign i 16
	%rep 16
		dw i
	%assign i i+1
	%endrep

	Palette_Vert_16:

	%assign i 16
	%rep 16
		dw (i * 64)
	%assign i i+1
	%endrep

	Palette_Rouge_16:

	%assign i 16
	%rep 16
		dw (i * 2048)
	%assign i i+1
	%endrep
	
	Palette_Blanc_32:

	%assign i 128
	%rep 16
		dd (i * 65536 + i * 256 + i + 0xFF000000) ; added alpha
	%assign i i+8
	%endrep

	Palette_Bleu_32:

	%assign i 128
	%rep 16
		dd (i + 0xFF000000) ; added alpha
	%assign i i+8
	%endrep

	Palette_Vert_32:

	%assign i 128
	%rep 16
		dd (i * 256 + 0xFF000000) ; added alpha
	%assign i i+8
	%endrep

	Palette_Rouge_32:

	%assign i 128
	%rep 16
		dd (i * 65536 + 0xFF000000) ; added alpha
	%assign i i+8
	%endrep

	Mask:			dd 0x00000000, 0x00000000
	Mask_GG_15:		dd 0x03E003E0, 0x03E003E0
	Mask_RBRB_15:	dd 0x7C1F7C1F, 0x7C1F7C1F
	Mask_GG_16:		dd 0x07C007C0, 0x07C007C0
	Mask_RBRB_16	dd 0xF81FF81F, 0xF81FF81F

	Mask_1001_64	dd 0xFFFF0000, 0x0000FFFF
	Mask_0011_64	dd 0xFFFFFFFF, 0x00000000
	Mask_1100_64	dd 0x00000000, 0xFFFFFFFF


section .bss align=64

	extern VDP_Reg
	extern Mode_555

	DECL CPU_Model
	resd 1
	DECL Have_MMX
	resd 1
	DECL MMX_Enable
	resd 1

section .text align=64

%macro AFF_PIXEL 2

		mov eax, ebx							; eax = data pixels
		shr eax, %2								; keep the first
		push ebx
		mov ebx, ebp
		shl bx, 1
		and eax, 0xF
		jz %%BG
		; ax now 0000iiii
		shr bl, 2
		or  al, bl ; 0000iiii-> hsppiiii
		and eax, 0x3F ; hsppiiii -> 00ppiiii
		mov ax, [CRam + eax * 2]	; 00ppiiii -> 0000bbbbggggrrrr
	%%BGr:
		and ax, 0xEEE
		shl bh, 4
		or  ah, bh ; 0000bbbbggggrrrr -> 00hsbbbbggggrrrr
		pop ebx
		mov ax, [Palette + eax * 2 + 0x4000 * 2]	; conversion 8->16 bits palette
		jmp %%DRAW
	%%BG:
		mov ax, [CRam]
		jmp %%BGr
	%%DRAW:		
		mov [edi + (%1 * 2)], ax				; write the pixel to Dest

%endmacro

%macro AFF_PIXEL32 2

		mov eax, ebx							; eax = data pixels
		shr eax, %2								; keep the first
		push ebx
		mov ebx, ebp
		and eax, 0xF
		jz %%BG
		; ax now 0000iiii
		shr bl, 2
		or  al, bl ; 0000iiii-> hsppiiii
		and al, 0x3F ; hsppiiii -> 00ppiiii
		mov ax, [CRam + eax * 2]	; 00ppiiii -> 0000bbbbggggrrrr
	%%BGr:
		and ax, 0xEEE
		shl bh, 4
		or  ah, bh ; 0000bbbbggggrrrr -> 00hsbbbbggggrrrr
		pop ebx
		mov eax, [Palette32 + eax * 4 + 0x4000 * 4]	; conversion 16->32 bits palette
		jmp %%DRAW
	%%BG:
		mov ax, [CRam]
		jmp %%BGr
	%%DRAW:		
		mov [edi + (%1 * 4)], eax				; write the pixel to Dest

%endmacro



%macro AFF_LINE_LETTER 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 2 * %1) + 0], ax

	%%Pix1
		mov	dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 2 * %1) + 2], ax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 2 * %1) + 4], ax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 2 * %1) + 6], ax

	%%End

%endmacro

%macro AFF_LINE_LETTER32 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 4 * %1) + 0], eax

	%%Pix1
		mov	dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 4 * %1) + 4], eax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 4 * %1) + 8], eax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 4 * %1) + 12], eax

	%%End

%endmacro

%macro AFF_LINE_LETTER_X2 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov ax, [ebx + edx * 2]
		rol eax, 16
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 4 * %1) + 0], eax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 0], eax

	%%Pix1
		mov	dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov ax, [ebx + edx * 2]
		rol eax, 16
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 4 * %1) + 4], eax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 4], eax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 2]
		rol eax, 16
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 4 * %1) + 8], eax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 8], eax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 2]
		rol eax, 16
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 4 * %1) + 12], eax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 12], eax

	%%End

%endmacro

%macro AFF_LINE_LETTER32_X2 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 8 * %1) + 0], eax
		mov [edi + (336 * 8 * %1) + 4], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 0], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 4], eax

	%%Pix1
		mov	dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 8 * %1) + 8], eax
		mov [edi + (336 * 8 * %1) + 12], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 8], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 12], eax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 8 * %1) + 16], eax
		mov [edi + (336 * 8 * %1) + 20], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 16], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 20], eax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 8 * %1) + 24], eax
		mov [edi + (336 * 8 * %1) + 28], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 24], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 28], eax

	%%End

%endmacro

%macro AFF_LINE_LETTER_T 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 2 * %1) + 0]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add ax, dx
		xor dh, dh
		mov [edi + (336 * 2 * %1) + 0], ax

	%%Pix1
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 2 * %1) + 2]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add ax, dx
		xor dh, dh
		mov [edi + (336 * 2 * %1) + 2], ax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 2 * %1) + 4]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add ax, dx
		xor dh, dh
		mov [edi + (336 * 2 * %1) + 4], ax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 2 * %1) + 6]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add ax, dx
		xor dh, dh
		mov [edi + (336 * 2 * %1) + 6], ax

	%%End

%endmacro

%macro AFF_LINE_LETTER32_T 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 4 * %1) + 0]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add eax, edx
		xor edx, edx
		mov [edi + (336 * 4 * %1) + 0], eax

	%%Pix1
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 4 * %1) + 4]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add eax, edx
		xor edx, edx
		mov [edi + (336 * 4 * %1) + 4], eax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 4 * %1) + 8]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add eax, edx
		xor edx, edx
		mov [edi + (336 * 4 * %1) + 8], eax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 4 * %1) + 12]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add eax, edx
		xor edx, edx
		mov [edi + (336 * 4 * %1) + 12], eax

	%%End

%endmacro

%macro AFF_LINE_LETTER_T_X2 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 4 * %1) + 0]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 0], dx
		mov dx, [edi + (336 * 4 * %1) + 2]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 2], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 0]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 0], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 2]
		and dx, [Mask]
		shr dx, 1
		add ax, dx
		mov [edi + (336 * 4 * %1) + (336 * 2) + 2], ax
		xor dh, dh

	%%Pix1
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 4 * %1) + 4]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 4], dx
		mov dx, [edi + (336 * 4 * %1) + 6]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 6], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 4]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 4], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 6]
		and dx, [Mask]
		shr dx, 1
		add ax, dx
		mov [edi + (336 * 4 * %1) + (336 * 2) + 6], ax
		xor dh, dh

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 4 * %1) + 8]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 8], dx
		mov dx, [edi + (336 * 4 * %1) + 10]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 10], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 8]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 8], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 10]
		and dx, [Mask]
		shr dx, 1
		add ax, dx
		mov [edi + (336 * 4 * %1) + (336 * 2) + 10], ax
		xor dh, dh

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 4 * %1) + 12]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 12], dx
		mov dx, [edi + (336 * 4 * %1) + 14]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 14], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 12]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 12], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 14]
		and dx, [Mask]
		shr dx, 1
		add ax, dx
		mov [edi + (336 * 4 * %1) + (336 * 2) + 14], ax
		xor dh, dh

	%%End

%endmacro

%macro AFF_LINE_LETTER32_T_X2 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 8 * %1) + 0]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 0], edx
		mov edx, [edi + (336 * 8 * %1) + 4]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 4], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 4) + 0]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 0], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 2) + 4]
		and edx, [Mask]
		shr edx, 1
		add eax, edx
		mov [edi + (336 * 8 * %1) + (336 * 4) + 4], eax
		xor edx, edx

	%%Pix1
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 8 * %1) + 8]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 8], edx
		mov edx, [edi + (336 * 8 * %1) + 12]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 12], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 4) + 8]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 8], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 2) + 12]
		and edx, [Mask]
		shr edx, 1
		add eax, edx
		mov [edi + (336 * 8 * %1) + (336 * 4) + 12], eax
		xor edx, edx

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 8 * %1) + 16]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 16], edx
		mov edx, [edi + (336 * 8 * %1) + 20]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 20], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 4) + 16]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 16], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 2) + 20]
		and edx, [Mask]
		shr edx, 1
		add eax, edx
		mov [edi + (336 * 8 * %1) + (336 * 4) + 20], eax
		xor edx, edx

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 8 * %1) + 24]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 24], edx
		mov edx, [edi + (336 * 8 * %1) + 28]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 28], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 4) + 24]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 24], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 2) + 28]
		and edx, [Mask]
		shr edx, 1
		add eax, edx
		mov [edi + (336 * 8 * %1) + (336 * 4) + 28], eax
		xor edx, edx

	%%End

%endmacro

	ALIGN4
	
	; void Identify_CPU(void)
	DECL Identify_CPU

		pushad

		mov dword [Have_MMX], 0
		mov dword [CPU_Model], 0
		pushfd
		pop eax
		mov ebx, eax
		xor eax, 0x200000
		push eax
		popfd
		pushfd
		pop eax
		xor eax, ebx
		and eax, 0x200000
		jz .not_supported		; CPUID instruction not supported

		xor eax, eax
		cpuid					; get number of CPUID functions
		test eax, eax
		jz .not_supported		; CPUID function 1 not supported

		mov eax, 1
		cpuid					; get family and features
		and eax, 0x000000F00	; family
		and edx, 0x0FFFFF0FF	; features flags
		or eax, edx				; combine bits
		mov [CPU_Model], eax	; Store CPU Model
		test eax, 0x00800000	; Having MMX ?
		setnz [Have_MMX]		; Store it

	.not_supported
		popad
		ret

	ALIGN32

	; int Half_Blur(void)
	DECL Half_Blur

		push ebx
		push ecx
		push edx
		push edi
		push esi
	
		test byte [Have_MMX], 0xFF
		jz near .No_MMX

		mov esi, MD_Screen
		mov ecx, (336 * 240 * 2) / 8
		xor edi, edi
		xor edx, edx

		movq mm6, [Mask_RBRB_16]
		movq mm7, [Mask_GG_16]

		test byte [Mode_555], 1
		jz short .MMX_Loop

		movq mm6, [Mask_RBRB_15]
		movq mm7, [Mask_GG_15]
		jmp short .MMX_Loop

	ALIGN32
	
	.MMX_Loop
			movq mm0, [esi]
			movq mm1, mm0
			movq mm2, [esi + 2]
			movq mm3, mm2
			pand mm0, mm6	;pixels 0-3 r b
			pand mm1, mm7	;pixels 0-3  g
			pand mm2, mm6	;pixels 1-4 r b
			pand mm3, mm7	;pixels 1-4  g
			psrlw mm0, 1
			psrlw mm1, 1
			psrlw mm2, 1
			psrlw mm3, 1
			paddusw mm0, mm2
			paddusw mm1, mm3
			pand mm0, mm6
			pand mm1, mm7
			add esi, 8
			paddw mm0, mm1
			dec ecx
			movq [esi - 8], mm0
			jnz short .MMX_Loop

		emms
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32

	.No_MMX
		mov esi, MD_Screen
		mov ecx, 336 * 240  ; nombre de pix 16 bits à rendre en blur
		xor edi, edi
		xor edx, edx
		test byte [Mode_555], 1
		jnz short .Loop_555
		jmp short .Loop_565

	ALIGN32

	.Loop_555
			mov ax, [esi]
			add esi, 2
			shr ax, 1
			mov bx, ax
			and ax, 0x3C0F
			and bx, 0x01E0
			add di, ax
			add dx, bx
			add dx, di
			dec ecx
			mov [esi - 2], dx
			mov di, ax
			mov dx, bx
			jnz short .Loop_555

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32

	.Loop_565
			mov ax, [esi]
			add esi, 2
			shr ax, 1
			mov bx, ax
			and ax, 0x780F
			and bx, 0x03E0
			add di, ax
			add dx, bx
			add dx, di
			dec ecx
			mov [esi - 2], dx
			mov di, ax
			mov dx, bx
			jnz .Loop_565

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN4

	; void Byte_Swap(void *Ptr, int NumByte)
	DECL Byte_Swap

		push eax
		push ecx
		push edi
	
		mov ecx, [esp + 20]
		mov edi, [esp + 16]
		shr ecx, 1
		jz short .end

	ALIGN4

	.loop
			mov ax, [edi]
			add edi, 2
			rol ax, 8
			dec ecx
			mov [edi - 2], ax
			jnz short .loop

	.end

		pop edi
		pop ecx
		pop eax
		ret


	ALIGN4

	; void Word_Swap(void *Ptr, int NumByte)
	DECL Word_Swap ;Important for 32X RAM getting searched correctly

		push eax
		push ecx
		push edi
	
		mov ecx, [esp + 20]
		mov edi, [esp + 16]
		shr ecx, 2
		jz short .end

	ALIGN4

	.loop
			mov eax, [edi]
			add edi, 4
			rol eax, 16
			dec ecx
			mov [edi - 4], eax
			jnz short .loop

	.end

		pop edi
		pop ecx
		pop eax
		ret


	ALIGN4
	
	; void Print_Text(char *str, int Size, int Pos_X, int Pos_Y, int Style)
	DECL Print_Text


		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov esi, [esp + 24]				; esi = *string
		mov ebx, 336 * 2				; Pitch Dest
		test [_Bits32], byte 1
		mov eax, [esp + 36]				; eax = Pos Y
		jnz Print_Text32
		mul ebx
		lea edi, [MD_Screen + 8 * 2]	; edi = Dest
		mov ecx, [esp + 32]				; Pos X
		lea edi, [edi + ecx * 2]		; offset for Pos X
		add edi, eax					; + offset Pos Y
		mov ebx, 320					; length of a row
		mov eax, [esp + 40]				; eax = Style
		mov ecx, [esp + 28]				; ecx = Size
		test eax, 0x1					; test the emulation mode
		jz short .No_Emulation

		test byte [VDP_Reg + 12 * 4], 1	; on teste si on est en mode 32 ou 40 cells
		jnz short .No_Emulation

		mov ebx, 256					; Taille = 256

	.No_Emulation
		mov edx, [esp + 32]				; edx = Pos X
		test eax, 0x10					; teste if mode x2
		lea edx, [edx + ecx * 4]		; edx = Pos X finale
		jz short .Size_x1

		lea edx, [edx + ecx * 4]		; edx = Pos X finale

	.Size_x1
		sub edx, ebx					; on teste si la chaine n'est pas trop grande
		jb short .String_OK				; si c ok on passe à la suite sinon

		shr edx, 2
		test eax, 0x10					; teste si mode x2
		jz short .Size_x1_2

		shr edx, 1

	.Size_x1_2
		inc edx							; edx = nombre de caractère en trop
		sub ecx, edx					; ecx = nouvelle taille pour être OK
		mov byte [esi + ecx - 2], '.'	; on termine la chiane avec des points
		mov byte [esi + ecx - 1], '.'

	.String_OK
		mov ebx, eax
		mov byte [esi + ecx - 0], 0		; fin de la chaine pour être sur
		shr ebx, 1
		sub esp, 4
		and ebx, 0x3					; on garde uniquement le type palette
		mov dword [Mask], 0xF7DE
		test byte [Mode_555], 1			; test the current mode
		jz short .Mode_565
		or ebx, 0x4
		mov dword [Mask], 0x7BDE

	.Mode_565
		test byte [esi], 0xFF				; test the first byte
		mov ebx, [.Palette_Table + ebx * 4]	; ebx pointe to the palette in use
		jz near .End						; if NULL then we leave...
		add ebx, 12							; ...a clear color

		and eax, 0x18						; isolate the bits for copying
		shr eax, 1
		mov eax, [.Table_Style + eax]
		mov [esp], eax
		jmp [esp]

	ALIGN4

	.Palette_Table
		dd Palette_Blanc_16, Palette_Bleu_16, Palette_Vert_16, Palette_Rouge_16
		dd Palette_Blanc_15, Palette_Bleu_15, Palette_Vert_15, Palette_Rouge_15

	ALIGN4

	.Table_Style
		dd .Mode_Normal, .Mode_Trans
		dd .Mode_Normal_X2, .Mode_Trans_X2

	ALIGN4

	.Mode_Normal
		movzx eax, byte [esi]
		sub eax, 32
		jae .MN_Car_OK
		xor eax, eax
	.MN_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER 0
		AFF_LINE_LETTER 1
		AFF_LINE_LETTER 2
		AFF_LINE_LETTER 3
		AFF_LINE_LETTER 4
		AFF_LINE_LETTER 5
		AFF_LINE_LETTER 6

		jmp .Next_Letter

	ALIGN4

	.Mode_Trans
		movzx eax, byte [esi]
		sub eax, 32
		jae .MT_Car_OK
		xor eax, eax
	.MT_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER_T 0
		AFF_LINE_LETTER_T 1
		AFF_LINE_LETTER_T 2
		AFF_LINE_LETTER_T 3
		AFF_LINE_LETTER_T 4
		AFF_LINE_LETTER_T 5
		AFF_LINE_LETTER_T 6

		jmp .Next_Letter

	ALIGN4

	.Mode_Normal_X2
		movzx eax, byte [esi]
		sub eax, 32
		jae short .MN_X2_Car_OK
		xor eax, eax
	.MN_X2_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER_X2 0
		AFF_LINE_LETTER_X2 1
		AFF_LINE_LETTER_X2 2
		AFF_LINE_LETTER_X2 3
		AFF_LINE_LETTER_X2 4
		AFF_LINE_LETTER_X2 5
		AFF_LINE_LETTER_X2 6

		jmp .Next_Letter_X2

	ALIGN4

	.Mode_Trans_X2
		movzx eax, byte [esi]
		sub eax, 32
		jae short .MT_X2_Car_OK
		xor eax, eax
	.MT_X2_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER_T_X2 0
		AFF_LINE_LETTER_T_X2 1
		AFF_LINE_LETTER_T_X2 2
		AFF_LINE_LETTER_T_X2 3
		AFF_LINE_LETTER_T_X2 4
		AFF_LINE_LETTER_T_X2 5
		AFF_LINE_LETTER_T_X2 6

		jmp .Next_Letter_X2

	ALIGN4
	
	.Next_Letter
		inc esi
		add edi, 4 * 2
		test byte [esi], 0xFF
		jz short .End
		jmp [esp]

	ALIGN4
	
	.Next_Letter_X2
		inc esi
		add edi, 8 * 2
		test byte [esi], 0xFF
		jz short .End
		jmp [esp]

	.End
		add esp, 4
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN4
	
	Print_Text32

		shl ebx, 1						; Pitch Dest
		lea edi, [MD_Screen32 + 8 * 4]	; edi = Dest
		mul ebx
		mov ecx, [esp + 32]				; Pos X
		lea edi, [edi + ecx * 4]		; offset for Pos X
		add edi, eax					; + offset Pos Y
		mov ebx, 320					; length of a row
		mov eax, [esp + 40]				; eax = Style
		mov ecx, [esp + 28]				; ecx = Size
		test eax, 0x1					; test the emulation mode
		jz short .No_Emulation

		test byte [VDP_Reg + 12 * 4], 1	; on teste si on est en mode 32 ou 40 cells
		jnz short .No_Emulation

		mov ebx, 256					; Taille = 256

	.No_Emulation
		mov edx, [esp + 32]				; edx = Pos X
		test eax, 0x10					; teste if mode x2
		lea edx, [edx + ecx * 4]		; edx = Pos X finale
		jz short .Size_x1

		lea edx, [edx + ecx * 4]		; edx = Pos X finale

	.Size_x1
		sub edx, ebx					; on teste si la chaine n'est pas trop grande
		jb short .String_OK				; si c ok on passe à la suite sinon

		shr edx, 2
		test eax, 0x10					; teste si mode x2
		jz short .Size_x1_2

		shr edx, 1

	.Size_x1_2
		inc edx							; edx = nombre de caractère en trop
		sub ecx, edx					; ecx = nouvelle taille pour être OK
		mov byte [esi + ecx - 2], '.'	; on termine la chiane avec des points
		mov byte [esi + ecx - 1], '.'

	.String_OK
		mov ebx, eax
		mov byte [esi + ecx - 0], 0		; fin de la chaine pour être sur
		shr ebx, 1
		sub esp, 4
		and ebx, 0x3					; on garde uniquement le type palette
		mov dword [Mask], 0xFEFEFEFE	; added alpha
		test byte [esi], 0xFF				; test the first byte
		mov ebx, [.Palette_Table + ebx * 4]	; ebx pointe to the palette in use
		jz near .End						; if NULL then we leave...
		add ebx, 12							; ...a clear color

		and eax, 0x18						; isolate the bits for copying
		shr eax, 1
		mov eax, [.Table_Style + eax]
		mov [esp], eax
		jmp [esp]

	ALIGN4

	.Palette_Table
		dd Palette_Blanc_32, Palette_Bleu_32, Palette_Vert_32, Palette_Rouge_32

	ALIGN4

	.Table_Style
		dd .Mode_Normal, .Mode_Trans
		dd .Mode_Normal_X2, .Mode_Trans_X2

	ALIGN4

	.Mode_Normal
		movzx eax, byte [esi]
		sub eax, 32
		jae .MN_Car_OK
		xor eax, eax
	.MN_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER32 0
		AFF_LINE_LETTER32 1
		AFF_LINE_LETTER32 2
		AFF_LINE_LETTER32 3
		AFF_LINE_LETTER32 4
		AFF_LINE_LETTER32 5
		AFF_LINE_LETTER32 6

		jmp .Next_Letter

	ALIGN4

	.Mode_Trans
		movzx eax, byte [esi]
		sub eax, 32
		jae .MT_Car_OK
		xor eax, eax
	.MT_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER32_T 0
		AFF_LINE_LETTER32_T 1
		AFF_LINE_LETTER32_T 2
		AFF_LINE_LETTER32_T 3
		AFF_LINE_LETTER32_T 4
		AFF_LINE_LETTER32_T 5
		AFF_LINE_LETTER32_T 6

		jmp .Next_Letter

	ALIGN4

	.Mode_Normal_X2
		movzx eax, byte [esi]
		sub eax, 32
		jae short .MN_X2_Car_OK
		xor eax, eax
	.MN_X2_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER32_X2 0
		AFF_LINE_LETTER32_X2 1
		AFF_LINE_LETTER32_X2 2
		AFF_LINE_LETTER32_X2 3
		AFF_LINE_LETTER32_X2 4
		AFF_LINE_LETTER32_X2 5
		AFF_LINE_LETTER32_X2 6

		jmp .Next_Letter_X2

	ALIGN4

	.Mode_Trans_X2
		movzx eax, byte [esi]
		sub eax, 32
		jae short .MT_X2_Car_OK
		xor eax, eax
	.MT_X2_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER32_T_X2 0
		AFF_LINE_LETTER32_T_X2 1
		AFF_LINE_LETTER32_T_X2 2
		AFF_LINE_LETTER32_T_X2 3
		AFF_LINE_LETTER32_T_X2 4
		AFF_LINE_LETTER32_T_X2 5
		AFF_LINE_LETTER32_T_X2 6

		jmp .Next_Letter_X2

	ALIGN4
	
	.Next_Letter
		inc esi
		add edi, 4 * 4
		test byte [esi], 0xFF
		jz short .End
		jmp [esp]

	ALIGN4
	
	.Next_Letter_X2
		inc esi
		add edi, 8 * 4
		test byte [esi], 0xFF
		jz short .End
		jmp [esp]

	.End
		add esp, 4
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN32
	
		;void Cell_8x8_Dump(unsigned char *Adr, int Palette)
		DECL Cell_8x8_Dump

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		xor eax, eax					; eax = 0
		mov ebp, [esp + 32]				; ebp = palette_number
		mov edx, 24						; edx = Number of rows of the pattern to be copied
		shl ebp, 5						; ebp = palette_number * 32
		test [_Bits32], byte 1
		mov esi, [esp + 28]				; esi = Address
		jnz	.32BIT
		lea edi, [MD_Screen	+ 6780]		; edi = MD_Screen + copy offset

	.Loop_EDX
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX
		push ebx
		mov ebx, [esi]
		AFF_PIXEL 0, 12
		AFF_PIXEL 1, 8
		AFF_PIXEL 2, 4
		AFF_PIXEL 3, 0
		AFF_PIXEL 4, 28
		AFF_PIXEL 5, 24
		AFF_PIXEL 6, 20
		AFF_PIXEL 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 2				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX				; then keep going

		sub edi, ((336 * 8) - 8) * 2	; we skip 8 rows from the top and 8 pixels from the left of Dest
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX				; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 2		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		dec edx
		jnz near .Loop_EDX
		jmp near .END
		
	.32BIT
		shl ebp, 1
		lea edi, [MD_Screen32 + 13560 ]	; edi = MD_Screen + copy offset

	.Loop_EDX32
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX32
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX32
		push ebx
		mov ebx, [esi]
		AFF_PIXEL32 0, 12
		AFF_PIXEL32 1, 8
		AFF_PIXEL32 2, 4
		AFF_PIXEL32 3, 0
		AFF_PIXEL32 4, 28
		AFF_PIXEL32 5, 24
		AFF_PIXEL32 6, 20
		AFF_PIXEL32 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 4				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX32				; then keep going

		sub edi, ((336 * 8) - 8) * 4	; we skip 8 rows from the top and 8 pixels from the left of Dest
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX32			; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		dec edx
		jnz near .Loop_EDX32
	.END
		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN32
	
		;void Cell_16x16_Dump(unsigned char *Adr, int Palette)
		DECL Cell_16x16_Dump

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		xor eax, eax					; eax = 0
		mov ebp, [esp + 32]				; ebp = palette_number
		mov edx, 12						; edx = Number of rows of the pattern to be copied
		shl ebp, 5						; ebp = palette_number * 32
		test [_Bits32], byte 1
		mov esi, [esp + 28]				; esi = Address
		jnz	near .32BIT
		lea edi, [MD_Screen	+ 6780]		; edi = MD_Screen + copy offset

	.Loop_EDX
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX
		push ebx
		mov ebx, [esi]
		AFF_PIXEL 0, 12
		AFF_PIXEL 1, 8
		AFF_PIXEL 2, 4
		AFF_PIXEL 3, 0
		AFF_PIXEL 4, 28
		AFF_PIXEL 5, 24
		AFF_PIXEL 6, 20
		AFF_PIXEL 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 2				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX				; then keep going

		sub edi, ((336 * 8) - 8) * 2	; we skip 8 rows from the top and 8 pixels from the left of Dest
		add esi, 0x20					; pattern à droite
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX				; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 2		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		sub esi, 0x400 - 0x20
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX_2
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX_2
		push ebx
		mov ebx, [esi]
		AFF_PIXEL 0, 12
		AFF_PIXEL 1, 8
		AFF_PIXEL 2, 4
		AFF_PIXEL 3, 0
		AFF_PIXEL 4, 28
		AFF_PIXEL 5, 24
		AFF_PIXEL 6, 20
		AFF_PIXEL 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 2				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX_2			; then keep going

		sub edi, ((336 * 8) - 8) * 2	; we skip 8 rows from the top and 8 pixels from the left of Dest
		add esi, 0x20					; pattern à droite
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX_2			; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 2		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		sub esi, 0x20
		dec edx
		jnz near .Loop_EDX
		jmp near .END
	.32BIT
		shl ebp, 1
		lea edi, [MD_Screen32 + 13560]	; edi = MD_Screen + copy offset

	.Loop_EDX32
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX32
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX32
		push ebx
		mov ebx, [esi]
		AFF_PIXEL32 0, 12
		AFF_PIXEL32 1, 8
		AFF_PIXEL32 2, 4
		AFF_PIXEL32 3, 0
		AFF_PIXEL32 4, 28
		AFF_PIXEL32 5, 24
		AFF_PIXEL32 6, 20
		AFF_PIXEL32 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 4				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX32			; then keep going

		sub edi, ((336 * 8) - 8) * 4	; we skip 8 rows from the top and 8 pixels from the left of Dest
		add esi, 0x20					; pattern à droite
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX32			; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		sub esi, 0x400 - 0x20
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX_232
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX_232
		push ebx
		mov ebx, [esi]
		AFF_PIXEL32 0, 12
		AFF_PIXEL32 1, 8
		AFF_PIXEL32 2, 4
		AFF_PIXEL32 3, 0
		AFF_PIXEL32 4, 28
		AFF_PIXEL32 5, 24
		AFF_PIXEL32 6, 20
		AFF_PIXEL32 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 4				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX_232			; then keep going

		sub edi, ((336 * 8) - 8) * 4	; we skip 8 rows from the top and 8 pixels from the left of Dest
		add esi, 0x20					; pattern à droite
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX_232			; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		sub esi, 0x20
		dec edx
		jnz near .Loop_EDX32

	.END
		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32
	
		;void Cell_32x32_Dump(unsigned char *Adr, int Palette)
		DECL Cell_32x32_Dump

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		xor eax, eax					; eax = 0
		mov ebp, [esp + 32]				; ebp = palette_number
		mov edx, 6						; edx = Number of rows of the pattern to be copied
		shl ebp, 5						; ebp = palette_number * 32
		test [_Bits32], byte 1
		mov esi, [esp + 28]				; esi = Address
		jnz	.32BIT
		lea edi, [MD_Screen	+ 6780]		; edi = MD_Screen + copy offset

	.Loop_EDX
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX
		mov ebx, 32						; ebx = Number of rows in each pattern

	.Loop_EBX
		push ebx
		mov ebx, [esi]
		AFF_PIXEL 0, 12
		AFF_PIXEL 1, 8
		AFF_PIXEL 2, 4
		AFF_PIXEL 3, 0
		AFF_PIXEL 4, 28
		AFF_PIXEL 5, 24
		AFF_PIXEL 6, 20
		AFF_PIXEL 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 2				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX				; then keep going

		sub edi, ((336 * 32) - 8) * 2	; we skip 8 rows from the top and 8 pixels from the left of Dest
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX				; then keep going

		add edi, ((336 * 32) - (8 * 16)) * 2		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		dec edx
		jnz near .Loop_EDX
		jmp near .END
		
	.32BIT
		shl ebp, 1
		lea edi, [MD_Screen32 + 13560 ]	; edi = MD_Screen + copy offset

	.Loop_EDX32
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX32
		mov ebx, 32						; ebx = Number of rows in each pattern

	.Loop_EBX32
		push ebx
		mov ebx, [esi]
		AFF_PIXEL32 0, 12
		AFF_PIXEL32 1, 8
		AFF_PIXEL32 2, 4
		AFF_PIXEL32 3, 0
		AFF_PIXEL32 4, 28
		AFF_PIXEL32 5, 24
		AFF_PIXEL32 6, 20
		AFF_PIXEL32 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 4				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX32				; then keep going

		sub edi, ((336 * 32) - 8) * 4	; we skip 8 rows from the top and 8 pixels from the left of Dest
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX32			; then keep going

		add edi, ((336 * 32) - (8 * 16)) * 4	; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		dec edx
		jnz near .Loop_EDX32
	.END
		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
		
	ALIGN32

	; void CDD_Export_Status(void)
	DECL CDD_Export_Status
		push ebx
		push ecx

		mov ax, [CDD.Status]
		mov bx, [CDD.Minute]
		mov cx, [CDD.Seconde]
		mov [CDD.Rcv_Status + 0], ax
		mov [CDD.Rcv_Status + 2], bx
		mov [CDD.Rcv_Status + 4], cx
		add al, bl
		add al, bh
		mov bx, [CDD.Frame]
		add al, ch
		add al, cl
		and byte [CDD.Control], 0x3
		add al, ah
		add al, bl
		mov ah, [CDD.Ext]
		add al, bh
		add al, ah
		mov [CDD.Rcv_Status + 6], bx
		not al
		and al, 0x0F
		mov [CDD.Rcv_Status + 8], ax

		pop ecx
		pop ebx
		ret

	ALIGN32


	; void Write_Sound_Mono_MMX(int *Left, int *Right, short *Dest, int length)
	DECL Write_Sound_Mono_MMX

		push ebx
		push ecx
		push edi
		push esi

		mov edi, [esp + 20]			; Left
		mov esi, [esp + 24]			; Right
		mov ecx, [esp + 32]			; Length
		mov ebx, [esp + 28]			; Dest

		shr ecx, 1
		jnc short .Double_Trans

	.Simple_Trans
		mov eax, [edi]
		mov dword [edi], 0
		add eax, [esi]
		mov dword [esi], 0

		cmp eax, 0xFFFF
		jle short .lower_s

		mov word [ebx], 0x7FFF
		jmp short .ok_s

	.lower_s
		cmp eax, -0xFFFF
		jge short .greater_s

		mov word [ebx], -0x7FFF
		jmp short .ok_s

	.greater_s
		shr eax, 1
		mov [ebx], ax

	.ok_s
		add edi, 4
		add esi, 4
		add ebx, 2

	.Double_Trans
		mov eax, 32
		pxor mm4, mm4
		movd mm5, eax
		mov eax, 1
		test ecx, ecx
		movd mm6, eax
		jnz short .Loop
		jmp short .End

	ALIGN32

	.Loop
		movq mm0, [edi]			; L2 | L1
		add ebx, 4
		movq [edi], mm4
		movq mm1, [esi]			; R2 | R1
		add edi, 8
		movq [esi], mm4
		packssdw mm0, mm0		; 0 | 0 | L2 | L1
		packssdw mm1, mm1		; 0 | 0 | R2 | R1
		psraw mm0, 1
		psraw mm1, 1
		add esi, 8
		paddw mm0, mm1			; 0 | 0 | R2 + L2 | R1 + L1
		dec ecx
		movd [ebx - 4], mm0
		jnz short .Loop

		emms

	.End
		pop esi
		pop edi
		pop ecx
		pop ebx
		ret


	ALIGN32

	; void Write_Sound_Stereo_MMX(int *Left, int *Right, short *Dest, int length)
	DECL Write_Sound_Stereo_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov edi, [esp + 24]			; Left
		mov esi, [esp + 28]			; Right
		mov ecx, [esp + 36]			; Length
		mov ebx, [esp + 32]			; Dest

		shr ecx, 1
		jnc short .Double_Trans

	.Simple_Trans
		mov eax, [edi]
		cmp eax, 0x7FFF
		mov dword [edi], 0
		jle short .lower_s1

		mov word [ebx + 0], 0x7FFF
		jmp short .right_s1

	.lower_s1
		cmp eax, -0x7FFF
		jge short .greater_s1

		mov word [ebx + 0], -0x7FFF
		jmp short .right_s1

	.greater_s1
		mov [ebx + 0], ax

	.right_s1
		mov edx, [esi]
		cmp edx, 0x7FFF
		mov dword [esi], 0
		jle short .lower_s2

		mov word [ebx + 2], 0x7FFF
		jmp short .ok_s1

	.lower_s2
		cmp edx, -0x7FFF
		jge short .greater_s2

		mov word [ebx + 2], -0x7FFF
		jmp short .ok_s1

	.greater_s2
		mov [ebx + 2], dx
		
	.ok_s1
		add edi, 4
		add esi, 4
		add ebx, 4

	.Double_Trans
		mov eax, 32
		pxor mm4, mm4
		test ecx, ecx
		movd mm5, eax
		jnz short .Loop
		jmp short .End

	ALIGN32

	.Loop
		movd mm0, [esi]			; 0  | R1
		add edi, 8
		movd mm1, [esi + 4]		; 0  | R2
		psllq mm0, mm5			; R1 |  0
		movq [esi], mm4
		psllq mm1, mm5			; R2 |  0
		movd mm2, [edi - 8]		; 0  | L1
		add esi, 8
		movd mm3, [edi - 8 + 4]	; 0  | L2
		add ebx, 8
		paddd mm0, mm2			; R1 | L1
		paddd mm1, mm3			; R2 | L2
		movq [edi - 8], mm4
		packssdw mm0, mm1		; R2 | L2 | R1 | L1

		dec ecx
		movq [ebx - 8], mm0
		jnz short .Loop

		emms

	.End
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
