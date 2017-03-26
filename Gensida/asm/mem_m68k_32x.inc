%define _PWM_BUF_SIZE 4

	; 32X extended Read Byte
	; *******************************************

	ALIGN32

	M68K_Read_Byte_Bios_32X:
		and ebx, 0xFF
		xor ebx, byte 1
		mov al, [_32X_Genesis_Rom + ebx]
		pop ebx
		ret


	ALIGN32

	M68K_Read_Byte_BiosR_32X:
		cmp ebx, 0x100
		jae short .Rom
		
		xor ebx, byte 1
		mov al, [_32X_Genesis_Rom + ebx]
		pop ebx
		ret

	ALIGN4

	.Rom
		xor ebx, byte 1
		mov al, [Rom_Data + ebx]
		pop ebx
		ret


	ALIGN32

	M68K_Read_Byte_Misc_32X:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz short .bad
		
		push ecx
		push edx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.bad
		xor al, al
		pop ebx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne short .no_busreq

		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call _main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja short .bus_taken

		mov al, [Last_BUS_REQ_St]
		pop ebx
		or al, 0x80
		ret

	ALIGN4

	.bus_taken
		mov al, 0x80
		pop ebx
		ret

	ALIGN4

	.z80_on
		mov al, 0x81
		pop ebx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA15100
		jae near .32X_Reg

		cmp ebx, 0xA130EC
		jae short .32X_ID

		cmp ebx, 0xA1000F
		ja short .bad

		and ebx, 0x00000E
		jmp [.Table_IO_RB + ebx * 2]

	ALIGN4

	.Table_IO_RB
		dd .MD_Spec, .Pad_1, .Pad_2, .Ser
		dd .CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad

	ALIGN4

	.MD_Spec
		mov al, [Game_Mode]
		add al, al
		or al, [CPU_Mode]
		shl al, 6
		pop ebx
		or al, [Gen_Version]
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop ebx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop ebx
		ret

	ALIGN4

	.Ser
		mov al, 0xFF
		pop ebx
		ret

	ALIGN4

	.CT_Pad_1
		mov al, [Controller_1_COM]
		pop ebx
		ret

	ALIGN4

	.CT_Pad_2
		mov al, [Controller_2_COM]
		pop ebx
		ret

	ALIGN4

	.CT_Ser
		xor al, al
		pop ebx
		ret

	ALIGN4

	.32X_ID
		and ebx, 3
		mov al, [.32X_ID_Tab + ebx]
		pop ebx
		ret

	ALIGN4
	
	.32X_ID_Tab
		db 'A', 'M', 'S', 'R'

	ALIGN32

	.32X_Reg
		cmp ebx, 0xA15180
		jae near .32X_VDP_Reg

		and ebx, 0x3F
		jmp [.Table_32X_Reg + ebx * 4]

	ALIGN4

	.Table_32X_Reg
		dd .32X_ACR_H, .32X_ACR_L, .32X_bad, .32X_Int		; 00-03
		dd .32X_bad, .32X_Bank, .32X_bad, .32X_DREQ_C		; 04-07
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 08-0B
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 0C-0F

		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 10-13
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 14-17
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 18-1B
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 1C-1F

		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm		; 20
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm

		dd .32X__PWM_Cont_H, .32X__PWM_Cont_L
		dd .32X__PWM_Cycle_H, .32X__PWM_Cycle_L
		dd .32X__PWM_Pulse_L, .32X_bad
		dd .32X__PWM_Pulse_R, .32X_bad
		dd .32X__PWM_Pulse_L, .32X_bad
		dd .32X_bad, .32X_bad
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad

	ALIGN32

	.32X_ACR_H
		mov al, [_32X_FM]
		pop ebx
		ret

	ALIGN4

	.32X_ACR_L
		mov al, [_32X_ADEN]
		mov ah, [_32X_RES]
		or al, ah
		pop ebx
		or al, 0x80
		ret

	ALIGN4

	.32X_Int
		xor al, al
		pop ebx
		ret

	ALIGN4

	.32X_Bank
		mov al, [Bank_SH2]
		pop ebx
		ret

	ALIGN32

	.32X_DREQ_C
		mov al, [_32X_RV]
		mov bl, [_32X_DREQ_ST + 0]
		mov ah, [_32X_DREQ_ST + 1]
		or al, bl
		and ah, 0x80
		pop ebx
		or al, ah
		ret


	ALIGN32

	.32X_Comm
		mov al, [_32X_Comm + ebx - 0x20]
		pop ebx
		ret


	ALIGN32
	
	.32X__PWM_Cont_H
		mov al, [_PWM_Mode + 1]
		pop ebx
		ret 

	ALIGN4
	
	.32X__PWM_Cont_L
		mov al, [_PWM_Mode + 0]
		pop ebx
		ret 

	ALIGN4
	
	.32X__PWM_Cycle_H
		mov al, [_PWM_Cycle_Tmp + 1]
		pop ebx
		ret

	ALIGN4
	
	.32X__PWM_Cycle_L
		mov al, [_PWM_Cycle_Tmp + 0]
		pop ebx
		ret

	ALIGN32

	.32X__PWM_Pulse_L
		mov ebx, [_PWM_RP_L]
		mov eax, [_PWM_WP_L]
		mov al, [_PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop ebx
		ret

	ALIGN4

	.32X__PWM_Pulse_R
		mov ebx, [_PWM_RP_R]
		mov eax, [_PWM_WP_R]
		mov al, [_PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop ebx
		ret

	ALIGN4

	.32X__PWM_Pulse_C
		mov ebx, [_PWM_RP_L]
		mov eax, [_PWM_WP_L]
		mov al, [_PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop ebx
		ret


	ALIGN32
	
	.32X_VDP_Reg
		test byte [_32X_FM], 0xFF
		jnz near .32X_bad
		cmp ebx, 0xA15200
		jae near .32X_bad

		and ebx, 0xF
		jmp [.Table_32X_VDP_Reg + ebx * 4]

	ALIGN4

	.Table_32X_VDP_Reg
		dd .32X_VDP_Mode_H, .32X_VDP_Mode_L, .32X_bad, .32X_VDP_Shift
		dd .32X_bad, .32X_VDP_AF_Len_L, .32X_VDP_AF_St_H, .32X_VDP_AF_St_L
		dd .32X_VDP_AF_Data_H, .32X_VDP_AF_Data_L, .32X_VDP_State_H, .32X_VDP_State_L
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad

	ALIGN32

	.32X_VDP_Mode_H
		mov al, [_32X_VDP + vx.Mode + 1]
		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_Mode_L
		mov al, [_32X_VDP + vx.Mode + 0]
		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_Shift
		mov al, [_32X_VDP + vx.Mode + 2]
		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_AF_Len_L
		mov al, [_32X_VDP + vx.AF_Len + 0]
		pop ebx
		ret

	ALIGN32
	
	.32X_VDP_AF_St_H
		mov al, [_32X_VDP + vx.AF_St + 1]
		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_AF_St_L
		mov al, [_32X_VDP + vx.AF_St + 0]
		pop ebx
		ret

	ALIGN4

	.32X_VDP_AF_Data_H
		mov al, [_32X_VDP + vx.AF_Data + 1]
		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_AF_Data_L
		mov al, [_32X_VDP + vx.AF_Data + 0]
		pop ebx
		ret

	ALIGN32
	
	.32X_VDP_State_H
		mov al, [_32X_VDP + vx.State + 1]
		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_State_L
		mov al, [_32X_VDP + vx.State]
		xor al, 2
		mov [_32X_VDP + vx.State], al
		pop ebx
		ret

	ALIGN4

	.32X_bad
		xor al, al
		pop ebx
		ret


	ALIGN32
	
	DECL M68K_Read_Byte_32X_FB0
		and ebx, 0x1FFFF
		xor ebx, byte 1
		mov al, [_32X_VDP_Ram + ebx]
		pop ebx
		ret


	ALIGN32
	
	DECL M68K_Read_Byte_32X_FB1
		and ebx, 0x1FFFF
		xor ebx, byte 1
		mov al, [_32X_VDP_Ram + ebx + 0x20000]
		pop ebx
		ret




	; 32X extended Read Word
	; *******************************************

	ALIGN32

	M68K_Read_Word_Bios_32X:
		and ebx, 0xFE
		mov ax, [_32X_Genesis_Rom + ebx]
		pop ebx
		ret


	ALIGN32

	M68K_Read_Word_BiosR_32X:
		cmp ebx, 0x100
		jae short .Rom
		
		mov ax, [_32X_Genesis_Rom + ebx]
		pop ebx
		ret

	ALIGN4

	.Rom
		mov ax, [Rom_Data + ebx]
		pop ebx
		ret


	ALIGN32

	M68K_Read_Word_Misc_32X:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram

		test byte [Z80_State], 6
		jnz near .bad

		push ecx
		push edx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.no_Z80_ram
		cmp ebx, 0xA11100
		jne short .no_busreq

		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call _main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja short .bus_taken

		mov al, [Fake_Fetch]
		mov ah, [Last_BUS_REQ_St]
		xor al, 0xFF
		add ah, 0x80
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		pop ebx
		ret

	ALIGN4

	.bus_taken
		mov al, [Fake_Fetch]
		mov ah, 0x80
		xor al, 0xFF
		pop ebx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.z80_on
		mov al, [Fake_Fetch]
		mov ah, 0x81
		xor al, 0xFF
		pop ebx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA15100
		jae near .32X_Reg

		cmp ebx, 0xA130EC
		jae short .32X_ID

		cmp ebx, 0xA1000F
		ja short .bad

		and ebx, 0x00000E
		jmp [.Table_IO_RW + ebx * 2]

	ALIGN4

	.Table_IO_RW
		dd .MD_Spec, .Pad_1, .Pad_2, .Ser
		dd .CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad

	ALIGN4

	.MD_Spec
		mov eax, [Game_Mode]
		add eax, eax
		or eax, [CPU_Mode]
		shl eax, 6
		pop ebx
		or eax, [Gen_Version]									; on recupere les infos hardware de la machine
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop ebx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop ebx
		ret

	ALIGN4

	.Ser
		mov ax, 0xFF00
		pop ebx
		ret

	ALIGN4

	.bad
		xor eax, eax
		pop ebx
		ret

	ALIGN4

	.CT_Pad_1
		mov eax, [Controller_1_COM]
		pop ebx
		ret

	ALIGN4

	.CT_Pad_2
		mov eax, [Controller_2_COM]
		pop ebx
		ret

	ALIGN4

	.CT_Ser
		xor eax, eax
		pop ebx
		ret

	ALIGN4

	.32X_ID
		and ebx, 3
		mov ax, [.32X_ID_Tab + ebx]
		pop ebx
		ret

	ALIGN4
	
	.32X_ID_Tab
		db 'A', 'M', 'S', 'R'

	ALIGN4

	.32X_Reg
		cmp ebx, 0xA15180
		jae near .32X_VDP_Reg

		and ebx, 0x3E
		jmp [.Table_32X_Reg + ebx * 2]

	ALIGN4

	.Table_32X_Reg
		dd .32X_ACR, .32X_INT, .32X_Bank, .32X_DREQ_C		; 00-07
		dd .32X_DREQ_Src_H, .32X_DREQ_Src_L,				; 08-0B
		dd .32X_DREQ_Dest_H, .32X_DREQ_Dest_L,				; 0C-0F

		dd .32X_DREQ_Len, .32X_FIFO, .32X_bad, .32X_bad		; 10-17
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 18-1F

		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm		; 20
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm

		dd .32X__PWM_Cont, .32X__PWM_Cycle
		dd .32X__PWM_Pulse_L, .32X__PWM_Pulse_R
		dd .32X__PWM_Pulse_C, .32X_bad
		dd .32X_bad, .32X_bad

	ALIGN32

	.32X_ACR
		mov al, [_32X_ADEN]
		mov ah, [_32X_RES]
		or al, ah
		mov ah, [_32X_FM]
		or al, 0x80
		pop ebx
		ret

	ALIGN4

	.32X_INT
		xor ax, ax
		pop ebx
		ret

	ALIGN4

	.32X_Bank
		mov al, [Bank_SH2]
		xor ah, ah
		pop ebx
		ret

	ALIGN4

	.32X_DREQ_C
		mov bl, [_32X_DREQ_ST + 0]
		mov ah, [_32X_DREQ_ST + 1]
		mov al, [_32X_RV]
		and ah, 0x80
		or al, bl
		or al, ah
		pop ebx
		xor ah, ah
		ret

	ALIGN4

	.32X_DREQ_Src_H
		mov ax, [_32X_DREQ_SRC + 2]
		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Src_L
		mov ax, [_32X_DREQ_SRC]
		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Dest_H
		mov ax, [_32X_DREQ_DST + 2]
		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Dest_L
		mov ax, [_32X_DREQ_DST]
		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Len
		mov ax, [_32X_DREQ_LEN]
		pop ebx
		ret

	ALIGN4

	.32X_FIFO
		pop ebx
		ret


	ALIGN32

	.32X_Comm
		mov ah, [_32X_Comm + ebx - 0x20 + 0]
		mov al, [_32X_Comm + ebx - 0x20 + 1]
		pop ebx
		ret


	ALIGN32
	
	.32X__PWM_Cont
		mov ax, [_PWM_Mode]
		pop ebx
		ret 
		
	ALIGN4
	
	.32X__PWM_Cycle
		mov ax, [_PWM_Cycle_Tmp]
		pop ebx
		ret

	ALIGN32

	.32X__PWM_Pulse_L
		mov ebx, [_PWM_RP_L]
		mov eax, [_PWM_WP_L]
		mov ah, [_PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop ebx
		ret

	ALIGN4

	.32X__PWM_Pulse_R
		mov ebx, [_PWM_RP_R]
		mov eax, [_PWM_WP_R]
		mov ah, [_PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop ebx
		ret

	ALIGN4

	.32X__PWM_Pulse_C
		mov ebx, [_PWM_RP_L]
		mov eax, [_PWM_WP_L]
		mov ah, [_PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop ebx
		ret


	ALIGN32

	.32X_VDP_Reg
		test byte [_32X_FM], 0xFF
		jnz near .32X_bad
		cmp ebx, 0xA15200
		jae near .32X_CRAM

		and ebx, 0xE
		jmp [.Table_32X_VDP_Reg + ebx * 2]

	ALIGN4

	.Table_32X_VDP_Reg
		; VDP REG

		dd .32X_VDP_Mode, .32X_VDP_Shift, .32X_VDP_AF_Len, .32X_VDP_AF_St
		dd .32X_VDP_AF_Data, .32X_VDP_State, .32X_bad, .32X_bad

	ALIGN32
	
	.32X_VDP_Mode
		mov ax, [_32X_VDP + vx.Mode]
		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_Shift
		mov al, [_32X_VDP + vx.Mode + 2]
		xor ah, ah
		pop ebx
		ret

	ALIGN4

	.32X_VDP_AF_Len
		mov al, [_32X_VDP + vx.AF_Len]
		xor ah, ah
		pop ebx
		ret

	ALIGN4

	.32X_VDP_AF_St
		mov ax, [_32X_VDP + vx.AF_St]
		pop ebx
		ret

	ALIGN4

	.32X_VDP_AF_Data
		mov ax, [_32X_VDP + vx.AF_Data]
		pop ebx
		ret

	ALIGN32

	.32X_VDP_State
		mov ax, [_32X_VDP + vx.State]
		xor ax, byte 2
		mov [_32X_VDP + vx.State], ax
		pop ebx
		ret

	ALIGN4

	.32X_bad
		xor ax, ax
		pop ebx
		ret

	ALIGN32

	.32X_CRAM
		cmp ebx, 0xA15400
		jae short .32X_bad

		mov ax, [_32X_VDP_CRam + ebx - 0xA15200]
		pop ebx
		ret


	ALIGN32
	
	DECL M68K_Read_Word_32X_FB0
		and ebx, 0x1FFFE
		mov ax, [_32X_VDP_Ram + ebx]
		pop ebx
		ret


	ALIGN32
	
	DECL M68K_Read_Word_32X_FB1
		and ebx, 0x1FFFE
		mov ax, [_32X_VDP_Ram + ebx + 0x20000]
		pop ebx
		ret



	; 32X extended Write Byte
	; *******************************************

	ALIGN32

	M68K_Write_Byte_Misc_32X:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz short .bad

		push edx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		mov edx, eax
		call [Z80_WriteB_Table + ebx]
		pop edx
;		pop ecx
;		pop ebx
		ret
		
	ALIGN4

	.bad
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne near .no_busreq

		xor ecx, ecx
		mov ah, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test al, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .deactivated

		test ah, 2
		jnz short .already_actived

		or ah, 2
		push edx
		mov [Z80_State], ah
		mov ebx, [Cycles_M68K]
		call _main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop edx

	.already_actived
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.deactivated
		call _main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_deactivated

		push edx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop edx

	.already_deactivated
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test al, 1
		jnz short .no_reset

		push edx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call _YM2612_Reset
		pop edx
;		pop ecx
;		pop ebx
		ret

	.no_reset
		and byte [Z80_State], ~4
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.no_reset_z80
		cmp ebx, 0xA15100
		jae near .32X_Reg

		cmp ebx, 0xA130F0
		jae short .Genesis_Bank

		cmp ebx, 0xA1000F
		ja near .bad

		and ebx, 0x00000E
		jmp [.Table_IO_WB + ebx * 2]

	ALIGN4

	.Table_IO_WB
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push eax
		call WR_Controller_1
		pop eax
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.Pad_2
		push eax
		call WR_Controller_2
		pop eax
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.Genesis_Bank
		cmp ebx, 0xA130F2
		jb short .bank_0
		cmp ebx, 0xA130FF
		ja near .bad

		and ebx, 0xF
		and eax, 0x1F
		shr ebx, 1
		mov ecx, [Genesis_M68K_Read_Byte_Table + eax * 4]
		mov [M68K_Read_Byte_Table + ebx * 4], ecx
		mov ecx, [Genesis_M68K_Read_Word_Table + eax * 4]
		mov [M68K_Read_Word_Table + ebx * 4], ecx

;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.bank_0
		test al, 1
		setnz [SRAM_ON]
		test al, 2
		setz [SRAM_Write]
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_Reg
		cmp ebx, 0xA15180
		jae near .32X_VDP_Reg

;pushad
;push eax
;push ebx
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		and ebx, 0x3F
		jmp [.Table_32X_Reg + ebx * 4]

	ALIGN4

	.Table_32X_Reg
		dd .32X_ACR_H, .32X_ACR_L, .32X_bad, .32X_Int		; 00-03
		dd .32X_bad, .32X_Bank, .32X_bad, .32X_DREQ_C		; 04-07
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 08-0B
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 0C-0F

		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 10-13
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 14-17
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 18-1B
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 1C-1F

		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm		; 20
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm

		dd .32X_bad, .32X__PWM_Cont_L
		dd .32X__PWM_Cycle_H, .32X__PWM_Cycle_L
		dd .32X__PWM_Pulse_L_H, .32X__PWM_Pulse_L_L
		dd .32X__PWM_Pulse_R_H, .32X__PWM_Pulse_R_L
		dd .32X__PWM_Pulse_C_H, .32X__PWM_Pulse_C_L
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad
		dd .32X_bad, .32X_bad

	ALIGN32

	.32X_ACR_H
		and al, 0x80
		mov ah, [_32X_FM]
;		pop ecx
		xor ah, al
;		pop ebx
		mov [_32X_FM], al
		jnz near __32X_Set_FB
		ret

	ALIGN32

	.32X_ACR_L
		mov ah, [_32X_RES]
		mov bl, al
		and al, 2
		mov [_32X_RES], al
		cmp ax, byte 2
		jne short .no_SH2_reset

		push edx
		mov ecx, M_SH2
		mov edx, 1
		call SH2_Reset
		mov ecx, S_SH2
		mov edx, 1
		call SH2_Reset
		pop edx

	.no_SH2_reset
		mov al, [_32X_ADEN]
		and bl, 1
		xor al, bl
		jz short .no_32X_change

		mov [_32X_ADEN], bl
		call _M68K_32X_Mode

	.no_32X_change
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_Int
		mov bl, al
		mov ah, [_32X_MINT]
		add al, al
		and bl, 2
		and al, 2
		mov bh, [_32X_SINT]
		test ah, al
		push edx
		jz short .no_MINT

		mov edx, 8
		mov ecx, M_SH2
		call SH2_Interrupt

	.no_MINT
		test bh, bl
		jz short .no_SINT

		mov edx, 8
		mov ecx, S_SH2
		call SH2_Interrupt

	.no_SINT
		pop edx
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_Bank
		and al, 3
		mov [Bank_SH2], al
		call _M68K_Set_32X_Rom_Bank
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_DREQ_C
		mov bl, al
		mov ah, [_32X_RV]
		and al, 1
		mov bh, [_32X_DREQ_ST]
		and bl, 4
		xor ah, al
		jz short .RV_not_changed

		mov [_32X_RV], al
		call _M68K_32X_Mode

	.RV_not_changed
		cmp bx, 0x0004
		jne short .No_DREQ

		xor al, al
		mov byte [_32X_DREQ_ST + 1], 0x40
		mov [_32X_FIFO_Block], al
		mov [_32X_FIFO_Read], al
		mov [_32X_FIFO_Write], al

	.No_DREQ
		mov [_32X_DREQ_ST], bl
;		pop ecx
;		pop ebx
		ret


	ALIGN32

	.32X_Comm
		mov [_32X_Comm + ebx - 0x20], al
;		pop ecx
;		pop ebx
		ret

	
	ALIGN4
	
	.32X__PWM_Cont_L
		mov [_PWM_Mode + 0], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X__PWM_Cycle_H
		mov cl, [_PWM_Cycle_Tmp + 0]
		mov [_PWM_Cycle_Tmp + 1], al
		mov ch, al
		call @PWM_Set_Cycle@4
;		pop ecx
;		pop ebx
		ret
	
	ALIGN4
	
	.32X__PWM_Cycle_L
		mov ch, [_PWM_Cycle_Tmp + 1]
		mov [_PWM_Cycle_Tmp + 0], al
		mov cl, al
		call @PWM_Set_Cycle@4
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X__PWM_Pulse_L_H
		mov [_PWM_FIFO_L_Tmp + 1], al
;		pop ecx
;		pop ebx
		ret
	
	ALIGN32
	
	.32X__PWM_Pulse_L_L
		mov ecx, [_PWM_RP_L]
		mov ebx, [_PWM_WP_L]
		mov ah, [_PWM_FIFO_L_Tmp + 1]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_L_full

		mov [_PWM_FIFO_L + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_L], ebx
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X__PWM_Pulse_L_full
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X__PWM_Pulse_R_H
		mov [_PWM_FIFO_R_Tmp + 1], al
;		pop ecx
;		pop ebx
		ret
	
	ALIGN32
	
	.32X__PWM_Pulse_R_L
		mov ecx, [_PWM_RP_R]
		mov ebx, [_PWM_WP_R]
		mov ah, [_PWM_FIFO_R_Tmp + 1]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_R_full

		mov [_PWM_FIFO_R + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_R], ebx
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X__PWM_Pulse_R_full
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X__PWM_Pulse_C_H
		mov [_PWM_FIFO_L_Tmp + 1], al
;		pop ecx
;		pop ebx
		ret
	
	ALIGN32
	
	.32X__PWM_Pulse_C_L
		mov ecx, [_PWM_RP_L]
		mov ebx, [_PWM_WP_L]
		mov ah, [_PWM_FIFO_L_Tmp + 1]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_C_full

		mov [_PWM_FIFO_L + ebx * 2], ax
		mov [_PWM_FIFO_R + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_L], ebx
		mov [_PWM_WP_R], ebx
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X__PWM_Pulse_C_full
;		pop ecx
;		pop ebx
		ret


	ALIGN32

	.32X_VDP_Reg
		test byte [_32X_FM], 0xFF
		jnz near .32X_bad
		cmp ebx, 0xA15200
		jae near .32X_bad

;pushad
;push eax
;push ebx
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		and ebx, 0xF
		jmp [.Table_32X_VDP_Reg + ebx * 4]

	ALIGN4

	.Table_32X_VDP_Reg
		dd .32X_bad, .32X_VDP_Mode, .32X_bad, .32X_VDP_Shift
		dd .32X_bad, .32X_VDP_AF_Len, .32X_bad, .32X_bad
		dd .32X_bad, .32X_bad, .32X_bad, .32X_VDP_State
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad

	ALIGN32
	
	.32X_VDP_Mode
		mov [_32X_VDP + vx.Mode], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_Shift
		mov [_32X_VDP + vx.Mode + 2], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_AF_Len
		mov [_32X_VDP + vx.AF_Len], al
;		pop ecx
;		pop ebx
		ret

	ALIGN32
	
	.32X_VDP_State
		mov bh, [_32X_VDP + vx.Mode + 0]
		mov bl, [_32X_VDP + vx.State + 1]
		test bh, 3
		mov [_32X_VDP + vx.State + 2], al
		jz short .32X_VDP_blank

		test bl, bl
		jns short .32X_VDP_State_nvb

	.32X_VDP_blank
		mov [_32X_VDP + vx.State + 0], al
		call __32X_Set_FB

	.32X_VDP_State_nvb
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_bad
;		pop ecx
;		pop ebx
		ret


	ALIGN32
	
	DECL M68K_Write_Byte_32X_FB0
		and ebx, 0x1FFFF
		test al, al
		jz short .blank

		xor ebx, byte 1
		mov [_32X_VDP_Ram + ebx], al

	.blank
;		pop ecx
;		pop ebx
		ret

	ALIGN32
	
	DECL M68K_Write_Byte_32X_FB1
		and ebx, 0x1FFFF
		test al, al
		jz short .blank

		xor ebx, byte 1
		mov [_32X_VDP_Ram + ebx + 0x20000], al

	.blank
;		pop ecx
;		pop ebx
		ret



	; 32X extended Write Word
	; *******************************************


	ALIGN32

	M68K_Write_Word_Misc_32X:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram
		
		test byte [Z80_State], 6
		jnz near .bad

		push edx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		mov dh, al
		shr ebx, 10
		mov dl, al
		call [Z80_WriteB_Table + ebx]
		pop edx
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.no_Z80_ram
		cmp ebx, 0xA11100
		jne near .no_busreq
		
		xor ecx, ecx
		mov al, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test ah, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .deactivated

		test al, 2
		jnz short .already_actived

		or al, 2
		push edx
		mov [Z80_State], al
		mov ebx, [Cycles_M68K]
		call _main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop edx

	.already_actived
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.deactivated
		call _main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_deactivated

		push edx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop edx

	.already_deactivated
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test ah, 1
		jnz short .no_reset

		push edx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call _YM2612_Reset
		pop edx
;		pop ecx
;		pop ebx
		ret

	.no_reset
		and byte [Z80_State], ~4
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.no_reset_z80
		cmp ebx, 0xA15100
		jae near .32X_Reg

		cmp ebx, 0xA130F0
		jae short .Genesis_Bank

		cmp ebx, 0xA1000F
		ja short .bad

		and ebx, 0x00000E
		jmp [.Table_IO_WW + ebx * 2]

	ALIGN4

	.Table_IO_WW
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push eax
		call WR_Controller_1
		pop eax
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.Pad_2
		push eax
		call WR_Controller_2
		pop eax
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], ax
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], ax
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.bad
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.Genesis_Bank
		cmp ebx, 0xA130F2
		jb short .bank_0
		cmp ebx, 0xA130FF
		ja near .bad

;		and ebx, 0xF
;		and eax, 0x1F
;		shr ebx, 1
;		mov ecx, [Genesis_M68K_Read_Byte_Table + eax * 4]
;		mov [M68K_Read_Byte_Table + ebx * 4], ecx
;		mov ecx, [Genesis_M68K_Read_Word_Table + eax * 4]
;		mov [M68K_Read_Word_Table + ebx * 4], ecx

;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.bank_0
		test al, 1
		setnz [SRAM_ON]
		test al, 2
		setz [SRAM_Write]
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_Reg
		cmp ebx, 0xA15180
		jae near .32X_VDP_Reg

;pushad
;push eax
;push ebx
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		and ebx, 0x3E
		jmp [.Table_32X_Reg + ebx * 2]

	ALIGN4

	.Table_32X_Reg
		dd .32X_ACR, .32X_INT, .32X_Bank, .32X_DREQ_C		; 00-07
		dd .32X_DREQ_Src_H, .32X_DREQ_Src_L,				; 08-0B
		dd .32X_DREQ_Dest_H, .32X_DREQ_Dest_L,				; 0C-0F

		dd .32X_DREQ_Len, .32X_FIFO, .32X_bad, .32X_bad		; 10-17
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 18-1F

		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm		; 20
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm

		dd .32X__PWM_Cont, .32X__PWM_Cycle
		dd .32X__PWM_Pulse_L, .32X__PWM_Pulse_R
		dd .32X__PWM_Pulse_C, .32X_bad
		dd .32X_bad, .32X_bad

	ALIGN32

	.32X_ACR
		mov bh, [_32X_FM]
		and ah, 0x80
		mov bl, al
		xor bh, ah
		mov [_32X_FM], ah
		jz short .no_update_FB

		call __32X_Set_FB

	.no_update_FB
		mov al, bl
		mov ah, [_32X_RES]
		and al, 2
		mov [_32X_RES], al
		cmp ax, byte 2
		jne short .no_SH2_reset

		push edx
		mov ecx, M_SH2
		mov edx, 1
		call SH2_Reset
		mov ecx, S_SH2
		mov edx, 1
		call SH2_Reset
		pop edx

	.no_SH2_reset
		mov al, [_32X_ADEN]
		and bl, 1
		xor al, bl
		jz short .no_32X_change

		mov [_32X_ADEN], bl
		call _M68K_32X_Mode

	.no_32X_change
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_INT
		mov bl, al
		mov ah, [_32X_MINT]
		add al, al
		and bl, 2
		and al, 2
		mov bh, [_32X_SINT]
		test ah, al
		push edx
		jz short .no_MINT

		mov edx, 8
		mov ecx, M_SH2
		call SH2_Interrupt

	.no_MINT
		test bh, bl
		jz short .no_SINT

		mov edx, 8
		mov ecx, S_SH2
		call SH2_Interrupt

	.no_SINT
		pop edx
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_Bank
		and al, 3
		mov [Bank_SH2], al
		call _M68K_Set_32X_Rom_Bank
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_DREQ_C
		mov bl, al
		mov ah, [_32X_RV]
		and al, 1
		mov bh, [_32X_DREQ_ST]
		and bl, 4
		xor ah, al
		jz short .RV_not_changed

		mov [_32X_RV], al
		call _M68K_32X_Mode

	.RV_not_changed
		cmp bx, 0x0004
		jne short .No_DREQ

		xor al, al
		mov byte [_32X_DREQ_ST + 1], 0x40
		mov [_32X_FIFO_Block], al
		mov [_32X_FIFO_Read], al
		mov [_32X_FIFO_Write], al

	.No_DREQ
		mov [_32X_DREQ_ST], bl
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Src_H
		mov [_32X_DREQ_SRC + 2], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Src_L
		and ax, byte ~1
;		pop ecx
		mov [_32X_DREQ_SRC], ax
;		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Dest_H
		mov [_32X_DREQ_DST + 2], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Dest_L
		mov [_32X_DREQ_DST], ax
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_DREQ_Len
		and ax, byte ~3
;		pop ecx
		mov [_32X_DREQ_LEN], ax
;		pop ebx
		ret

	ALIGN4

	.32X_FIFO
		mov cx, [_32X_DREQ_ST]
		mov ebx, [_32X_FIFO_Write]
		and cx, 0x8004
		cmp cx, 0x0004
		mov ecx, [_32X_FIFO_Block]
		jne short .32X_FIFO_End

;pushad
;push eax
;lea eax, [0xA15500 + ecx + ebx * 2]
;push eax
;call _Write_To_68K_Space
;pop eax
;pop eax
;popad

		mov [_32X_FIFO_A + ecx + ebx * 2], ax
		inc ebx
		cmp ebx, 4
		jae short .32X_FIFO_Full_A

		mov [_32X_FIFO_Write], ebx

	.32X_FIFO_End
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_FIFO_Full_A
		mov bl, [_32X_DREQ_ST + 1]
		push edx
		test bl, 0x40
		jz short .32X_FIFO_Full_B

		xor al, al
		xor ecx, byte (4 * 2)
		mov [_32X_DREQ_ST + 1], al
		mov [_32X_FIFO_Write], al
		mov [_32X_FIFO_Read], al
		mov [_32X_FIFO_Block], ecx

		mov dl, 1
		mov ecx, M_SH2
		call SH2_DMA0_Request

		pop edx
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_FIFO_Full_B
		mov byte [_32X_DREQ_ST + 1], 0x80

		mov dl, 1
		mov ecx, M_SH2
		call SH2_DMA0_Request

		pop edx
;		pop ecx
;		pop ebx
		ret


	ALIGN32

	.32X_Comm
		mov [_32X_Comm + ebx - 0x20 + 0], ah
		mov [_32X_Comm + ebx - 0x20 + 1], al
;		pop ecx
;		pop ebx
		ret


	ALIGN32
	
	.32X__PWM_Cont
		and al, 0x0F
;		pop ecx
		mov [_PWM_Mode], al
;		pop ebx
		ret
		
	ALIGN32
	
	.32X__PWM_Cycle
		mov ecx, eax
		call @PWM_Set_Cycle@4
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X__PWM_Pulse_L
		mov ecx, [_PWM_RP_L]
		mov ebx, [_PWM_WP_L]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_L_full

		mov [_PWM_FIFO_L + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_L], ebx
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X__PWM_Pulse_L_full
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X__PWM_Pulse_R
		mov ecx, [_PWM_RP_R]
		mov ebx, [_PWM_WP_R]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_R_full

		mov [_PWM_FIFO_R + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_R], ebx
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X__PWM_Pulse_R_full
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X__PWM_Pulse_C
		mov ecx, [_PWM_RP_L]
		mov ebx, [_PWM_WP_L]
		test byte [_PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_C_full

		mov [_PWM_FIFO_L + ebx * 2], ax
		mov [_PWM_FIFO_R + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [_PWM_WP_L], ebx
		mov [_PWM_WP_R], ebx
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X__PWM_Pulse_C_full
;		pop ecx
;		pop ebx
		ret


	ALIGN32

	.32X_VDP_Reg
		test byte [_32X_FM], 0xFF
		jnz near .32X_bad
		cmp ebx, 0xA15200
		jae near .32X_CRAM

;pushad
;push eax
;push ebx
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		and ebx, 0xE
		jmp [.Table_32X_VDP_Reg + ebx * 2]

	ALIGN4

	.Table_32X_VDP_Reg
		dd .32X_VDP_Mode, .32X_VDP_Shift, .32X_VDP_AF_Len, .32X_VDP_AF_St
		dd .32X_VDP_AF_Data, .32X_VDP_State, .32X_bad, .32X_bad

	ALIGN32

	.32X_VDP_Mode
		mov [_32X_VDP + vx.Mode], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4
	
	.32X_VDP_Shift
		mov [_32X_VDP + vx.Mode + 2], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_VDP_AF_Len
		mov [_32X_VDP + vx.AF_Len], al
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_VDP_AF_St
		mov [_32X_VDP + vx.AF_St], ax
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_VDP_AF_Data
		push edi
		mov [_32X_VDP + vx.AF_Data], ax
		mov bx, ax
		mov edi, [_32X_VDP + vx.State]
		shl eax, 16
		and edi, byte 1
		mov ax, bx
		xor edi, byte 1
		mov ebx, [_32X_VDP + vx.AF_St]
		mov ecx, [_32X_VDP + vx.AF_Len]
		shl edi, 17
		inc ecx
		shr ecx, 1
		lea edi, [edi + _32X_VDP_Ram]
		jz short .Spec_Fill
		jnc short .Loop

		mov [edi + ebx * 2], ax
		inc bl
		jmp short .Loop

	ALIGN32

	.Loop
		mov [edi + ebx * 2], eax
		add bl, byte 2
		dec ecx
		jns short .Loop

		mov [_32X_VDP + vx.AF_St], ebx
		pop edi
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.Spec_Fill
		mov [edi + ebx * 2], ax
		inc bl
		pop edi
		mov [_32X_VDP + vx.AF_St], ebx
;		pop ecx
;		pop ebx
		ret


	ALIGN4

	.32X_VDP_State
		mov bh, [_32X_VDP + vx.Mode + 0]
		mov bl, [_32X_VDP + vx.State + 1]
		test bh, 3
		mov [_32X_VDP + vx.State + 2], al
		jz short .32X_VDP_blank

		test bl, bl
		jns short .32X_VDP_State_nvb
		
	.32X_VDP_blank
		mov [_32X_VDP + vx.State + 0], al
		call __32X_Set_FB

	.32X_VDP_State_nvb
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.32X_bad
;		pop ecx
;		pop ebx
		ret

	ALIGN32

	.32X_CRAM
		cmp ebx, 0xA15400
		jae short .32X_bad
		push edx
		
		and eax, 0xFFFF
		mov cx, [_32X_Palette_16B + eax * 2]
		mov edx, [_32X_Palette_32B + eax * 4]
		mov [_32X_VDP_CRam + ebx - 0xA15200], ax
		mov [_32X_VDP_CRam_Ajusted + ebx - 0xA15200], cx
		mov [_32X_VDP_CRam_Ajusted32 + (ebx - 0xA15200) * 2],edx

		pop edx
;		pop ecx
;		pop ebx
		ret


	ALIGN32

	DECL M68K_Write_Word_32X_FB0
		and ebx, 0x3FFFE
		test ebx, 0x20000
		jnz short .overwrite

		mov [_32X_VDP_Ram + ebx], ax
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.overwrite
		test al, al
		jz short .blank1

		mov [_32X_VDP_Ram + ebx - 0x20000 + 0], al

	.blank1
		test ah, ah
		jz short .blank2

		mov [_32X_VDP_Ram + ebx - 0x20000 + 1], ah

	.blank2
;		pop ecx
;		pop ebx
		ret


	ALIGN32

	DECL M68K_Write_Word_32X_FB1
		and ebx, 0x3FFFE
		test ebx, 0x20000
		jnz short .overwrite

		mov [_32X_VDP_Ram + ebx + 0x20000], ax
;		pop ecx
;		pop ebx
		ret

	ALIGN4

	.overwrite
		test al, al
		jz short .blank1

		mov [_32X_VDP_Ram + ebx - 0x20000 + 0x20000 + 0], al

	.blank1
		test ah, ah
		jz short .blank2

		mov [_32X_VDP_Ram + ebx - 0x20000 + 0x20000 + 1], ah

	.blank2
;		pop ecx
;		pop ebx
		ret

