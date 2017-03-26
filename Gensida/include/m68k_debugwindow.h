#ifndef M68K_DEBUG_WINDOW_H
#define M68K_DEBUG_WINDOW_H

#include "debugwindow.h"

struct M68kDebugWindow :DebugWindow
{
    M68kDebugWindow();

    uint32 last_pc;

    void DoStepOver();
    void TracePC(int pc);
    void TraceRegValue(unsigned char reg_idx, uint32 value, bool is_vdp);
    void TraceRead(uint32 start, uint32 stop, bool is_vdp);
    void TraceWrite(uint32 start, uint32 stop, bool is_vdp);
    ~M68kDebugWindow();
};

extern M68kDebugWindow M68kDW;

#endif
