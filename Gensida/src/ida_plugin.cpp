// Copyright (C) 2015 Dr. MefistO
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http ://www.gnu.org/licenses/

#include <Windows.h>

#include <ida.hpp>
#include <dbg.hpp>
#include <idd.hpp>
#include <loader.hpp>

#include "ida_plugin.h"

#include "m68k_debugwindow.h"
#include "resource.h"

#include "ida_debmod.h"
#include "ida_registers.h"

extern debugger_t debugger;

static bool plugin_inited;
static bool dbg_started;

static int idaapi hook_dbg(void *user_data, int notification_code, va_list va)
{
	switch (notification_code)
	{
	case dbg_notification_t::dbg_process_start:
		dbg_started = true;
		break;

	case dbg_notification_t::dbg_process_exit:
		dbg_started = false;
		break;
	}
	return 0;
}

static int idaapi idp_to_dbg_reg(int idp_reg)
{
	int reg_idx = idp_reg;
	if (idp_reg >= 0 && idp_reg <= 7)
		reg_idx = R_D0 + idp_reg;
	else if (idp_reg >= 8 && idp_reg <= 39)
		reg_idx = R_A0 + (idp_reg % 8);
	else if (idp_reg == 91)
		reg_idx = R_PC;
	else if (idp_reg == 92 || idp_reg == 93)
		reg_idx = R_SR;
	else if (idp_reg == 94)
		reg_idx = R_A7;
	else
	{
		char buf[MAXSTR];
		qsnprintf(buf, MAXSTR, "reg: %d\n", idp_reg);
		warning("SEND THIS MESSAGE TO meffi@lab313.ru:\n%s\n", buf);
		return 0;
	}
	return reg_idx;
}

static int idaapi hook_ui(void *user_data, int notification_code, va_list va)
{
	switch (notification_code)
	{
	case ui_notification_t::ui_get_custom_viewer_hint:
	{
		TCustomControl *viewer = va_arg(va, TCustomControl *);
		place_t *place = va_arg(va, place_t *);
		int *important_lines = va_arg(va, int *);
		qstring &hint = *va_arg(va, qstring *);

		if (place == NULL)
			return 0;

		int x, y;
		if (get_custom_viewer_place(viewer, true, &x, &y) == NULL)
			return 0;

		char buf[MAXSTR];
		const char *line = get_custom_viewer_curline(viewer, true);
		tag_remove(line, buf, sizeof(buf));
		if (x >= (int)strlen(buf))
			return 0;

		idaplace_t &pl = *(idaplace_t *)place;
		if (decode_insn(pl.ea) && dbg_started)
		{
			insn_t _cmd = cmd;

			int flags = calc_default_idaplace_flags();
			linearray_t ln(&flags);

			for (int i = 0; i < qnumber(_cmd.Operands); i++)
			{
				op_t op = _cmd.Operands[i];

				if (op.type != o_void)
				{
					switch (op.type)
					{
					case o_mem:
					case o_near:
					{
						idaplace_t here;
						here.ea = op.addr;
						here.lnnum = 0;

						ln.set_place(&here);

						hint.cat_sprnt((COLSTR(SCOLOR_INV"OPERAND#%d (ADDRESS: $%a)\n", SCOLOR_DREF)), op.n, op.addr);
						(*important_lines)++;

						int n = qmin(ln.get_linecnt(), 10);		   // how many lines for this address?
						(*important_lines) += n;
						for (int j = 0; j < n; ++j)
						{
							hint.cat_sprnt("%s\n", ln.down());
						}
					} break;
					case o_phrase:
					case o_reg:
					{
						regval_t reg;
						int reg_idx = idp_to_dbg_reg(op.reg);

						const char *reg_name = dbg->registers(reg_idx).name;
						if (get_reg_val(reg_name, &reg))
						{
							idaplace_t here;
							here.ea = (uint32)reg.ival;
							here.lnnum = 0;

							ln.set_place(&here);

							hint.cat_sprnt((COLSTR(SCOLOR_INV"OPERAND#%d (REGISTER: %s)\n", SCOLOR_DREF)), op.n, reg_name);
							(*important_lines)++;

							int n = qmin(ln.get_linecnt(), 10);		   // how many lines for this address?
							(*important_lines) += n;
							for (int j = 0; j < n; ++j)
							{
								hint.cat_sprnt("%s\n", ln.down());
							}
						}
					} break;
					case o_displ:
					{
						regval_t main_reg, add_reg;
						int main_reg_idx = idp_to_dbg_reg(op.reg);
						int add_reg_idx = idp_to_dbg_reg(op.specflag1 & 0xF);

						main_reg.ival = 0;
						add_reg.ival = 0;
						if (op.specflag2 & 0x10)
						{
							get_reg_val(dbg->registers(add_reg_idx).name, &add_reg);
							if (op.specflag1 & 0x10)
								add_reg.ival &= 0xFFFF;
						}

						if (main_reg_idx != R_PC)
							get_reg_val(dbg->registers(main_reg_idx).name, &main_reg);

						idaplace_t here;
						ea_t addr = (uint32)main_reg.ival + op.addr + (uint32)add_reg.ival; // TODO: displacements with PC and other regs unk_123(pc, d0.l); unk_111(d0, d2.w)
						here.ea = addr;
						here.lnnum = 0;

						ln.set_place(&here);

						hint.cat_sprnt((COLSTR(SCOLOR_INV"OPERAND#%d (DISPLACEMENT: [$%s%X($%X", SCOLOR_DREF)),
							op.n,
							((int)op.addr < 0) ? "-" : "", ((int)op.addr < 0) ? -(int)op.addr : op.addr,
							(uint32)main_reg.ival
							);

						if (op.specflag2 & 0x10)
							hint.cat_sprnt((COLSTR(",$%X", SCOLOR_DREF)), (uint32)add_reg.ival);

						hint.cat_sprnt((COLSTR(")])\n", SCOLOR_DREF)));

						(*important_lines)++;

						int n = qmin(ln.get_linecnt(), 10);		   // how many lines for this address?
						(*important_lines) += n;
						for (int j = 0; j < n; ++j)
						{
							hint.cat_sprnt("%s\n", ln.down());
						}
					} break;
					}
				}
			}

			return 1;
		}
	}
	default:
		return 0;
	}
}

//--------------------------------------------------------------------------
static void print_version()
{
	static const char format[] = NAME " debugger plugin v%s;\nAuthor: Dr. MefistO [Lab 313] <meffi@lab313.ru>.";
	info(format, VERSION);
	msg(format, VERSION);
}

//--------------------------------------------------------------------------
// Initialize debugger plugin
static bool init_plugin(void)
{
	if (ph.id != PLFM_68K)
		return false;

	return true;
}

//--------------------------------------------------------------------------
// Initialize debugger plugin
static int idaapi init(void)
{
	if (init_plugin())
	{
		dbg = &debugger;
		plugin_inited = true;
		dbg_started = false;
		hook_to_notification_point(HT_UI, hook_ui, NULL);
		hook_to_notification_point(HT_DBG, hook_dbg, NULL);

		print_version();
		return PLUGIN_KEEP;
	}
	return PLUGIN_SKIP;
}

//--------------------------------------------------------------------------
// Terminate debugger plugin
static void idaapi term(void)
{
	if (plugin_inited)
	{
		//term_plugin();
		unhook_from_notification_point(HT_UI, hook_ui, NULL);
		unhook_from_notification_point(HT_DBG, hook_dbg, NULL);
		plugin_inited = false;
		dbg_started = false;
	}
}

//--------------------------------------------------------------------------
// The plugin method - usually is not used for debugger plugins
static void idaapi run(int /*arg*/)
{
}

//--------------------------------------------------------------------------
char comment[] = NAME " debugger plugin by Dr. MefistO.";

char help[] =
NAME " debugger plugin by Dr. MefistO.\n"
"\n"
"This module lets you debug Genesis roms in IDA.\n";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	PLUGIN_PROC | PLUGIN_HIDE | PLUGIN_DBG | PLUGIN_MOD, // plugin flags
	init, // initialize

	term, // terminate. this pointer may be NULL.

	run, // invoke plugin

	comment, // long comment about the plugin
	// it could appear in the status line
	// or as a hint

	help, // multiline help about the plugin

	NAME " debugger plugin", // the preferred short name of the plugin

	"" // the preferred hotkey to run the plugin
};
