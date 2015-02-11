/*
*      Interactive disassembler (IDA).
*      Copyright (c) 1990-2000 by Ilfak Guilfanov, <ig@datarescue.com>
*      ALL RIGHTS RESERVED.
*
*/

#define VERSION "1.0.2"
/*
*      SEGA MEGA DRIVE/GENESIS constants identifier plugin
*      Author: Dr. MefistO [Lab 313] <meffi@lab313.ru>
*/

#include <ida.hpp>
#include <idp.hpp>
#include <bytes.hpp>
#include <loader.hpp>
#include <kernwin.hpp>

static const char format[] = "Sega Genesis/Megadrive constants identifier plugin v%s;\nAuthor: Dr. MefistO [Lab 313] <meffi@lab313.ru>.\n";
static const char wrong_vdp_cmd[] = "Wrong command to send to VDP_CTRL!\n";

//--------------------------------------------------------------------------
static void print_version()
{
	msg(format, VERSION);
}

//--------------------------------------------------------------------------
int idaapi init(void)
{
	print_version();
	return PLUGIN_OK;
}

//--------------------------------------------------------------------------
void idaapi term(void)
{
}

//--------------------------------------------------------------------------
static unsigned int mask(unsigned char bit_idx, unsigned char bits_cnt = 1)
{
	return (((1 << bits_cnt) - 1) << bit_idx);
}

//--------------------------------------------------------------------------
static bool is_vdp_send_cmd(uval_t val)
{
	if (val & 0xFFFF0000)
	{
		return ((val & 0x9F000000) >= 0x80000000) && ((val & 0x9F000000) <= 0x97000000);
	}
	else
	{
		return ((val & 0x9F00) >= 0x8000) && ((val & 0x9F00) <= 0x9700);
	}
}

//--------------------------------------------------------------------------
static bool is_vdp_rw_cmd(uval_t val)
{
	if (val & 0xFFFF0000) // command was sended by one dword
	{
		switch ((val >> 24) & mask(6, 2))
		{
		case 0 /*00*/ << 6:
		case 1 /*01*/ << 6:
		case 3 /*11*/ << 6:
		{
			switch ((val & 0xFF) & mask(4, 2))
			{
			case 0 /*00*/ << 4:
			case 1 /*01*/ << 4:
			case 2 /*10*/ << 4:
			{
				return true;
			}
			}
			return false;
		}
		}
		return false;
	}
	else // command was sended by halfs (this is high word of it)
	{
		switch ((val >> 8) & mask(6, 2))
		{
		case 0 /*00*/ << 6:
		case 1 /*01*/ << 6:
		case 3 /*11*/ << 6:
		{
			return true;
		}
		}
		return false;
	}
}

//--------------------------------------------------------------------------
static bool do_cmt_vdp_reg_const(ea_t ea, uval_t val)
{
	if (!val) return false;

	char name[250];
	unsigned int addr = 0;
	switch (val & 0x9F00)
	{
	case 0x8000:
	{
		if (val & mask(0))	append_cmt(ea, "DISPLAY_OFF", false);
		else append_cmt(ea, "DISPLAY_ON", false);

		if (val & mask(1))	append_cmt(ea, "PAUSE_HV_WHEN_EXT_INT", false);
		else append_cmt(ea, "NORMAL_HV_COUNTER", false);

		if (val & mask(2))	append_cmt(ea, "EIGHT_COLORS_MODE", false);
		else append_cmt(ea, "FULL_COLORS_MODE", false);

		if (val & mask(4))	append_cmt(ea, "ENABLE_HBLANK", false);
		else append_cmt(ea, "DISABLE_HBLANK", false);

		return true;
	}
	case 0x8100:
	{
		if (val & mask(2))	append_cmt(ea, "GENESIS_DISPLAY_MODE_BIT2", false);
		else append_cmt(ea, "SMS_DISPLAY_MODE_BIT2", false);

		if (val & mask(3))	append_cmt(ea, "SET_PAL_MODE", false);
		else append_cmt(ea, "SET_NTSC_MODE", false);

		if (val & mask(4))	append_cmt(ea, "ENABLE_DMA", false);
		else append_cmt(ea, "DISABLE_DMA", false);

		if (val & mask(5))	append_cmt(ea, "ENABLE_VBLANK", false);
		else append_cmt(ea, "DISABLE_VBLANK", false);

		if (val & mask(6))	append_cmt(ea, "ENABLE_DISPLAY", false);
		else append_cmt(ea, "DISABLE_DISPLAY", false);

		if (val & mask(7))	append_cmt(ea, "TMS9918_DISPLAY_MODE_BIT7", false);
		else append_cmt(ea, "GENESIS_DISPLAY_MODE_BIT7", false);

		return true;
	}
	case 0x8200:
	{
		addr = (val & mask(3, 3));
		qsnprintf(name, sizeof(name), "SET_PLANE_A_ADDR_$%.4X", addr * 0x400);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x8300:
	{
		addr = (val & mask(3, 3));
		qsnprintf(name, sizeof(name), "SET_WINDOW_PLANE_ADDR_$%.4X", addr * 0x400);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x8400:
	{
		addr = (val & mask(0, 3));
		qsnprintf(name, sizeof(name), "SET_PLANE_B_ADDR_$%.4X", addr * 0x2000);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x8500:
	{
		addr = (val & mask(0, 7));
		qsnprintf(name, sizeof(name), "SET_SPRITE_TBL_ADDR_$%.4X", addr * 0x200);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x8600:
	{
		if (val & mask(5))	append_cmt(ea, "ENABLE_SPRITES_REBASE", false);
		else append_cmt(ea, "DISABLE_SPRITES_REBASE", false);

		return true;
	}
	case 0x8700:
	{
		unsigned int xx = (val & mask(4, 2));
		unsigned int yyyy = (val & mask(0, 4));

		qsnprintf(name, sizeof(name), "SET_BG_AS_%dPAL_%dTH_COLOR", xx + 1, yyyy + 1);
		append_cmt(ea, name, false);

		return true;
	}
	case 0x8A00:
	{
		addr = (val & mask(0, 8));
		qsnprintf(name, sizeof(name), "SET_HBLANK_COUNTER_VALUE_$%.4X", addr);
		append_cmt(ea, name, false);
		return true;
	} break;
	case 0x8B00:
	{
		switch (val & mask(0, 2))
		{
		case 0 /*00*/: append_cmt(ea, "SET_HSCROLL_TYPE_AS_FULLSCREEN", false); break;
		case 1 /*01*/: append_cmt(ea, "SET_HSCROLL_TYPE_AS_LINE_SCROLL", false); break;
		case 2 /*10*/: append_cmt(ea, "SET_HSCROLL_TYPE_AS_CELL_SCROLL", false); break;
		case 3 /*11*/: append_cmt(ea, "SET_HSCROLL_TYPE_AS_LINE__SCROLL", false); break;
		}

		if (val & mask(2))	append_cmt(ea, "_2CELLS_COLUMN_VSCROLL_MODE", false);
		else append_cmt(ea, "FULLSCREEN_VSCROLL_MODE", false);

		if (val & mask(3))	append_cmt(ea, "ENABLE_EXT_INTERRUPT", false);
		else append_cmt(ea, "DISABLE_EXT_INTERRUPT", false);

		return true;
	}
	case 0x8C00:
	{
		switch (val & 0x81)
		{
		case 0 /*0XXXXXX0*/: append_cmt(ea, "SET_40_TILES_WIDTH_MODE", false); break;
		case 0x81 /*1XXXXXX1*/: append_cmt(ea, "SET_32_TILES_WIDTH_MODE", false); break;
		}

		if (val & mask(3)) append_cmt(ea, "ENABLE_SHADOW_HIGHLIGHT_MODE", false);
		else append_cmt(ea, "DISABLE_SHADOW_HIGHLIGHT_MODE", false);

		switch (val & mask(1, 2))
		{
		case 0 /*00*/ << 1: append_cmt(ea, "NO_INTERLACE_MODE", false); break;
		case 1 /*01*/ << 1: append_cmt(ea, "ENABLE_SIMPLE_INTERLACE_MODE", false); break;
		case 3 /*11*/ << 1: append_cmt(ea, "ENABLE_DOUBLE_INTERLACE_MODE", false); break;
		}

		if (val & mask(4)) append_cmt(ea, "ENABLE_EXTERNAL_PIXEL_BUS", false);
		else append_cmt(ea, "DISABLE_EXTERNAL_PIXEL_BUS", false);

		if (val & mask(6)) append_cmt(ea, "DO_PIXEL_CLOCK_INSTEAD_OF_VSYNC", false);
		else append_cmt(ea, "DO_VSYNC_INSTEAD_OF_PIXEL_CLOCK", false);

		return true;
	}
	case 0x8D00:
	{
		addr = (val & mask(0, 6));
		qsnprintf(name, sizeof(name), "SET_HSCROLL_DATA_ADDR_$%.4X", addr * 0x400);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x8E00:
	{
		if (val & mask(0))	append_cmt(ea, "ENABLE_PLANE_A_REBASE", false);
		else append_cmt(ea, "DISABLE_PLANE_A_REBASE", false);

		if (val & mask(4))	append_cmt(ea, "ENABLE_PLANE_B_REBASE", false);
		else append_cmt(ea, "DISABLE_PLANE_B_REBASE", false);

		return true;
	}
	case 0x8F00:
	{
		addr = (val & mask(0, 8));
		qsnprintf(name, sizeof(name), "SET_VDP_AUTO_INC_VALUE_$%.4X", addr);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x9000:
	{
		switch (val & mask(0, 2))
		{
		case 0 /*00*/: append_cmt(ea, "SET_PLANEA_PLANEB_WIDTH_TO_32_TILES", false); break;
		case 1 /*01*/: append_cmt(ea, "SET_PLANEA_PLANEB_WIDTH_TO_64_TILES", false); break;
		case 3 /*11*/: append_cmt(ea, "SET_PLANEA_PLANEB_WIDTH_TO_128_TILES", false); break;
		}

		switch (val & mask(4, 2))
		{
		case 0 /*00*/ << 4: append_cmt(ea, "SET_PLANEA_PLANEB_HEIGHT_TO_32_TILES", false); break;
		case 1 /*01*/ << 4: append_cmt(ea, "SET_PLANEA_PLANEB_HEIGHT_TO_64_TILES", false); break;
		case 3 /*11*/ << 4: append_cmt(ea, "SET_PLANEA_PLANEB_HEIGHT_TO_128_TILES", false); break;
		}

		return true;
	}
	case 0x9100:
	{
		if (val & mask(7)) append_cmt(ea, "MOVE_WINDOW_HORZ_RIGHT", false);
		else append_cmt(ea, "MOVE_WINDOW_HORZ_LEFT", false);

		addr = (val & mask(0, 5));
		qsnprintf(name, sizeof(name), "MOVE_BY_%d_CELLS", addr);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x9200:
	{
		if (val & mask(7)) append_cmt(ea, "MOVE_WINDOW_VERT_RIGHT", false);
		else append_cmt(ea, "MOVE_WINDOW_VERT_LEFT", false);

		addr = (val & mask(0, 5));
		qsnprintf(name, sizeof(name), "MOVE_BY_%d_CELLS", addr);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x9300:
	{
		addr = (val & mask(0, 8));
		qsnprintf(name, sizeof(name), "SET_LOWER_BYTE_OF_DMA_LEN_TO_$%.2X", addr);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x9400:
	{
		addr = (val & mask(0, 8));
		qsnprintf(name, sizeof(name), "SET_HIGHER_BYTE_OF_DMA_LEN_TO_$%.2X", addr);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x9500:
	{
		addr = (val & mask(0, 8));
		qsnprintf(name, sizeof(name), "SET_LOWER_BYTE_OF_DMA_SRC_TO_$%.2X", addr);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x9600:
	{
		addr = (val & mask(0, 8));
		qsnprintf(name, sizeof(name), "SET_MIDDLE_BYTE_OF_DMA_SRC_TO_$%.2X", addr);
		append_cmt(ea, name, false);
		return true;
	}
	case 0x9700:
	{
		addr = (val & mask(0, 6));
		qsnprintf(name, sizeof(name), "SET_HIGH_BYTE_OF_DMA_SRC_TO_$%.2X", addr);
		append_cmt(ea, name, false);

		if (val & mask(7)) append_cmt(ea, "ADD_$800000_TO_DMA_SRC_ADDR", false);
		else append_cmt(ea, "SET_COPY_M68K_TO_VRAM_DMA_MODE", false);

		switch (val & mask(6, 2))
		{
		case 2 /*10*/ << 6: append_cmt(ea, "SET_VRAM_FILL_DMA_MODE", false); break;
		case 3 /*11*/ << 6: append_cmt(ea, "SET_VRAM_COPY_DMA_MODE", false); break;
		}

		return true;
	}
	default:
	{
		msg(wrong_vdp_cmd);
		return false;
	}
	}
}

//--------------------------------------------------------------------------
static void do_cmt_sr_ccr_reg_const(ea_t ea, uval_t val)
{
	if (val & mask(4)) append_cmt(ea, "SET_X", false);
	else append_cmt(ea, "CLR_X", false);

	if (val & mask(3)) append_cmt(ea, "SET_N", false);
	else append_cmt(ea, "CLR_N", false);

	if (val & mask(2)) append_cmt(ea, "SET_Z", false);
	else append_cmt(ea, "CLR_Z", false);

	if (val & mask(1)) append_cmt(ea, "SET_V", false);
	else append_cmt(ea, "CLR_V", false);

	if (val & mask(0)) append_cmt(ea, "SET_C", false);
	else append_cmt(ea, "CLR_C", false);

	if (val & mask(15)) append_cmt(ea, "SET_T1", false);
	else append_cmt(ea, "CLR_T1", false);

	if (val & mask(14)) append_cmt(ea, "SET_T0", false);
	else append_cmt(ea, "CLR_T0", false);

	if (val & mask(13)) append_cmt(ea, "SET_SF", false);
	else append_cmt(ea, "CLR_SF", false);

	if (val & mask(12)) append_cmt(ea, "SET_MF", false);
	else append_cmt(ea, "CLR_MF", false);

	switch ((val & mask(8, 3)))
	{
	case 0x7 /*111*/ << 8: append_cmt(ea, "DISABLE_ALL_INTERRUPTS", false); break;
	case 0x6 /*110*/ << 8: append_cmt(ea, "ENABLE_NO_INTERRUPTS", false); break;

	case 0x5 /*101*/ << 8: append_cmt(ea, "DISABLE_ALL_INTERRUPTS_EXCEPT_VBLANK", false); break;
	case 0x4 /*100*/ << 8: append_cmt(ea, "ENABLE_ONLY_VBLANK_INTERRUPT", false); break;

	case 0x3 /*011*/ << 8: append_cmt(ea, "DISABLE_ALL_INTERRUPTS_EXCEPT_VBLANK_HBLANK", false); break;
	case 0x2 /*010*/ << 8: append_cmt(ea, "ENABLE_ONLY_VBLANK_HBLANK_INTERRUPTS", false); break;

	case 0x1 /*001*/ << 8: append_cmt(ea, "DISABLE_NO_INTERRUPTS", false); break;
	case 0x0 /*000*/ << 8: append_cmt(ea, "ENABLE_ALL_INTERRUPTS", false); break;
	}
}

//--------------------------------------------------------------------------
static void do_cmt_vdp_rw_command(ea_t ea, uval_t val)
{
	char name[250];

	if (val & 0xFFFF0000) // command was sended by one dword
	{
		unsigned int addr = ((val & mask(0, 2)) << 14) | ((val & mask(16, 14)) >> 16);

		switch ((val >> 24) & mask(6))
		{
		case 0 << 6: // read operation
		{
			switch (val & ((1 << 31) | (1 << 5) | (1 << 4)))
			{
			case ((0 << 31) | (0 << 5) | (0 << 4)) /*000*/ : // VRAM
			{
				qsnprintf(name, sizeof(name), "DO_READ_VRAM_FROM_$%.4X", addr);
				append_cmt(ea, name, false);
			} break;
			case ((0 << 31) | (0 << 5) | (1 << 4)) /*001*/ : // VSRAM
			{
				qsnprintf(name, sizeof(name), "DO_READ_VSRAM_FROM_$%.4X", addr);
				append_cmt(ea, name, false);
			} break;
			case ((0 << 31) | (1 << 5) | (0 << 4)) /*010*/ : // CRAM
			{
				qsnprintf(name, sizeof(name), "DO_READ_CRAM_FROM_$%.4X", addr);
				append_cmt(ea, name, false);
			} break;
			default:
			{
				msg(wrong_vdp_cmd);
			} break;
			}
		} break;
		case 1 << 6: // write operation
		{
			switch (val & ((1 << 31) | (1 << 5) | (1 << 4)))
			{
			case ((0 << 31) | (0 << 5) | (0 << 4)) /*000*/ : // VRAM
			{
				qsnprintf(name, sizeof(name), "DO_WRITE_TO_VRAM_AT_$%.4X_ADDR", addr);
				append_cmt(ea, name, false);
			} break;
			case ((0 << 31) | (0 << 5) | (1 << 4)) /*001*/ : // VSRAM
			{
				qsnprintf(name, sizeof(name), "DO_WRITE_TO_VSRAM_AT_$%.4X_ADDR", addr);
				append_cmt(ea, name, false);
			} break;
			case ((1 << 31) | (0 << 5) | (0 << 4)) /*100*/ : // CRAM
			{
				qsnprintf(name, sizeof(name), "DO_WRITE_TO_CRAM_AT_$%.4X_ADDR", addr);
				append_cmt(ea, name, false);
			} break;
			default:
			{
				msg(wrong_vdp_cmd);
			} break;
			}
		} break;
		default:
		{
			msg(wrong_vdp_cmd);
		} break;
		}
	}
	else // command was sended by halfs (this is high word of it)
	{
		switch ((val >> 8) & mask(6, 2))
		{
		case 0 /*00*/ << 6: append_cmt(ea, "VRAM_OR_VSRAM_OR_CRAM_READ_MODE", false); break;
		case 1 /*01*/ << 6: append_cmt(ea, "VRAM_OR_VSRAM_WRITE_MODE", false); break;
		case 3 /*11*/ << 6: append_cmt(ea, "CRAM_WRITE_MODE", false); break;
		}
	}

	if (val & mask(6)) append_cmt(ea, "VRAM_COPY_DMA_MODE", false);

	if (val & mask(7)) append_cmt(ea, "DO_OPERATION_USING_DMA", false);
	else append_cmt(ea, "DO_OPERATION_WITHOUT_DMA", false);
}

//--------------------------------------------------------------------------
void idaapi run(int /*arg*/)
{
	char name[250];
	ea_t ea = get_screen_ea();
	if (isEnabled(ea)) // address belongs to disassembly
	{
		if (get_cmt(ea, false, name, sizeof(name)) != -1) // remove previous comment and exit
		{
			set_cmt(ea, "", false);
			return;
		}

		ua_ana0(ea); // deprecated, but should be used because of old IDAs (new decode_insn)
		ua_outop(ea, name, sizeof(name), 1); // deprecated, but should be used because of old IDAs (new ua_outop2)
		tag_remove(name, name, sizeof(name));

		uval_t value = 0;
		get_operand_immvals(ea, 0, &value);
		if (cmd.Operands[0].type == o_imm && cmd.Operands[1].type == o_reg && !qstrcmp(name, "sr"))
		{
			do_cmt_sr_ccr_reg_const(ea, value);
		}
		else if (is_vdp_rw_cmd(value))
		{
			do_cmt_vdp_rw_command(ea, value);
		}
		else if (is_vdp_send_cmd(value)) // comment set vdp reg cmd
		{
			do_cmt_vdp_reg_const(ea, value);
			do_cmt_vdp_reg_const(ea, value >> 16);
			return;
		}
	}
}

//--------------------------------------------------------------------------
static const char comment[] = "Identify SMD constant";
static const char help[] = "Identify SMD constant\n";

//--------------------------------------------------------------------------
// This is the preferred name of the plugin module in the menu system
// The preferred name may be overriden in plugins.cfg file

static const char wanted_name[] = "Identify SMD constant";

// This is the preferred hotkey for the plugin module
// The preferred hotkey may be overriden in plugins.cfg file
// Note: IDA won't tell you if the hotkey is not correct
//       It will just disable the hotkey.

static const char wanted_hotkey[] = "J";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	0,                    // plugin flags
	init,                 // initialize

	term,                 // terminate. this pointer may be NULL.

	run,                  // invoke plugin

	comment,              // long comment about the plugin
	// it could appear in the status line
	// or as a hint

	help,                 // multiline help about the plugin

	wanted_name,          // the preferred short name of the plugin
	wanted_hotkey         // the preferred hotkey to run the plugin
};
