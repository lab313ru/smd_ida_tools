%include "nasmhead.inc"

	srcPtr        equ 8
	srcPitch      equ 12
	width         equ 16
	dstOffset     equ 20
	dstPitch      equ 24
	dstSegment    equ 28

	colorI   equ -2
	colorE   equ 0
	colorF   equ 2
	colorJ   equ 4

	colorG   equ -2
	colorA   equ 0
	colorB   equ 2
	colorK   equ 4

	colorH   equ -2
	colorC   equ 0
	colorD   equ 2
	colorL   equ 4

	colorM   equ -2
	colorN   equ 0
	colorO   equ 2
	colorP   equ 4

section .data align=64

	extern MD_Screen
	extern TAB336
	extern Have_MMX
	extern Mode_555

	Var:	dd 0
	NB_X:	dd 0
	NB_X2:	dd 0
	NB_X4:	dd 0
	MASK_DIV2:		dd 0x7BCF7BCF, 0x7BCF7BCF
	MASK_DIV2_15:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16:	dd 0x7BEF7BEF, 0x7BEF7BEF
	MASK_DIV4_15:	dd 0x1CE71CE7, 0x1CEF1CE7
	MASK_DIV4_16:	dd 0x39E739E7, 0x39E739E7
	MASK_DIV8_15:	dd 0x0C630C63, 0x0C630C63
	MASK_DIV8_16:	dd 0x18E318E3, 0x18E318E3
	MASK_RBG_15:	dd 0x7C1F03E0, 0x7C1F03E0
	MASK_GRB_15:	dd 0x03E07C1F, 0x03E07C1F
	MASK_RBG_16:	dd 0xF81F07E0, 0xF81F07E0
	MASK_GRB_16:	dd 0x07E0F81F, 0x07E0F81F
	MASK_RBG_15_2:	dd 0x3C0F01E0, 0x3C0F01E0
	MASK_GRB_15_2:	dd 0x01E03C0F, 0x01E03C0F
	MASK_RBG_16_2:	dd 0x780F03E0, 0x780F03E0
	MASK_GRB_16_2:	dd 0x03E0780F, 0x03E0780F
	MASK_GG_15:		dd 0x03E003E0, 0x03E003E0
	MASK_RBRB_15:	dd 0x7C1F7C1F, 0x7C1F7C1F
	MASK_GG_16:		dd 0x07C007C0, 0x07C007C0
	MASK_RBRB_16	dd 0xF81FF81F, 0xF81FF81F

	; 2xSAI

	ALIGNB32

	colorMask:		dd 0xF7DEF7DE,0xF7DEF7DE
	lowPixelMask:	dd 0x08210821,0x08210821

	qcolorMask:		dd 0xE79CE79C,0xE79CE79C
	qlowpixelMask:	dd 0x18631863,0x18631863

	darkenMask:		dd 0xC718C718,0xC718C718
	GreenMask:		dd 0x07E007E0,0x07E007E0
	RedBlueMask:	dd 0xF81FF81F,0xF81FF81F

	FALSE:			dd 0x00000000,0x00000000
	TRUE:			dd 0xffffffff,0xffffffff
	ONE:			dd 0x00010001,0x00010001

	colorMask15		dd 0x7BDE7BDE,0x7BDE7BDE
	lowPixelMask15	dd 0x04210421,0x04210421

	qcolorMask15	dd 0x739C739C,0x739C739C
	qlowpixelMask15	dd 0x0C630C63,0x0C630C63

	darkenMask15	dd 0x63186318,0x63186318
	GreenMask15		dd 0x03E003E0,0x03E003E0
	RedBlueMask15	dd 0x7C1F7C1F,0x7C1F7C1F



section .bss align=64

	LineBuffer:	resb 32
	Mask1:		resb 8
	Mask2:		resb 8
	ACPixel:	resb 8

	Line1Int:	resb 640 * 2
	Line2Int:	resb 640 * 2
	Line1IntP:	resd 1
	Line2IntP:	resb 1

section .text align=64


	ALIGN64
	
	;************************************************************************
	; void Blit_X1(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_X1

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		add ecx, ecx					; ecx = Number of bytes per row
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 3						; we transfer 8 bytes in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				mov eax, [esi]			; we transfer 2 pixels at a time	
				mov edx, [esi + 4]		; we transfer 2 pixels at a time	
				add esi, 8
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 8
				dec ecx
				jnz .Loop_X
	
			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN64
	
	;************************************************************************
	; void Blit_X1_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_X1_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		add ecx, ecx					; ecx = Number of bytes per row
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 6						; we transfer 64 bytes in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				movq mm0, [esi]
				add edi, 64
				movq mm1, [esi + 8]
				movq mm2, [esi + 16]
				movq mm3, [esi + 24]
				movq mm4, [esi + 32]
				movq mm5, [esi + 40]
				movq mm6, [esi + 48]
				movq mm7, [esi + 56]
				movq [edi + 0 - 64], mm0
				add esi, 64
				movq [edi + 8 - 64], mm1
				movq [edi + 16 - 64], mm2
				movq [edi + 24 - 64], mm3
				movq [edi + 32 - 64], mm4
				movq [edi + 40 - 64], mm5
				movq [edi + 48 - 64], mm6
				dec ecx
				movq [edi + 56 - 64], mm7
				jnz .Loop_X
	
			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64

	;************************************************************************
	; void Blit_X2(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_X2

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 4						; we transfer 16 bytes in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				mov eax, [esi]					; we transfer 2 pixels at a time	
				add esi, 8
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 16
				mov eax, [esi - 4]				; we transfer 2 pixels at a time	
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				dec ecx
				mov [edi - 8], eax
				mov [edi - 4], edx
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			add edi, ebx				; add the remaining pitch to the destination
			shl ecx, 3
			sub esi, ecx
			shr ecx, 3
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				mov eax, [esi]					; we transfer 2 pixels at a time	
				add esi, 8
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 16
				mov eax, [esi - 4]				; we transfer 2 pixels at a time	
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				dec ecx
				mov [edi - 8], eax
				mov [edi - 4], edx
				jnz short .Loop_X2

			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN64

	;************************************************************************
	; void Blit_X2_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_X2_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 6						; we transfer 64 bytes in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add edi, byte 64
				movq mm2, [esi + 8]
				movq mm1, mm0
				movq mm4, [esi + 16]
				movq mm3, mm2
				movq mm6, [esi + 24]
				movq mm5, mm4
				movq mm7, mm6
				punpcklwd mm1, mm1
				punpckhwd mm0, mm0
				movq [edi + 0 - 64], mm1
				punpcklwd mm3, mm3
				movq [edi + 8 - 64], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 64], mm3
				punpcklwd mm5, mm5
				movq [edi + 24 - 64], mm2
				punpckhwd mm4, mm4
				movq [edi + 32 - 64], mm5
				punpcklwd mm7, mm7
				movq [edi + 40 - 64], mm4
				punpckhwd mm6, mm6
				movq [edi + 48 - 64], mm7
				add esi, byte 32
				dec ecx
				movq [edi + 56 - 64], mm6
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			add edi, ebx				; add the remaining pitch to the destination
			shl ecx, 5
			sub esi, ecx
			shr ecx, 5
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				movq mm0, [esi]
				add edi, byte 64
				movq mm2, [esi + 8]
				movq mm1, mm0
				movq mm4, [esi + 16]
				movq mm3, mm2
				movq mm6, [esi + 24]
				movq mm5, mm4
				movq mm7, mm6
				punpcklwd mm1, mm1
				punpckhwd mm0, mm0
				movq [edi + 0 - 64], mm1
				punpcklwd mm3, mm3
				movq [edi + 8 - 64], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 64], mm3
				punpcklwd mm5, mm5
				movq [edi + 24 - 64], mm2
				punpckhwd mm4, mm4
				movq [edi + 32 - 64], mm5
				punpcklwd mm7, mm7
				movq [edi + 40 - 64], mm4
				punpckhwd mm6, mm6
				movq [edi + 48 - 64], mm7
				add esi, byte 32
				dec ecx
				movq [edi + 56 - 64], mm6
				jnz short .Loop_X2

			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64
	
	;*********************************************************************************
	; void Blit_X2_Int(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_X2_Int

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		mov ecx, [esp + 36]				; ecx = Number of pixels per row
		mov ebx, [esp + 32]				; ebx = pitch of the Dest surface
		mov eax, Line1Int				; eax = offset Line 1 Int buffer
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		mov [Line1IntP], eax			; store first buffer addr
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		mov eax, Line2Int				; eax = offset Line 2 Int buffer
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 2						; we transfer 4 bytes in each loop
		mov [Line2IntP], eax			; store second buffer addr
		mov edi, [esp + 28]				; edi = Dest
		mov [esp + 32], ebx				; store "Adjust" offset for the following row
		mov [esp + 36], ecx				; store this new value for X 
		mov ebp, [Line1IntP]			; ebp = First Line buffer
		mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
		jmp short .First_Copy

	ALIGN4

	.First_Copy
	.Loop_X_FL
			mov ax, [esi]
			mov dx, [esi + 2]
			shr ax, 1
			add esi, byte 2
			shr dx, 1
			and ax, 0x7BCF
			and dx, 0x7BCF
			add ebp, byte 4
			add ax, dx
			dec ecx
			mov dx, [esi]
			mov [ebp + 0 - 4], ax
			mov [ebp + 2 - 4], dx
			jnz short .Loop_X_FL

		dec dword [esp + 40]			; one row less
		jz near .Last_Line

		mov ecx, [esp + 36]				; we transfer 4 Dest bytes per loop
		add esi, [esp + 44]				; increment source to point to the next row
		mov ebp, [Line2IntP]			; ebp = Second Line buffer
		mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				mov ax, [esi]
				mov dx, [esi + 2]
				shr ax, 1
				add esi, byte 2
				shr dx, 1
				and ax, 0x7BCF
				and dx, 0x7BCF
				add ebp, byte 4
				add ax, dx
				dec ecx
				mov dx, [esi]
				mov [ebp + 0 - 4], ax
				mov [ebp + 2 - 4], dx
				jnz short .Loop_X1

			mov ecx, [esp + 36]				; Dest num bytes / 4
			add esi, [esp + 44]				; increment source to point to the next row
			shr ecx, 1						; Dest num bytes / 8
			mov ebx, [Line1IntP]			; ebx = First Line buffer
			jmp short .Loop_X2

	ALIGN64

	.Loop_X2
				mov eax, [ebx]
				mov edx, [ebx + 4]
				add edi, byte 8
				add ebx, byte 8
				dec ecx
				mov [edi + 0 - 8], eax
				mov [edi + 4 - 8], edx
				jnz short .Loop_X2

			mov ecx, [esp + 36]				; Dest num bytes / 4
			add edi, [esp + 32]				; Next Dest line
			mov ebx, [Line1IntP]			; ebx = First Line buffer
			mov ebp, [Line2IntP]			; ebp = Second Line buffer
			jmp short .Loop_X3

	ALIGN64

	.Loop_X3
				mov eax, [ebx]
				mov edx, [ebp]
				shr eax, 1
				add edi, byte 4
				shr edx, 1
				and eax, 0x7BCF7BCF
				and edx, 0x7BCF7BCF
				add ebx, byte 4
				add eax, edx
				add ebp, byte 4
				dec ecx
				mov [edi - 4], eax
				jnz short .Loop_X3

			add edi, [esp + 32]				; Next Dest line
			mov ebx, [Line1IntP]			; Swap line buffer
			mov ebp, [Line2IntP]
			mov [Line2IntP], ebx
			mov [Line1IntP], ebp
			mov ecx, [esp + 36]				; we transfer 4 Dest bytes per loop
			dec dword [esp + 40]			; do any rows remain ?
			mov ebp, [Line2IntP]			; ebp = Second Line buffer
			mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
			jnz near .Loop_Y

	.Last_Line
		mov ecx, [esp + 36]				; Dest num bytes / 4
		mov ebx, [Line1IntP]			; ebx = First Line buffer
		shr ecx, 1						; Dest num bytes / 8
		jmp short .Loop_X1_LL

	ALIGN4

	.Loop_X1_LL
			mov eax, [ebx]
			mov edx, [ebx + 4]
			add edi, byte 8
			add ebx, byte 8
			dec ecx
			mov [edi + 0 - 8], eax
			mov [edi + 4 - 8], edx
			jnz short .Loop_X1_LL

		add edi, [esp + 32]				; Next Dest line
		mov ecx, [esp + 36]				; Dest num bytes / 4
		mov ebx, [Line1IntP]			; ebx = First Line buffer
		shr ecx, 1						; Dest num bytes / 8
		jmp short .Loop_X2_LL

	ALIGN4

	.Loop_X2_LL
			mov eax, [ebx]
			add edi, byte 8
			mov edx, [ebx + 4]
			shr eax, 1
			add ebx, byte 8
			shr edx, 1
			and eax, 0x7BCF7BCF
			and edx, 0x7BCF7BCF
			mov [edi + 0 - 8], eax
			dec ecx
			mov [edi + 4 - 8], edx
			jnz short .Loop_X2_LL

		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret




	ALIGN64
	
	;*********************************************************************************
	; void Blit_X2_Int_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_X2_Int_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		mov ecx, [esp + 36]				; ecx = Number of pixels per row
		mov ebx, [esp + 32]				; ebx = pitch of the Dest surface
		mov eax, Line1Int				; eax = offset Line 1 Int buffer
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		mov [Line1IntP], eax			; store first buffer addr
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		mov eax, Line2Int				; eax = offset Line 2 Int buffer
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 4						; we transfer 16 bytes in each loop
		mov [Line2IntP], eax			; store second buffer addr
		mov edi, [esp + 28]				; edi = Dest
		mov [esp + 32], ebx				; store "Adjust" offset for the following row
		mov [esp + 36], ecx				; store this new value for X 
		mov ebp, [Line1IntP]			; ebp = First Line buffer
		mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation

		test byte [Mode_555], 1			; set good mask for current video mode
		movq mm7, [MASK_DIV2_15]
		jnz short .First_Copy

		movq mm7, [MASK_DIV2_16]
		jmp short .First_Copy

	ALIGN4

	.First_Copy
	.Loop_X_FL
			movq mm0, [esi]
			add ebp, byte 16
			movq mm2, mm0
			movq mm1, [esi + 2]
			psrlw mm0, 1
			psrlw mm1, 1
			pand mm0, mm7
			pand mm1, mm7
			movq mm3, mm2
			paddw mm0, mm1
			add esi, byte 8
			punpcklwd mm2, mm0
			punpckhwd mm3, mm0
			movq [ebp + 0 - 16], mm2
			dec ecx
			movq [ebp + 8 - 16], mm3
			jnz short .Loop_X_FL

		dec dword [esp + 40]			; one row less
		jz near .Last_Line

		mov ecx, [esp + 36]				; we transfer 16 Dest bytes per loop
		add esi, [esp + 44]				; increment source to point to the next row
		mov ebp, [Line2IntP]			; ebp = Second Line buffer
		mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add ebp, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 1
				psrlw mm1, 1
				pand mm0, mm7
				pand mm1, mm7
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [ebp + 0 - 16], mm2
				dec ecx
				movq [ebp + 8 - 16], mm3
				jnz short .Loop_X1

			mov ecx, [esp + 36]				; Dest num bytes / 16
			add esi, [esp + 44]				; increment source to point to the next row
			mov ebx, [Line1IntP]			; ebx = First Line buffer
			jmp short .Loop_X2

	ALIGN64

	.Loop_X2
				movq mm0, [ebx]
				add edi, byte 16
				movq mm1, [ebx + 8]
				add ebx, byte 16
				movq [edi + 0 - 16], mm0
				dec ecx
				movq [edi + 8 - 16], mm1
				jnz short .Loop_X2

			mov ecx, [esp + 36]				; Dest num bytes / 16
			add edi, [esp + 32]				; Next Dest line
			mov ebx, [Line1IntP]			; ebx = First Line buffer
			mov ebp, [Line2IntP]			; ebp = Second Line buffer
			jmp short .Loop_X3

	ALIGN64

	.Loop_X3
				movq mm0, [ebx]
				add edi, byte 16
				movq mm1, [ebx + 8]
				psrlw mm0, 1
				movq mm2, [ebp]
				psrlw mm1, 1
				movq mm3, [ebp + 8]
				psrlw mm2, 1
				psrlw mm3, 1
				pand mm0, mm7
				pand mm1, mm7
				pand mm2, mm7
				pand mm3, mm7
				paddw mm0, mm2
				paddw mm1, mm3
				add ebx, byte 16
				movq [edi + 0 - 16], mm0
				add ebp, byte 16
				dec ecx
				movq [edi + 8 - 16], mm1
				jnz short .Loop_X3

			add edi, [esp + 32]				; Next Dest line
			mov ebx, [Line1IntP]			; Swap line buffer
			mov ebp, [Line2IntP]
			mov [Line2IntP], ebx
			mov [Line1IntP], ebp
			mov ecx, [esp + 36]				; we transfer 4 Dest bytes per loop
			dec dword [esp + 40]			; do any rows remain ?
			mov ebp, [Line2IntP]			; ebp = Second Line buffer
			mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
			jnz near .Loop_Y

	.Last_Line
		mov ecx, [esp + 36]				; Dest num bytes / 16
		mov ebx, [Line1IntP]			; ebx = First Line buffer
		jmp short .Loop_X1_LL

	ALIGN4

	.Loop_X1_LL
			movq mm0, [ebx]
			add edi, byte 16
			movq mm1, [ebx + 8]
			add ebx, byte 16
			movq [edi + 0 - 16], mm0
			dec ecx
			movq [edi + 8 - 16], mm1
			jnz short .Loop_X1_LL

		add edi, [esp + 32]				; Next Dest line
		mov ecx, [esp + 36]				; Dest num bytes / 16
		mov ebx, [Line1IntP]			; ebx = First Line buffer
		jmp short .Loop_X2_LL

	ALIGN4

	.Loop_X2_LL
			movq mm0, [ebx]
			add edi, byte 16
			movq mm1, [ebx + 8]
			psrlw mm0, 1
			add ebx, byte 16
			psrlw mm1, 1
			pand mm0, mm7
			pand mm1, mm7
			movq [edi + 0 - 16], mm0
			dec ecx
			movq [edi + 8 - 16], mm1
			jnz short .Loop_X2_LL

		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64

	;************************************************************************
	; void Blit_Scanline(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_Scanline

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		add ebx, ebx					; ebx = pitch * 2
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 4						; we transfer 16 bytes in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				mov eax, [esi]					; we transfer 2 pixels at a time	
				add esi, byte 4
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				add edi, byte 8
				mov eax, [esi]					; we transfer 2 pixels at a time	
				add esi, byte 4
				mov edx, eax
				rol eax, 16
				add edi, byte 8
				xchg ax, dx
				dec ecx
				mov [edi - 8], eax
				mov [edi - 4], edx
				jnz short .Loop_X
	
			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz short .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN64

	;************************************************************************
	; void Blit_Scanline_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_Scanline_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		add ebx, ebx					; ebx = pitch * 2
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 5						; we transfer 32 Dest bytes to in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		jnp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				movq mm0, [esi]
				add edi, byte 32
				movq mm2, [esi + 8]
				movq mm1, mm0
				movq mm3, mm2
				punpcklwd mm1, mm0
				add esi, byte 16
				punpckhwd mm0, mm0
				movq [edi + 0 - 32], mm1
				punpcklwd mm3, mm2
				movq [edi + 8 - 32], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 32], mm3
				dec ecx
				movq [edi + 24 - 32], mm2
				jnz short .Loop_X
	
			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]
			jnz short .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64

	;*******************************************************************************
	; void Blit_Scanline_50_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_Scanline_50_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 5						; we transfer 32 Dest bytes to in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		test byte [Mode_555], 1
		movq mm7, [MASK_DIV2_15]
		jnz short .Loop_Y

		movq mm7, [MASK_DIV2_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add edi, byte 32
				movq mm2, [esi + 8]
				movq mm1, mm0
				movq mm3, mm2
				punpcklwd mm1, mm0
				add esi, byte 16
				punpckhwd mm0, mm0
				movq [edi + 0 - 32], mm1
				punpcklwd mm3, mm2
				movq [edi + 8 - 32], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 32], mm3
				dec ecx
				movq [edi + 24 - 32], mm2
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]
			add edi, ebx				; add the remaining pitch to the destination
			shl ecx, 4
			sub esi, ecx
			shr ecx, 4
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				movq mm0, [esi]
				add edi, byte 32
				movq mm2, [esi + 8]
				psrlq mm0, 1
				psrlq mm2, 1
				pand mm0, mm7
				pand mm2, mm7
				movq mm1, mm0
				movq mm3, mm2
				punpcklwd mm1, mm1
				add esi, byte 16
				punpckhwd mm0, mm0
				movq [edi + 0 - 32], mm1
				punpcklwd mm3, mm3
				movq [edi + 8 - 32], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 32], mm3
				dec ecx
				movq [edi + 24 - 32], mm2
				jnz short .Loop_X2

			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]
			jnz near .Loop_Y

	.End
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64

	;*******************************************************************************
	; void Blit_Scanline_25_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_Scanline_25_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 5						; we transfer 32 Dest bytes to in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		test byte [Mode_555], 1
		movq mm6, [MASK_DIV2_15]
		movq mm7, [MASK_DIV4_15]
		jnz short .Loop_Y

		movq mm6, [MASK_DIV2_16]
		movq mm7, [MASK_DIV4_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add edi, byte 32
				movq mm2, [esi + 8]
				movq mm1, mm0
				movq mm3, mm2
				punpcklwd mm1, mm0
				add esi, byte 16
				punpckhwd mm0, mm0
				movq [edi + 0 - 32], mm1
				punpcklwd mm3, mm2
				movq [edi + 8 - 32], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 32], mm3
				dec ecx
				movq [edi + 24 - 32], mm2
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = Number of pixels / 32 per row
			add edi, ebx				; add the remaining pitch to the destination
			shl ecx, 4
			sub esi, ecx
			shr ecx, 4
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				movq mm0, [esi]
				add edi, byte 32
				movq mm3, [esi + 8]
				movq mm2, mm0
				movq mm5, mm3
				psrlq mm0, 2
				psrlq mm2, 1
				psrlq mm3, 2
				psrlq mm5, 1
				pand mm0, mm7
				pand mm2, mm6
				pand mm3, mm7
				pand mm5, mm6
				paddw mm0, mm2
				paddw mm3, mm5
				movq mm1, mm0
				movq mm4, mm3
				punpcklwd mm1, mm1
				add esi, byte 16
				punpckhwd mm0, mm0
				movq [edi + 0 - 32], mm1
				punpcklwd mm4, mm4
				movq [edi + 8 - 32], mm0
				punpckhwd mm3, mm3
				movq [edi + 16 - 32], mm4
				dec ecx
				movq [edi + 24 - 32], mm3
				jnz short .Loop_X2

			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64
	
	;*********************************************************************************
	; void Blit_Scanline_Int(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_Scanline_Int

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		add ebx, ebx					; ebx = pitch * 2
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 2						; we transfer 4 bytes in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
;				mov eax, [esi]
;				add esi, byte 2
;				shr eax, 1
;				mov edx, eax
;				and eax, 0x03E0780F
;				and edx, 0x780F03E0
;				rol eax, 16
;				add edi, byte 4
;				add eax, edx
;				mov dx, ax
;				shr eax, 16
;				or dx, ax
;				dec ecx
;				mov ax, [esi]
;				mov [edi + 0 - 4], dx
;				mov [edi + 2 - 4], ax
;				jnz short .Loop_X

				mov ax, [esi]
				mov dx, [esi + 2]
				shr ax, 1
				add esi, byte 2
				shr dx, 1
				and ax, 0x7BCF
				and dx, 0x7BCF
				add edi, byte 4
				add ax, dx
				dec ecx
				mov dx, [esi]
				mov [edi + 0 - 4], ax
				mov [edi + 2 - 4], dx
				jnz short .Loop_X

			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz short .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN64
	
	;*********************************************************************************
	; void Blit_Scanline_Int_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_Scanline_Int_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		add ebx, ebx					; ebx = pitch * 2
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 4						; we transfer 16 bytes in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 

		test byte [Mode_555], 1			; set good mask for current video mode
		movq mm7, [MASK_DIV2_15]
		jnz short .Loop_Y

		movq mm7, [MASK_DIV2_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				movq mm0, [esi]
				add edi, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 1
				psrlw mm1, 1
				pand mm0, mm7
				pand mm1, mm7
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [edi + 0 - 16], mm2
				dec ecx
				movq [edi + 8 - 16], mm3
				jnz short .Loop_X

			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz short .Loop_Y

		emms
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN64

	;*******************************************************************************
	; void Blit_Scanline_50_Int_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_Scanline_50_Int_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 4						; we transfer 32 Dest bytes to in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		test byte [Mode_555], 1
		movq mm6, [MASK_DIV2_15]
		movq mm7, [MASK_DIV4_15]
		jnz short .Loop_Y

		movq mm6, [MASK_DIV2_16]
		movq mm7, [MASK_DIV4_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add edi, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 1
				psrlw mm1, 1
				pand mm0, mm6
				pand mm1, mm6
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [edi + 0 - 16], mm2
				dec ecx
				movq [edi + 8 - 16], mm3
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]
			add edi, ebx				; add the remaining pitch to the destination
			shl ecx, 3
			sub esi, ecx
			shr ecx, 3
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				movq mm0, [esi]
				add edi, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 2
				psrlw mm2, 1
				psrlw mm1, 2
				pand mm0, mm7
				pand mm2, mm6
				pand mm1, mm7
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [edi + 0 - 16], mm2
				dec ecx
				movq [edi + 8 - 16], mm3
				jnz short .Loop_X2

			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]
			jnz near .Loop_Y

	.End
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64

	;*******************************************************************************
	; void Blit_Scanline_25_Int_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_Scanline_25_Int_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per row
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea ecx, [ecx * 4]				; ecx = Number of bytes per row Dest
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Adjust offset for the following row
		shr ecx, 4						; we transfer 32 Dest bytes to in each loop
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; store this new value for X 
		test byte [Mode_555], 1
		movq mm6, [MASK_DIV2_15]
		movq mm7, [MASK_DIV4_15]
		movq mm5, [MASK_DIV8_15]
		jnz short .Loop_Y

		movq mm6, [MASK_DIV2_16]
		movq mm7, [MASK_DIV4_16]
		movq mm5, [MASK_DIV8_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add edi, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 1
				psrlw mm1, 1
				pand mm0, mm6
				pand mm1, mm6
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [edi + 0 - 16], mm2
				dec ecx
				movq [edi + 8 - 16], mm3
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = Number of pixels / 32 per row
			add edi, ebx				; add the remaining pitch to the destination
			shl ecx, 3
			sub esi, ecx
			shr ecx, 3
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				movq mm0, [esi]
				add edi, byte 16
				movq mm1, [esi + 2]
				movq mm2, mm0
				movq mm3, mm1
				psrlq mm0, 2
				psrlq mm2, 1
				pand mm0, mm7
				pand mm2, mm6
				psrlq mm1, 3
				paddw mm0, mm2
				psrlq mm3, 2
				movq mm4, mm0
				pand mm1, mm5
				pand mm3, mm7
				psrlq mm0, 1
				paddw mm1, mm3
				pand mm0, mm6
				movq mm2, mm4
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm4, mm0
				punpckhwd mm2, mm0
				movq [edi + 0 - 16], mm4
				dec ecx
				movq [edi + 8 - 16], mm2
				jnz short .Loop_X2

			add esi, [esp + 40]			; increment source to point to the next row
			add edi, ebx				; add the remaining pitch to the destination
			dec dword [esp + 36]		; we keep going until there are no more rows
			mov ecx, [esp + 32]			; ecx = (Number of pixels / 4) in a row
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64
	
	;*************************************************************************
	;void Blit_2xSAI_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_2xSAI_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 36]				; ecx = Number of rows
		mov edx, [esp + 32]				; width
		mov ebx, [esp + 28]				; ebx = pitch of the Dest surface
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		mov edi, [esp + 24]				; edi = Destination
		test byte [Have_MMX], 0xFF		; check for MMX support
		jz near .End

		sub esp, byte 4 * 5				; 5 params for _2xSaILine
		test byte [Mode_555], 1
		mov [esp], esi					; 1st Param = *Src
		mov [esp + 4], dword (336 * 2)	; 2nd Param = SrcPitch
		mov [esp + 8], edx				; 3rd Param = width
		mov [esp + 12], edi				; 4th Param = *Dest
		mov [esp + 16], ebx				; 5th Param = DestPitch
		jz short .Loop

		movq mm0, [colorMask15]
		movq mm1, [lowPixelMask15]
		movq [colorMask], mm0
		movq [lowPixelMask], mm1
		movq mm0, [qcolorMask15]
		movq mm1, [qlowpixelMask15]
		movq [qcolorMask], mm0
		movq [qlowpixelMask], mm1
		movq mm0, [darkenMask15]
		movq mm1, [GreenMask15]
		movq mm2, [RedBlueMask15]
		movq [darkenMask], mm0
		movq [GreenMask], mm1
		movq [RedBlueMask], mm2
		jmp short .Loop

	ALIGN64

	.Loop
			mov word [esi + 320 * 2], 0		; clear clipping

			call _2xSaILine					; Do one line

			add esi, 336 * 2				; esi = *Src + 1 line
			lea edi, [edi + ebx * 2]		; edi = *Dest + 2 lines
			mov [esp], esi					; 1st Param = *Src
			dec ecx
			mov [esp + 12], edi				; 4th Param = *Dest
			jnz short .Loop

		add esp, byte 4 * 5					; Free 5 params

	.End
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64
	
	;***********************************************************************************************
	;void _2xSaILine(uint8 *srcPtr, uint32 srcPitch, uint32 width, uint8 *dstPtr, uint32 dstPitch);
	_2xSaILine:

		push ebp
		mov ebp, esp
		pushad

		mov edx, [ebp+dstOffset]		; edx points to the screen

		mov eax, [ebp+srcPtr]			; eax points to colorA
		mov ebx, [ebp+srcPitch]
		mov ecx, [ebp+width]
		
		sub eax, ebx					; eax now points to colorE

		pxor mm0, mm0
		movq [LineBuffer], mm0
		jmp short .Loop

	ALIGN64
	
	.Loop:
			push ecx

		;1	------------------------------------------

		;if ((colorA == colorD) && (colorB != colorC) && (colorA == colorE) && (colorB == colorL)

			movq mm0, [eax+ebx+colorA]        ;mm0 and mm1 contain colorA
			movq mm2, [eax+ebx+colorB]        ;mm2 and mm3 contain colorB

			movq mm1, mm0
			movq mm3, mm2

			pcmpeqw mm0, [eax+ebx+ebx+colorD]
			pcmpeqw mm1, [eax+colorE]
			pcmpeqw mm2, [eax+ebx+ebx+colorL]
			pcmpeqw mm3, [eax+ebx+ebx+colorC]

			pand mm0, mm1
			pxor mm1, mm1
			pand mm0, mm2
			pcmpeqw mm3, mm1
			pand mm0, mm3                 ;result in mm0

		;if ((colorA == colorC) && (colorB != colorE) && (colorA == colorF) && (colorB == colorJ)

			movq mm4, [eax+ebx+colorA]        ;mm4 and mm5 contain colorA
			movq mm6, [eax+ebx+colorB]        ;mm6 and mm7 contain colorB
			movq mm5, mm4
			movq mm7, mm6

			pcmpeqw mm4, [eax+ebx+ebx+colorC]
			pcmpeqw mm5, [eax+colorF]
			pcmpeqw mm6, [eax+colorJ]
			pcmpeqw mm7, [eax+colorE]

			pand mm4, mm5
			pxor mm5, mm5
			pand mm4, mm6
			pcmpeqw mm7, mm5
			pand mm4, mm7                 ;result in mm4

			por mm0, mm4                  ;combine the masks
			movq [Mask1], mm0

		;2	-------------------------------------------

         ;if ((colorB == colorC) && (colorA != colorD) && (colorB == colorF) && (colorA == colorH)

			movq mm0, [eax+ebx+colorB]        ;mm0 and mm1 contain colorB
			movq mm2, [eax+ebx+colorA]        ;mm2 and mm3 contain colorA
			movq mm1, mm0
			movq mm3, mm2

			pcmpeqw mm0, [eax+ebx+ebx+colorC]
			pcmpeqw mm1, [eax+colorF]
			pcmpeqw mm2, [eax+ebx+ebx+colorH]
			pcmpeqw mm3, [eax+ebx+ebx+colorD]

			pand mm0, mm1
			pxor mm1, mm1
			pand mm0, mm2
			pcmpeqw mm3, mm1
			pand mm0, mm3                 ;result in mm0

		;if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)

			movq mm4, [eax+ebx+colorB]        ;mm4 and mm5 contain colorB
			movq mm6, [eax+ebx+colorA]        ;mm6 and mm7 contain colorA
			movq mm5, mm4
			movq mm7, mm6

			pcmpeqw mm4, [eax+ebx+ebx+colorD]
			pcmpeqw mm5, [eax+colorE]
			pcmpeqw mm6, [eax+colorI]
			pcmpeqw mm7, [eax+colorF]

			pand mm4, mm5
			pxor mm5, mm5
			pand mm4, mm6
			pcmpeqw mm7, mm5
			pand mm4, mm7                 ;result in mm4

			por mm0, mm4                  ;combine the masks
			movq [Mask2], mm0


		;interpolate colorA and colorB

			movq mm0, [eax+ebx+colorA]
			movq mm1, [eax+ebx+colorB]

			movq mm2, mm0
			movq mm3, mm1

			pand mm0, [colorMask]
			pand mm1, [colorMask]

			psrlw mm0, 1
			psrlw mm1, 1

			pand mm3, [lowPixelMask]
			paddw mm0, mm1

			pand mm3, mm2
			paddw mm0, mm3                ;mm0 contains the interpolated values

		;assemble the pixels

			movq mm1, [eax+ebx+colorA]
			movq mm2, [eax+ebx+colorB]

			movq mm3, [Mask1]
			movq mm5, mm1
			movq mm4, [Mask2]
			movq mm6, mm1

			pand mm1, mm3
			por mm3, mm4
			pxor mm7, mm7
			pand mm2, mm4

			pcmpeqw mm3, mm7
			por mm1, mm2
			pand mm0, mm3

			por mm0, mm1

			punpcklwd mm5, mm0
			punpckhwd mm6, mm0
;			movq mm0, [eax+ebx+colorA+8] ;Only the first pixel is needed

			movq [edx], mm5
			movq [edx+8], mm6

		;3 Create the Nextline  -------------------

		;if ((colorA == colorD) && (colorB != colorC) && (colorA == colorG) && (colorC == colorO)

			movq mm0, [eax+ebx+colorA]			;mm0 and mm1 contain colorA
			movq mm2, [eax+ebx+ebx+colorC]		;mm2 and mm3 contain colorC
			movq mm1, mm0
			movq mm3, mm2

			push eax
			add eax, ebx
			pcmpeqw mm0, [eax+ebx+colorD]
			pcmpeqw mm1, [eax+colorG]
			pcmpeqw mm2, [eax+ebx+ebx+colorO]
			pcmpeqw mm3, [eax+colorB]
			pop eax

			pand mm0, mm1
			pxor mm1, mm1
			pand mm0, mm2
			pcmpeqw mm3, mm1
			pand mm0, mm3                 ;result in mm0

		;if ((colorA == colorB) && (colorG != colorC) && (colorA == colorH) && (colorC == colorM)

			movq mm4, [eax+ebx+colorA]			;mm4 and mm5 contain colorA
			movq mm6, [eax+ebx+ebx+colorC]		;mm6 and mm7 contain colorC
			movq mm5, mm4
			movq mm7, mm6

			push eax
			add eax, ebx
			pcmpeqw mm4, [eax+ebx+colorH]
			pcmpeqw mm5, [eax+colorB]
			pcmpeqw mm6, [eax+ebx+ebx+colorM]
			pcmpeqw mm7, [eax+colorG]
			pop eax

			pand mm4, mm5
			pxor mm5, mm5
			pand mm4, mm6
			pcmpeqw mm7, mm5
			pand mm4, mm7                 ;result in mm4

			por mm0, mm4                  ;combine the masks
			movq [Mask1], mm0

		;4  ----------------------------------------

		;if ((colorB == colorC) && (colorA != colorD) && (colorC == colorH) && (colorA == colorF)

			movq mm0, [eax+ebx+ebx+colorC]		;mm0 and mm1 contain colorC
			movq mm2, [eax+ebx+colorA]			;mm2 and mm3 contain colorA
			movq mm1, mm0
			movq mm3, mm2

			pcmpeqw mm0, [eax+ebx+colorB]
			pcmpeqw mm1, [eax+ebx+ebx+colorH]
			pcmpeqw mm2, [eax+colorF]
			pcmpeqw mm3, [eax+ebx+ebx+colorD]

			pand mm0, mm1
			pxor mm1, mm1
			pand mm0, mm2
			pcmpeqw mm3, mm1
			pand mm0, mm3                 ;result in mm0

		;if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)

			movq mm4, [eax+ebx+ebx+colorC]		;mm4 and mm5 contain colorC
			movq mm6, [eax+ebx+colorA]			;mm6 and mm7 contain colorA
			movq mm5, mm4
			movq mm7, mm6

			pcmpeqw mm4, [eax+ebx+ebx+colorD]
			pcmpeqw mm5, [eax+ebx+colorG]
			pcmpeqw mm6, [eax+colorI]
			pcmpeqw mm7, [eax+ebx+ebx+colorH]

			pand mm4, mm5
			pxor mm5, mm5
			pand mm4, mm6
			pcmpeqw mm7, mm5
			pand mm4, mm7                 ;result in mm4

			por mm0, mm4                  ;combine the masks
			movq [Mask2], mm0

		;----------------------------------------------

		;interpolate colorA and colorC

			movq mm0, [eax+ebx+colorA]
			movq mm1, [eax+ebx+ebx+colorC]

			movq mm2, mm0
			movq mm3, mm1

			pand mm0, [colorMask]
			pand mm1, [colorMask]

			psrlw mm0, 1
			psrlw mm1, 1

			pand mm3, [lowPixelMask]
			paddw mm0, mm1

			pand mm3, mm2
			paddw mm0, mm3                ;mm0 contains the interpolated values

		;-------------

		;assemble the pixels

			movq mm1, [eax+ebx+colorA]
			movq mm2, [eax+ebx+ebx+colorC]

			movq mm3, [Mask1]
			movq mm4, [Mask2]

			pand mm1, mm3
			pand mm2, mm4

			por mm3, mm4
			pxor mm7, mm7
			por mm1, mm2

			pcmpeqw mm3, mm7
			pand mm0, mm3
			por mm0, mm1
			movq [ACPixel], mm0

		;////////////////////////////////
		; Decide which "branch" to take
		;--------------------------------

			movq mm0, [eax+ebx+colorA]
			movq mm1, [eax+ebx+colorB]
			movq mm6, mm0
			movq mm7, mm1
			pcmpeqw mm0, [eax+ebx+ebx+colorD]
			pcmpeqw mm1, [eax+ebx+ebx+colorC]
			pcmpeqw mm6, mm7

			movq mm2, mm0
			movq mm3, mm0

			pand mm0, mm1       ;colorA == colorD && colorB == colorC
			pxor mm7, mm7

			pcmpeqw mm2, mm7
			pand mm6, mm0
			pand mm2, mm1       ;colorA != colorD && colorB == colorC

			pcmpeqw mm1, mm7

			pand mm1, mm3       ;colorA == colorD && colorB != colorC
			pxor mm0, mm6
			por mm1, mm6
			movq mm7, mm0
			movq [Mask2], mm2
			packsswb mm7, mm7
			movq [Mask1], mm1

			movd ecx, mm7
			test ecx, ecx
			jz near .SKIP_GUESS

		;-------------------------------------
		; Map of the pixels:           I|E F|J
		;                              G|A B|K
		;                              H|C D|L
		;                              M|N O|P

			movq mm6, mm0
			movq mm4, [eax+ebx+colorA]
			movq mm5, [eax+ebx+colorB]
			pxor mm7, mm7
			pand mm6, [ONE]

			movq mm0, [eax+colorE]
			movq mm1, [eax+ebx+colorG]
			movq mm2, mm0
			movq mm3, mm1
			pcmpeqw mm0, mm4
			pcmpeqw mm1, mm4
			pcmpeqw mm2, mm5
			pcmpeqw mm3, mm5
			pand mm0, mm6
			pand mm1, mm6
			pand mm2, mm6
			pand mm3, mm6
			paddw mm0, mm1
			paddw mm2, mm3

			pxor mm3, mm3
			pcmpgtw mm0, mm6
			pcmpgtw mm2, mm6
			pcmpeqw mm0, mm3
			pcmpeqw mm2, mm3
			pand mm0, mm6
			pand mm2, mm6
			paddw mm7, mm0
			psubw mm7, mm2

			movq mm0, [eax+colorF]
			movq mm1, [eax+ebx+colorK]
			movq mm2, mm0
			movq mm3, mm1
			pcmpeqw mm0, mm4
			pcmpeqw mm1, mm4
			pcmpeqw mm2, mm5
			pcmpeqw mm3, mm5
			pand mm0, mm6
			pand mm1, mm6
			pand mm2, mm6
			pand mm3, mm6
			paddw mm0, mm1
			paddw mm2, mm3

			pxor mm3, mm3
			pcmpgtw mm0, mm6
			pcmpgtw mm2, mm6
			pcmpeqw mm0, mm3
			pcmpeqw mm2, mm3
			pand mm0, mm6
			pand mm2, mm6
			paddw mm7, mm0
			psubw mm7, mm2

			push eax
			add eax, ebx
			movq mm0, [eax+ebx+colorH]
			movq mm1, [eax+ebx+ebx+colorN]
			movq mm2, mm0
			movq mm3, mm1
			pcmpeqw mm0, mm4
			pcmpeqw mm1, mm4
			pcmpeqw mm2, mm5
			pcmpeqw mm3, mm5
			pand mm0, mm6
			pand mm1, mm6
			pand mm2, mm6
			pand mm3, mm6
			paddw mm0, mm1
			paddw mm2, mm3

			pxor mm3, mm3
			pcmpgtw mm0, mm6
			pcmpgtw mm2, mm6
			pcmpeqw mm0, mm3
			pcmpeqw mm2, mm3
			pand mm0, mm6
			pand mm2, mm6
			paddw mm7, mm0
			psubw mm7, mm2

			movq mm0, [eax+ebx+colorL]
			movq mm1, [eax+ebx+ebx+colorO]
			movq mm2, mm0
			movq mm3, mm1
			pcmpeqw mm0, mm4
			pcmpeqw mm1, mm4
			pcmpeqw mm2, mm5
			pcmpeqw mm3, mm5
			pand mm0, mm6
			pand mm1, mm6
			pand mm2, mm6
			pand mm3, mm6
			paddw mm0, mm1
			paddw mm2, mm3

			pxor mm3, mm3
			pcmpgtw mm0, mm6
			pcmpgtw mm2, mm6
			pcmpeqw mm0, mm3
			pcmpeqw mm2, mm3
			pand mm0, mm6
			pand mm2, mm6
			paddw mm7, mm0
			psubw mm7, mm2

			pop eax
			movq mm1, mm7
			pxor mm0, mm0
			pcmpgtw mm7, mm0
			pcmpgtw mm0, mm1

			por mm7, [Mask1]
			por mm1, [Mask2]
			movq [Mask1], mm7
			movq [Mask2], mm1

		.SKIP_GUESS:

		;----------------------------
		;interpolate A, B, C and D

			movq mm0, [eax+ebx+colorA]
			movq mm1, [eax+ebx+colorB]
			movq mm4, mm0
			movq mm2, [eax+ebx+ebx+colorC]
			movq mm5, mm1
			movq mm3, [qcolorMask]
			movq mm6, mm2
			movq mm7, [qlowpixelMask]

			pand mm0, mm3
			pand mm1, mm3
			pand mm2, mm3
			pand mm3, [eax+ebx+ebx+colorD]

			psrlw mm0, 2
			pand mm4, mm7
			psrlw mm1, 2
			pand mm5, mm7
			psrlw mm2, 2
			pand mm6, mm7
			psrlw mm3, 2
			pand mm7, [eax+ebx+ebx+colorD]

			paddw mm0, mm1
			paddw mm2, mm3

			paddw mm4, mm5
			paddw mm6, mm7

			paddw mm4, mm6
			paddw mm0, mm2
			psrlw mm4, 2
			pand mm4, [qlowpixelMask]
			paddw mm0, mm4      ;mm0 contains the interpolated value of A, B, C and D

		;assemble the pixels

			movq mm1, [Mask1]
			movq mm2, [Mask2]
			movq mm4, [eax+ebx+colorA]
			movq mm5, [eax+ebx+colorB]
			pand mm4, mm1
			pand mm5, mm2

			pxor mm7, mm7
			por mm1, mm2
			por mm4, mm5
			pcmpeqw mm1, mm7
			pand mm0, mm1
			por mm4, mm0        ;mm4 contains the diagonal pixels

			movq mm0, [ACPixel]
			movq mm1, mm0
			punpcklwd mm0, mm4
			punpckhwd mm1, mm4

			push edx
			add edx, [ebp+dstPitch]

			movq [edx], mm0
			movq [edx+8], mm1

			pop edx

			add edx, 16
			add eax, 8

			pop ecx
			sub ecx, 4
			cmp ecx, 0
			jg  near .Loop

	; Restore some stuff

		popad
		mov esp, ebp
		pop ebp
		emms
		ret
