#include <stdio.h>
#include <memory.h>
#include <vector>

#include "cpu_68k.h"
#include "m68k_debugwindow.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "vdp_io.h"
#include "luascript.h"

#include "tracer.h"

extern bool trace_map;
extern bool hook_trace;

extern "C" {
    extern uint32 hook_address_cd;
    extern uint32 hook_value_cd;
    extern uint32 hook_pc_cd;

    void trace_read_byte_cd();
    void trace_read_word_cd();
    void trace_read_dword_cd();
    void trace_write_byte_cd();
    void trace_write_word_cd();
    void trace_write_dword_cd();
};

extern char *mapped_cd;
uint32 Current_PC_cd;
int Debug_CD = 2;

unsigned short Next_Word_T_cd(void)
{
    unsigned short val;

    if (Debug_CD == 1) val = M68K_RW(Current_PC_cd);
    else if (Debug_CD >= 2) val = S68K_RW(Current_PC_cd);

    Current_PC_cd += 2;

    return(val);
}

unsigned int Next_Long_T_cd(void)
{
    unsigned int val;

    if (Debug_CD == 1)
    {
        val = M68K_RW(Current_PC_cd);
        val <<= 16;
        val |= M68K_RW(Current_PC_cd + 2);
    }
    else if (Debug_CD >= 2)
    {
        val = S68K_RW(Current_PC_cd);
        val <<= 16;
        val |= S68K_RW(Current_PC_cd + 2);
    }

    Current_PC_cd += 4;

    return(val);
}

void trace_exec_pc_cd()
{
    CallRegisteredLuaMemHook(hook_pc_cd, 2, 0, LUAMEMHOOK_EXEC_SUB);
}

void trace_read_byte_cd()
{
    CallRegisteredLuaMemHook(hook_address_cd, 1, hook_value_cd, LUAMEMHOOK_READ_SUB);
}

void trace_read_word_cd()
{
    CallRegisteredLuaMemHook(hook_address_cd, 2, hook_value_cd, LUAMEMHOOK_READ_SUB);
}

void trace_read_dword_cd()
{
    CallRegisteredLuaMemHook(hook_address_cd, 4, hook_value_cd, LUAMEMHOOK_READ_SUB);
}

void trace_write_byte_cd()
{
    CallRegisteredLuaMemHook(hook_address_cd, 1, hook_value_cd, LUAMEMHOOK_WRITE_SUB);
}

void trace_write_word_cd()
{
    CallRegisteredLuaMemHook(hook_address_cd, 2, hook_value_cd, LUAMEMHOOK_WRITE_SUB);
}

void trace_write_dword_cd()
{
    CallRegisteredLuaMemHook(hook_address_cd, 4, hook_value_cd, LUAMEMHOOK_WRITE_SUB);
}