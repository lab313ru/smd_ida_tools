#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H
#include <windows.h>
#include <vector>
#include <string>

/*#define WM_DEBUG_DUMMY_EXIT (WM_USER+1000)

#define BRK_PC      0x000001
#define BRK_READ    0x000002
#define BRK_WRITE   0x000004
#define BRK_VDP     0x000010
#define BRK_FORBID  0x000100*/

typedef unsigned int uint32;
typedef unsigned short ushort;

enum class bp_type
{
    BP_PC = 1,
    BP_READ,
    BP_WRITE,
};

struct Breakpoint
{
    bp_type type;

    uint32 start;
    uint32 end;

    bool enabled;
    bool is_vdp, is_forbid;

    Breakpoint(bp_type _type, uint32 _start, uint32 _end, bool _enabled, bool _is_vdp, bool _is_forbid) :
        type(_type), start(_start), end(_end), enabled(_enabled), is_vdp(_is_vdp), is_forbid(_is_forbid) {};
};

typedef std::vector<Breakpoint> bp_list;

struct DebugWindow
{
    DebugWindow();
    std::vector<uint32> callstack;
    bp_list Breakpoints;

    bool DebugStop;

    bool StepInto;
    uint32 StepOver;

    void Breakpoint(int pc);
    void SetWhyBreak(LPCSTR lpString);

    bool BreakPC(int pc);
    bool BreakRegValue(int pc, unsigned char reg_idx, uint32 value, bool is_vdp);
    bool BreakRead(int pc, uint32 start, uint32 stop, bool is_vdp);
    bool BreakWrite(int pc, uint32 start, uint32 stop, bool is_vdp);

    virtual void DoStepOver();
    virtual void TracePC(int pc);
    virtual void TraceRead(uint32 start, uint32 stop);
    virtual void TraceWrite(uint32 start, uint32 stop);
    virtual ~DebugWindow();
};

#endif
