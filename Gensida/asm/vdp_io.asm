%include "nasmhead.inc"

IN_VRAM		equ 0
IN_CRAM		equ 1
IN_VSRAM	equ 2
RD_Mode		equ 0
WR_Mode		equ 1

section .data align=64

	extern _hook_read_byte
	extern _hook_read_word
	extern _hook_read_dword
	extern _hook_write_byte
	extern _hook_write_word
	extern _hook_write_dword

	extern _hook_write_vram_byte
	extern _hook_write_vram_word
	extern _hook_read_vram_byte
	extern _hook_read_vram_word

	extern _hook_pc
	extern _hook_address
	extern _hook_value

	extern _hook_dma
	extern _dma_src
	extern _dma_len

	extern Rom_Data
	extern Rom_Size
	extern Cell_Conv_Tab
	extern MD_Palette
	extern MD_Palette32
	extern MD_Screen
	extern MD_Screen32
	extern Sprite_Struct
	extern Sprite_Visible
	extern CPL_M68K
	extern Cycles_M68K
	extern _main68k_context		; Starscream context (for interrupts)
	extern Ram_Word_State
	extern PalLock

	ALIGN64
	
	CD_Table:
		dd 0x0005, 0x0009, 0x0000, 0x000A	; bits 0-2	= Location (0x00:WRONG, 0x01:VRAM, 0x02:CRAM, 0x03:VSRAM)
		dd 0x0007, 0x000B, 0x0000, 0x0000	; bits 3-4	= Access   (0x00:WRONG, 0x04:READ, 0x08:WRITE)
		dd 0x0006, 0x0000, 0x0000, 0x0000	; bits 8-11	= DMA MEM TO VRAM (0x0000:NO DMA, 0x0100:MEM TO VRAM, 0x0200: MEM TO CRAM, 0x0300: MEM TO VSRAM)
		dd 0x0000, 0x0000, 0x0000, 0x0000	; bits 12	= DMA VRAM FILL (0x0000:NO DMA, 0x0400:VRAM FILL)

		dd 0x0005, 0x0009, 0x0000, 0x000A	; bits 13	= DMA VRAM COPY (0x0000:NO DMA, 0x0800:VRAM COPY)
		dd 0x0007, 0x000B, 0x0000, 0x0000
		dd 0x0006, 0x0000, 0x0000, 0x0000
		dd 0x0000, 0x0000, 0x0000, 0x0000

		dd 0x0005, 0x0509, 0x0000, 0x020A
		dd 0x0007, 0x030B, 0x0000, 0x0000
		dd 0x0006, 0x0000, 0x0000, 0x0000
		dd 0x0000, 0x0000, 0x0000, 0x0000

;		dd 0x0800, 0x0000, 0x0000, 0x0000
;		dd 0x0000, 0x0000, 0x0000, 0x0000
;		dd 0x0000, 0x0000, 0x0000, 0x0000
;		dd 0x0000, 0x0000, 0x0000, 0x0000

		dd 0x0800, 0x0100, 0x0000, 0x0200
		dd 0x0000, 0x0300, 0x0000, 0x0000
		dd 0x0000, 0x0000, 0x0000, 0x0000
		dd 0x0000, 0x0000, 0x0000, 0x0000

	ALIGN64

	DMA_Timing_Table:
;		dd 83,  167, 166,  83,
;		dd 102, 205, 204, 102,
;		dd 8,    16,  15,   8,
;		dd 9,    18,  17,   9

;		dd 92,  167, 166,  83,
;		dd 118, 205, 204, 102,
;		dd 9,    16,  15,   8,
;		dd 10,   18,  17,   9

		dd 83,  167, 166,  83,
		dd 102, 205, 204, 102,
		dd 8,    16,  15,   8,
		dd 9,    18,  17,   9	

	DECL Genesis_Started
		dd 0

	DECL SegaCD_Started
		dd 0

	DECL _32X_Started
		dd 0

	Size_V_Scroll:
		dd 255, 511, 255, 1023

	H_Scroll_Mask_Table:
		dd 0x0000, 0x0007, 0x01F8, 0x01FF

section .bss align=64

	extern Ram_68k
	extern Ram_Prg
	extern Ram_Word_2M
	extern Ram_Word_1M
	extern Bank_M68K

	DECL VRam
	resb 64 * 1024

	DECL CRam
	resd 64

	VSRam_Over
	resd 8

	DECL VSRam
	resd 64

	DECL H_Counter_Table
	resb 512 * 2

	DECL VDP_Reg
	.Set_1			resd 1
	.Set_2			resd 1
	.Pat_ScrA_Adr	resd 1
	.Pat_WIN_Adr	resd 1
	.Pat_ScrB_Adr	resd 1
	.Spr_Att_Adr	resd 1
	.Reg6			resd 1
	.BG_Color		resd 1
	.Reg8			resd 1
	.Reg9			resd 1
	.H_Int_Reg		resd 1
	.Set_3			resd 1
	.Set_4			resd 1
	.H_Scr_Adr		resd 1
	.Reg14			resd 1
	.Auto_Inc		resd 1
	.Scr_Size		resd 1
	.Win_H_Pos		resd 1
	.Win_V_Pos		resd 1
	.DMA_Length_L	resd 1
	.DMA_Length_H	resd 1
	.DMA_Src_Adr_L	resd 1
	.DMA_Src_Adr_M	resd 1
	.DMA_Src_Adr_H	resd 1

	.DMA_Length		resd 1
	.DMA_Address	resd 1

	DECL ScrA_Addr
	resd 1
	DECL ScrB_Addr
	resd 1
	DECL Win_Addr
	resd 1
	DECL Spr_Addr
	resd 1
	DECL H_Scroll_Addr
	resd 1

	DECL H_Cell
	resd 1
	DECL H_Win_Mul
	resd 1
	DECL H_Pix
	resd 1
	DECL H_Pix_Begin
	resd 1

	DECL H_Scroll_Mask
	resd 1
	DECL H_Scroll_CMul
	resd 1
	DECL H_Scroll_CMask
	resd 1
	DECL V_Scroll_CMask
	resd 1
	DECL V_Scroll_MMask
	resd 1

	DECL Win_X_Pos
	resd 1
	DECL Win_Y_Pos
	resd 1

	DECL Ctrl
	.Flag			resd 1
	.Data			resd 1
	.Write			resd 1
	.Access			resd 1
	.Address		resd 1
	.DMA_Mode		resd 1
	.DMA			resd 1

	DECL DMAT_Tmp
	resd 1
	DECL DMAT_Length
	resd 1
	DECL DMAT_Type
	resd 1

	DECL VDP_Status
	resd 1
	DECL VDP_Int
	resd 1
	DECL VDP_Current_Line
	resd 1
	DECL VDP_Num_Lines
	resd 1
	DECL VDP_Num_Vis_Lines
	resd 1
	DECL CRam_Flag
	resd 1
	DECL VRam_Flag
	resd 1

section .text align=64

	extern _main68k_readOdometer
	extern _main68k_releaseCycles
	extern _main68k_interrupt

	extern _Write_To_68K_Space

; ******************************************

; macro DMA_LOOP
; entree:
; esi = Source Address
; edi = Destination Address
; ecx = Nombre de words a transferer
; edx = Incrementation Destination
; sortie:
; param :
; %1 = 0 : source = ROM, 1 : source = RAM    2 : source = Prog RAM
; %1 = 3 : source = Word RAM 2M              4 : incorrect value
; %1 = 5 : source = Word RAM 1M Bank 1       6 : source = Word RAM 1M Bank 0
; %1 = 7 : source = Cell arranged Bank 0     8 : source = Cell arranged Bank 1
; %2 = 0 : dest = VRAM, 1 : dest = CRAM, 2 : dest = VSRAM

%macro DMA_LOOP 2

%if %1 < 1
	and esi, 0x003FFFFE
%elif %1 < 2
	and esi, 0xFFFE
%elif %1 < 3
	and esi, 0x0001FFFE
	add esi, [Bank_M68K]
%elif %1 < 4
	sub esi, 2
	and esi, 0x0003FFFE
%elif %1 < 7
	sub esi, 2
	and esi, 0x0001FFFE
%else
	sub esi, 2					; cell rearranged
	xor eax, eax				; for offset
	and esi, 0x0001FFFE
%endif
	mov ebx, edi
%if %2 < 1
	mov dword [VRam_Flag], 1
	mov byte [DMAT_Type], 0
%else
	%if %2 < 2
		mov dword [CRam_Flag], 1
	%endif
	mov byte [DMAT_Type], 1
%endif
	xor edi, edi
	mov dword [Ctrl.DMA], 0
	jmp short %%Loop

	ALIGN32

%%Loop
	mov di, bx
%if %1 < 1
	mov ax, [Rom_Data + esi]
	add esi, 2
%elif %1 < 2
	mov ax, [Ram_68k + esi]
	add si, 2
%elif %1 < 3
	mov ax, [Ram_Prg + esi]
	add esi, 2
%elif %1 < 4
	mov ax, [Ram_Word_2M + esi]
	add esi, 2
%elif %1 < 6
	mov ax, [Ram_Word_1M + esi + 0x00000]
	add esi, 2
%elif %1 < 7
	mov ax, [Ram_Word_1M + esi + 0x20000]
	add esi, 2
%elif %1 < 8
	mov ax, [Cell_Conv_Tab + esi]
	add esi, 2
	mov ax, [Ram_Word_1M + eax * 2 + 0x00000]
%elif %1 < 9
	mov ax, [Cell_Conv_Tab + esi]
	add esi, 2
	mov ax, [Ram_Word_1M + eax * 2 + 0x20000]
%endif
%if %2 < 1
	shr di, 1
	jnc short %%No_Swap
		rol ax, 8
%%No_Swap
%else
	and di, byte 0x7E
%endif
	add bx, dx
	dec ecx
%if %2 < 1
	mov [VRam + edi * 2], ax
%elif %2 < 2
	mov [CRam + edi], ax
%else
	mov [VSRam + edi], ax
%endif
	jnz short %%Loop

%%End_Loop
	jmp .End_DMA


%endmacro

;***********************************************

	ALIGN32

	;void Reset_VDP(void)
	DECL Reset_VDP

		push ebx
		push ecx
		push edx

		xor eax, eax

		mov ebx, MD_Screen
		mov ecx, (336 * 240 / 2)
	.loop_MD_Screen
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_MD_Screen

		mov ebx, MD_Screen32
		mov ecx, (336 * 240 / 1)
	.loop_MD_Screen32
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_MD_Screen32

		mov ebx, VRam
		mov ecx, (1024 * 16)
	.loop_VRam
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_VRam

		mov ebx, CRam
		mov ecx, 40
	.loop_CRam
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_CRam

		mov ebx, VSRam
		mov ecx, 20
	.loop_VSRam
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_VSRam
		
		test [PalLock], byte 1
		jnz .palette_Locked
		mov ebx, MD_Palette
		mov ecx, (0x100 / 2)
	.loop_Palette
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_Palette
		
		mov ebx, MD_Palette32
		mov ecx, 0x100
	.loop_Palette32
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_Palette32

	.palette_Locked
		mov ebx, Sprite_Struct
		mov ecx, (100 * 8)
	.loop_Sprite_Struct
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_Sprite_Struct

		mov ebx, Sprite_Visible
		mov ecx, 100
	.loop_Sprite_Visible
		mov [ebx], eax
		add ebx, 4
		dec ecx
		jnz .loop_Sprite_Visible

		push eax
		push dword 0
		mov ecx, 23

	.loop_Reg
		call Set_VDP_Reg
		inc dword [esp]
		dec ecx
		jnz short .loop_Reg

		add esp, 8

;		mov dword [ebx + 4 * 00], 0x00
;		mov dword [ebx + 4 * 01], 0x00
;		mov dword [ebx + 4 * 02], 0x00
;		mov dword [ebx + 4 * 03], 0x00
;		mov dword [ebx + 4 * 04], 0x00
;		mov dword [ebx + 4 * 05], 0x00
;		mov dword [ebx + 4 * 06], 0x00
;		mov dword [ebx + 4 * 07], 0x00
;		mov dword [ebx + 4 * 08], 0x00
;		mov dword [ebx + 4 * 09], 0x00
;		mov dword [ebx + 4 * 10], 0xFF
;		mov dword [ebx + 4 * 11], 0x00
;		mov dword [ebx + 4 * 12], 0x81
;		mov dword [ebx + 4 * 13], 0x00
;		mov dword [ebx + 4 * 14], 0x00
;		mov dword [ebx + 4 * 15], 0x02
;		mov dword [ebx + 4 * 16], 0x00
;		mov dword [ebx + 4 * 17], 0x00
;		mov dword [ebx + 4 * 18], 0x00
;		mov dword [ebx + 4 * 19], 0x00
;		mov dword [ebx + 4 * 20], 0x00
;		mov dword [ebx + 4 * 21], 0x00
;		mov dword [ebx + 4 * 22], 0x00
;		mov dword [ebx + 4 * 23], 0x00

		mov dword [ebx + 4 * 24], 0x00
		mov dword [ebx + 4 * 25], 0x00
		mov dword [ebx + 4 * 26], 0x00

		mov dword [VDP_Status], 0x0200
		mov dword [VDP_Int], 0
		mov dword [DMAT_Tmp], 0
		mov dword [DMAT_Length], 0
		mov dword [DMAT_Type], 0
		mov dword [Ctrl.Flag], 0
		mov dword [Ctrl.Data], 0
		mov dword [Ctrl.Write], 0
		mov dword [Ctrl.Access], 0
		mov dword [Ctrl.Address], 0
		mov dword [Ctrl.DMA_Mode], 0
		mov dword [Ctrl.DMA], 0
		mov dword [CRam_Flag], 1
		mov dword [VRam_Flag], 1

		xor ebx, ebx

	.Loop_HC
		mov ecx, 170
		mov eax, ebx
		mul ecx
		xor edx, edx
		mov ecx, 488
		div ecx
		sub eax, 0x18
		mov [H_Counter_Table + ebx * 2 + 0], al

		mov ecx, 205
		mov eax, ebx
		mul ecx
		xor edx, edx
		mov ecx, 488
		div ecx
		sub eax, 0x1C
		mov [H_Counter_Table + ebx * 2 + 1], al

		inc ebx
		cmp ebx, 512
		jb short .Loop_HC

		mov dword [VSRam_Over + 28], 0
		
		pop edx
		pop ecx
		pop ebx
		ret

	error:
		xor ax, ax
		pop ecx
		pop ebx
		ret
	
	ALIGN32

	;void Update_DMA(void)
	DECL Update_DMA

		push ebx
		push ecx
		push edx

		mov ebx, [VDP_Reg + 12 * 4]	; 32 / 40 Cell ?
		mov edx, [DMAT_Type]
		mov eax, [VDP_Current_Line]
		mov ecx, [VDP_Num_Vis_Lines]
		and ebx, byte 1
		and edx, byte 3
		cmp eax, ecx
		lea ebx, [ebx * 4 + edx]
		jae short .Blanking
		test byte [VDP_Reg + 1 * 4], 0x40	; VDP Enable ?
		jz short .Blanking

		add ebx, byte 8

	.Blanking
		mov ecx, [DMA_Timing_Table + ebx * 4]
		mov eax, [CPL_M68K]
		sub dword [DMAT_Length], ecx
		ja short .DMA_Not_Finished

			shl eax, 16
			mov ebx, [DMAT_Length]
			xor edx, edx
			add ebx, ecx
			mov [DMAT_Length], edx
			div ecx
			and word [VDP_Status], 0xFFFD
			mul ebx
			shr eax, 16
			test byte [DMAT_Type], 2
			jnz short .DMA_68k_CRam_VSRam

			pop edx
			pop ecx
			pop ebx
			ret

	.DMA_Not_Finished
		test byte [DMAT_Type], 2
		jz short .DMA_68k_VRam

	.DMA_68k_CRam_VSRam

		xor eax, eax
			
	.DMA_68k_VRam
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN32

	;unsigned short Read_VDP_Data(void)
	DECL Read_VDP_Data
		push ebx
		mov byte [Ctrl.Flag], 0			; on en a finit avec Address Set
		push ecx
	 	mov ebx, [Ctrl.Address]
		mov eax, [Ctrl.Access]
		mov ecx, ebx
		jmp [.Table_Read + eax * 4]

	ALIGN4

	.Table_Read:
			dd	error, error, error, error				; Wrong
			dd	error, .RD_VRAM, .RD_CRAM, .RD_VSRAM	; READ
			dd	error, error, error, error				; WRITE
			dd	error, error, error, error				; WRITE & READ (WRONG)

	ALIGN4
	
	.RD_VRAM
		add ecx, [VDP_Reg.Auto_Inc]
		and ebx, 0xFFFE
		mov [Ctrl.Address], cx
		mov ax, [VRam + ebx]

		pushad
		sub esi,ebp
		sub esi,byte 2
		mov [_hook_pc],esi
		mov [_hook_value],eax
		call _hook_read_vram_byte
		popad

		pop ecx
		pop ebx
		ret

	ALIGN4

	.RD_CRAM
		add ecx, [VDP_Reg.Auto_Inc]
		and ebx, byte 0x7E
		mov [Ctrl.Address], cx
		mov ax, [CRam + ebx]

	.End
		pop ecx
		pop ebx
		ret

	ALIGN4

	.RD_VSRAM
		add ecx, [VDP_Reg.Auto_Inc]
		and ebx, byte 0x7E
		mov [Ctrl.Address], cx
		mov ax, [VSRam + ebx]
		pop ecx
		pop ebx
		ret

	ALIGN32

	;unsigned short Read_VDP_Status(void)
	DECL Read_VDP_Status
		mov ax, [VDP_Status]
		push ax
		xor ax, 0xFF00
		and ax, 0xFF9F
		test ax, 0x0008
		jnz short .In_VBlank
		and ax, 0xFF1F

	.In_VBlank
		mov [VDP_Status], ax
		test byte [VDP_Reg.Set_2], 0x40
		pop ax
		jz short .Display_OFF
		ret

	ALIGN4

	.Display_OFF
		or ax, 8
		ret

	ALIGN32
	
	;unsigned char Read_VDP_H_Counter(void)
	DECL Read_VDP_H_Counter
		push ebx
			
		call _main68k_readOdometer
		mov ebx, [Cycles_M68K]
		sub ebx, [CPL_M68K]
		sub eax, ebx						; Nb cycles effectués sur cette ligne.
		xor ebx, ebx
		and eax, 0x1FF
		test byte [VDP_Reg.Set_4], 0x81		; 40 cell mode ?
		setnz bl
		mov al, [H_Counter_Table + eax * 2 + ebx]
		xor ah, ah

		pop ebx
		ret

	ALIGN32
	
	;unsigned char Read_VDP_V_Counter(void)
	DECL Read_VDP_V_Counter
		push ebx

		call _main68k_readOdometer
		mov ebx, [Cycles_M68K]
		sub ebx, [CPL_M68K]
		sub eax, ebx						; Nb cycles effectués sur cette ligne.
		xor ebx, ebx
		and eax, 0x1FF
		test byte [VDP_Reg.Set_4], 0x81		; 40 cell mode ?
		jz short .mode_32

	.mode_40
		mov al, [H_Counter_Table + eax * 2 + 1]
		mov bl, 0xA4
		jmp short .ok

	ALIGN4

	 .mode_32
		mov al, [H_Counter_Table + eax * 2 + 0]
		mov bl, 0x84

	 .ok
		cmp al, 0xE0
		setbe bh
		cmp al, bl
		setae bl
		and bl, bh

		test byte [VDP_Status], 1			; PAL ?
		jnz short .PAL

		mov eax, [VDP_Current_Line]
		shr bl, 1
		adc eax, 0
		cmp eax, 0xEB
		jb short .No_Over_Line_XX

		sub eax, 6
		jmp short .No_Over_Line_XX

	.PAL
		mov eax, [VDP_Current_Line]
		shr bl, 1
		adc eax, byte 0
		cmp eax, 0x103
		jb short .No_Over_Line_XX

		sub eax, byte 56

	.No_Over_Line_XX
		test byte [VDP_Reg.Set_4], 2
		jz short .No_Interlace

		rol al, 1

	.No_Interlace
		xor ah, ah
		pop ebx
		ret




	ALIGN32
	
	;void Write_Byte_VDP_Data(unsigned char Data)
	DECL Write_Byte_VDP_Data
		test byte [Ctrl.DMA], 0x4
		mov al, [esp + 4]
		mov byte [Ctrl.Flag], 0			; on en a finit avec Address Set
		mov ah, al
		jnz near DMA_Fill

		pushad
		sub esi,ebp
		sub esi,byte 2
		mov [_hook_pc],esi
		mov [_hook_value],eax
		call _hook_write_vram_byte
		popad

		jmp short Write_VDP_Data

	ALIGN32

	;void Write_Word_VDP_Data(unsigned short Data)
	DECL Write_Word_VDP_Data
		test byte [Ctrl.DMA], 0x4
		mov byte [Ctrl.Flag], 0			; on en a finit avec Address Set
		mov ax, [esp + 4]
		jnz near DMA_Fill

		pushad
		sub esi,ebp
		sub esi,byte 2
		mov [_hook_pc],esi
		mov [_hook_value],eax
		call _hook_write_vram_word
		popad

	Write_VDP_Data:
		push ebx
		push ecx
		mov ecx, [Ctrl.Access]
		mov ebx, [Ctrl.Address]
		jmp [.Table_Write_W + ecx * 4]
	
	ALIGN4

	.Table_Write_W:
			dd	error, error, error, error				; Wrong
			dd	error, error, error, error				; READ
			dd	error, .WR_VRAM, .WR_CRAM, .WR_VSRAM	; WRITE
			dd	error, error, error, error				; WRITE & READ (WRONG)

	ALIGN4
	
	.WR_VRAM
		mov ecx, ebx
		shr ebx, 1
		mov byte [VRam_Flag], 1
		jnc short .Address_Even
		rol ax, 8
	.Address_Even
		add ecx, [VDP_Reg.Auto_Inc]
		mov [VRam + ebx * 2], ax
		mov [Ctrl.Address], cx
		pop ecx
		pop ebx
		ret


	ALIGN4
	
	.WR_CRAM
		mov ecx, ebx
		and ebx, byte 0x7E
		add ecx, [VDP_Reg.Auto_Inc]
		mov byte [CRam_Flag], 1
		mov [Ctrl.Address], cx
		mov [CRam + ebx], ax

	.End
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.WR_VSRAM
		mov ecx, ebx
		and ebx, byte 0x7E
		add ecx, [VDP_Reg.Auto_Inc]
		mov [VSRam + ebx], ax
		mov [Ctrl.Address], cx
		pop ecx
		pop ebx
		ret

	ALIGN32
	
	DMA_Fill:
		push ebx
		push ecx
		push edx

		mov ebx, [Ctrl.Address]					; bx = Address Dest
		mov ecx, [VDP_Reg.DMA_Length]			; DMA Length
		mov edx, [VDP_Reg.Auto_Inc]				; edx = Auto_Inc
		mov dword [VDP_Reg.DMA_Length], 0		; Clear DMA.Length
		and ebx, 0xFFFF
		mov dword [Ctrl.DMA], 0					; Flag DMA Fill = 0
		xor ebx, 1
		or word [VDP_Status], 0x0002
		mov [VRam + ebx], al
		xor ebx, 1
		mov dword [DMAT_Type], 0x2
		and ecx, 0xFFFF
		mov byte [VRam_Flag], 1
		mov dword [DMAT_Length], ecx
		jnz short .Loop

		mov ecx, 0xFFFF
		mov [DMAT_Length], ecx
		jmp short .Loop

	ALIGN4

		.Loop
			mov [VRam + ebx], ah					; VRam[Adr] = Fill Data
			add bx, dx								; Adr = Adr + Auto_Inc
			dec ecx									; un transfert de moins
			jns short .Loop							; s'il en reste alors on continue

		mov [Ctrl.Address], bx					; on stocke la nouvelle valeur de Data_Address
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32
	
	;void Write_VDP_Ctrl(unsigned short Data)
	DECL Write_VDP_Ctrl

;		push ebx
;		mov eax, [esp + 8]
;
;		mov ebx, eax
;		and eax, 0xC000					; on isole pour tester le mode
;		cmp eax, 0x8000					; on est en mode set register
;		je short .Set_Register
;
;		test dword [Ctrl.Flag], 1		; est-on à la 1ère ecriture ??
;		mov eax, ebx
;		jz short .First_Word			; si oui on y va !
;		jmp .Second_Word				; sinon
;
;	ALIGN32
;	
;	.Set_Register
;		mov eax, ebx
;		shr ebx, 8						; ebx = numero du registre 
;		mov byte [Ctrl.Access], 5
;		and eax, 0xFF					; on isole la valeur du registre
;		and ebx, 0x1F					; on isole le numero du registre
;		mov word [Ctrl.Address], 0
;		jmp [Table_Set_Reg + ebx * 4]	; on affecte en fonction


		mov eax, [esp + 4]
		test byte [Ctrl.Flag], 1		; est-on à la 2eme ecriture ??
		push ebx
		jnz near .Second_Word			; sinon

		mov ebx, eax
		and eax, 0xC000					; on isole pour tester le mode
		cmp eax, 0x8000					; on est en mode set register
		jne short .First_Word

		mov eax, ebx
		mov bl, bh						; bl = numero du registre 
		mov dword [Ctrl.Access], 5
		and eax, 0xFF					; on isole la valeur du registre
		mov dword [Ctrl.Address], 0
		and ebx, 0x1F					; on isole le numero du registre 

		jmp [Table_Set_Reg + ebx * 4]	; on affecte en fonction

	ALIGN32
	
	.First_Word							; 1st Write
		push ecx
		push edx
		mov ax, [Ctrl.Data + 2]			; ax = 2nd word (AS)
		mov ecx, ebx					; cx = bx = 1st word (AS)
		mov [Ctrl.Data], bx				; et on sauvegarde les premiers 16 bits (AS)
		mov edx, eax					; dx = ax = 2nd word (AS)
		mov byte [Ctrl.Flag], 1			; la prochaine ecriture sera Second
		shl eax, 14						; on isole l'adresse
		and ebx, 0x3FFF					; on isole l'adresse
		and ecx, 0xC000					; on isole les bits de CD
		or ebx, eax						; ebx = Address IO VRAM
		and edx, 0xF0					; on isole les bits de CD
		shr ecx, 12						;		"		"
		mov [Ctrl.Address], bx			; Ctrl.Address = Address de depart pour le port VDP Data
		or edx, ecx						; edx = CD
		mov eax, [CD_Table + edx]		; eax = Location & Read/Write
		mov [Ctrl.Access], al			; on stocke l'accés

		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32
	
	.Second_Word
		push ecx
		push edx
		mov cx, [Ctrl.Data]					; cx = 1st word (AS)
		mov edx, eax						; dx = ax = 2nd word (AS)
		mov [Ctrl.Data + 2], ax				; on stocke le controle complet
		mov ebx, ecx						; bx = 1st word (AS)
		shl eax, 14							; on isole l'adresse
		and ebx, 0x3FFF						; on isole l'adresse
		and ecx, 0xC000						; on isole les bits de CD
		or ebx, eax							; ebx = Address IO VRAM
		and edx, 0xF0						; on isole les bits de CD
		shr ecx, 12							;		"		"
		mov [Ctrl.Address], bx				; Ctrl.Address = Address de depart pour le port VDP Data
		or edx, ecx							; edx = CD
		mov eax, [CD_Table + edx]			; eax = Location & Read/Write
		mov byte [Ctrl.Flag], 0				; on en a finit avec Address Set
		test ah, ah							; on teste si il y a transfert DMA
		mov [Ctrl.Access], al				; on stocke l'accés
		mov al, ah
		jnz short DO_DMA					; si oui on y va

		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32
	
	DO_DMA:
		push edi
		push esi

	pushad
	sub esi,ebp
	sub esi,byte 2
	mov [_hook_pc],esi
	mov [_hook_value],eax
	call _hook_dma
	popad

		test dword [VDP_Reg.Set_2], 0x10		; DMA enable ?
		jz near NO_DMA
		test al, 0x4							; DMA FILL ?
		jz short .No_Fill
			cmp byte [Ctrl.DMA_Mode], 0x80
			jne short .No_Fill
			mov [Ctrl.DMA], al						; on stocke le type de DMA
			pop esi
			pop edi
			pop edx
			pop ecx
			pop ebx
			ret

	ALIGN32
	
	.No_Fill
		mov ecx, [VDP_Reg.DMA_Length]		; ecx = DMA Length
		mov esi, [VDP_Reg.DMA_Address]		; esi = DMA Source Address / 2
		and ecx, 0xFFFF
		mov edi, [Ctrl.Address]				; edi = Address Dest
		jz near NO_DMA

		and edi, 0xFFFF						; edi = Address Dest
		cmp byte [Ctrl.DMA_Mode], 0xC0		; DMA Copy ?
		mov edx, [VDP_Reg.Auto_Inc]			; edx = Auto Inc
		je near V_RAM_Copy

	MEM_To_V_RAM:
		add esi, esi						; esi = DMA Source Address
		test dword [Ctrl.DMA_Mode], 0x80
		jnz near NO_DMA
		xor ebx, ebx
		and eax, byte 3						; eax = destination DMA (1:VRAM, 2:CRAM, 3:VSRAM)
		cmp esi, [Rom_Size]
		jb short .DMA_Src_OK				; Src = ROM (ebx = 0)
		mov ebx, 1
		test byte [SegaCD_Started], 0xFF
		jz short .DMA_Src_OK				; Src = Normal RAM (ebx = 1)

		cmp esi, 0x00240000
		jae short .DMA_Src_OK				; Src = Normal RAM (ebx = 1)
		cmp esi, 0x00040000
		mov ebx, 2
		jb short .DMA_Src_OK				; Src = PRG RAM (ebx = 2)

		mov bh, [Ram_Word_State]
		mov bl, 3							; Src = WORD RAM ; 3 = WORD RAM 2M
		and bh, 3							; 4 = BAD
		add bl, bh							; 5 = WORD RAM 1M Bank 0
		xor bh, bh							; 6 = WORD RAM 1M Bank 1
		cmp bl, 5
		jb short .DMA_Src_OK
		cmp esi, 0x00220000
		jb short .DMA_Src_OK				; 7 = CELL ARRANGED Bank 0
		add bl, 2							; 8 = CELL ARRANGED Bank 1

	.DMA_Src_OK
		test eax, 0x2						; Dest = CRAM or VSRAM
		lea ebx, [ebx * 4 + eax]
		jz short .DMA_Dest_OK
		and edi, byte 0x7F
	.DMA_Dest_OK
		or word [VDP_Status], 0x0002
		xor eax, eax
		jmp [.Table_DMA + ebx * 4]			; on effectue le transfert DMA adéquat

	ALIGN4

	.Table_DMA
		dd NO_DMA, .ROM_To_VRam_Star, .ROM_To_CRam_Star, .ROM_To_VSRam_Star
		dd NO_DMA, .RAM_To_VRam_Star, .RAM_To_CRam_Star, .RAM_To_VSRam_Star
		dd NO_DMA, .RAMPRG_To_VRam_Star, .RAMPRG_To_CRam_Star, .RAMPRG_To_VSRam_Star
		dd NO_DMA, .RAMWORD2M_To_VRam_Star, .RAMWORD2M_To_CRam_Star, .RAMWORD2M_To_VSRam_Star

		dd NO_DMA, NO_DMA, NO_DMA, NO_DMA
		dd NO_DMA, .RAMWORD1M0_To_VRam_Star, .RAMWORD1M0_To_CRam_Star, .RAMWORD1M0_To_VSRam_Star
		dd NO_DMA, .RAMWORD1M1_To_VRam_Star, .RAMWORD1M1_To_CRam_Star, .RAMWORD1M1_To_VSRam_Star
		dd NO_DMA, .CELL0_To_VRam_Star, .CELL0_To_CRam_Star, .CELL0_To_VSRam_Star

		dd NO_DMA, .CELL1_To_VRam_Star, .CELL1_To_CRam_Star, .CELL1_To_VSRam_Star
		dd NO_DMA, NO_DMA, NO_DMA, NO_DMA
		dd NO_DMA, NO_DMA, NO_DMA, NO_DMA
		dd NO_DMA, NO_DMA, NO_DMA, NO_DMA

	ALIGN32
	
	.ROM_To_VRam_Star
		DMA_LOOP 0, 0

	ALIGN32
	
	.ROM_To_CRam_Star
		DMA_LOOP 0, 1

	ALIGN32
	
	.ROM_To_VSRam_Star
		DMA_LOOP 0, 2

	ALIGN32
	
	.RAM_To_VRam_Star
		DMA_LOOP 1, 0

	ALIGN32
	
	.RAM_To_CRam_Star
		DMA_LOOP 1, 1

	ALIGN32
	
	.RAM_To_VSRam_Star
		DMA_LOOP 1, 2

	ALIGN32
	
	.RAMPRG_To_VRam_Star
		DMA_LOOP 2, 0

	ALIGN32
	
	.RAMPRG_To_CRam_Star
		DMA_LOOP 2, 1

	ALIGN32
	
	.RAMPRG_To_VSRam_Star
		DMA_LOOP 2, 2

	ALIGN32
	
	.RAMWORD2M_To_VRam_Star
		DMA_LOOP 3, 0

	ALIGN32
	
	.RAMWORD2M_To_CRam_Star
		DMA_LOOP 3, 1

	ALIGN32
	
	.RAMWORD2M_To_VSRam_Star
		DMA_LOOP 3, 2

	ALIGN32
	
	.RAMWORD1M0_To_VRam_Star
		DMA_LOOP 5, 0

	ALIGN32
	
	.RAMWORD1M0_To_CRam_Star
		DMA_LOOP 5, 1

	ALIGN32
	
	.RAMWORD1M0_To_VSRam_Star
		DMA_LOOP 5, 2

	ALIGN32
	
	.RAMWORD1M1_To_VRam_Star
		DMA_LOOP 6, 0

	ALIGN32
	
	.RAMWORD1M1_To_CRam_Star
		DMA_LOOP 6, 1

	ALIGN32
	
	.RAMWORD1M1_To_VSRam_Star
		DMA_LOOP 6, 2

	ALIGN32
	
	.CELL0_To_VRam_Star
		DMA_LOOP 7, 0

	ALIGN32
	
	.CELL0_To_CRam_Star
		DMA_LOOP 7, 1

	ALIGN32
	
	.CELL0_To_VSRam_Star
		DMA_LOOP 7, 2

	ALIGN32
	
	.CELL1_To_VRam_Star
		DMA_LOOP 8, 0

	ALIGN32
	
	.CELL1_To_CRam_Star
		DMA_LOOP 8, 1

	ALIGN32
	
	.CELL1_To_VSRam_Star
		DMA_LOOP 8, 2

	ALIGN32

	.End_DMA
		mov eax, [VDP_Reg.DMA_Length]
		mov [Ctrl.Address], bx
		mov esi, [VDP_Reg.DMA_Address]
		sub eax, ecx
		mov [VDP_Reg.DMA_Length], ecx
		lea esi, [esi + eax]
		jbe short .Nothing_To_Do

		and esi, 0x7FFFFF
		mov [DMAT_Length], eax
		mov [VDP_Reg.DMA_Address], esi
		call Update_DMA
		call _main68k_releaseCycles
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32
	
	.Nothing_To_Do
		and word [VDP_Status], 0xFFFD
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32
	
	V_RAM_Copy:
		or word [VDP_Status], 0x0002
		and esi, 0xFFFF
		mov dword [VDP_Reg.DMA_Length], 0
		mov dword [DMAT_Length], ecx
		mov dword [DMAT_Type], 0x3
		mov dword [VRam_Flag], 1
		jmp short .VRam_Copy_Loop

	ALIGN32

	.VRam_Copy_Loop
			mov al, [VRam + esi]					; ax = Src
			inc si									; on augment pointeur Src de 1
			mov [VRam + edi], al					; VRam[Dest] = Src.W
			add di, dx								; Adr = Adr + Auto_Inc
			dec ecx									; un transfert de moins
			jnz short .VRam_Copy_Loop				; si DMA Length >= 0 alors on continue le transfert DMA

		mov [VDP_Reg.DMA_Address], esi
		mov [Ctrl.Address], di					; on stocke la nouvelle Data_Address
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN32
	
	NO_DMA:
		mov dword [Ctrl.DMA], 0
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
	
	ALIGN32
	
	Table_Set_Reg:
		dd	Set_Regs.Set1, Set_Regs.Set2, Set_Regs.ScrA, Set_Regs.Win
		dd	Set_Regs.ScrB, Set_Regs.Spr, Set_Regs, Set_Regs.BGCol
		dd	Set_Regs, Set_Regs, Set_Regs.HInt, Set_Regs.Set3
		dd	Set_Regs.Set4, Set_Regs.HScr, Set_Regs, Set_Regs
		dd	Set_Regs.ScrSize, Set_Regs.WinH, Set_Regs.WinV, Set_Regs.DMALL
		dd	Set_Regs.DMALH, Set_Regs.DMAAL, Set_Regs.DMAAM, Set_Regs.DMAAH
		dd	Set_Regs.Wrong, Set_Regs.Wrong, Set_Regs.Wrong, Set_Regs.Wrong
		dd	Set_Regs.Wrong, Set_Regs.Wrong, Set_Regs.Wrong, Set_Regs.Wrong
		
	ALIGN32

	;void Set_VDP_Reg(int Num_Reg, unsigned char val);
	DECL Set_VDP_Reg

		push ebx

		mov ebx, [esp + 8]
		mov eax, [esp + 12]
		and ebx, byte 0x1F
		and eax, 0xFF
		jmp [Table_Set_Reg + ebx * 4]

	ALIGN32
	
	Set_Regs:
	; al = valeur
	; ebx = numero de registre
	; ne pas oublier de depiler ebx a la fin

		mov [VDP_Reg + ebx * 4], al
		pop ebx
		ret

	ALIGN32
	
	.Set1
		mov [VDP_Reg.Set_1], al
		call Update_IRQ_Line
		pop ebx
		ret

	ALIGN32
	
	.Set2
		mov [VDP_Reg.Set_2], al
		call Update_IRQ_Line
		pop ebx
		ret
 	
	ALIGN32
	
	.Set3
		test al, 4
		mov [VDP_Reg.Set_3], al
		jnz short .VScroll_Cell

		and eax, 3
		pop ebx
		mov eax, [H_Scroll_Mask_Table + eax * 4]
		mov byte [V_Scroll_MMask], 0
		mov [H_Scroll_Mask], eax
		ret

	ALIGN4
	
	.VScroll_Cell
		and eax, 3
		pop ebx
		mov eax, [H_Scroll_Mask_Table + eax * 4]
		mov byte [V_Scroll_MMask], 0x7E
		mov [H_Scroll_Mask], eax
		ret

	ALIGN32
	
	.Set4
		mov [VDP_Reg.Set_4], al
		mov byte [CRam_Flag], 1
		test al, 0x81
		pop ebx
		jz short .HCell_32

		mov dword [H_Cell], 40
		mov dword [H_Win_Mul], 6
		mov dword [H_Pix], 320
		mov dword [H_Pix_Begin], 0

		mov eax, [VDP_Reg.Pat_WIN_Adr]
		and eax, byte 0x3C
		shl eax, 10
		add eax, VRam
		mov [Win_Addr], eax

		mov al, [VDP_Reg.Win_H_Pos]
		and	al, 0x1F
		add al, al
		cmp al, 40
		jbe short .HCell_40_ok

		mov al, 40

	.HCell_40_ok
		mov [Win_X_Pos], al
		mov eax, [VDP_Reg.Spr_Att_Adr]
		and eax, byte 0x7E
		shl eax, 9
		add eax, VRam
		mov [Spr_Addr], eax
		ret

	ALIGN32

	.HCell_32
		mov dword [H_Cell], 32
		mov dword [H_Win_Mul], 5
		mov dword [H_Pix], 256
		mov dword [H_Pix_Begin], 32

		mov eax, [VDP_Reg.Pat_WIN_Adr]
		and eax, byte 0x3E
		shl eax, 10
		add eax, VRam
		mov [Win_Addr], eax

		mov al, [VDP_Reg.Win_H_Pos]
		and	al, 0x1F
		add al, al
		cmp al, 32
		jbe short .HCell_32_ok

		mov al, 32

	.HCell_32_ok
		mov [Win_X_Pos], al
		mov eax, [VDP_Reg.Spr_Att_Adr]
		and eax, byte 0x7F
		shl eax, 9
		add eax, VRam
		mov [Spr_Addr], eax
		ret

	ALIGN32
	
	.ScrA
		mov [VDP_Reg.Pat_ScrA_Adr], al
		and eax, 0x38
		shl eax, 10
		pop ebx
		add eax, VRam
		mov [ScrA_Addr], eax
		ret

	ALIGN32
	
	.Win
		test byte [VDP_Reg.Set_4], 0x1
		mov [VDP_Reg.Pat_WIN_Adr], al
		jnz short .w2

		and eax, 0x3E
		shl eax, 10
		pop ebx
		add eax, VRam
		mov [Win_Addr], eax
		ret

	ALIGN4

	.w2
		and eax, 0x3C
		shl eax, 10
		pop ebx
		add eax, VRam
		mov [Win_Addr], eax
		ret

	ALIGN32
	
	.ScrB
		mov [VDP_Reg.Pat_ScrB_Adr], al
		and eax, 0x7
		shl eax, 13
		pop ebx
		add eax, VRam
		mov [ScrB_Addr], eax
		ret

	ALIGN32
	
	.Spr
		test byte [VDP_Reg.Set_4], 0x1
		mov [VDP_Reg.Spr_Att_Adr], al
		jnz short .spr2

		and eax, 0x7F
		or byte [VRam_Flag], 2
		shl eax, 9
		pop ebx
		add eax, VRam
		mov [Spr_Addr], eax
		ret

	ALIGN4

	.spr2
		and eax, 0x7E
		or byte [VRam_Flag], 2
		shl eax, 9
		pop ebx
		add eax, VRam
		mov [Spr_Addr], eax
		ret

	ALIGN32
	
	.BGCol
		and eax, 0x3F
		pop ebx
		mov byte [CRam_Flag], 1
		mov [VDP_Reg.BG_Color], eax
		ret

	ALIGN32
	
	.HInt
		mov	[VDP_Reg.H_Int_Reg], al
		pop ebx
		ret

	ALIGN32
	
	.HScr
		mov [VDP_Reg.H_Scr_Adr], al
		and eax, 0x3F
		shl eax, 10
		pop ebx
		add eax, VRam
		mov [H_Scroll_Addr], eax
		ret

	ALIGN32
	
	.ScrSize
		mov ebx, eax
		mov [VDP_Reg.Scr_Size], al
		and ebx, 0x03
		and eax, 0x30
		jmp [.ScrSize_Table + eax + ebx * 4]

	ALIGN4
	
	.ScrSize_Table

		dd .V32_H32, .V32_H64, .V32_HXX, .V32_H128
		dd .V64_H32, .V64_H64, .V64_HXX, .V64_H128
		dd .VXX_H32, .VXX_H64, .VXX_HXX, .VXX_H128
		dd .V128_H32, .V128_H64, .V128_HXX, .V128_H128

	ALIGN4

	.V32_H32
	.VXX_H32
		mov dword [H_Scroll_CMul], 5
		mov dword [H_Scroll_CMask], 31
		mov dword [V_Scroll_CMask], 31
		pop ebx
		ret

	ALIGN4

	.V64_H32
		mov dword [H_Scroll_CMul], 5
		mov dword [H_Scroll_CMask], 31
		mov dword [V_Scroll_CMask], 63
		pop ebx
		ret

	ALIGN4

	.V128_H32
		mov dword [H_Scroll_CMul], 5
		mov dword [H_Scroll_CMask], 31
		mov dword [V_Scroll_CMask], 127
		pop ebx
		ret

	ALIGN4

	.V32_H64
	.VXX_H64
		mov dword [H_Scroll_CMul], 6
		mov dword [H_Scroll_CMask], 63
		mov dword [V_Scroll_CMask], 31
		pop ebx
		ret

	ALIGN4

	.V64_H64
	.V128_H64
		mov dword [H_Scroll_CMul], 6
		mov dword [H_Scroll_CMask], 63
		mov dword [V_Scroll_CMask], 63
		pop ebx
		ret

	ALIGN4

	.V32_HXX
	.V64_HXX
	.VXX_HXX
	.V128_HXX
		mov dword [H_Scroll_CMul], 6
		mov dword [H_Scroll_CMask], 63
		mov dword [V_Scroll_CMask], 0
		pop ebx
		ret

	ALIGN4

	.V32_H128
	.V64_H128
	.VXX_H128
	.V128_H128
		mov dword [H_Scroll_CMul], 7
		mov dword [H_Scroll_CMask], 127
		mov dword [V_Scroll_CMask], 31
		pop ebx
		ret

	ALIGN32
	
	.WinH
		mov [VDP_Reg.Win_H_Pos], al
		and	eax, 0x1F
		pop ebx
		add eax, eax
		cmp eax, [H_Cell]
		jbe short .WinH_ok

		mov eax, [H_Cell]

	.WinH_ok
		mov [Win_X_Pos], eax
		ret

	ALIGN32
	
	.WinV
		mov [VDP_Reg.Win_V_Pos], al
		and eax, 0x1F
		pop ebx
		mov [Win_Y_Pos], eax
		ret

	ALIGN32
	
	.DMALL
		mov [VDP_Reg.DMA_Length_L], al
		pop ebx
		mov [VDP_Reg.DMA_Length], al
		ret

	ALIGN32
	
	.DMALH
		mov [VDP_Reg.DMA_Length_H], al
		pop ebx
		mov [VDP_Reg.DMA_Length + 1], al
		pushad
		sub esi,ebp
		sub esi,byte 2
		mov [_dma_len],esi
		popad
		ret

	ALIGN32
	
	.DMAAL
		mov [VDP_Reg.DMA_Src_Adr_L], al
		pop ebx
		mov [VDP_Reg.DMA_Address], al
		ret

	ALIGN32
	
	.DMAAM
		mov [VDP_Reg.DMA_Src_Adr_M], al
		pop ebx
		mov [VDP_Reg.DMA_Address + 1], al
		ret

	ALIGN32
	
	.DMAAH
		mov [VDP_Reg.DMA_Src_Adr_H], al
		mov ebx, eax
		and eax, 0x7F
		and ebx, 0xC0
		mov [VDP_Reg.DMA_Address + 2], al
		mov [Ctrl.DMA_Mode], ebx				; DMA Mode

		pushad
		sub esi,ebp
		sub esi,byte 2
		mov [_dma_src],esi
		popad

		pop ebx
		ret

	ALIGN32
	
	.Wrong
		pop ebx
		ret


	ALIGN32

	;void Int_Ack(void);	function called by the 68k when an interrupt is acknowledged...
	DECL Int_Ack
		test byte [VDP_Reg.Set_2], 0x20
		jz short .H_Ack
		test byte [VDP_Int], 8
		jz short .H_Ack

	.V_Ack
		mov ah, [VDP_Reg.Set_1]
		mov al, [VDP_Int]
		and ah, 0x10
		and al, ~8
		shr ah, 2
		mov [VDP_Int], al
		and al, ah
		ret

	ALIGN4

	.No_H_Int
		xor al, al
		ret

	ALIGN4

	.H_Ack
		xor al, al
		mov byte [VDP_Int], al
		ret

	ALIGN32

	;void Update_IRQ_Line(void);
	DECL Update_IRQ_Line
		test byte [VDP_Reg.Set_2], 0x20
		jz short .No_V_Int
		test byte [VDP_Int], 0x8
		jz short .No_V_Int

		push dword -1
		push dword 6
		call _main68k_interrupt
		add esp, 8
		ret

	ALIGN4

	.No_V_Int
		test byte [VDP_Reg.Set_1], 0x10
		jz short .No_H_Int
		test byte [VDP_Int], 0x4
		jz short .No_H_Int

		push dword -1
		push dword 4
		call _main68k_interrupt
		add esp, 8
		ret

	ALIGN4

	.No_H_Int
		and byte [_main68k_context + 35 * 4], 0xF0
		ret