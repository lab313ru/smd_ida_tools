%include "nasmhead.inc"
%define PWM_BUF_SIZE 4


section .bss align=64

	DECL PWM_FIFO_R
	resw PWM_BUF_SIZE

	DECL PWM_FIFO_L
	resw PWM_BUF_SIZE

	DECL PWM_RP_R
	resd 1

	DECL PWM_WP_R
	resd 1

	DECL PWM_RP_L
	resd 1

	DECL PWM_WP_L
	resd 1

	DECL PWM_Cycle
	resd 1

	DECL PWM_Cycle_Cnt
	resd 1

	DECL PWM_Int_Cnt
	resd 1

	DECL PWM_Out_R
	resd 1

	DECL PWM_Out_L
	resd 1

	DECL PWM_Mode
	resd 1

	DECL PWM_Cycles
	resd 1

	DECL PWM_Int
	resd 1

	DECL PWM_Enable
	resd 1

	DECL PWM_Cycle_Tmp
	resd 1

	DECL PWM_Int_Tmp
	resd 1

	DECL PWM_FIFO_L_Tmp
	resd 1

	DECL PWM_FIFO_R_Tmp
	resd 1
	
	DECL PWM_Out_L_Tmp
	resd 1

	DECL PWM_Out_R_Tmp
	resd 1


section .text align=64

	extern _ApplyPWMVol
	
	; void PWM_Update(int **buf, int length)
	DECLF PWM_Update, 8

		push ebx
		push esi

		test byte [PWM_Enable], 0xFF
		mov eax, [PWM_Out_L]
		jz short .End

		mov ebx, ecx
		shl eax, 5
		mov ecx, edx
		mov edx, [PWM_Out_R]
		and eax, 0xFFFF
		shl edx, 5
		mov esi, [ebx + 4]
		and edx, 0xFFFF
		sub eax, 0x4000
		sub edx, 0x4000
		mov [PWM_Out_L_Tmp],eax
		mov [PWM_Out_R_Tmp],edx
		pushad
		call _ApplyPWMVol
		popad
		mov eax,[PWM_Out_L_Tmp]
		mov edx,[PWM_Out_R_Tmp]
		test ecx, ecx
		mov ebx, [ebx]
		jnz short .Loop

		pop esi
		pop ebx
		ret

	ALIGN32

	.Loop
		add [ebx], eax
		add [esi], edx
		add ebx, byte 4
		add esi, byte 4
		dec ecx
		jnz short .Loop

	.End
		pop esi
		pop ebx
		ret