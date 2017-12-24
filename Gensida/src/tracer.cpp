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

extern bool hook_trace;

#ifdef __cplusplus
extern "C" {
#endif
    extern uint32 hook_address;
    extern uint32 hook_value;
    extern uint32 hook_pc;

    unsigned int dma_src, dma_len;

    void trace_read_byte();
    void trace_read_word();
    void trace_read_dword();
    void trace_write_byte();
    void trace_write_word();
    void trace_write_dword();

    void trace_write_vram_byte();
    void trace_write_vram_word();
    void trace_read_vram_byte();
    void trace_read_vram_word();

    void hook_dma();

    void trace_exec_pc();
#ifdef __cplusplus
};
#endif

extern UINT16 allow0_breaks;
extern UINT32 allow1_breaks;

void trace_exec_pc()
{
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
    if (allow0_breaks)
    {
        for (unsigned char i = 0; i < 16; ++i)
        {
            if (!CHECK_BIT(allow0_breaks, i)) continue;
            M68kDW.TraceRegValue(15 - i, main68k_context.dreg[15 - i], false); // Check for M68k register breakpoints
        }
    }
    if (allow1_breaks)
    {
        for (unsigned char i = 0; i < 24; ++i)
        {
            if (!CHECK_BIT(allow1_breaks, i)) continue;
            M68kDW.TraceRegValue(16 + (23 - i), VDP_Reg.regs[23 - i], true); // Check for VDP register breakpoints
        }
    }
#undef CHECK_BIT

    CallRegisteredLuaMemHook(hook_pc, 2, 0, LUAMEMHOOK_EXEC);
    M68kDW.TracePC(hook_pc);
}

void trace_read_byte()
{
    CallRegisteredLuaMemHook(hook_address, 1, hook_value, LUAMEMHOOK_READ);
    M68kDW.TraceRead(hook_address, hook_address, false);
}

void trace_read_word()
{
    CallRegisteredLuaMemHook(hook_address, 2, hook_value, LUAMEMHOOK_READ);
    M68kDW.TraceRead(hook_address, hook_address + 1, false);
}

void trace_read_dword()
{
    CallRegisteredLuaMemHook(hook_address, 4, hook_value, LUAMEMHOOK_READ);
    M68kDW.TraceRead(hook_address, hook_address + 3, false);
}

void trace_write_byte()
{
    CallRegisteredLuaMemHook(hook_address, 1, hook_value, LUAMEMHOOK_WRITE);
    M68kDW.TraceWrite(hook_address, hook_address, false);
}

void trace_write_word()
{
    CallRegisteredLuaMemHook(hook_address, 2, hook_value, LUAMEMHOOK_WRITE);
    M68kDW.TraceWrite(hook_address, hook_address + 1, false);
}

void trace_write_dword()
{
    CallRegisteredLuaMemHook(hook_address, 4, hook_value, LUAMEMHOOK_WRITE);
    M68kDW.TraceWrite(hook_address, hook_address + 3, false);
}

static void hook_dma_internal()
{
    unsigned int start, stop;

    // VDP area
    hook_value &= 3;

    // Memory breakpoints
    start = VDP_Reg.DMA_Address << 1;
    stop = start + (VDP_Reg.DMA_Length << 1) - 1;

    M68kDW.TraceRead(start, stop, false);

    /**************************************************/
    /**************************************************/

    unsigned int start_l, stop_l;

    start = Ctrl.Address;
    stop = start + (VDP_Reg.DMA_Length << 1) - 1;

    // local linear
    if (hook_value <= 1)
    {
        // VRAM
        start_l = start + 0x00000;
        stop_l = stop + 0x00000;
    }
    else if (hook_value == 2)
    {
        // CRAM
        start_l = start + 0x10000;
        stop_l = stop + 0x10000;
    }
    else if (hook_value == 3)
    {
        // VSRAM
        start_l = start + 0x20000;
        stop_l = stop + 0x20000;
    }
    else
    {
        // Error
        return;
    }

    M68kDW.TraceWrite(start_l, stop_l, true);
}
void hook_dma()
{
    if (hook_trace)
        hook_dma_internal();
}

static void trace_write_vram_byte_internal()
{
    unsigned int start, stop;
    unsigned int start_l, stop_l;

    hook_pc &= 0x00ffffff;
    hook_address &= 0x00ffffff;

    start = Ctrl.Address;
    stop = start + 0;

    // local linear
    if (Ctrl.Access == 9)
    {
        // VRAM
        start_l = start + 0x00000;
        stop_l = stop + 0x00000;
    }
    else if (Ctrl.Access == 10)
    {
        // CRAM
        start_l = start + 0x10000;
        stop_l = stop + 0x10000;
    }
    else if (Ctrl.Access == 11)
    {
        // VSRAM
        start_l = start + 0x20000;
        stop_l = stop + 0x20000;
    }
    else
    {
        // Error
        return;
    }

    M68kDW.TraceWrite(start_l, stop_l, true);
}
void trace_write_vram_byte()
{
    if (hook_trace)
        trace_write_vram_byte_internal();
}

static void trace_write_vram_word_internal()
{
    unsigned int start, stop;
    unsigned int start_l, stop_l;

    hook_pc &= 0x00ffffff;
    hook_address &= 0x00ffffff;

    start = Ctrl.Address;
    stop = start + 1;

    // local linear
    if (Ctrl.Access == 9)
    {
        // VRAM
        start_l = start + 0x00000;
        stop_l = stop + 0x00000;
    }
    else if (Ctrl.Access == 10)
    {
        // CRAM
        start_l = start + 0x10000;
        stop_l = stop + 0x10000;
    }
    else if (Ctrl.Access == 11)
    {
        // VSRAM
        start_l = start + 0x20000;
        stop_l = stop + 0x20000;
    }
    else
    {
        // Error
        return;
    }

    M68kDW.TraceWrite(start_l, stop_l, true);
}
void trace_write_vram_word()
{
    if (hook_trace)
        trace_write_vram_word_internal();
}

static void trace_read_vram_byte_internal()
{
    unsigned int start, stop;
    unsigned int start_l, stop_l;

    hook_pc &= 0x00ffffff;
    hook_address &= 0x00ffffff;

    start = Ctrl.Address;
    stop = start + 0;

    // local linear
    if (Ctrl.Access == 5)
    {
        // VRAM
        start_l = start + 0x00000;
        stop_l = stop + 0x00000;
    }
    else if (Ctrl.Access == 6)
    {
        // CRAM
        start_l = start + 0x10000;
        stop_l = stop + 0x10000;
    }
    else if (Ctrl.Access == 7)
    {
        // VSRAM
        start_l = start + 0x20000;
        stop_l = stop + 0x20000;
    }
    else
    {
        // Error
        return;
    }

    M68kDW.TraceRead(start_l, stop_l, true);
}
void trace_read_vram_byte()
{
    if (hook_trace)
        trace_read_vram_byte_internal();
}

static void trace_read_vram_word_internal()
{
    unsigned int start, stop;
    unsigned int start_l, stop_l;

    hook_pc &= 0x00ffffff;
    hook_address &= 0x00ffffff;

    start = Ctrl.Address;
    stop = start + 1;

    // local linear
    if (Ctrl.Access == 5)
    {
        // VRAM
        start_l = start + 0x00000;
        stop_l = stop + 0x00000;
    }
    else if (Ctrl.Access == 6)
    {
        // CRAM
        start_l = start + 0x10000;
        stop_l = stop + 0x10000;
    }
    else if (Ctrl.Access == 7)
    {
        // VSRAM
        start_l = start + 0x20000;
        stop_l = stop + 0x20000;
    }
    else
    {
        // Error
        return;
    }

    M68kDW.TraceRead(start_l, stop_l, true);
}
void trace_read_vram_word()
{
    if (hook_trace)
        trace_read_vram_word_internal();
}