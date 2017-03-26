%include "nasmhead.inc"

%define HIGH_B   0x80
%define SHAD_B   0x40
%define BACK_B   0x01 ; backdrop
%define PRIO_B   0x02
%define SPR_B    0x20

%define HIGH_W   0x8080
%define SHAD_W   0x4040
%define NOSHAD_W 0xBFBF
%define BACK_W   0x0100 ; backdrop
%define PRIO_W   0x0200
%define SPR_W    0x2000

%define SHAD_D   0x40404040
%define NOSHAD_D 0xBFBFBFBF


section .data align=64

	DECL TAB336

	%assign i 0
	%rep 240
		dd (i * 336)
%assign i i+1
	%endrep

	ALIGN32
	
	Mask_N	dd 0xFFFFFFFF, 0xFFF0FFFF, 0xFF00FFFF, 0xF000FFFF, 
			dd 0x0000FFFF, 0x0000FFF0, 0x0000FF00, 0x0000F000

	Mask_F	dd 0xFFFFFFFF, 0xFFFF0FFF, 0xFFFF00FF, 0xFFFF000F, 
			dd 0xFFFF0000, 0x0FFF0000, 0x00FF0000, 0x000F0000


section .bss align=64

	extern VRam
	extern VRam_Flag
	extern CRam
	extern CRam_Flag
	extern VSRam
	extern VDP_Reg
	extern VDP_Current_Line
	extern VDP_Status
	extern H_Cell
	extern H_Win_Mul
	extern H_Pix
	extern H_Pix_Begin
	extern H_Scroll_Mask
	extern H_Scroll_CMul
	extern H_Scroll_CMask
	extern V_Scroll_CMask
	extern V_Scroll_MMask
	extern Win_X_Pos
	extern Win_Y_Pos
	extern ScrA_Addr
	extern ScrB_Addr
	extern Win_Addr
	extern Spr_Addr
	extern H_Scroll_Addr
	extern _32X_Started
	extern _32X_Palette_16B
	extern _32X_VDP_Ram
	extern _32X_VDP_CRam
	extern _32X_VDP_CRam_Ajusted
	extern _32X_VDP

	struc vx
		.Mode		resd 1
		.State		resd 1
		.AF_Data	resd 1
		.AF_St		resd 1
		.AF_Len		resd 1
	endstruc

	resw (320 + 32)

	DECL Screen_16X
	resw (336 * 240)

	DECL Screen_32X
	resw (336 * 240)

	DECL MD_Screen
	resw (336 * 240)

	resw (320 + 32)	

	resd (320 + 32)

	DECL MD_Screen32
	resd (336 * 240)

	resd (320 + 32)

	DECL MD_Palette
	resw 0x100

	DECL Palette
	resw 0x8000

	DECL MD_Palette32
	resd 0x100

	DECL Palette32
	resd 0x8000
	
	DECL LockedPalette
	resw 0x40

	DECL Sprite_Struct
	resd (0x100 * 8)
	
	DECL Sprite_Visible
	resd 0x100

	DECL Data_Spr
	.H_Min			resd 1
	.H_Max			resd 1

	ALIGN_32
	
	DECL Data_Misc
	.Pattern_Adr	resd 1
	.Line_7			resd 1
	.X				resd 1
	.Cell			resd 1
	.Start_A		resd 1
	.Length_A		resd 1
	.Start_W		resd 1
	.Length_W		resd 1
	.Mask			resd 1
	.Spr_End		resd 1
	.Next_Cell		resd 1
	.Palette		resd 1
	.Borne			resd 1

	ALIGN_4
	DECL _32X_Rend_Mode	
	resb 1
	
	DECL Sprite_Over
	resd 1
	DECL Sprite_Boxing
	resd 1
	DECL Mode_555
	resd 1
	DECL VScrollAl ;Nitsuja added this
	resb 1
	DECL VScrollBl ;Nitsuja added this
	resd 1
	DECL VScrollAh ;Nitsuja added this
	resb 1
	DECL VScrollBh ;Nitsuja added this
	resb 1
	DECL VSpritel ;Nitsuja added this
	resb 1
	DECL VSpriteh ;Nitsuja added this
	resb 1
	DECL Sprite_Always_Top
	resb 1
	DECL Swap_Scroll_PriorityA
	resb 1
	DECL Swap_Scroll_PriorityB
	resb 1
	DECL Swap_Sprite_Priority
	resb 1
	DECL Swap_32X_Plane_Priority
	resb 1
	DECL _32X_Plane_High_On
	resb 1
	DECL _32X_Plane_Low_On
	resb 1
	DECL _32X_Plane_On
	resb 1
	DECL ScrollAOn
	resb 1
	DECL ScrollBOn
	resb 1
	DECL SpriteOn
	resb 1
	DECL PalLock
	resb 1
	DECL Bits32
	resb 1
	DECL PinkBG
	resb 1

section .text align=64


;****************************************

; macro GET_X_OFFSET
; param :
; %1 = 0 for scroll B and 1 scroll A
; return :
; - esi contains X offset of the line in court
; - edi contains the number of line in court

%macro GET_X_OFFSET 1

	mov eax, [VDP_Current_Line]
	mov ebx, [H_Scroll_Addr]			; ebx point on the data of H-Scroll
	mov edi, eax
	and eax, [H_Scroll_Mask]

%if %1 > 0
	mov esi, [ebx + eax * 4]			; X Cell offset
%else
	mov si, [ebx + eax * 4 + 2]			; X Cell offset
%endif

%endmacro


;****************************************

; macro UPDATE_Y_OFFSET
; takes :
; eax = current cell
; param :
; %1 = 0 for scroll B and 1 for scroll A
; %2 = 0 for normal mode and 1 for interlaced mode
; returns :
; edi = Y Offset in function of current cell

%macro UPDATE_Y_OFFSET 2

	mov eax, [Data_Misc.Cell]				; Current cell for the V Scroll
	test eax, 0xFF81						; outside the limits of the VRAM? Then don't change…
	jnz short %%End
	mov edi, [VDP_Current_Line]				; edi = line number

%if %1 > 0
	mov eax, [VSRam + eax * 2 + 0]
%else
	mov ax, [VSRam + eax * 2 + 2]
%endif

%if %2 > 0
	shr eax, 1								; divide Y scroll by 2 if interlaced
%endif

	add edi, eax
	mov eax, edi
	shr edi, 3								; V Cell Offset
	and eax, byte 7							; adjusts for pattern
	and edi, [V_Scroll_CMask]				; prevents V Cell Offset from overflowing
	mov [Data_Misc.Line_7], eax

%%End

%endmacro


;****************************************

; macro GET_PATTERN_INFO
; takes :
; - H_Scroll_CMul must be correctly initialized
; - esi and edi contain X offset and Y offset respectively
; param :
; %1 = 0 for scroll B and 1 for scroll A
; returns :
; -  ax = Pattern Info

%macro GET_PATTERN_INFO 1

	mov cl, [H_Scroll_CMul]
	mov eax, edi								; eax = V Cell Offset
	mov edx, esi								; edx = H Cell Offset

	shl eax, cl									; eax = V Cell Offset * H Width

%if %1 > 0
	mov ebx, [ScrA_Addr]
%else
	mov ebx, [ScrB_Addr]
%endif

	add edx, eax								; edx = (V Offset / 8) * H Width + (H Offset / 8)
	mov ax, [ebx + edx * 2]						; ax = Cell Info inverse

%endmacro


;****************************************

; macro GET_PATTERN_DATA
; param :
; %1 = 0 for normal mode and 1 for interlaced mode
; %2 = 0 pour les scrolls, 1 for la window
; takes :
; - ax = Pattern Info
; - edi contains Y offset (if %2 = 0)
; - Data_Misc.Line_7 contains Line & 7 (if %2! = 0)
; returns :
; - ebx = Pattern Data
; - edx = Palette Num * 16

%macro GET_PATTERN_DATA 2

	mov ebx, [Data_Misc.Line_7]					; ebx = V Offset
	mov edx, eax								; edx = Cell Info
	mov ecx, eax								; ecx = Cell Info
	shr edx, 9
	and ecx, 0x7FF
	and edx, byte 0x30							; edx = Palette

%if %1 > 0
	shl ecx, 6									; pattern number * 64 (interlaced)
%else
	shl ecx, 5									; pattern number * 32 (normal)
%endif

	test eax, 0x1000							; V-Flip?
	jz %%No_V_Flip								; if yes, then

	xor ebx, byte 7

%%No_V_Flip

%if %1 > 0
	mov ebx, [VRam + ecx + ebx * 8]				; ebx = Line of the pattern = Pattern Data (interlaced)
%else
	mov ebx, [VRam + ecx + ebx * 4]				; ebx = Line of the pattern = Pattern Data (normal)
%endif

%endmacro


;****************************************

; macro MAKE_SPRITE_STRUCT
; param :
; %1 = 0 for normal mode and 1 for interlaced mode

%macro MAKE_SPRITE_STRUCT 1

	mov ebp, [Spr_Addr]
	xor edi, edi							; edi = 0
	mov esi, ebp							; esi point on the table of sprite data
	jmp short %%Loop

	ALIGN32
	
%%Loop
		mov ax, [ebp + 0]						; ax = Pos Y
		mov cx, [ebp + 6]						; cx = Pos X
		mov dl, [ebp + (2 ^ 1)]					; dl = Sprite Size
	%if %1 > 0
		shr eax, 1								; if interlaced, the position is divided by 2
	%endif
		mov dh, dl
		and eax, 0x1FF
		and ecx, 0x1FF
		and edx, 0x0C03							; isolate Size X and Size Y in dh and dl respectively
		sub eax, 0x80							; eax = Pos Y correct
		sub ecx, 0x80							; ecx = Pos X correct
		shr dh, 2								; dh = Size X - 1
		mov [Sprite_Struct + edi + 4], eax		; store Pos Y
		inc dh									; dh = Size X
		mov [Sprite_Struct + edi + 0], ecx		; store Pos X
		mov bl, dh								; bl = Size X
		mov [Sprite_Struct + edi + 8], dh		; store Size X
		and ebx, byte 7							; ebx = Size X
		mov [Sprite_Struct + edi + 12], dl		; store Size Y - 1
		and edx, byte 3							; edx = Size Y - 1
		lea ecx, [ecx + ebx * 8 - 1]			; ecx = Pos X Max
		lea eax, [eax + edx * 8 + 7]			; eax = Pos Y Max
		mov bl, [ebp + (3 ^ 1)]					; bl = Pointer towards next the sprite
		mov [Sprite_Struct + edi + 16], ecx		; store Pos X Max
		mov dx, [ebp + 4]						; dx = 1st tile of the sprite
		mov [Sprite_Struct + edi + 20], eax		; store Pos Y Max
		add edi, byte (8 * 4)					; advance to the next sprite structure
		and ebx, byte 0x7F						; clear the highest order bit.
		mov [Sprite_Struct + edi - 32 + 24], dx	; store the first tile of the sprite
		jz short %%End							; if the next pointer is 0, end
		lea ebp, [esi + ebx * 8]				; ebp Pointer towards next the sprite
		cmp edi, (8 * 4 * 80)					; if there are already 80 sprites defined then stop
		jb near %%Loop

%%End
	sub edi, 8 * 4
	mov [Data_Misc.Spr_End], edi			; store the pointer to the last sprite

%endmacro


;****************************************

; macro MAKE_SPRITE_STRUCT_PARTIAL
; param :

%macro MAKE_SPRITE_STRUCT_PARTIAL 0

	mov ebp, [Spr_Addr]
;	xor eax, eax
	xor ebx, ebx
	xor edi, edi							; edi = 0
	mov esi, ebp							; esi point on the table of sprite data
	jmp short %%Loop

	ALIGN32
	
%%Loop
		mov al, [ebp + (2 ^ 1)]					; al = Sprite Size
		mov bl, [ebp + (3 ^ 1)]					; bl = point towards the next sprite
		mov cx, [ebp + 6]						; cx = Pos X
		mov dx, [ebp + 4]						; dx = 1st tile of the sprite
		and ecx, 0x1FF
		mov [Sprite_Struct + edi + 24], dx		; store the 1st tile of the sprite
		sub ecx, 0x80							; ecx = Pos X correct
		and eax, 0x0C
		mov [Sprite_Struct + edi + 0], ecx		; store Pos X
		lea ecx, [ecx + eax * 2 + 7]			; ecx = Pos X Max
		and bl, 0x7F							; clear the highest order bit.
		mov [Sprite_Struct + edi + 16], ecx		; store Pos X Max
		jz short %%End							; if the next pointer is 0, end

		add edi, byte (8 * 4)					; advance to the next sprite structure
		lea ebp, [esi + ebx * 8]				; ebp Pointer towards next the sprite
		cmp edi, (8 * 4 * 80)					; if there are already 80 sprites defined then stop
		jb short %%Loop

%%End

%endmacro


;****************************************

; macro UPDATE_MASK_SPRITE
; param :
; %1 = Sprite Limit Emulation (1 = enabled and 0 = disabled)
; takes :
; - Sprite_Struct must be correctly initialized
; returns :
; - edi points on the first sprite structure to post.
; - edx contains the number of line

%macro UPDATE_MASK_SPRITE 1

	xor edi, edi
%if %1 > 0
	mov ecx, [H_Cell]
%endif
	xor ax, ax						; used for masking
	mov ebx, [H_Pix]
	xor esi, esi
	mov edx, [VDP_Current_Line]
	jmp short %%Loop_1

	ALIGN4
	
%%Loop_1
		cmp [Sprite_Struct + edi + 4], edx			; one tests if the sprite is on the current line
		jg short %%Out_Line_1
		cmp [Sprite_Struct + edi + 20], edx			; one tests if the sprite is on the current line
		jl short %%Out_Line_1

%if %1 > 0
		sub ecx, [Sprite_Struct + edi + 8]
%endif
		cmp [Sprite_Struct + edi + 0], ebx			; one tests if the sprite is not outside of the screen
		jge short %%Out_Line_1_2
		cmp dword [Sprite_Struct + edi + 16], 0		; one tests if the sprite is not outside of the screen
		jl short %%Out_Line_1_2

		mov [Sprite_Visible + esi], edi
		add esi, byte 4

%%Out_Line_1_2
		add edi, byte (8 * 4)
		cmp edi, [Data_Misc.Spr_End]
		jle short %%Loop_2

		jmp %%End

	ALIGN4

%%Out_Line_1
		add edi, byte (8 * 4)
		cmp edi, [Data_Misc.Spr_End]
		jle short %%Loop_1

		jmp %%End

	ALIGN4

%%Loop_2
		cmp [Sprite_Struct + edi + 4], edx			; one tests if the sprite is on the current line
		jg short %%Out_Line_2
		cmp [Sprite_Struct + edi + 20], edx			; one tests if the sprite is on the current line
		jl short %%Out_Line_2

%%Loop_2_First
		cmp dword [Sprite_Struct + edi + 0], -128	; the sprite is a mask?
		je short %%End								; next sprites are masked

%if %1 > 0
		sub ecx, [Sprite_Struct + edi + 8]
%endif
		cmp [Sprite_Struct + edi + 0], ebx			; one tests if the sprite is not outside of the screen
		jge short %%Out_Line_2
		cmp dword [Sprite_Struct + edi + 16], 0		; one tests if the sprite is not outside of the screen
		jl short %%Out_Line_2

		mov [Sprite_Visible + esi], edi
		add esi, byte 4

%%Out_Line_2
		add edi, byte (8 * 4)
%if %1 > 0
		cmp ecx, byte 0
		jle short %%Sprite_Overflow
%endif
		cmp edi, [Data_Misc.Spr_End]
		jle short %%Loop_2
		jmp short %%End

	ALIGN4
	
%%Sprite_Overflow
	cmp edi, [Data_Misc.Spr_End]
	jg short %%End
	jmp short %%Loop_3

	ALIGN4
	
	%%Loop_3
		cmp [Sprite_Struct + edi + 4], edx			; one tests if the sprite is on the current line
		jg short %%Out_Line_3
		cmp [Sprite_Struct + edi + 20], edx			; one tests if the sprite is on the current line
		jl short %%Out_Line_3

		or byte [VDP_Status], 0x40
		jmp short %%End

%%Out_Line_3
		add edi, byte (8 * 4)
		cmp edi, [Data_Misc.Spr_End]
		jle short %%Loop_3
		jmp short %%End

	ALIGN4

%%End
	mov [Data_Misc.Borne], esi


%endmacro


;****************************************

; macro PUTPIXEL_P0
; param :
; %1 = Number of the pixel
; %2 = Mask to isolate the good pixel
; %3 = Shift
; %4 = 0 for scroll B and one if not
; %5 = Shadow/Highlight enable
; takes :
; - ebx = Pattern Data
; - edx = Palette number * 64

%macro PUTPIXEL_P0 5

	mov eax, ebx
	and eax, %2
	jz short %%Trans

%if %4 > 0
	; Scroll A
	%if %5 > 0
		; Shadow/Highlight
		mov cl, [Screen_16X + ebp * 2 + (%1 * 2) + 1]
		test cl, PRIO_B
		jnz short %%Trans
	%else
		; No Shadow/Highlight
		test byte [Screen_16X + ebp * 2 + (%1 * 2) + 1], PRIO_B
		jnz short %%Trans
	%endif
%endif

%if %3 > 0
	; shift > 0
	shr eax, %3
%endif

%if %4 > 0
	; Scroll A
	%if %5 > 0
		; Shadow / Highlight
		and cl, SHAD_B
		add al, dl
		add al, cl
	%else
		add al, dl
	%endif
%else
	; Scroll B
	%if %5 > 0
		; Shadow / Highlight
		lea eax, [eax + edx + SHAD_W]
	%else
		add al, dl
	%endif
%endif

	mov [Screen_16X + ebp * 2 + (%1 * 2)], al	; set the pixel
	or [Screen_16X + ebp * 2 + (%1 * 2) + 1], byte BACK_B

%%Trans

%endmacro


;****************************************
; background layer graphics background graphics layer 1
; macro PUTPIXEL_P1
; param :
; %1 = pixel number
; %2 = mask to isolate the good pixel
; %3 = Shift
; %4 = Shadow/Highlight enable
; takes :
; - ebx = Pattern Data
; - edx = Palette number * 64

%macro PUTPIXEL_P1 4

	mov eax, ebx
	and eax, %2
	jz short %%Trans

%if %3 > 0
	shr eax, %3
%endif

	lea eax, [eax + edx + PRIO_W + BACK_W]
	mov [Screen_16X + ebp * 2 + (%1 * 2)], ax

%%Trans

%endmacro


;****************************************

; macro PUTPIXEL_SPRITE
; param :
; %1 = pixel number
; %2 = mask to isolate the good pixel
; %3 = Shift
; %4 = Priorité
; %5 = Highlight/Shadow Enable
; takes :
; - ebx = Pattern Data
; - edx = Palette number * 16

%macro PUTPIXEL_SPRITE 5

	mov eax, ebx
	and eax, %2
	jz short %%Trans

	mov cl, [Screen_16X + ebp * 2 + (%1 * 2) + 16 + 1]
	test cl, (PRIO_B + SPR_B - %4)
	jz short %%Affich

%%Prio
	or ch, cl
%if %4 < 1
	or byte [Screen_16X + ebp * 2 + (%1 * 2) + 16 + 1], SPR_B
%endif
	jmp %%Trans

ALIGN4

%%Affich

%if %3 > 0
	shr eax, %3
%endif

	lea eax, [eax + edx + SPR_W]

%if %5 > 0
	%if %4 < 1
		and cl, SHAD_B | HIGH_B
	%else
		and cl, HIGH_B
	%endif

	cmp eax, (0x3E + SPR_W)
	jb short %%Normal
	ja short %%Shadow

%%Highlight
	or word [Screen_16X + ebp * 2 + (%1 * 2) + 16], HIGH_W
	jmp short %%Trans
	
%%Shadow
	or word [Screen_16X + ebp * 2 + (%1 * 2) + 16], SHAD_W
	jmp short %%Trans

%%Normal
	add al, cl

%endif

	mov [Screen_16X + ebp * 2 + (%1 * 2) + 16], ax
	or [Screen_16X + ebp * 2 + (%1 * 2) + 16 + 1], byte BACK_B

%%Trans

%endmacro


;****************************************

; macro PUTLINE_P0
; param :
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_P0 2


%if %1 < 1
	; Scroll B
	%if %2 > 0
		; Shadow/Highlight is on
		mov dword [Screen_16X + ebp * 2 +  0], SHAD_D
		mov dword [Screen_16X + ebp * 2 +  4], SHAD_D
		mov dword [Screen_16X + ebp * 2 +  8], SHAD_D
		mov dword [Screen_16X + ebp * 2 + 12], SHAD_D
	%else
		; Shadow/Highlight is off
		mov dword [Screen_16X + ebp * 2 +  0], 0x00000000
		mov dword [Screen_16X + ebp * 2 +  4], 0x00000000
		mov dword [Screen_16X + ebp * 2 +  8], 0x00000000
		mov dword [Screen_16X + ebp * 2 + 12], 0x00000000
	%endif

	test byte [ScrollBOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
	test dword [VScrollBl], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%else
	; Scroll A
	test byte [ScrollAOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
	test dword [VScrollAl], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%endif

	test ebx, ebx
	jz near %%Full_Trans

	PUTPIXEL_P0 0, 0x0000f000, 12, %1, %2
	PUTPIXEL_P0 1, 0x00000f00,  8, %1, %2
	PUTPIXEL_P0 2, 0x000000f0,  4, %1, %2
	PUTPIXEL_P0 3, 0x0000000f,  0, %1, %2
	PUTPIXEL_P0 4, 0xf0000000, 28, %1, %2
	PUTPIXEL_P0 5, 0x0f000000, 24, %1, %2
	PUTPIXEL_P0 6, 0x00f00000, 20, %1, %2
	PUTPIXEL_P0 7, 0x000f0000, 16, %1, %2

%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_FLIP_P0
; param :
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_FLIP_P0 2

%if %1 < 1
	; Scroll B
	%if %2 > 0
		; Shadow/Highlight is on
		mov dword [Screen_16X + ebp * 2 +  0], SHAD_D
		mov dword [Screen_16X + ebp * 2 +  4], SHAD_D
		mov dword [Screen_16X + ebp * 2 +  8], SHAD_D
		mov dword [Screen_16X + ebp * 2 + 12], SHAD_D
	%else
		; Shadow/Highlight is off
		mov dword [Screen_16X + ebp * 2 +  0], 0x00000000
		mov dword [Screen_16X + ebp * 2 +  4], 0x00000000
		mov dword [Screen_16X + ebp * 2 +  8], 0x00000000
		mov dword [Screen_16X + ebp * 2 + 12], 0x00000000
	%endif

	test byte [ScrollBOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
	test dword [VScrollBl], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%else
	; Scroll A
	test byte [ScrollAOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
	test dword [VScrollAl], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%endif

	test ebx, ebx
	jz near %%Full_Trans

	PUTPIXEL_P0 0, 0x000f0000, 16, %1, %2
	PUTPIXEL_P0 1, 0x00f00000, 20, %1, %2
	PUTPIXEL_P0 2, 0x0f000000, 24, %1, %2
	PUTPIXEL_P0 3, 0xf0000000, 28, %1, %2
	PUTPIXEL_P0 4, 0x0000000f,  0, %1, %2
	PUTPIXEL_P0 5, 0x000000f0,  4, %1, %2
	PUTPIXEL_P0 6, 0x00000f00,  8, %1, %2
	PUTPIXEL_P0 7, 0x0000f000, 12, %1, %2

%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_P1
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_P1 2

%if %1 < 1
	; Scroll B
	mov dword [Screen_16X + ebp * 2 +  0], 0x00000000
	mov dword [Screen_16X + ebp * 2 +  4], 0x00000000
	mov dword [Screen_16X + ebp * 2 +  8], 0x00000000
	mov dword [Screen_16X + ebp * 2 + 12], 0x00000000

	test byte [ScrollBOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
	test dword [VScrollBh], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%else
	; Scroll A
	test byte [ScrollAOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
	test dword [VScrollAh], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this

	%if %2 > 0

		; Faster on almost CPU (because of pairable instructions)

		mov eax, [Screen_16X + ebp * 2 +  0]
		mov ecx, [Screen_16X + ebp * 2 +  4]
		and eax, NOSHAD_D
		and ecx, NOSHAD_D
		mov [Screen_16X + ebp * 2 +  0], eax
		mov [Screen_16X + ebp * 2 +  4], ecx
		mov eax, [Screen_16X + ebp * 2 +  8]
		mov ecx, [Screen_16X + ebp * 2 + 12]
		and eax, NOSHAD_D
		and ecx, NOSHAD_D
		mov [Screen_16X + ebp * 2 +  8], eax
		mov [Screen_16X + ebp * 2 + 12], ecx

		; Faster on K6 CPU

		;and dword [Screen_16X + ebp * 2 +  0], NOSHAD_D
		;and dword [Screen_16X + ebp * 2 +  4], NOSHAD_D
		;and dword [Screen_16X + ebp * 2 +  8], NOSHAD_D
		;and dword [Screen_16X + ebp * 2 + 12], NOSHAD_D
	%endif
%endif

	test ebx, ebx
	jz near %%Full_Trans

	PUTPIXEL_P1 0, 0x0000f000, 12, %2
	PUTPIXEL_P1 1, 0x00000f00,  8, %2
	PUTPIXEL_P1 2, 0x000000f0,  4, %2
	PUTPIXEL_P1 3, 0x0000000f,  0, %2
	PUTPIXEL_P1 4, 0xf0000000, 28, %2
	PUTPIXEL_P1 5, 0x0f000000, 24, %2
	PUTPIXEL_P1 6, 0x00f00000, 20, %2
	PUTPIXEL_P1 7, 0x000f0000, 16, %2

%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_FLIP_P1
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_FLIP_P1 2

%if %1 < 1
	; Scroll B
	mov dword [Screen_16X + ebp * 2 +  0], 0x00000000
	mov dword [Screen_16X + ebp * 2 +  4], 0x00000000
	mov dword [Screen_16X + ebp * 2 +  8], 0x00000000
	mov dword [Screen_16X + ebp * 2 + 12], 0x00000000

	test byte [ScrollBOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
	test dword [VScrollBh], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%else
	; Scroll A
	test byte [ScrollAOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
	test dword [VScrollAh], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this

	%if %2 > 0
		; Shadow/Highlight is on

		; Faster on almost CPU (because of pairable instructions)

		mov eax, [Screen_16X + ebp * 2 +  0]
		mov ecx, [Screen_16X + ebp * 2 +  4]
		and eax, NOSHAD_D
		and ecx, NOSHAD_D
		mov [Screen_16X + ebp * 2 +  0], eax
		mov [Screen_16X + ebp * 2 +  4], ecx
		mov eax, [Screen_16X + ebp * 2 +  8]
		mov ecx, [Screen_16X + ebp * 2 + 12]
		and eax, NOSHAD_D
		and ecx, NOSHAD_D
		mov [Screen_16X + ebp * 2 +  8], eax
		mov [Screen_16X + ebp * 2 + 12], ecx

		; Faster on K6 CPU

		;and dword [Screen_16X + ebp * 2 +  0], NOSHAD_D
		;and dword [Screen_16X + ebp * 2 +  4], NOSHAD_D
		;and dword [Screen_16X + ebp * 2 +  8], NOSHAD_D
		;and dword [Screen_16X + ebp * 2 + 12], NOSHAD_D
	%endif
%endif

	test ebx, ebx
	jz near %%Full_Trans

	PUTPIXEL_P1 0, 0x000f0000, 16, %2
	PUTPIXEL_P1 1, 0x00f00000, 20, %2
	PUTPIXEL_P1 2, 0x0f000000, 24, %2
	PUTPIXEL_P1 3, 0xf0000000, 28, %2
	PUTPIXEL_P1 4, 0x0000000f,  0, %2
	PUTPIXEL_P1 5, 0x000000f0,  4, %2
	PUTPIXEL_P1 6, 0x00000f00,  8, %2
	PUTPIXEL_P1 7, 0x0000f000, 12, %2

%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_SPRITE
; param :
; %1 = Priorité
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest mais sans le screen

%macro PUTLINE_SPRITE 2

	test byte [SpriteOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%if %1 > 0
	test dword [VSpriteh], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%else
	test dword [VSpritel], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%endif

	xor ecx, ecx
	add ebp, [esp]

	PUTPIXEL_SPRITE 0, 0x0000f000, 12, %1, %2
	PUTPIXEL_SPRITE 1, 0x00000f00,  8, %1, %2
	PUTPIXEL_SPRITE 2, 0x000000f0,  4, %1, %2
	PUTPIXEL_SPRITE 3, 0x0000000f,  0, %1, %2
	PUTPIXEL_SPRITE 4, 0xf0000000, 28, %1, %2
	PUTPIXEL_SPRITE 5, 0x0f000000, 24, %1, %2
	PUTPIXEL_SPRITE 6, 0x00f00000, 20, %1, %2
	PUTPIXEL_SPRITE 7, 0x000f0000, 16, %1, %2

	and ch, 0x20
	sub ebp, [esp]
	or byte [VDP_Status], ch

%%Full_Trans ;Nitsuja added this

%endmacro


;****************************************

; macro PUTLINE_SPRITE_FLIP
; param :
; %1 = Priorité
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_SPRITE_FLIP 2

	test byte [SpriteOn], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%if %1 > 0
	test dword [VSpriteh], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%else
	test dword [VSpritel], 1 ;Nitsuja added this
	jz near %%Full_Trans ;Nitsuja added this
%endif

	xor ecx, ecx
	add ebp, [esp]

	PUTPIXEL_SPRITE 0, 0x000f0000, 16, %1, %2
	PUTPIXEL_SPRITE 1, 0x00f00000, 20, %1, %2
	PUTPIXEL_SPRITE 2, 0x0f000000, 24, %1, %2
	PUTPIXEL_SPRITE 3, 0xf0000000, 28, %1, %2
	PUTPIXEL_SPRITE 4, 0x0000000f,  0, %1, %2
	PUTPIXEL_SPRITE 5, 0x000000f0,  4, %1, %2
	PUTPIXEL_SPRITE 6, 0x00000f00,  8, %1, %2
	PUTPIXEL_SPRITE 7, 0x0000f000, 12, %1, %2

	and ch, 0x20
	sub ebp, [esp]
	or byte [VDP_Status], ch

%%Full_Trans ;Nitsuja added this

%endmacro


;****************************************

; macro SPRITE_BOXING
; param :
; %1 = HFLIP (0 = Disable and 1 = Enable)
; %2 = Interlaced (0 = No and 1 = Yes)
; entree :
; - ebx = Pattern Data

%macro SPRITE_BOXING 2

	push eax
	push edi
	push edx

	mov edi, dword [Data_Misc.X]
	mov edi, [Sprite_Visible + edi]
	mov edx, [VDP_Current_Line]
	sub edx, [Sprite_Struct + edi + 4]
	cmp edx, 0
	jz near %%VBORDER
	mov eax, [Sprite_Struct + edi + 12] ; SIZE_Y - 1
	shl eax, 3
	add eax, 7
	cmp eax, edx
	jz  near %%VBORDER
	jmp near %%Not_VBORDER

%%VBORDER
    mov ebx,11111111h
	jmp near %%END_SPRITE_BOXING

%%Not_VBORDER
	mov edx,[Sprite_Struct + edi + 0] ;16]
	cmp edx,ebp
	jz  near %%LBORDER
	jmp near %%Not_LBORDER

%%LBORDER
%if %1 > 0
	and ebx,0xFFF0FFFF
	or  ebx,0x00010000
%else
	and ebx,0xFFFF0FFF
	or  ebx,0x00001000
%endif
	
%%Not_LBORDER
	mov edx,[Sprite_Struct + edi + 16]
	sub edx,7
	cmp edx,ebp
	jz  near %%RBORDER
	jmp near %%Not_RBORDER

%%RBORDER
%if %1 > 0
	and ebx,0xFFFF0FFF
	or  ebx,0x00001000
%else
	and ebx,0xFFF0FFFF
	or  ebx,0x00010000
%endif
	
%%Not_RBORDER

%%END_SPRITE_BOXING
	pop edx
	pop edi
	pop eax

%endmacro

	
;****************************************

; macro UPDATE_PALETTE
; param :
; %1 = Highlight/Shadow Enable

%macro UPDATE_PALETTE 1

	test byte [PalLock], 1
	jnz near %%End
	xor eax, eax
	mov byte [CRam_Flag], 0						; one updates the palette, one gives the flag has 0 for modified
	mov cx, 0x7BEF
	xor edx, edx
	test byte [Mode_555], 1
	mov ebx, (64 / 2) - 1								; ebx = Number of Colours
	jz short %%Loop

	mov cx, 0x3DEF
	jmp short %%Loop

	ALIGN32

%%Loop
		mov ax, [CRam + ebx * 4 + 0]					; ax = data color
		mov dx, [CRam + ebx * 4 + 2]					; dx = data color
		and ax, 0x0EEE
		and dx, 0x0EEE
		mov ax, [Palette + eax * 2]
		mov dx, [Palette + edx * 2]
		mov [MD_Palette + ebx * 4 + 0], ax				; normal color
		mov [MD_Palette + ebx * 4 + 2], dx				; normal color

;%if %1 > 0
		mov [MD_Palette + ebx * 4 + 192 * 2 + 0], ax	; normal color
		mov [MD_Palette + ebx * 4 + 192 * 2 + 2], dx	; normal color

		mov ax, [CRam + ebx * 4 + 0]					; ax = data color
		mov dx, [CRam + ebx * 4 + 2]					; dx = data color
		and ax, 0x0EEE
		and dx, 0x0EEE
		shr ax, 1
		shr dx, 1
		mov ax, [Palette + eax * 2]
		mov dx, [Palette + edx * 2]
		mov [MD_Palette + ebx * 4 + 64 * 2 + 0], ax		; darkened color
		mov [MD_Palette + ebx * 4 + 64 * 2 + 2], dx		; darkened color

		mov ax, [CRam + ebx * 4 + 0]					; ax = data color
		mov dx, [CRam + ebx * 4 + 2]					; dx = data color
		and ax, 0x0EEE
		and dx, 0x0EEE
		shr ax, 1
		shr dx, 1
		mov ax, [Palette + eax * 2 + 0x777 * 2]
		mov dx, [Palette + edx * 2 + 0x777 * 2]
		mov [MD_Palette + ebx * 4 + 128 * 2 + 0], ax	; lightened color
		mov [MD_Palette + ebx * 4 + 128 * 2 + 2], dx	; lightened color
;%endif

		dec ebx										; if the 64 colors were not yet made
		jns near %%Loop								; then one continues

		; Backdrop color setup
		test byte [PinkBG], 1
		jz short %%Normal_BG16

		; Pink backdrop
		mov ax, 0x7C1F								; pink (xrrrrrgggggbbbbb if mode_555)
		test byte [Mode_555], 1
		jnz %%Mode_555 
		mov ax, 0xF81F								; pink (rrrrrggggggbbbbb if mode_565)

	%%Mode_555
		mov [MD_Palette + 0 * 2], ax				; normal color
		mov [MD_Palette + 0 * 2 + 192 * 2], ax		; normal color
		mov [MD_Palette + 0 * 2 + 64 * 2], ax		; darkened color (same)
		mov [MD_Palette + 0 * 2 + 128 * 2], ax		; lightened color (same)
		jmp short %%End

	%%Normal_BG16 ; Normal backdrop
		mov ebx, [VDP_Reg + 7 * 4]
		and ebx, byte 0x3F
		mov ax, [MD_Palette + ebx * 2]
		mov [MD_Palette + 0 * 2], ax				; normal color

;%if %1 > 0
		mov [MD_Palette + 0 * 2 + 192 * 2], ax		; normal color

		mov ax, [MD_Palette + ebx * 2 + 64 * 2]
		mov [MD_Palette + 0 * 2 + 64 * 2], ax		; darkened color

		mov ax, [MD_Palette + ebx * 2 + 128 * 2]
		mov [MD_Palette + 0 * 2 + 128 * 2], ax		; lightened color
;%endif
%%End
%endmacro

; %1 = Highlight/Shadow Enable

%macro UPDATE_PALETTE32 1
; temporary turned off
	; test byte [PalLock], 1
	; jnz near %%End
	; xor eax, eax
	; mov byte [CRam_Flag], 0						; one updates the palette, one gives the flag has 0 for modified
	; mov ecx, 0x007F7F7F
	; xor edx, edx
	; mov ebx, (64 / 2) - 1								; ebx = Number of Colours
	; jmp short %%Loop32
	; ALIGN32

; %%Loop32
		; mov ax, [CRam + ebx * 4 + 0]					; ax = data color
		; mov dx, [CRam + ebx * 4 + 2]					; dx = data color
		; and eax, 0x0EEE
		; and edx, 0x0EEE
		; mov eax, [Palette32 + eax * 4]
		; mov edx, [Palette32 + edx * 4]
		; mov [MD_Palette32 + ebx * 8 + 0], eax				; normal color
		; mov [MD_Palette32 + ebx * 8 + 4], edx				; normal color

		; mov [MD_Palette32 + ebx * 8 + 192 * 4 + 0], eax	; normal color
		; mov [MD_Palette32 + ebx * 8 + 192 * 4 + 4], edx	; normal color

		; mov ax, [CRam + ebx * 4 + 0]					; ax = data color
		; mov dx, [CRam + ebx * 4 + 2]					; dx = data color
		; and eax, 0x0EEE
		; and edx, 0x0EEE
		; shr ax, 1
		; shr dx, 1
		; mov eax, [Palette32 + eax * 4]
		; mov edx, [Palette32 + edx * 4]
		; mov [MD_Palette32 + ebx * 8 + 64 * 4 + 0], eax	; darkened color
		; mov [MD_Palette32 + ebx * 8 + 64 * 4 + 4], edx	; darkened color

		; mov ax, [CRam + ebx * 4 + 0]					; ax = data color
		; mov dx, [CRam + ebx * 4 + 2]					; dx = data color
		; and eax, 0x0EEE
		; and edx, 0x0EEE
		; shr ax, 1
		; shr dx, 1
		; mov eax, [Palette32 + eax * 4 + 0x777 * 4]
		; mov edx, [Palette32 + edx * 4 + 0x777 * 4]
		; mov [MD_Palette32 + ebx * 8 + 128 * 4 + 0], eax	; lightened color
		; mov [MD_Palette32 + ebx * 8 + 128 * 4 + 4], edx	; lightened color

		; dec ebx										; if the 64 colors were not yet made
		; jns near %%Loop32							; then one continues

		; Backdrop color setup
		; test byte [PinkBG], 1
		; jz short %%Normal_BG32

		; Pink backdrop
		; mov eax, 0x00ff00ff							; pink
		; mov [MD_Palette32 + 0 * 4], eax				; normal color
		; mov [MD_Palette32 + 0 * 4 + 192 * 4], eax	; normal color
		; mov [MD_Palette32 + 0 * 4 + 64 * 4], eax	; darkened color (same)
		; mov [MD_Palette32 + 0 * 4 + 128 * 4], eax	; lightened color (same)
		; jmp short %%Update_Pal16

	; %%Normal_BG32 ; Normal backdrop
		; mov ebx, [VDP_Reg + 7 * 4]
		; and ebx, byte 0x3F
		; mov eax, [MD_Palette32 + ebx * 4]
		; mov [MD_Palette32 + 0 * 4], eax					; normal color

		; mov [MD_Palette32 + 0 * 4 + 192 * 4], eax		; normal color

		; mov eax, [MD_Palette32 + ebx * 4 + 64 * 4]
		; mov [MD_Palette32 + 0 * 4 + 64 * 4], eax		; darkened color

		; mov eax, [MD_Palette32 + ebx * 4 + 128 * 4]
		; mov [MD_Palette32 + 0 * 4 + 128 * 4], eax		; lightened color

; %%Update_Pal16
	; UPDATE_PALETTE %1

%%End
%endmacro

;****************************************

; macro RENDER_LINE_SCROLL_B
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = 1 if V-Scroll mode in 2 cell and 0 if full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_B 3

	mov ebp, [esp]				; ebp point on surface where one renders

	GET_X_OFFSET 0

	mov eax, esi				; eax = scroll X inv
	xor esi, 0x3FF				; esi = scroll X norm
	and eax, byte 7				; eax = completion for offset
	shr esi, 3					; esi = current cell
	add ebp, eax				; ebp updated for clipping
	mov ebx, esi
	and esi, [H_Scroll_CMask]	; prevent H Cell Offset from overflowing
	and ebx, byte 1
	mov eax, [H_Cell]
	sub ebx, byte 2				; start with cell -2 or -1 (for V Scroll)
	mov [Data_Misc.X], eax		; number of cells to post
	mov [Data_Misc.Cell], ebx	; Current cell for the V Scroll


	mov edi, [VDP_Current_Line]		; edi = line number
	mov eax, [VSRam + 2]

%if %1 > 0
	shr eax, 1						; divide Y scroll in 2 if interlaced
%endif

	add edi, eax
	mov eax, edi
	shr edi, 3								; V Cell Offset
	and eax, byte 7							; adjust for pattern
	and edi, [V_Scroll_CMask]				; prevent V Cell Offset from overflowing
	mov [Data_Misc.Line_7], eax

	jmp short %%First_Loop

	ALIGN32

	%%Loop

%if %2 > 0
		UPDATE_Y_OFFSET 0, %1
%endif

	%%First_Loop

		GET_PATTERN_INFO 0
		GET_PATTERN_DATA %1, 0
		
		test byte [Swap_Scroll_PriorityB], 1
		jz short %%No_Invert
		xor ax, 0x8000

	%%No_Invert
		test eax, 0x0800							; test if H-Flip ?
		jz near %%No_H_Flip							; if yes, then

	%%H_Flip

			test eax, 0x8000						; test the priority of the current pattern
			jnz near %%H_Flip_P1

	%%H_Flip_P0
				PUTLINE_FLIP_P0 0, %3
				jmp %%End_Loop

	ALIGN32

	%%H_Flip_P1
				PUTLINE_FLIP_P1 0, %3
				jmp %%End_Loop

	ALIGN32
	
	%%No_H_Flip

			test eax, 0x8000						; test the priority of the current pattern
			jnz near %%No_H_Flip_P1

	%%No_H_Flip_P0
				PUTLINE_P0 0, %3
				jmp %%End_Loop

	ALIGN32

	%%No_H_Flip_P1
				PUTLINE_P1 0, %3
				jmp short %%End_Loop

	ALIGN32

	%%End_Loop
		inc dword [Data_Misc.Cell]		; Next H cell for the V Scroll
		inc esi							; Next H cell
		add ebp, byte 8					; advance to the next pattern
		and esi, [H_Scroll_CMask]		; prevent H Offset from overflowing
		dec byte [Data_Misc.X]			; un cell de moins à traiter
		jns near %%Loop
		
%%End


%endmacro


;****************************************

; macro RENDER_LINE_SCROLL_A_WIN
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = 1 si V-Scroll mode en 2 cell et 0 si full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_A_WIN 3

	mov eax, [VDP_Current_Line]
	mov cl, [VDP_Reg + 18 * 4]
	shr eax, 3
	mov ebx, [H_Cell]
	shr cl, 7							; cl = 1 si window at bottom
	cmp eax, [Win_Y_Pos]
	setae ch							; ch = 1 si current line >= pos Y window
	xor cl, ch							; cl = 0 si line window sinon line Scroll A
	jz near %%Full_Win

	test byte [VDP_Reg + 17 * 4], 0x80
	mov edx, [Win_X_Pos]
	jz short %%Win_Left

%%Win_Right
	sub ebx, edx
	mov [Data_Misc.Start_W], edx		; Start Win (Cell)
	mov [Data_Misc.Length_W], ebx		; Length Win (Cell)
	dec edx								; 1 cell en moins car on affiche toujours le dernier à part
	mov dword [Data_Misc.Start_A], 0	; Start Scroll A (Cell)
	mov [Data_Misc.Length_A], edx		; Length Scroll A (Cell)
	jns short %%Scroll_A
	jmp %%Window

	ALIGN4
	
%%Win_Left
	sub ebx, edx
	mov dword [Data_Misc.Start_W], 0	; Start Win (Cell)
	mov [Data_Misc.Length_W], edx		; Length Win (Cell)
	dec ebx								; 1 cell en moins car on affiche toujours le dernier à part
	mov [Data_Misc.Start_A], edx		; Start Scroll A (Cell)
	mov [Data_Misc.Length_A], ebx		; Length Scroll A (Cell)
	jns short %%Scroll_A
	jmp %%Window

	ALIGN4

%%Scroll_A
	mov ebp, [esp]					; ebp point on surface where one renders

	GET_X_OFFSET 1

	mov eax, esi					; eax = scroll X inv
	mov ebx, [Data_Misc.Start_A]	; Premier Cell
	xor esi, 0x3FF					; esi = scroll X norm
	and eax, byte 7					; eax = completion pour offset
	shr esi, 3						; esi = cell courant (début scroll A)
	mov [Data_Misc.Mask], eax		; mask for the dernier pattern
	mov ecx, esi					; ecx = cell courant (début scroll A) 
	add esi, ebx					; esi = cell courant ajusté pour window clip
	and ecx, byte 1
	lea eax, [eax + ebx * 8]		; clipping + window clip
	sub ecx, byte 2					; on démarre au cell -2 ou -1 (for the V Scroll)
	and esi, [H_Scroll_CMask]		; on prevent H Cell Offset from overflowing
	add ebp, eax					; ebp mis à jour pour clipping + window clip
	add ebx, ecx					; ebx = Cell courant for the V Scroll

	mov edi, [VDP_Current_Line]		; edi = numero ligne
	mov [Data_Misc.Cell], ebx		; Cell courant for the V Scroll
	jns short %%Not_First_Cell

	mov eax, [VSRam + 0]
	jmp short %%First_VScroll_OK

%%Not_First_Cell
	and ebx, [V_Scroll_MMask]
	mov eax, [VSRam + ebx * 2]

%%First_VScroll_OK

%if %1 > 0
	shr eax, 1						; divide Y scroll in 2 if interlaced
%endif
	add edi, eax
	mov eax, edi
	shr edi, 3								; V Cell Offset
	and eax, byte 7							; adjust for pattern
	and edi, [V_Scroll_CMask]				; prevent V Cell Offset from overflowing
	mov [Data_Misc.Line_7], eax

	jmp short %%First_Loop_SCA

	ALIGN32

%%Loop_SCA

%if %2 > 0
		UPDATE_Y_OFFSET 1, %1
%endif

%%First_Loop_SCA

		GET_PATTERN_INFO 1
		GET_PATTERN_DATA %1, 0
		
		test byte [Swap_Scroll_PriorityA], 1
		jz short %%No_Invert2
		xor ax, 0x8000

	%%No_Invert2
		test eax, 0x0800							; test if H-Flip ?
		jz near %%No_H_Flip							; if yes, then

	%%H_Flip
			test eax, 0x8000						; test the priority of the current pattern
			jnz near %%H_Flip_P1

	%%H_Flip_P0
				PUTLINE_FLIP_P0 1, %3
				jmp %%End_Loop

	ALIGN32

	%%H_Flip_P1
				PUTLINE_FLIP_P1 1, %3
				jmp %%End_Loop

	ALIGN32
	
	%%No_H_Flip
			test eax, 0x8000						; test the priority of the current pattern
			jnz near %%No_H_Flip_P1

	%%No_H_Flip_P0
				PUTLINE_P0 1, %3
				jmp %%End_Loop

	ALIGN32

	%%No_H_Flip_P1
				PUTLINE_P1 1, %3
				jmp short %%End_Loop

	ALIGN32

	%%End_Loop
		inc dword [Data_Misc.Cell]		; Next H cell for the V Scroll
		inc esi							; Next H cell
		add ebp, byte 8					; advance to the next pattern
		and esi, [H_Scroll_CMask]		; prevent H Offset from overflowing
		dec byte [Data_Misc.Length_A]	; decrement number of cells to treat for Scroll A
		jns near %%Loop_SCA




%%LC_SCA

%if %2 > 0
	UPDATE_Y_OFFSET 1, %1
%endif

	GET_PATTERN_INFO 1
	GET_PATTERN_DATA %1, 0

	test byte [Swap_Scroll_PriorityA], 1
	jz short %%No_Invert3
	xor ax, 0x8000

%%No_Invert3
	test eax, 0x0800							; test if H-Flip ?
	mov ecx, [Data_Misc.Mask]
	jz near %%LC_SCA_No_H_Flip					; if yes, then

	%%LC_SCA_H_Flip
		and ebx, [Mask_F + ecx * 4]				; apply the mask
		test eax, 0x8000						; test the priority of the current pattern
		jnz near %%LC_SCA_H_Flip_P1

	%%LC_SCA_H_Flip_P0
			PUTLINE_FLIP_P0 1, %3
			jmp %%LC_SCA_End

	ALIGN32

	%%LC_SCA_H_Flip_P1
			PUTLINE_FLIP_P1 1, %3
			jmp %%LC_SCA_End

	ALIGN32
	
	%%LC_SCA_No_H_Flip
		and ebx, [Mask_N + ecx * 4]				; apply the mask
		test eax, 0x8000						; test the priority of the current pattern
		jnz near %%LC_SCA_No_H_Flip_P1

	%%LC_SCA_No_H_Flip_P0
			PUTLINE_P0 1, %3
			jmp %%LC_SCA_End

	ALIGN32

	%%LC_SCA_No_H_Flip_P1
			PUTLINE_P1 1, %3
			jmp short %%LC_SCA_End

	ALIGN32

%%LC_SCA_End
	test byte [Data_Misc.Length_W], 0xFF
	jnz short %%Window
	jmp %%End





	ALIGN4

%%Full_Win
	xor esi, esi							; Start Win (Cell)
	mov edi, ebx							; Length Win (Cell)
	jmp short %%Window_Initialised

	ALIGN4

%%Window
	mov esi, [Data_Misc.Start_W]
	mov edi, [Data_Misc.Length_W]			; edi = # of cells to render

%%Window_Initialised
	mov edx, [VDP_Current_Line]
	mov cl, [H_Win_Mul]
	mov ebx, edx							; ebx = Line
	mov ebp, [esp]							; ebp point on surface where one renders
	shr edx, 3								; edx = Line / 8
	mov eax, [Win_Addr]
	shl edx, cl
	lea ebp, [ebp + esi * 8 + 8]			; no clipping for the window, return directly to the first pixel
	lea eax, [eax + edx * 2]				; eax point on the pattern data for the window
	and ebx, byte 7							; ebx = Line & 7 for the V Flip
	mov [Data_Misc.Pattern_Adr], eax		; store this pointer
	mov [Data_Misc.Line_7], ebx				; store Line & 7
	jmp short %%Loop_Win

	ALIGN32

%%Loop_Win
		mov ebx, [Data_Misc.Pattern_Adr]
		mov ax, [ebx + esi * 2]

		GET_PATTERN_DATA %1, 1

		test ax, 0x0800					; test if H-Flip ?
		jz near %%W_No_H_Flip			; if yes, then

	%%W_H_Flip
			test ax, 0x8000						; test the priority of the current pattern
			jnz near %%W_H_Flip_P1

	%%W_H_Flip_P0
				PUTLINE_FLIP_P0 1, %3
				jmp %%End_Loop_Win

	ALIGN32

	%%W_H_Flip_P1
				PUTLINE_FLIP_P1 1, %3
				jmp %%End_Loop_Win

	ALIGN32
	
	%%W_No_H_Flip
			test ax, 0x8000						; test the priority of the current pattern
			jnz near %%W_No_H_Flip_P1

	%%W_No_H_Flip_P0
				PUTLINE_P0 1, %3
				jmp %%End_Loop_Win

	ALIGN32

	%%W_No_H_Flip_P1
				PUTLINE_P1 1, %3
				jmp short %%End_Loop_Win

	ALIGN32

	%%End_Loop_Win
		inc esi						; next pattern		
		add ebp, byte 8				;  "      "   for the render
		dec edi
		jnz near %%Loop_Win

%%End


%endmacro


;****************************************

; macro RENDER_LINE_SPR
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = Shadow / Highlight (0 = Disable and 1 = Enable)
; %3 = Sprite Boxing (0 = Disable and 1 = Enable)

%macro RENDER_LINE_SPR 3 

	test dword [Sprite_Over], 1
	jz near %%No_Sprite_Over

%%Sprite_Over

	UPDATE_MASK_SPRITE 1		; edi point on the sprite to post
	xor edi, edi
	test esi, esi
	mov dword [Data_Misc.X], edi
	jnz near %%First_Loop
	jmp %%End					; quit

%%No_Sprite_Over
	UPDATE_MASK_SPRITE 0		; edi point on the sprite to post
	xor edi, edi
	test esi, esi
	mov dword [Data_Misc.X], edi
	jnz short %%First_Loop
	jmp %%End					; quit

	ALIGN32

%%Sprite_Loop
		mov edx, [VDP_Current_Line]
%%First_Loop
		mov edi, [Sprite_Visible + edi]
		mov eax, [Sprite_Struct + edi + 24]		; eax = CellInfo of the sprite
		sub edx, [Sprite_Struct + edi + 4]		; edx = Line - Y Pos (Y Offset)
		mov ebx, eax							; ebx = CellInfo
		mov esi, eax							; esi = CellInfo
		test byte [Swap_Sprite_Priority], 1
		jz short %%No_InvertS
		xor ax, 0x8000

	%%No_InvertS
		shr bx, 9								; isolate the pallet in ebx
		mov ecx, edx							; ecx = Y Offset
		and ebx, 0x30							; keep the pallet number an even multiple of 16
	
		and esi, 0x7FF							; esi = number of the first pattern of the sprite
		mov [Data_Misc.Palette], ebx			; store the palette number * 64 in Palette
		and edx, 0xF8							; one erases the 3 lest significant bits = Num Pattern * 8
		mov ebx, [Sprite_Struct + edi + 12]		; ebx = Size Y
		and ecx, byte 7							; ecx = (Y Offset & 7) = Line of the current pattern
%if %1 > 0
		shl ebx, 6								; ebx = Size Y * 64
		lea edx, [edx * 8]						; edx = Num Pattern * 64
		shl esi, 6								; esi = point on on the contents of the pattern
%else
		shl ebx, 5								; ebx = Size Y * 32
		lea edx, [edx * 4]						; edx = Num Pattern * 32
		shl esi, 5								; esi = point on on the contents of the pattern
%endif

		test eax, 0x1000						; test for V Flip
		jz %%No_V_Flip

	%%V_Flip
		sub ebx, edx
		xor ecx, 7								; ecx = 7 - (Y Offset & 7)
		add esi, ebx							; esi point on the pattern to post
%if %1 > 0
		lea ebx, [ebx + edx + 64]				; restore the value of ebx + 64
		lea esi, [esi + ecx * 8]				; and load the good line of the pattern
		jmp short %%Suite
%else
		lea ebx, [ebx + edx + 32]				; restore the value of ebx + 32
		lea esi, [esi + ecx * 4]				; and load the good line of the pattern
		jmp short %%Suite
%endif

	ALIGN4
	
	%%No_V_Flip
		add esi, edx							; esi point on the pattern to post
%if %1 > 0
		add ebx, byte 64						; add 64 to ebx
		lea esi, [esi + ecx * 8]				; and load the good line of the pattern
%else			
		add ebx, byte 32						; add 32 to ebx
		lea esi, [esi + ecx * 4]				; and load the good line of the pattern
%endif

	%%Suite
		mov [Data_Misc.Next_Cell], ebx			; next Cell X of this sprite is with ebx bytes
		mov edx, [Data_Misc.Palette]			; edx = Palette number * 64

		test eax, 0x800							; test H Flip
		jz near %%No_H_Flip
			
	%%H_Flip
		mov ebx, [Sprite_Struct + edi + 0]
		mov ebp, [Sprite_Struct + edi + 16]		; position for X
		cmp ebx, -7								; test for the minimum edge of the sprite
		mov edi, [Data_Misc.Next_Cell]
		jg short %%Spr_X_Min_Norm
		mov ebx, -7								; minimum edge = clip screen

	%%Spr_X_Min_Norm
		mov [Data_Spr.H_Min], ebx				; spr min = minimum edge

	%%Spr_X_Min_OK
		sub ebp, byte 7							; to post the last pattern in first
		jmp short %%Spr_Test_X_Max

	ALIGN4

	%%Spr_Test_X_Max_Loop
			sub ebp, byte 8							; one moves back on the preceding pattern (screen)
			add esi, edi							; one goes on next the pattern (mem)

	%%Spr_Test_X_Max
			cmp ebp, [H_Pix]
			jge %%Spr_Test_X_Max_Loop

		test [Sprite_Always_Top], byte 0xFF
		jnz near %%H_Flip_P1
		test eax, 0x8000						; test the priority
		jnz near %%H_Flip_P1
		jmp short %%H_Flip_P0

	ALIGN32
	
	%%H_Flip_P0
	%%H_Flip_P0_Loop
			mov ebx, [VRam + esi]					; ebx = Pattern Data
%if %3 > 0
			SPRITE_BOXING 1, %1
%endif
			PUTLINE_SPRITE_FLIP 0, %2				; one posts the line of the sprite pattern 

			sub ebp, byte 8							; one posts the previous pattern
			add esi, edi							; one goes on next the pattern
			cmp ebp, [Data_Spr.H_Min]				; test if one did all the sprite patterns
			jge near %%H_Flip_P0_Loop				; if not, continue
		jmp %%End_Sprite_Loop

	ALIGN32
	
	%%H_Flip_P1
	%%H_Flip_P1_Loop
			mov ebx, [VRam + esi]					; ebx = Pattern Data
%if %3 > 0
			SPRITE_BOXING 1, %1
%endif
			PUTLINE_SPRITE_FLIP PRIO_B, %2				; one posts the line of the sprite pattern 

			sub ebp, byte 8							; one posts the previous pattern
			add esi, edi							; one goes on next the pattern
			cmp ebp, [Data_Spr.H_Min]				; test if one did all the sprite patterns
			jge near %%H_Flip_P1_Loop				; if not, continue
		jmp %%End_Sprite_Loop
				
	ALIGN32
	
	%%No_H_Flip
		mov ebx, [Sprite_Struct + edi + 16]
		mov ecx, [H_Pix]
		mov ebp, [Sprite_Struct + edi + 0]		; position the pointer ebp
		cmp ebx, ecx							; test for the maximum edge of the sprite
		mov edi, [Data_Misc.Next_Cell]
		jl %%Spr_X_Max_Norm
		mov [Data_Spr.H_Max], ecx				; max edge = clip screan
		jmp short %%Spr_Test_X_Min

	ALIGN4

	%%Spr_X_Max_Norm
		mov [Data_Spr.H_Max], ebx				; spr max = max edge
		jmp short %%Spr_Test_X_Min

	ALIGN4

	%%Spr_Test_X_Min_Loop
			add ebp, byte 8						; advance to the next pattern (screen)
			add esi, edi						; one goes on next the pattern (mem)

	%%Spr_Test_X_Min
			cmp ebp, -7
			jl %%Spr_Test_X_Min_Loop

		test [Sprite_Always_Top], byte 0xFF
		jnz near %%No_H_Flip_P1
		test ax, 0x8000							; test the priority
		jnz near %%No_H_Flip_P1
		jmp short %%No_H_Flip_P0

	ALIGN32
	
	%%No_H_Flip_P0
	%%No_H_Flip_P0_Loop
			mov ebx, [VRam + esi]					; ebx = Pattern Data
%if %3 > 0
			SPRITE_BOXING 0, %1
%endif
			PUTLINE_SPRITE 0, %2					; one posts the line of the sprite pattern 

			add ebp, byte 8							; one posts the previous pattern
			add esi, edi							; one goes on next the pattern
			cmp ebp, [Data_Spr.H_Max]				; test if one did all the sprite patterns
			jl near %%No_H_Flip_P0_Loop				; if not, continue
		jmp %%End_Sprite_Loop

	ALIGN32
	
	%%No_H_Flip_P1
	%%No_H_Flip_P1_Loop
			mov ebx, [VRam + esi]					; ebx = Pattern Data
%if %3 > 0
			SPRITE_BOXING 0, %1
%endif
			PUTLINE_SPRITE PRIO_B, %2					; one posts the line of the sprite pattern 

			add ebp, byte 8							; one posts the previous pattern
			add esi, edi							; one goes on next the pattern
			cmp ebp, [Data_Spr.H_Max]				; test if one did all the sprite patterns
			jl near %%No_H_Flip_P1_Loop				; if not, continue
		jmp short %%End_Sprite_Loop
				
	ALIGN32
	
	%%End_Sprite_Loop
		mov edi, [Data_Misc.X]
		add edi, byte 4
		cmp edi, [Data_Misc.Borne]
		mov [Data_Misc.X], edi
		jb near %%Sprite_Loop

%%End

%endmacro


;****************************************

; macro RENDER_LINE
; param :
; %1 = 1 for interlace mode and 0 if not
; %2 = Shadow / Highlight (0 = Disable et 1 = Enable)

%macro RENDER_LINE 2

	test dword [VDP_Reg + 11 * 4], 4
	jz near %%Full_VScroll

%%Cell_VScroll
	RENDER_LINE_SCROLL_B     %1, 1, %2
	RENDER_LINE_SCROLL_A_WIN %1, 1, %2
	jmp %%Scroll_OK

%%Full_VScroll
	RENDER_LINE_SCROLL_B     %1, 0, %2
	RENDER_LINE_SCROLL_A_WIN %1, 0, %2

%%Scroll_OK
	test dword [Sprite_Boxing], 1
	jz near %%No_Sprite_Boxing

	RENDER_LINE_SPR          %1, %2, 1
	jmp near %%End_RENDER_LINE

%%No_Sprite_Boxing
	RENDER_LINE_SPR          %1, %2, 0

%%End_RENDER_LINE

%endmacro


; *******************************************************

	DECL Render_Line

		pushad

		mov ebx, [VDP_Current_Line]
		xor eax, eax
		mov edi, [TAB336 + ebx * 4]
		test dword [VDP_Reg + 1 * 4], 0x40		; test if the VDP is active
		push edi								; we need this value later
		jnz short .VDP_Enable					; if not, nothing is posted

			test byte [VDP_Reg + 12 * 4], 0x08
			cld
			mov ecx, 160
			jz short .No_Shadow

			mov eax, 0x40404040

	.No_Shadow
			lea edi, [Screen_16X + edi * 2 + 8 * 2]
			rep stosd
			jmp .VDP_OK

	ALIGN4

	.VDP_Enable
		mov ebx, [VRam_Flag]
		mov eax, [VDP_Reg + 12 * 4]
		and ebx, byte 3
		and eax, byte 4
		mov byte [VRam_Flag], 0
		jmp [.Table_Sprite_Struct + ebx * 8 + eax]

	ALIGN4
	
	.Table_Sprite_Struct
		dd 	.Sprite_Struc_OK
		dd 	.Sprite_Struc_OK
		dd 	.MSS_Complete, .MSS_Complete_Interlace
		dd 	.MSS_Partial, .MSS_Partial_Interlace
		dd 	.MSS_Complete, .MSS_Complete_Interlace

	ALIGN4

	.MSS_Complete
			MAKE_SPRITE_STRUCT 0
			jmp .Sprite_Struc_OK

	ALIGN32

	.MSS_Complete_Interlace
			MAKE_SPRITE_STRUCT 1
			jmp .Sprite_Struc_OK

	ALIGN32

	.MSS_Partial
	.MSS_Partial_Interlace
			MAKE_SPRITE_STRUCT_PARTIAL
			jmp short .Sprite_Struc_OK

	ALIGN32
	
	.Sprite_Struc_OK
		mov eax, [VDP_Reg + 12 * 4]
		and eax, byte 0xC
		jmp [.Table_Render_Line + eax]

	ALIGN4
	
	.Table_Render_Line
		dd 	.NHS_NInterlace
		dd 	.NHS_Interlace
		dd 	.HS_NInterlace
		dd 	.HS_Interlace
		
	ALIGN4

	.NHS_NInterlace
			RENDER_LINE 0, 0
			jmp .VDP_OK

	ALIGN32
	
	.NHS_Interlace
			RENDER_LINE 1, 0
			jmp .VDP_OK

	ALIGN32

	.HS_NInterlace
			RENDER_LINE 0, 1
			jmp .VDP_OK

	ALIGN32
	
	.HS_Interlace
			RENDER_LINE 1, 1
			jmp short .VDP_OK

	ALIGN32
	
	.VDP_OK
		test byte [CRam_Flag], 1		; test if palette was modified
		jz near Palette_OK				; if yes

		test byte [VDP_Reg + 12 * 4], 8
		jnz near .Palette_HS

		UPDATE_PALETTE32 0
		jmp Palette_OK

	ALIGN4
		
	.Palette_HS
		UPDATE_PALETTE32 1

	ALIGN4
	
	Palette_OK
		; Render
		mov ecx, 160
		mov eax, [H_Pix_Begin]
		mov edi, [esp]
		sub ecx, eax
		lea edi, [Screen_16X + edi * 2 + 8 * 2]
		mov esi, CRam
		test byte [PalLock], 1
		jz near .PalNotLocked
		mov esi, LockedPalette
	
	.PalNotLocked
		; Get backdrop color
		mov ebp, [VDP_Reg + 7 * 4]
		and ebp, byte 0x3F
		jmp short .Genesis_Loop

		ALIGN32
		
	.Genesis_Loop
		movzx eax, word [edi + 0]
		mov ebx, eax
		and ax, 0x3F
		jnz .not_backdrop
		mov ax, bp ; replace backdrop

	.not_backdrop
		and bx, 0x1C0
		shl bx, 6
		mov ax, [esi + eax * 2]
		and ax, 0xEEE
		or  ax, bx
		mov [edi + 0], ax

		movzx eax, word [edi + 2]
		mov ebx, eax
		and ax, 0x3F
		jnz .not_backdrop1
		mov ax, bp ; replace backdrop

	.not_backdrop1
		mov ax, [esi + eax * 2]
		and ax, 0xEEE
		and bx, 0x1C0
		shl bx, 6
		or  ax, bx
		mov [edi + 2], ax

		add edi, byte 4
		dec ecx
		jnz short .Genesis_Loop

		add esp, byte 4

	popad
	ret

; *******************************************************

	DECL Render_Line_32X

		pushad

		mov ebx, [VDP_Current_Line]
		xor eax, eax
		mov edi, [TAB336 + ebx * 4]
		test dword [VDP_Reg + 1 * 4], 0x40		; test if the VDP is active
		push edi								; we need this value later
		jnz short .VDP_Enable					; if not, nothing is posted

			test byte [VDP_Reg + 12 * 4], 0x08
			cld
			mov ecx, 160
			jz short .No_Shadow

			mov eax, 0x40404040

	.No_Shadow
			lea edi, [Screen_16X + edi * 2 + 8 * 2]
			rep stosd
			jmp .VDP_OK

	ALIGN4

	.VDP_Enable
		mov ebx, [VRam_Flag]
		xor eax, eax
		and ebx, byte 3
		mov [VRam_Flag], eax
		jmp [.Table_Sprite_Struct + ebx * 4]

	ALIGN4
	
	.Table_Sprite_Struct
		dd 	.Sprite_Struc_OK
		dd 	.MSS_Complete
		dd 	.MSS_Partial
		dd 	.MSS_Complete

	ALIGN32

	.MSS_Complete
			MAKE_SPRITE_STRUCT 0
			jmp .Sprite_Struc_OK

	ALIGN32

	.MSS_Partial
			MAKE_SPRITE_STRUCT_PARTIAL

	ALIGN4
	
	.Sprite_Struc_OK
		test byte [VDP_Reg + 12 * 4], 0x8	; no interlace in 32X mode
		jnz near .HS

	.NHS
			RENDER_LINE 0, 0
			test byte [CRam_Flag], 1		; test if palette was modified
			jz near .VDP_OK
			UPDATE_PALETTE32 0
			jmp .VDP_OK

	ALIGN32

	.HS
			RENDER_LINE 0, 1
			test byte [CRam_Flag], 1		; test if palette was modified
			jz near .VDP_OK
			UPDATE_PALETTE32 1

	ALIGN4
	
	.VDP_OK
		; Render
		mov ecx, 160
		mov eax, [H_Pix_Begin]
		mov edi, [esp]
		sub ecx, eax
		lea edi, [Screen_32X + edi * 2 + 8 * 2]
		mov esi, [_32X_VDP + vx.State]
		mov eax, [_32X_VDP + vx.Mode]
		and esi, byte 1
		mov edx, eax
		shl esi, 17
		mov ebp, eax
		shr edx, 3
		mov ebx, [VDP_Current_Line]
		shr ebp, 11
		and eax, byte 3
		and edx, byte 0x10
		and ebp, byte 0x20
		mov bx, [_32X_VDP_Ram + esi + ebx * 2]
		or edx, ebp
		lea esi, [_32X_VDP_Ram + esi + ebx * 2]
		shr edx, 2
		mov [_32X_Rend_Mode], al
		or  [_32X_Rend_Mode], dl
		shl	edx, 2
		mov [edi-2], word 1 ; mark that 32X was ON
		jmp [.Table_32X_Draw + eax * 4 + edx]

	ALIGN4

	.Table_32X_Draw
		dd .32X_Draw_M00, .32X_Draw_M01, .32X_Draw_M10, .32X_Draw_M11
		dd .32X_Draw_M00, .32X_Draw_M01_P, .32X_Draw_M10_P, .32X_Draw_M11_P
		dd .32X_Draw_M00, .32X_Draw_SM01, .32X_Draw_M10, .32X_Draw_M11
		dd .32X_Draw_M00, .32X_Draw_SM01_P, .32X_Draw_M10_P, .32X_Draw_M11_P

	ALIGN32
	
	.Genesis_Loop

	ALIGN32

	; Direct color mode
	.32X_Draw_M10
			mov ebp, [esi + 0]
			mov ebx, [esi + 4]
			mov [edi + 0], ebp
			mov [edi + 4], ebx
			add esi, byte 8
			add edi, byte 8
			sub ecx, 2
			jns short .32X_Draw_M10

		jmp Palette_OK

	ALIGN32

	; Direct color mode + Priority
	.32X_Draw_M10_P
			mov ebp, [esi + 0]
			mov ebx, [esi + 4]
			xor ebp, 0x80008000
			xor ebx, 0x80008000
			mov [edi + 0], ebp
			mov [edi + 4], ebx
			add esi, byte 8
			add edi, byte 8
			sub ecx, 2
			jns short .32X_Draw_M10_P

		jmp Palette_OK

	ALIGN32

	; Packed pixel mode (indexed mode)
	.32X_Draw_M01
			movzx ebp, byte [esi + 1]
			movzx ebx, byte [esi + 0]
			mov ax, [_32X_VDP_CRam + ebp * 2]
			mov dx, [_32X_VDP_CRam + ebx * 2]
			mov [edi + 0], ax
			mov [edi + 2], dx
			add esi, byte 2
			add edi, byte 4
			dec ecx
			jnz short .32X_Draw_M01

		jmp Palette_OK

	ALIGN32

	; Packed pixel mode (indexed mode) + Priority
	.32X_Draw_M01_P
			movzx ebp, byte [esi + 1]
			movzx ebx, byte [esi + 0]
			mov ax, [_32X_VDP_CRam + ebp * 2]
			mov dx, [_32X_VDP_CRam + ebx * 2]
			xor ax, 0x8000
			xor dx, 0x8000
			mov [edi + 0], ax
			mov [edi + 2], dx
			add esi, byte 2
			add edi, byte 4
			dec ecx
			jnz short .32X_Draw_M01_P

		jmp Palette_OK

	ALIGN32

	; Packed pixel mode (indexed mode) + Shift
	.32X_Draw_SM01
			movzx ebp, byte [esi + 0]
			movzx ebx, byte [esi + 3]
			mov ax, [_32X_VDP_CRam + ebp * 2]
			mov dx, [_32X_VDP_CRam + ebx * 2]
			mov [edi + 0], ax
			mov [edi + 2], dx
			add esi, byte 2
			add edi, byte 4
			dec ecx
			jnz short .32X_Draw_SM01

		jmp Palette_OK

	ALIGN32

	; Packed pixel mode (indexed mode) + Shift + Priority
	.32X_Draw_SM01_P
			movzx ebp, byte [esi + 0]
			movzx ebx, byte [esi + 3]
			mov ax, [_32X_VDP_CRam + ebp * 2]
			mov dx, [_32X_VDP_CRam + ebx * 2]
			xor ax, 0x8000
			xor dx, 0x8000
			mov [edi + 0], ax
			mov [edi + 2], dx
			add esi, byte 2
			add edi, byte 4
			dec ecx
			jnz short .32X_Draw_SM01_P

		jmp Palette_OK


	ALIGN32

	; Run length mode
	.32X_Draw_M11
			lea edx, [ecx * 2]
			jmp short .32X_Draw_M11_Loop

	ALIGN4
	
		.32X_Draw_M11_Loop
			movzx eax, byte [esi + 0]
			movzx ecx, byte [esi + 1]
			mov ax, [_32X_VDP_CRam + eax * 2]
			inc ecx
			add esi, byte 2
			sub edx, ecx
			jbe short .32X_Draw_M11_End
			rep stosw
			jmp short .32X_Draw_M11_Loop

	ALIGN4

	.32X_Draw_M11_End
		add ecx, edx
		rep stosw
		jmp Palette_OK

	ALIGN32

	; Run length mode + Priority
	.32X_Draw_M11_P
			lea edx, [ecx * 2]
			jmp short .32X_Draw_M11_P_Loop

	ALIGN4
	
		.32X_Draw_M11_P_Loop
			movzx eax, byte [esi + 0]
			movzx ecx, byte [esi + 1]
			mov ax, [_32X_VDP_CRam + eax * 2]
			xor ax, 0x8000
			inc ecx
			add esi, byte 2
			sub edx, ecx
			jbe short .32X_Draw_M11_P_End
			rep stosw
			jmp short .32X_Draw_M11_P_Loop

	ALIGN4

	.32X_Draw_M11_P_End
		add ecx, edx
		rep stosw
		jmp Palette_OK

	ALIGN32

	.32X_Draw_M00
		mov [edi-2], word 0 ; mark that 32X was OFF
		jmp Palette_OK
