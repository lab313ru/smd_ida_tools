%include "nasmhead.inc"

%define _PWM_BUF_SIZE 4

; For some convenience, EBP register is never modified because assumed "alive"
; so we can access the current SH2 context by using EBP. 

section .data align=64

	extern VDP_Current_Line

section .bss align=64

%macro SH2_CONTEXT 0

		.Cache		resb 0x1000

		.R			resd 0x10

		.SR
		.SR_T		resb 1
		.SR_S		resb 1
		.SR_IMask	resb 1
		.SR_MQ		resb 1

		.INT
		.INT_Vect	resb 1
		.INT_Prio	resb 1
		.INT_res1	resb 1
		.INT_res2	resb 1

		.GBR		resd 1
		.VBR		resd 1

		.INT_QUEUE	resb 0x20

		.MACH		resd 1
		.MACL		resd 1
		.PR			resd 1
		.PC			resd 1

		.Status			resd 1
		.Base_PC		resd 1
		.Fetch_Start	resd 1
		.Fetch_End		resd 1

		.DS_Inst		resd 1
		.DS_PC			resd 1
		.Unused1		resd 1
		.Unused2		resd 1

		.Odometer		resd 1
		.Cycle_TD		resd 1
		.Cycle_IO		resd 1
		.Cycle_Sup		resd 1

		.Reset_Size

		.Read_Byte		resd 0x100
		.Read_Word		resd 0x100
		.Read_Long		resd 0x100

		.Write_Byte		resd 0x100
		.Write_Word		resd 0x100
		.Write_Long		resd 0x100

		.Fetch_Region	resd 0x100 * 3

		.IO_Reg		resb 0x200

		.DVCR		resd 1
		.DVSR		resd 1
		.DVDNTH		resd 1
		.DVDNTL		resd 1		; 4 dword

		.DRCR0		resb 1
		.DRCR1		resb 1
		.DREQ0		resb 1
		.DREQ1		resb 1

		.DMAOR		resd 1

		.SAR0		resd 1
		.DAR0		resd 1		; 4 dword
		.TCR0		resd 1
		.CHCR0		resd 1

		.SAR1		resd 1
		.DAR1		resd 1		; 4 dword
		.TCR1		resd 1
		.CHCR1		resd 1

		.VCRDIV		resd 1
		.VCRDMA0	resd 1		; 4 dword
		.VCRDMA1	resd 1
		.VCRWDT		resd 1

		.IPDIV		resd 1
		.IPDMA		resd 1		; 4 dword
		.IPWDT		resd 1
		.IPBSC		resd 1

		.BARA		resd 1
		.BAMRA		resd 1		; 4 dword

		.WDT_Tab    resb 8
		.WDTCNT		resd 1
		.WDT_Sft	resb 1
		.WDTSR		resb 1
		.WDTRST		resb 1
		.Unused3	resb 1		; 4 dword

		.FRT_Tab    resb 4
		.FRTCNT		resd 1
		.FRTOCRA	resd 1
		.FRTOCRB	resd 1		; 4 dword

		.FRTTIER	resb 1
		.FRTCSR		resb 1
		.FRTTCR		resb 1
		.FRTTOCR	resb 1
		.FRTICR		resd 1
		.FRT_Sft	resd 1
		.BCR1		resd 1		; 4 dword

		.Init_Size

%endmacro


	struc SH2

		SH2_CONTEXT

	endstruc
	extern M_SH2
	extern S_SH2

	extern _32X_Palette_16B
	extern _32X_Palette_32B
	extern _32X_VDP_Ram
	extern _32X_VDP_CRam
	extern _32X_VDP_CRam_Ajusted
	extern _32X_VDP_CRam_Ajusted32
	extern _32X_VDP

	extern _PWM_FIFO_R
	extern _PWM_FIFO_L
	extern _PWM_FULL_TAB
	extern _PWM_RP_R
	extern _PWM_WP_R
	extern _PWM_RP_L
	extern _PWM_WP_L
	extern _PWM_Mode

	extern _PWM_Cycle_Tmp
	extern _PWM_Int_Tmp
	extern _PWM_FIFO_L_Tmp
	extern _PWM_FIFO_R_Tmp


	struc vx
		.Mode		resd 1
		.State		resd 1
		.AF_Data	resd 1
		.AF_St		resd 1
		.AF_Len		resd 1
		.AF_Line	resd 1
	endstruc

	DECL _32X_Rom
	resb 4 * 1024 * 1024

	DECL _32X_Ram
	resb 256 * 1024

	DECL _32X_MSH2_Rom
	resb 2 * 1024

	DECL _32X_SSH2_Rom
	resb 1 * 1024

	ALIGNB32

	DECL _MSH2_Reg
	resb 0x40

	DECL _SSH2_Reg
	resb 0x40

	DECL _SH2_VDP_Reg
	resb 0x10

	DECL _32X_Comm
	resb 0x10

	DECL _32X_ADEN
	resb 1

	DECL _32X_RES
	resb 1

	DECL _32X_FM
	resb 1

	DECL _32X_RV
	resb 1

	ALIGN_32
	
	DECL _32X_FIFO_A
	resw 4

	DECL _32X_FIFO_B
	resw 4

	DECL _32X_FIFO_Block
	resd 1

	DECL _32X_FIFO_Read
	resd 1

	DECL _32X_FIFO_Write
	resd 1

	DECL _32X_DREQ_ST
	resd 1

	DECL _32X_DREQ_SRC
	resd 1

	DECL _32X_DREQ_DST
	resd 1

	DECL _32X_DREQ_LEN
	resd 1

	ALIGN_32

	DECL _32X_MINT
	resw 1

	DECL _32X_SINT
	resw 1

	DECL _32X_HIC
	resb 1

	DECL CPL_MSH2
	resd 1

	DECL CPL_SSH2
	resd 1

	DECL Cycles_MSH2
	resd 1

	DECL Cycles_SSH2
	resd 1


section .text align=64

	extern _Write_To_68K_Space
	extern __32X_Set_FB
	extern SH2_DMA0_Request
	extern @PWM_Set_Cycle@4
	extern @PWM_Set_Int@4

;*********************
;
;	READ FUNCTIONS
;
;	IN:
;	ecx = addresse
;
;	OUT:
;	eax = value
;   (big indian)
;
;*********************


;*********************
;	READ BYTE
;*********************


	DECLF MSH2_Read_Byte_00, 4
		test ecx, 0xFFC000
		jz short MSH2_Read_Byte_00_Rom
		test ecx, 0xFFBF00
		jnz short SH2_Read_Byte_VDP

		jmp MSH2_Read_Byte_32X_Reg


	ALIGN4

	MSH2_Read_Byte_00_Rom		
		and ecx, 0x07FF
		mov al, [_32X_MSH2_Rom + ecx]
		ret


	ALIGN4

	SH2_Read_Byte_VDP
		test byte [_32X_FM], 0xFF
		jz short SH2_RB_Bad
		test ecx, 0xFFBE00
		jnz short SH2_RB_Bad

		jmp SH2_Read_Byte_VDP_Reg


	ALIGN32

	DECLF SSH2_Read_Byte_00, 4
		test ecx, 0xFFC000
		jz short SSH2_Read_Byte_00_Rom
		test ecx, 0xFFBF00
		jnz short SH2_Read_Byte_VDP

		jmp SSH2_Read_Byte_32X_Reg


	ALIGN4

	SSH2_Read_Byte_00_Rom		
		and ecx, 0x03FF
		mov al, [_32X_SSH2_Rom + ecx]
		ret


	ALIGN32

	SH2_RB_Bad
		xor al, al
		ret


	ALIGN32
	
	DECLF SH2_Read_Byte_Rom, 4
		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x3FFFFF
		sub edx, byte 6
		mov al, [_32X_Rom + ecx]
		mov [ebp + SH2.Cycle_IO], edx
		ret


	ALIGN32
	
	DECLF SH2_Read_Byte_FB0, 4
		and ecx, 0x1FFFF
		mov edx, [ebp + SH2.Cycle_IO]
		xor ecx, byte 1
		sub edx, byte 4
		mov al, [_32X_VDP_Ram + ecx]
		mov [ebp + SH2.Cycle_IO], edx
		ret


	ALIGN32
	
	DECLF SH2_Read_Byte_FB1, 4
		and ecx, 0x1FFFF
		mov edx, [ebp + SH2.Cycle_IO]
		xor ecx, byte 1
		sub edx, byte 4
		mov al, [_32X_VDP_Ram + ecx + 0x20000]
		mov [ebp + SH2.Cycle_IO], edx
		ret


	ALIGN32
	
	DECLF SH2_Read_Byte_Ram, 4
		and ecx, 0x3FFFF
		mov al, [_32X_Ram + ecx]
		ret



;*********************
;	READ WORD
;*********************


	ALIGN32

	DECLF MSH2_Read_Word_00, 4
		test ecx, 0xFFC000
		jz short MSH2_Read_Word_00_Rom
		test ecx, 0xFFBF00
		jnz short SH2_Read_Word_VDP

		jmp MSH2_Read_Word_32X_Reg


	ALIGN4

	MSH2_Read_Word_00_Rom		
		and ecx, 0x07FE
		mov ah, [_32X_MSH2_Rom + ecx]
		mov al, [_32X_MSH2_Rom + ecx + 1]
		ret


	ALIGN4

	SH2_Read_Word_VDP
		test byte [_32X_FM], 0xFF
		jz short SH2_RW_Bad
		test ecx, 0xFFBE00
		jnz short SH2_Read_Word_VDP_Palette

		jmp SH2_Read_Word_VDP_Reg


	ALIGN32

	DECLF SSH2_Read_Word_00, 4
		test ecx, 0xFFC000
		jz short SSH2_Read_Word_00_Rom
		test ecx, 0xFFBF00
		jnz short SH2_Read_Word_VDP

		jmp SSH2_Read_Word_32X_Reg


	ALIGN4

	SSH2_Read_Word_00_Rom		
		and ecx, 0x03FE
		mov ah, [_32X_SSH2_Rom + ecx]
		mov al, [_32X_SSH2_Rom + ecx + 1]
		ret


	ALIGN4

	SH2_RW_Bad
		xor ax, ax
		ret


	ALIGN32

	SH2_Read_Word_VDP_Palette
		test ecx, 0xFFBC01
		jnz short SH2_RW_Bad

		and ecx, 0x1FE
		mov ax, [_32X_VDP_CRam + ecx]
		ret


	ALIGN32
	
	DECLF SH2_Read_Word_Rom, 4
		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x3FFFFE
		sub edx, byte 8
		mov ah, [_32X_Rom + ecx + 0]
		mov al, [_32X_Rom + ecx + 1]
		mov [ebp + SH2.Cycle_IO], edx
		ret


	ALIGN32
	
	DECLF SH2_Read_Word_FB0, 4
		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x1FFFE
		sub edx, byte 5
		mov ax, [_32X_VDP_Ram + ecx]
		mov [ebp + SH2.Cycle_IO], edx
		ret


	ALIGN32
	
	DECLF SH2_Read_Word_FB1, 4
		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x1FFFE
		sub edx, byte 5
		mov ax, [_32X_VDP_Ram + ecx + 0x20000]
		mov [ebp + SH2.Cycle_IO], edx
		ret


	ALIGN32
	
	DECLF SH2_Read_Word_Ram, 4
		and ecx, 0x3FFFE
		mov ah, [_32X_Ram + ecx + 0]
		mov al, [_32X_Ram + ecx + 1]
		ret



;*********************
;	READ LONG
;*********************


	ALIGN32

	DECLF MSH2_Read_Long_00, 4
		test ecx, 0xFFC000
		jz short MSH2_Read_Long_00_Rom
		test ecx, 0xFFBF00
		jnz short SH2_Read_Long_VDP_Reg

		; should replace that by a real LONG read function !

		push ecx
		add ecx, byte 2
		call MSH2_Read_Word_32X_Reg
		pop ecx
		push ax
		call MSH2_Read_Word_32X_Reg
		shl eax, 16
		pop ax
		ret


	ALIGN4

	MSH2_Read_Long_00_Rom
		and ecx, 0x07FC
		mov eax, [_32X_MSH2_Rom + ecx]
		bswap eax
		ret


	ALIGN32

	SH2_Read_Long_VDP_Reg
		test byte [_32X_FM], 0xFF
		jz short SH2_RL_Bad
		test ecx, 0xFFBE00
		jnz short SH2_Read_Long_VDP_Palette

		push ecx
		add ecx, byte 2
		call SH2_Read_Word_VDP_Reg
		pop ecx
		push ax
		call SH2_Read_Word_VDP_Reg
		shl eax, 16
		pop ax
		ret

	ALIGN4

	SH2_Read_Long_VDP_Palette
		test ecx, 0xFFBC03
		jnz short SH2_RL_Bad

		and ecx, 0x1FC
		mov eax, [_32X_VDP_CRam + ecx]
		rol eax, 16
		ret


	ALIGN32

	DECLF SSH2_Read_Long_00, 4
		test ecx, 0xFFC000
		jz short SSH2_Read_Long_00_Rom
		test ecx, 0xFFBF00
		jnz short SH2_Read_Long_VDP_Reg

		push ecx
		add ecx, byte 2
		call SSH2_Read_Word_32X_Reg
		pop ecx
		push ax
		call SSH2_Read_Word_32X_Reg
		shl eax, 16
		pop ax
		ret


	ALIGN4

	SSH2_Read_Long_00_Rom		
		and ecx, 0x03FC
		mov eax, [_32X_SSH2_Rom + ecx]
		bswap eax
		ret


	ALIGN32

	SH2_RL_Bad
		xor eax, eax
		ret


	ALIGN32
	
	DECLF SH2_Read_Long_Rom, 4
		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x3FFFFC
		sub edx, byte 8
		mov eax, [_32X_Rom + ecx]
		mov [ebp + SH2.Cycle_IO], edx
		bswap eax
		ret


	ALIGN32
	
	DECLF SH2_Read_Long_FB0, 4
		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x1FFFC
		sub edx, byte 5
		mov eax, [_32X_VDP_Ram + ecx]
		mov [ebp + SH2.Cycle_IO], edx
		rol eax, 16
		ret


	ALIGN32
	
	DECLF SH2_Read_Long_FB1, 4
		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x1FFFC
		sub edx, byte 5
		mov eax, [_32X_VDP_Ram + ecx + 0x20000]
		mov [ebp + SH2.Cycle_IO], edx
		rol eax, 16
		ret


	ALIGN32
	
	DECLF SH2_Read_Long_Ram, 4
		and ecx, 0x3FFFC
		mov eax, [_32X_Ram + ecx]
		bswap eax
		ret




;*********************
;
;	WRITE FUNCTIONS
;
;	IN:
;	ecx = addresse
;	edx = value
;
;	OUT:
;	nothing
;
;*********************


;*********************
;	WRITE BYTE
;*********************


	ALIGN32

	DECLF MSH2_Write_Byte_00, 8
		test ecx, 0xFFC000
		jz short SH2_WB_Bad
		test ecx, 0xFFBF00
		jnz short SH2_Write_Byte_VDP

		jmp MSH2_Write_Byte_32X_Reg


	ALIGN4
	
	SH2_Write_Byte_VDP
		test byte [_32X_FM], 0xFF
		jz short SH2_WB_Bad
		test ecx, 0xFFBE00
		jnz short SH2_WB_Bad

		jmp SH2_Write_Byte_VDP_Reg


	ALIGN32

	SH2_WB_Bad
		ret


	ALIGN32

	DECLF SSH2_Write_Byte_00, 8
		test ecx, 0xFFC000
		jz short SH2_WB_Bad
		test ecx, 0xFFBF00
		jnz short SH2_Write_Byte_VDP

		jmp SSH2_Write_Byte_32X_Reg


	ALIGN32
	
	DECLF SH2_Write_Byte_FB0, 8
		and ecx, 0x1FFFF
		test dl, dl
		jz short .blank

		xor ecx, byte 1
		mov [_32X_VDP_Ram + ecx], dl

	.blank
		ret


	ALIGN32
	
	DECLF SH2_Write_Byte_FB1, 8
		and ecx, 0x1FFFF
		test dl, dl
		jz short .blank

		xor ecx, byte 1
		mov [_32X_VDP_Ram + ecx + 0x20000], dl

	.blank
		ret


	ALIGN32
	
	DECLF SH2_Write_Byte_Ram, 8
		and ecx, 0x3FFFF
		mov [_32X_Ram + ecx], dl
		ret



;*********************
;	WRITE WORD
;*********************


	ALIGN32

	DECLF MSH2_Write_Word_00, 8
		test ecx, 0xFFC000
		jz short SH2_WW_Bad
		test ecx, 0xFFBF00
		jnz short SH2_Write_Word_VDP

		jmp MSH2_Write_Word_32X_Reg


	ALIGN4
	
	SH2_Write_Word_VDP
		test byte [_32X_FM], 0xFF
		jz short SH2_WW_Bad
		test ecx, 0xFFBE00
		jnz short SH2_Write_Word_VDP_Palette

		jmp SH2_Write_Word_VDP_Reg


	ALIGN4

	SH2_Write_Word_VDP_Palette
		test ecx, 0xFFBC01
		jnz short SH2_WW_Bad
		

		and edx, 0xFFFF
		and ecx, 0x1FE
		mov ax, [_32X_Palette_16B + edx * 2]
		mov ebx, [_32X_Palette_32B + edx * 4]
		mov [_32X_VDP_CRam + ecx], dx
		mov [_32X_VDP_CRam_Ajusted + ecx], ax
		mov [_32X_VDP_CRam_Ajusted32 + ecx * 2], ebx
		ret


	ALIGN32

	SH2_WW_Bad
		ret


	ALIGN32

	DECLF SSH2_Write_Word_00, 8
		test ecx, 0xFFC000
		jz short SH2_WW_Bad
		test ecx, 0xFFBF00
		jnz near SH2_Write_Word_VDP

		jmp SSH2_Write_Word_32X_Reg


	ALIGN32
	
	DECLF SH2_Write_Word_FB0, 8
		and ecx, 0x3FFFE
		test ecx, 0x20000
		jnz short .overwrite

		mov [_32X_VDP_Ram + ecx], dx
		ret

	ALIGN4

	.overwrite
		test dl, dl
		jz short .blank1

		mov [_32X_VDP_Ram + ecx - 0x20000 + 0], dl

	.blank1
		test dh, dh
		jz short .blank2

		mov [_32X_VDP_Ram + ecx - 0x20000 + 1], dh

	.blank2
		ret

	ALIGN32
	
	DECLF SH2_Write_Word_FB1, 8
		and ecx, 0x3FFFE
		test ecx, 0x20000
		jnz short .overwrite

		mov [_32X_VDP_Ram + ecx + 0x20000], dx
		ret

	ALIGN4

	.overwrite
		test dl, dl
		jz short .blank1

		mov [_32X_VDP_Ram + ecx - 0x20000 + 0x20000 + 0], dl

	.blank1
		test dh, dh
		jz short .blank2

		mov [_32X_VDP_Ram + ecx - 0x20000 + 0x20000 + 1], dh

	.blank2
		ret


	ALIGN32
	
	DECLF SH2_Write_Word_Ram, 8
		and ecx, 0x3FFFE
		mov [_32X_Ram + ecx + 0], dh
		mov [_32X_Ram + ecx + 1], dl
		ret



;*********************
;	WRITE LONG
;*********************


	ALIGN32

	DECLF MSH2_Write_Long_00, 8
		test ecx, 0xFFC000
		jz short SH2_WL_Bad
		test ecx, 0xFFBF00
		jnz short SH2_Write_Long_VDP_Reg

		push ecx
		push edx
		add ecx, byte 2
		call MSH2_Write_Word_32X_Reg
		pop edx
		pop ecx
		shr edx, 16
		jmp MSH2_Write_Word_32X_Reg


	ALIGN32

		DECLF SSH2_Write_Long_00, 8
		test ecx, 0xFFC000
		jz near SH2_WL_Bad
		test ecx, 0xFFBF00
		jnz short SH2_Write_Long_VDP_Reg

		push ecx
		push edx
		add ecx, byte 2
		call SSH2_Write_Word_32X_Reg
		pop edx
		pop ecx
		shr edx, 16
		jmp SSH2_Write_Word_32X_Reg


	ALIGN4

	SH2_WL_Bad
		ret


	ALIGN32

	SH2_Write_Long_VDP_Reg
		test byte [_32X_FM], 0xFF
		jz short SH2_WL_Bad
		test ecx, 0xFFBE00
		jnz short SH2_Write_Long_VDP_Palette

		push ecx
		push edx
		add ecx, byte 2
		call SH2_Write_Word_VDP_Reg
		pop edx
		pop ecx
		shr edx, 16
		jmp SH2_Write_Word_VDP_Reg


	ALIGN4

	SH2_Write_Long_VDP_Palette
		test ecx, 0xFFBC03
		jnz short SH2_WL_Bad

		
		mov eax, edx
		and ecx, 0x1FC
		rol eax, 16
		and edx, 0xFFFF
		mov [_32X_VDP_CRam + ecx], eax
		mov dx, [_32X_Palette_16B + edx * 2]
		and eax, 0xFFFF
		mov [_32X_VDP_CRam_Ajusted + ecx + 2], dx
		mov ax, [_32X_Palette_16B + eax * 2]
		mov [_32X_VDP_CRam_Ajusted + ecx + 0], ax
		
		mov eax, [_32X_VDP_CRam + ecx]
		mov edx, eax
		shl ecx, 1
		rol	edx, 16
		and eax, 0xFFFF
		and edx, 0xFFFF
		mov eax, [_32X_Palette_32B + eax * 4]
		mov edx, [_32X_Palette_32B + edx * 4]
		mov [_32X_VDP_CRam_Ajusted32 + ecx + 0], eax
		mov [_32X_VDP_CRam_Ajusted32 + ecx + 4], edx
		
		ret


	ALIGN32
	
	DECLF SH2_Write_Long_FB0, 8
		mov eax, edx
		and ecx, 0x3FFFC
		rol edx, 16
		test ecx, 0x20000
		jnz short .overwrite

		mov [_32X_VDP_Ram + ecx], edx
		ret

	ALIGN4

	.overwrite
		test dl, dl
		jz short .blank1

		mov [_32X_VDP_Ram + ecx - 0x20000 + 0], dl

	.blank1
		test dh, dh
		jz short .blank2

		mov [_32X_VDP_Ram + ecx - 0x20000 + 1], dh

	.blank2
		test al, al
		jz short .blank3

		mov [_32X_VDP_Ram + ecx - 0x20000 + 2], al

	.blank3
		test ah, ah
		jz short .blank4

		mov [_32X_VDP_Ram + ecx - 0x20000 + 3], ah

	.blank4
		ret


	ALIGN32
	
	DECLF SH2_Write_Long_FB1, 8
		mov eax, edx
		and ecx, 0x3FFFC
		rol edx, 16
		test ecx, 0x20000
		jnz short .overwrite

		mov [_32X_VDP_Ram + ecx + 0x20000], edx
		ret

	ALIGN4

	.overwrite
		test dl, dl
		jz short .blank1

		mov [_32X_VDP_Ram + ecx - 0x20000 + 0x20000 + 0], dl

	.blank1
		test dh, dh
		jz short .blank2

		mov [_32X_VDP_Ram + ecx - 0x20000 + 0x20000 + 1], dh

	.blank2
		test al, al
		jz short .blank3

		mov [_32X_VDP_Ram + ecx - 0x20000 + 0x20000 + 2], al

	.blank3
		test ah, ah
		jz short .blank4

		mov [_32X_VDP_Ram + ecx - 0x20000 + 0x20000 + 3], ah

	.blank4
		ret


	ALIGN32
	
	DECLF SH2_Write_Long_Ram, 8
		and ecx, 0x3FFFC
		bswap edx
		mov [_32X_Ram + ecx], edx
		ret
















;**************************************
;
; Function for register read / write
;
;**************************************


; READ BYTE
;**********


	ALIGN32

	MSH2_Read_Byte_32X_Reg

		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x3F
		sub edx, byte 10
		mov [ebp + SH2.Cycle_IO], edx
		jmp [.Table_MSH2_Reg + ecx * 4]

	ALIGN4

	.Table_MSH2_Reg
		dd MSH2_RB_IntC_H, MSH2_RB_IntC_L, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_HCnt, SH2_RB_DREQ_H, SH2_RB_DREQ_L
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad

		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad

		dd SH2_RB_Com, SH2_RB_Com, SH2_RB_Com, SH2_RB_Com
		dd SH2_RB_Com, SH2_RB_Com, SH2_RB_Com, SH2_RB_Com
		dd SH2_RB_Com, SH2_RB_Com, SH2_RB_Com, SH2_RB_Com
		dd SH2_RB_Com, SH2_RB_Com, SH2_RB_Com, SH2_RB_Com

		dd SH2_RB__PWM_Cont_H, SH2_RB__PWM_Cont_L
		dd SH2_RB__PWM_Cycle_H, SH2_RB__PWM_Cycle_L
		dd SH2_RB__PWM_Pulse_L, SH2_RB_Bad
		dd SH2_RB__PWM_Pulse_R, SH2_RB_Bad
		dd SH2_RB__PWM_Pulse_L, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad

	ALIGN32

	MSH2_RB_IntC_H
	SSH2_RB_IntC_H
		mov al, [_32X_ADEN]
		mov ah, [_32X_FM]
		add al, al
		or al, ah
		ret

	ALIGN32

	MSH2_RB_IntC_L
		mov al, [_32X_MINT]
		ret

	ALIGN32

	SH2_RB_HCnt
		mov al, [_32X_HIC]
		ret

	ALIGN32

	SH2_RB_DREQ_H
		mov al, [_32X_DREQ_ST + 1]
		ret

	ALIGN32

	SH2_RB_DREQ_L
		mov al, [_32X_RV]
		mov ah, [_32X_DREQ_ST]
		or al, ah
		ret

	ALIGN32

	SH2_RB_Com
		mov al, [_32X_Comm + ecx - 0x20]
		ret

	ALIGN32
	
	SH2_RB__PWM_Cont_H
		mov al, [_PWM_Mode + 1]
		ret 

	ALIGN4
	
	SH2_RB__PWM_Cont_L
		mov al, [_PWM_Mode + 0]
		ret 

	ALIGN4
	
	SH2_RB__PWM_Cycle_H
		mov al, [_PWM_Cycle_Tmp + 1]
		ret

	ALIGN4
	
	SH2_RB__PWM_Cycle_L
		mov al, [_PWM_Cycle_Tmp + 0]
		ret

	ALIGN32

	SH2_RB__PWM_Pulse_L
		mov ecx, [_PWM_RP_L]
		mov edx, [_PWM_WP_L]
		mov al, [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + edx]
		ret

	ALIGN4

	SH2_RB__PWM_Pulse_R
		mov ecx, [_PWM_RP_R]
		mov edx, [_PWM_WP_R]
		mov al, [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + edx]
		ret

	ALIGN4

	SH2_RB__PWM_Pulse_C
		mov ecx, [_PWM_RP_L]
		mov edx, [_PWM_WP_L]
		mov al, [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + edx]
		ret


	ALIGN32

	SSH2_Read_Byte_32X_Reg

		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x3F
		sub edx, byte 10
		mov [ebp + SH2.Cycle_IO], edx
		jmp [.Table_SSH2_Reg + ecx * 4]

	ALIGN4

	.Table_SSH2_Reg
		dd SSH2_RB_IntC_H, SSH2_RB_IntC_L, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_HCnt, SH2_RB_DREQ_H, SH2_RB_DREQ_L
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad

		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad

		dd SH2_RB_Com, SH2_RB_Com, SH2_RB_Com, SH2_RB_Com
		dd SH2_RB_Com, SH2_RB_Com, SH2_RB_Com, SH2_RB_Com
		dd SH2_RB_Com, SH2_RB_Com, SH2_RB_Com, SH2_RB_Com
		dd SH2_RB_Com, SH2_RB_Com, SH2_RB_Com, SH2_RB_Com

		dd SH2_RB__PWM_Cont_H, SH2_RB__PWM_Cont_L
		dd SH2_RB__PWM_Cycle_H, SH2_RB__PWM_Cycle_L
		dd SH2_RB__PWM_Pulse_L, SH2_RB_Bad
		dd SH2_RB__PWM_Pulse_R, SH2_RB_Bad
		dd SH2_RB__PWM_Pulse_L, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad

	ALIGN32

	SSH2_RB_IntC_L
		mov al, [_32X_SINT]
		ret


	ALIGN32

	SH2_Read_Byte_VDP_Reg

		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0xF
		sub edx, byte 3
		mov [ebp + SH2.Cycle_IO], edx
		jmp [.Table_VDP_Reg + ecx * 4]

	ALIGN4

	.Table_VDP_Reg
		dd SH2_RB_VDP_Mode_H, SH2_RB_VDP_Mode_L, SH2_RB_Bad, SH2_RB_VDP_Shift
		dd SH2_RB_Bad, SH2_RB_VDP_AF_Len, SH2_RB_VDP_AF_St_H, SH2_RB_VDP_AF_St_L
		dd SH2_RB_VDP_AF_Data_H, SH2_RB_VDP_AF_Data_L, SH2_RB_VDP_State_H, SH2_RB_VDP_State_L
		dd SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad, SH2_RB_Bad


	ALIGN32

	SH2_RB_VDP_Mode_H
		mov al, [_32X_VDP + vx.Mode + 1]
		ret

	ALIGN4

	SH2_RB_VDP_Mode_L
		mov al, [_32X_VDP + vx.Mode + 0]
		ret

	ALIGN4

	SH2_RB_VDP_Shift
		mov al, [_32X_VDP + vx.Mode + 2]
		ret

	ALIGN4

	SH2_RB_VDP_AF_Len
		mov al, [_32X_VDP + vx.AF_Len + 0]
		ret

	ALIGN4

	SH2_RB_VDP_AF_St_H
		mov al, [_32X_VDP + vx.AF_St + 1]
		ret

	ALIGN4

	SH2_RB_VDP_AF_St_L
		mov al, [_32X_VDP + vx.AF_St + 0]
		ret

	ALIGN4

	SH2_RB_VDP_AF_Data_H
		mov al, [_32X_VDP + vx.AF_Data + 1]
		ret

	ALIGN4
	
	SH2_RB_VDP_AF_Data_L
		mov al, [_32X_VDP + vx.AF_Data + 0]
		ret

	ALIGN32
	
	SH2_RB_VDP_State_H
		mov al, [_32X_VDP + vx.State + 1]
		ret

	ALIGN4
	
	SH2_RB_VDP_State_L
		mov al, [_32X_VDP + vx.State]
		xor al, 2
		mov [_32X_VDP + vx.State], al
		ret



; READ WORD
;**********


	ALIGN32

	MSH2_Read_Word_32X_Reg

		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x3E
		sub edx, byte 16
		mov [ebp + SH2.Cycle_IO], edx
		jmp [.Table_MSH2_Reg + ecx * 2]

	ALIGN4

	.Table_MSH2_Reg
		dd MSH2_RW_IntC, SH2_RW_Bad, SH2_RW_HCnt, SH2_RW_DREQ
		dd SH2_RW_DREQ_Src_H, SH2_RW_DREQ_Src_L, SH2_RW_DREQ_Dst_H, SH2_RW_DREQ_Dst_L

		dd SH2_RW_DREQ_Len, SH2_RW_FIFO, SH2_RW_Bad, SH2_RW_Bad
		dd SH2_RW_Bad, SH2_RW_Bad, SH2_RW_Bad, SH2_RW_Bad

		dd SH2_RW_Com, SH2_RW_Com, SH2_RW_Com, SH2_RW_Com
		dd SH2_RW_Com, SH2_RW_Com, SH2_RW_Com, SH2_RW_Com

		dd SH2_RW__PWM_Cont, SH2_RW__PWM_Cycle
		dd SH2_RW__PWM_Pulse_L, SH2_RW__PWM_Pulse_R
		dd SH2_RW__PWM_Pulse_C, SH2_RW_Bad
		dd SH2_RW_Bad, SH2_RW_Bad

	ALIGN32

	MSH2_RW_IntC
		mov al, [_32X_ADEN]
		mov ah, [_32X_FM]
		add al, al
		or ah, al
		mov al, [_32X_MINT]
		ret

	ALIGN32

	SH2_RW_HCnt
		mov al, [_32X_HIC]
		xor ah, ah
		ret

	ALIGN32

	SH2_RW_Com
		mov ah, [_32X_Comm + ecx - 0x20 + 0]
		mov al, [_32X_Comm + ecx - 0x20 + 1]
		ret

	ALIGN32

	SH2_RW_DREQ
		mov al, [_32X_RV]
		mov ah, [_32X_DREQ_ST]
		or al, ah
		mov ah, [_32X_DREQ_ST + 1]
		ret

	ALIGN32

	SH2_RW_DREQ_Src_H
		mov ax, [_32X_DREQ_SRC + 2]
		ret

	ALIGN4
	
	SH2_RW_DREQ_Src_L
		mov ax, [_32X_DREQ_SRC]
		ret

	ALIGN4
	
	SH2_RW_DREQ_Dst_H
		mov ax, [_32X_DREQ_DST + 2]
		ret

	ALIGN4
	
	SH2_RW_DREQ_Dst_L
		mov ax, [_32X_DREQ_DST]
		ret

	ALIGN4

	SH2_RW_DREQ_Len
		mov ax, [_32X_DREQ_LEN]
		ret


	ALIGN32
	
	SH2_RW_FIFO
		mov ax, [_32X_DREQ_ST]
		mov edx, [_32X_FIFO_Read]
		mov ecx, [_32X_FIFO_Block]
		and ax, 0x4004
		xor ecx, byte (4 * 2)
		cmp ax, 0x0004
		jne short .FIFO_End

		mov ax, [_32X_FIFO_A + ecx + edx * 2]

;pushad
;push eax
;lea eax, [0x20004500 + ecx + edx * 2]
;push eax
;call _Write_To_68K_Space
;pop eax
;pop eax
;popad

		inc edx
		cmp edx, 4
		jae short .32X_FIFO_Empty_A

		mov [_32X_FIFO_Read], edx
		ret

	ALIGN4
	
	.FIFO_End
		xor ax, ax
		ret

	ALIGN4
	
	.32X_FIFO_Empty_A
		mov dx, [_32X_DREQ_LEN]
		push eax
		sub dx, byte 4
		mov al, [_32X_DREQ_ST + 1]
		mov [_32X_DREQ_LEN], dx
		jz short .32X_FIFO_Read_End

		test al, 0x80
		jz short .32X_FIFO_Empty_B

		xor dl, dl
		mov [_32X_FIFO_Block], ecx
		mov [_32X_DREQ_ST + 1], dl
		mov [_32X_FIFO_Write], dl
		mov [_32X_FIFO_Read], dl
		pop eax
		ret

	ALIGN4

	.32X_FIFO_Empty_B
		mov byte [_32X_DREQ_ST + 1], 0x40
		xor edx, edx
		mov ecx, M_SH2						; we use M_SH2 instead of EBP (like real 32X).
		call SH2_DMA0_Request
		pop eax
		ret

	ALIGN32

	.32X_FIFO_Read_End
		xor edx, edx
		mov ecx, M_SH2						; we use M_SH2 instead of EBP (like real 32X).
		mov [_32X_DREQ_ST], dx
		call SH2_DMA0_Request
		pop eax
		ret
		
	ALIGN32
	
	SH2_RW__PWM_Cont
		mov ax, [_PWM_Mode]
		ret 
		
	ALIGN32
	
	SH2_RW__PWM_Cycle
		mov ax, [_PWM_Cycle_Tmp]
		ret

	ALIGN32

	SH2_RW__PWM_Pulse_L
		mov ecx, [_PWM_RP_L]
		mov edx, [_PWM_WP_L]
		mov ah, [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + edx]
		ret

	ALIGN4

	SH2_RW__PWM_Pulse_R
		mov ecx, [_PWM_RP_R]
		mov edx, [_PWM_WP_R]
		mov ah, [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + edx]
		ret

	ALIGN4

	SH2_RW__PWM_Pulse_C
		mov ecx, [_PWM_RP_L]
		mov edx, [_PWM_WP_L]
		mov ah, [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + edx]
		ret


	ALIGN32

	SSH2_Read_Word_32X_Reg

		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0x3E
		sub edx, byte 14
		mov [ebp + SH2.Cycle_IO], edx
		jmp [.Table_SSH2_Reg + ecx * 2]

	ALIGN4

	.Table_SSH2_Reg
		dd SSH2_RW_IntC, SH2_RW_Bad, SH2_RW_HCnt, SH2_RW_DREQ
		dd SH2_RW_DREQ_Src_H, SH2_RW_DREQ_Src_L, SH2_RW_DREQ_Dst_H, SH2_RW_DREQ_Dst_L

		dd SH2_RW_DREQ_Len, SH2_RW_FIFO, SH2_RW_Bad, SH2_RW_Bad
		dd SH2_RW_Bad, SH2_RW_Bad, SH2_RW_Bad, SH2_RW_Bad

		dd SH2_RW_Com, SH2_RW_Com, SH2_RW_Com, SH2_RW_Com
		dd SH2_RW_Com, SH2_RW_Com, SH2_RW_Com, SH2_RW_Com

		dd SH2_RW__PWM_Cont, SH2_RW__PWM_Cycle
		dd SH2_RW__PWM_Pulse_L, SH2_RW__PWM_Pulse_R
		dd SH2_RW__PWM_Pulse_C, SH2_RW_Bad
		dd SH2_RW_Bad, SH2_RW_Bad

	ALIGN32

	SSH2_RW_IntC
		mov al, [_32X_ADEN]
		mov ah, [_32X_FM]
		add al, al
		or ah, al
		mov al, [_32X_SINT]
		ret

	ALIGN32

	SH2_Read_Word_VDP_Reg
		mov edx, [ebp + SH2.Cycle_IO]
		and ecx, 0xE
		sub edx, byte 5
		mov [ebp + SH2.Cycle_IO], edx
		jmp [.Table_VDP_Reg + ecx * 2]

	ALIGN4

	.Table_VDP_Reg
		dd SH2_RW_VDP_Mode, SH2_RW_VDP_Shift, SH2_RW_VDP_AF_Len, SH2_RW_VDP_AF_St
		dd SH2_RW_VDP_AF_Data, SH2_RW_VDP_State, SH2_RW_Bad, SH2_RW_Bad

	ALIGN32

	SH2_RW_VDP_Mode
		mov ax, [_32X_VDP + vx.Mode]
		ret

	ALIGN4
	
	SH2_RW_VDP_Shift
		mov al, [_32X_VDP + vx.Mode + 2]
		xor ah, ah
		ret

	ALIGN4

	SH2_RW_VDP_AF_Len
		mov al, [_32X_VDP + vx.AF_Len]
		xor ah, ah
		ret

	ALIGN4

	SH2_RW_VDP_AF_St
		mov ax, [_32X_VDP + vx.AF_St]
		ret

	ALIGN4

	SH2_RW_VDP_AF_Data
		mov ax, [_32X_VDP + vx.AF_Data]
		ret

	ALIGN32

	SH2_RW_VDP_State
		mov ax, [_32X_VDP + vx.State]
		xor ax, byte 2
		mov [_32X_VDP + vx.State], ax
		ret



; WRITE BYTE
;***********


	ALIGN32

	MSH2_Write_Byte_32X_Reg

;pushad
;push edx
;push ecx
;call _Write_To_68K_Space
;pop ecx
;pop edx
;popad

		mov eax, [ebp + SH2.Cycle_IO]
		and ecx, 0x3F
		sub eax, byte 10
		mov [_MSH2_Reg + ecx], dl
		mov [ebp + SH2.Cycle_IO], eax
		jmp [.Table_MSH2_Reg + ecx * 4]

	ALIGN4

	.Table_MSH2_Reg
		dd MSH2_WB_IntC_H, MSH2_WB_IntC_L, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_HCnt, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad

		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad

		dd SH2_WB_Com, SH2_WB_Com, SH2_WB_Com, SH2_WB_Com
		dd SH2_WB_Com, SH2_WB_Com, SH2_WB_Com, SH2_WB_Com
		dd SH2_WB_Com, SH2_WB_Com, SH2_WB_Com, SH2_WB_Com
		dd SH2_WB_Com, SH2_WB_Com, SH2_WB_Com, SH2_WB_Com

		dd SH2_WB__PWM_Cont_H, SH2_WB__PWM_Cont_L
		dd SH2_WB__PWM_Cycle_H, SH2_WB__PWM_Cycle_L
		dd SH2_WB__PWM_Pulse_L_H, SH2_WB__PWM_Pulse_L_L
		dd SH2_WB__PWM_Pulse_R_H, SH2_WB__PWM_Pulse_R_L
		dd SH2_WB__PWM_Pulse_C_H, SH2_WB__PWM_Pulse_C_L
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad

	ALIGN32

	MSH2_WB_IntC_H
	SSH2_WB_IntC_H
		and dl, 0x80
		mov al, [_32X_FM]
		xor al, dl
		mov [_32X_FM], dl
		jnz near __32X_Set_FB
		ret

	ALIGN32

	MSH2_WB_IntC_L
		mov [_32X_MINT], dl
		ret

	ALIGN32

	SH2_WB_HCnt
		mov [_32X_HIC], dl
		ret

	ALIGN32

	SH2_WB_Com
		mov [_32X_Comm + ecx - 0x20], dl
		ret

	ALIGN32
	
	SH2_WB__PWM_Cont_H
		mov [_PWM_Mode + 1], dl
		mov cl, dl
		jmp @PWM_Set_Int@4
	
	ALIGN4
	
	SH2_WB__PWM_Cont_L
		mov [_PWM_Mode], dl
		ret

	ALIGN4

	SH2_WB__PWM_Cycle_H
		mov cl, [_PWM_Cycle_Tmp + 0]
		mov [_PWM_Cycle_Tmp + 1], dl
		mov ch, dl
		jmp @PWM_Set_Cycle@4
	
	ALIGN4
	
	SH2_WB__PWM_Cycle_L
		mov ch, [_PWM_Cycle_Tmp + 1]
		mov [_PWM_Cycle_Tmp + 0], dl
		mov cl, dl
		jmp @PWM_Set_Cycle@4

	ALIGN32

	SH2_WB__PWM_Pulse_L_H
		mov [_PWM_FIFO_L_Tmp + 1], dl
		ret
	
	ALIGN32
	
	SH2_WB__PWM_Pulse_L_L
		mov ecx, [_PWM_RP_L]
		mov eax, [_PWM_WP_L]
		mov dh, [_PWM_FIFO_L_Tmp + 1]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + eax], 0x80
		jnz short .full

		mov [_PWM_FIFO_L + eax * 2], dx
		inc eax
		and eax, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_L], eax
		ret

	ALIGN4
	
	.full
		ret

	ALIGN32

	SH2_WB__PWM_Pulse_R_H
		mov [_PWM_FIFO_R_Tmp + 1], dl
		ret
	
	ALIGN32
	
	SH2_WB__PWM_Pulse_R_L
		mov ecx, [_PWM_RP_R]
		mov eax, [_PWM_WP_R]
		mov dh, [_PWM_FIFO_R_Tmp + 1]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + eax], 0x80
		jnz short .full

		mov [_PWM_FIFO_R + eax * 2], dx
		inc eax
		and eax, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_R], eax
		ret

	ALIGN4
	
	.full
		ret

	ALIGN32

	SH2_WB__PWM_Pulse_C_H
		mov [_PWM_FIFO_L_Tmp + 1], dl
		ret
	
	ALIGN32
	
	SH2_WB__PWM_Pulse_C_L
		mov ecx, [_PWM_RP_L]
		mov eax, [_PWM_WP_L]
		mov dh, [_PWM_FIFO_L_Tmp + 1]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + eax], 0x80
		jnz short .full

		mov [_PWM_FIFO_L + eax * 2], dx
		mov [_PWM_FIFO_R + eax * 2], dx
		inc eax
		and eax, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_L], eax
		mov [_PWM_WP_R], eax
		ret

	ALIGN4
	
	.full
		ret


	ALIGN32

	SSH2_Write_Byte_32X_Reg

;pushad
;push edx
;push ecx
;call _Write_To_68K_Space
;pop ecx
;pop edx
;popad

		mov eax, [ebp + SH2.Cycle_IO]
		and ecx, 0x3F
		sub eax, byte 10
		mov [_SSH2_Reg + ecx], dl
		mov [ebp + SH2.Cycle_IO], eax
		jmp [.Table_SSH2_Reg + ecx * 4]

	ALIGN4

	.Table_SSH2_Reg
		dd SSH2_WB_IntC_H, SSH2_WB_IntC_L, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_HCnt, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad

		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad

		dd SH2_WB_Com, SH2_WB_Com, SH2_WB_Com, SH2_WB_Com
		dd SH2_WB_Com, SH2_WB_Com, SH2_WB_Com, SH2_WB_Com
		dd SH2_WB_Com, SH2_WB_Com, SH2_WB_Com, SH2_WB_Com
		dd SH2_WB_Com, SH2_WB_Com, SH2_WB_Com, SH2_WB_Com

		dd SH2_WB__PWM_Cont_H, SH2_WB__PWM_Cont_L
		dd SH2_WB__PWM_Cycle_H, SH2_WB__PWM_Cycle_L
		dd SH2_WB__PWM_Pulse_L_H, SH2_WB__PWM_Pulse_L_L
		dd SH2_WB__PWM_Pulse_R_H, SH2_WB__PWM_Pulse_R_L
		dd SH2_WB__PWM_Pulse_C_H, SH2_WB__PWM_Pulse_C_L
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad


	ALIGN32

	SSH2_WB_IntC_L
		mov [_32X_SINT], dl
		ret


	ALIGN32

	SH2_Write_Byte_VDP_Reg

;pushad
;push edx
;push ecx
;call _Write_To_68K_Space
;pop ecx
;pop edx
;popad

		mov eax, [ebp + SH2.Cycle_IO]
		and ecx, 0xF
		sub eax, byte 3
		mov [_SH2_VDP_Reg + ecx], dl
		mov [ebp + SH2.Cycle_IO], eax
		jmp [.Table_VDP_Reg + ecx * 4]

	ALIGN4

	.Table_VDP_Reg
		dd SH2_WB_Bad, SH2_WB_VDP_Mode, SH2_WB_Bad, SH2_WB_VDP_Shift
		dd SH2_WB_Bad, SH2_WB_VDP_AF_Len, SH2_WB_Bad, SH2_WB_Bad
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_VDP_State
		dd SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad, SH2_WB_Bad

	ALIGN32

	SH2_WB_VDP_Mode
		mov [_32X_VDP + vx.Mode], dl
		ret

	ALIGN4
	
	SH2_WB_VDP_Shift
		mov [_32X_VDP + vx.Mode + 2], dl
		ret

	ALIGN4
	
	SH2_WB_VDP_AF_Len
		mov [_32X_VDP + vx.AF_Len], dl
		ret

	ALIGN32
	
	SH2_WB_VDP_State
		mov ah, [_32X_VDP + vx.Mode + 0]
		mov al, [_32X_VDP + vx.State + 1]
		test ah, 3
		mov [_32X_VDP + vx.State + 2], dl
		jz short .blank

		test al, al
		jns short .not_in_vblank
		
	.blank
		mov [_32X_VDP + vx.State + 0], dl
		jmp __32X_Set_FB

	ALIGN4

	.not_in_vblank
		ret



; WRITE WORD
;***********


	ALIGN32

	MSH2_Write_Word_32X_Reg

;pushad
;push edx
;push ecx
;call _Write_To_68K_Space
;pop ecx
;pop edx
;popad

		mov eax, [ebp + SH2.Cycle_IO]
		and ecx, 0x3E
		sub eax, byte 16
		mov [_MSH2_Reg + ecx + 0], dh
		mov [ebp + SH2.Cycle_IO], eax
		mov [_MSH2_Reg + ecx + 1], dl
		jmp [.Table_MSH2_Reg + ecx * 2]

	ALIGN4

	.Table_MSH2_Reg
		dd MSH2_WW_IntC, SH2_WW_Bad, SH2_WB_HCnt, SH2_WW_Bad
		dd SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad

		dd SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad
		dd SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad

		dd SH2_WW_Com, SH2_WW_Com, SH2_WW_Com, SH2_WW_Com
		dd SH2_WW_Com, SH2_WW_Com, SH2_WW_Com, SH2_WW_Com

		dd SH2_WW__PWM_Cont, SH2_WW__PWM_Cycle
		dd SH2_WW__PWM_Pulse_L, SH2_WW__PWM_Pulse_R
		dd SH2_WW__PWM_Pulse_C, SH2_WW_Bad
		dd SH2_WW_Bad, SH2_WW_Bad

	ALIGN32

	MSH2_WW_IntC
		and dh, 0x80
		mov al, [_32X_FM]
		mov [_32X_MINT], dl
		xor al, dh
		mov [_32X_FM], dh
		jnz near __32X_Set_FB
		ret

	ALIGN32

	SH2_WW_Com
		mov [_32X_Comm + ecx - 0x20 + 0], dh
		mov [_32X_Comm + ecx - 0x20 + 1], dl
		ret

	ALIGN32
	
	SH2_WW__PWM_Cont
		mov [_PWM_Mode + 0], dl
		mov cl, dh
		mov [_PWM_Mode + 1], dh
		jmp @PWM_Set_Int@4
		
	ALIGN32
	
	SH2_WW__PWM_Cycle
		mov ecx, edx
		jmp @PWM_Set_Cycle@4

	ALIGN32

	SH2_WW__PWM_Pulse_L
		mov ecx, [_PWM_RP_L]
		mov eax, [_PWM_WP_L]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + eax], 0x80
		jnz short .full

		mov [_PWM_FIFO_L + eax * 2], dx
		inc eax
		and eax, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_L], eax
		ret

	ALIGN4
	
	.full
		ret

	ALIGN32

	SH2_WW__PWM_Pulse_R
		mov ecx, [_PWM_RP_R]
		mov eax, [_PWM_WP_R]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + eax], 0x80
		jnz short .full

		mov [_PWM_FIFO_R + eax * 2], dx
		inc eax
		and eax, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_R], eax
		ret

	ALIGN4
	
	.full
		ret

	ALIGN32

	SH2_WW__PWM_Pulse_C
		mov ecx, [_PWM_RP_L]
		mov eax, [_PWM_WP_L]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + eax], 0x80
		jnz short .full

		mov [_PWM_FIFO_L + eax * 2], dx
		mov [_PWM_FIFO_R + eax * 2], dx
		inc eax
		and eax, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_L], eax
		mov [_PWM_WP_R], eax
		ret

	ALIGN4
	
	.full
		ret


	ALIGN32

	SSH2_Write_Word_32X_Reg

;pushad
;push edx
;push ecx
;call _Write_To_68K_Space
;pop ecx
;pop edx
;popad

		mov eax, [ebp + SH2.Cycle_IO]
		and ecx, 0x3E
		sub eax, byte 14
		mov [_SSH2_Reg + ecx + 0], dh
		mov [ebp + SH2.Cycle_IO], eax
		mov [_SSH2_Reg + ecx + 1], dl
		jmp [.Table_SSH2_Reg + ecx * 2]

	ALIGN4

	.Table_SSH2_Reg
		dd SSH2_WW_IntC, SH2_WW_Bad, SH2_WB_HCnt, SH2_WW_Bad
		dd SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad

		dd SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad,
		dd SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad, SH2_WW_Bad

		dd SH2_WW_Com, SH2_WW_Com, SH2_WW_Com, SH2_WW_Com
		dd SH2_WW_Com, SH2_WW_Com, SH2_WW_Com, SH2_WW_Com

		dd SH2_WW__PWM_Cont, SH2_WW__PWM_Cycle
		dd SH2_WW__PWM_Pulse_L, SH2_WW__PWM_Pulse_R
		dd SH2_WW__PWM_Pulse_C, SH2_WW_Bad
		dd SH2_WW_Bad, SH2_WW_Bad

	ALIGN32

	SSH2_WW_IntC
		and dh, 0x80
		mov al, [_32X_FM]
		mov [_32X_SINT], dl
		xor al, dh
		mov [_32X_FM], dh
		jnz near __32X_Set_FB
		ret


	ALIGN32

	SH2_Write_Word_VDP_Reg

;pushad
;push edx
;push ecx
;call _Write_To_68K_Space
;pop ecx
;pop edx
;popad

		mov eax, [ebp + SH2.Cycle_IO]
		and ecx, 0xE
		sub eax, byte 5
		mov [_SH2_VDP_Reg + ecx + 0], dh
		mov [ebp + SH2.Cycle_IO], eax
		mov [_SH2_VDP_Reg + ecx + 1], dl
		jmp [.Table_VDP_Reg + ecx * 2]

	ALIGN4

	.Table_VDP_Reg
		dd SH2_WB_VDP_Mode, SH2_WB_VDP_Shift
		dd SH2_WB_VDP_AF_Len, SH2_WW_VDP_AF_St
		dd SH2_WW_VDP_AF_Data, SH2_WB_VDP_State
		dd SH2_WW_Bad, SH2_WW_Bad

	ALIGN32

	SH2_WW_VDP_AF_St
		mov [_32X_VDP + vx.AF_St], dx
		ret

	ALIGN4

	SH2_WW_VDP_AF_Data
		mov [_32X_VDP + vx.AF_Data], dx
		push edi
		mov ax, dx
		mov edi, [_32X_VDP + vx.State]
		shl eax, 16
		and edi, byte 1
		mov ax, dx
		xor edi, byte 1
		mov edx, [_32X_VDP + vx.AF_St]
		movzx ecx, byte [_32X_VDP + vx.AF_Len]
		shl edi, 17
		inc ecx
		shr ecx, 1
		lea edi, [edi + _32X_VDP_Ram]
		jz short .Spec
		jnc short .Loop

		mov [edi + edx * 2], ax
		inc dl
		jmp short .Loop

	ALIGN32

	.Loop
		mov [edi + edx * 2], eax
		add dl, byte 2
		dec ecx
		jnz short .Loop

		pop edi
		mov [_32X_VDP + vx.AF_St], edx
		ret

	ALIGN32

	.Spec
		mov [edi + edx * 2], ax
		inc dl
		pop edi
		mov [_32X_VDP + vx.AF_St], edx
		ret
