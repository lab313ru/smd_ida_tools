// A few notes about this implementation of a RAM dump window:
//
// Speed of update was one of the highest priories.
// This is because I wanted the RAM search window to be able to
// update every single value in RAM every single frame,
// without causing the emulation to run noticeably slower than normal.
//

#include "resource.h"
#include "gens.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "misc.h"
#include "mem_z80.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "save.h"
#include "ram_search.h"
#include "vdp_ram.h"
#include "g_main.h"
#include <assert.h>
#include <commctrl.h>
#include "g_dsound.h"
#include "ramwatch.h"
#include "luascript.h"
#include <list>
#include <vector>
#include "ida_debug.h"
#include <sstream>
#include <iomanip>
#include <Windowsx.h>

int VDPRamPal, VDPRamTile;
bool IsVRAM;

#define VDP_PAL_COUNT 4
#define VDP_PAL_COLORS 16
#define VDP_TILES_IN_ROW 16
#define VDP_TILES_IN_COL 24
#define VDP_TILE_W 8
#define VDP_TILE_H 8
#define VDP_TILE_ZOOM 2
#define VDP_BLOCK_W (VDP_TILE_W * VDP_TILE_ZOOM)
#define VDP_BLOCK_H (VDP_TILE_H * VDP_TILE_ZOOM)
#define VDP_SCROLL_MAX (sizeof(VRam) / (VDP_TILES_IN_ROW * 0x20) - VDP_TILES_IN_COL)

struct TabInfo
{
    TabInfo(const std::string& atabName, int adialogID, DLGPROC adialogProc)
        :tabName(atabName), dialogID(adialogID), dialogProc(adialogProc), hwndDialog(NULL)
    {}

    std::string tabName;
    int dialogID;
    DLGPROC dialogProc;
    HWND hwndDialog;
};

std::string previousText;
unsigned int currentControlFocus;
HWND activeTabWindow;
std::vector<TabInfo> tabItems;

void WndProcDialogImplementSaveFieldWhenLostFocus(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        //Make sure no textbox is selected on startup, and remove focus from textboxes when
        //the user clicks an unused area of the window.
    case WM_LBUTTONDOWN:
    case WM_SHOWWINDOW:
        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(0, EN_SETFOCUS), NULL);
        SetFocus(NULL);
        break;
    }
}

//----------------------------------------------------------------------------------------
std::string GetDlgItemString(HWND hwnd, int controlID)
{
    std::string result;

    const unsigned int maxTextLength = 1024;
    char currentTextTemp[maxTextLength];
    if (GetDlgItemText(hwnd, controlID, currentTextTemp, maxTextLength) == 0)
    {
        currentTextTemp[0] = '\0';
    }
    result = currentTextTemp;

    return result;
}

//----------------------------------------------------------------------------------------
unsigned int GetDlgItemHex(HWND hwnd, int controlID)
{
    unsigned int value = 0;

    const unsigned int maxTextLength = 1024;
    char currentTextTemp[maxTextLength];
    if (GetDlgItemText(hwnd, controlID, currentTextTemp, maxTextLength) == 0)
    {
        currentTextTemp[0] = '\0';
    }
    std::stringstream buffer;
    buffer << std::hex << currentTextTemp;
    buffer >> value;

    return value;
}

//----------------------------------------------------------------------------------------
void UpdateDlgItemHex(HWND hwnd, int controlID, unsigned int width, unsigned int data)
{
    const unsigned int maxTextLength = 1024;
    char currentTextTemp[maxTextLength];
    if (GetDlgItemText(hwnd, controlID, currentTextTemp, maxTextLength) == 0)
    {
        currentTextTemp[0] = '\0';
    }
    std::string currentText = currentTextTemp;
    std::stringstream text;
    text << std::setw(width) << std::setfill('0') << std::hex << std::uppercase;
    text << data;
    if (text.str() != currentText)
    {
        SetDlgItemText(hwnd, controlID, text.str().c_str());
    }
}

//----------------------------------------------------------------------------------------
void UpdateDlgItemBin(HWND hwnd, int controlID, unsigned int data)
{
    const unsigned int maxTextLength = 1024;
    char currentTextTemp[maxTextLength];
    if (GetDlgItemText(hwnd, controlID, currentTextTemp, maxTextLength) == 0)
    {
        currentTextTemp[0] = '\0';
    }
    std::string currentText = currentTextTemp;
    std::stringstream text;
    text << data;
    if (text.str() != currentText)
    {
        SetDlgItemText(hwnd, controlID, text.str().c_str());
    }
}

//----------------------------------------------------------------------------------------
void msgModeRegistersUPDATE(HWND hwnd)
{
    //Mode registers
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_VSI, (VDP_Reg.Set1 & mask(7)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_HSI, (VDP_Reg.Set1 & mask(6)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_LCB, (VDP_Reg.Set1 & mask(5)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_IE1, (VDP_Reg.Set1 & mask(4)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_SS, (VDP_Reg.Set1 & mask(3)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_PS, (VDP_Reg.Set1 & mask(2)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_M2, (VDP_Reg.Set1 & mask(1)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_ES, (VDP_Reg.Set1 & mask(0)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_EVRAM, (VDP_Reg.Set2 & mask(7)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_DISP, (VDP_Reg.Set2 & mask(6)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_IE0, (VDP_Reg.Set2 & mask(5)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_M1, (VDP_Reg.Set2 & mask(4)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_M3, (VDP_Reg.Set2 & mask(3)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_M5, (VDP_Reg.Set2 & mask(2)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_SZ, (VDP_Reg.Set2 & mask(1)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_MAG, (VDP_Reg.Set2 & mask(0)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_0B7, (VDP_Reg.Set3 & mask(7)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_0B6, (VDP_Reg.Set3 & mask(6)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_0B5, (VDP_Reg.Set3 & mask(5)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_0B4, (VDP_Reg.Set3 & mask(4)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_IE2, (VDP_Reg.Set3 & mask(3)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_VSCR, (VDP_Reg.Set3 & mask(2)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_HSCR, (VDP_Reg.Set3 & mask(1)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_LSCR, (VDP_Reg.Set3 & mask(0)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_RS0, (VDP_Reg.Set4 & mask(7)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_U1, (VDP_Reg.Set4 & mask(6)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_U2, (VDP_Reg.Set4 & mask(5)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_U3, (VDP_Reg.Set4 & mask(4)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_STE, (VDP_Reg.Set4 & mask(3)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_LSM1, (VDP_Reg.Set4 & mask(2)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_LSM0, (VDP_Reg.Set4 & mask(1)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_RS1, (VDP_Reg.Set4 & mask(0)) ? BST_CHECKED : BST_UNCHECKED);
}

#define GET_BITS(number, n, c) ((number & mask(n, c)) >> n)
//----------------------------------------------------------------------------------------
void msgOtherRegistersUPDATE(HWND hwnd)
{
    unsigned int value = 0;
    bool mode4Enabled = !(VDP_Reg.Set2 & mask(2));
    int extendedVRAMModeEnabled = (VDP_Reg.Set2 & mask(7));
    int h40ModeActive = (VDP_Reg.Set4 & mask(0));

    //Other registers
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_077, (VDP_Reg.BG_Color & mask(7)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_076, (VDP_Reg.BG_Color & mask(6)) ? BST_CHECKED : BST_UNCHECKED);
    if (currentControlFocus != IDC_VDP_REGISTERS_BACKGROUNDPALETTEROW)
    {
        value = GET_BITS(VDP_Reg.BG_Color, 4, 2);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_BACKGROUNDPALETTEROW, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_BACKGROUNDPALETTECOLUMN)
    {
        value = GET_BITS(VDP_Reg.BG_Color, 0, 4);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_BACKGROUNDPALETTECOLUMN, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_BACKGROUNDSCROLLX)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_BACKGROUNDSCROLLX, 2, VDP_Reg.Reg8);
    if (currentControlFocus != IDC_VDP_REGISTERS_BACKGROUNDSCROLLY)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_BACKGROUNDSCROLLY, 2, VDP_Reg.Reg9);
    if (currentControlFocus != IDC_VDP_REGISTERS_HINTLINECOUNTER)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_HINTLINECOUNTER, 2, VDP_Reg.H_Int);
    if (currentControlFocus != IDC_VDP_REGISTERS_AUTOINCREMENT)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_AUTOINCREMENT, 2, VDP_Reg.Auto_Inc);
    if (currentControlFocus != IDC_VDP_REGISTERS_SCROLLABASE)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SCROLLABASE, 2, VDP_Reg.Pat_ScrA_Adr);
    if (currentControlFocus != IDC_VDP_REGISTERS_SCROLLABASE_E)
    {
        value = GET_BITS(VDP_Reg.Pat_ScrA_Adr, 3, (extendedVRAMModeEnabled) ? 4 : 3) << 13;
        if (mode4Enabled)
        {
            value = GET_BITS(VDP_Reg.Pat_ScrA_Adr, 1, 3) << 11;
        }

        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SCROLLABASE_E, 5, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_WINDOWBASE)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_WINDOWBASE, 2, VDP_Reg.Pat_Win_Adr);
    if (currentControlFocus != IDC_VDP_REGISTERS_WINDOWBASE_E)
    {
        value = GET_BITS(VDP_Reg.Pat_Win_Adr, 1, (extendedVRAMModeEnabled) ? 6 : 5) << 11;
        if (h40ModeActive)
        {
            value = GET_BITS(VDP_Reg.Pat_Win_Adr, 2, (extendedVRAMModeEnabled) ? 5 : 4) << 12;
        }
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_WINDOWBASE_E, 5, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_SCROLLBBASE)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SCROLLBBASE, 2, VDP_Reg.Pat_ScrB_Adr);
    if (currentControlFocus != IDC_VDP_REGISTERS_SCROLLBBASE_E)
    {
        value = GET_BITS(VDP_Reg.Pat_ScrB_Adr, 0, (extendedVRAMModeEnabled) ? 4 : 3) << 13;
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SCROLLBBASE_E, 5, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_SPRITEBASE)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SPRITEBASE, 2, VDP_Reg.Spr_Att_Adr);
    if (currentControlFocus != IDC_VDP_REGISTERS_SPRITEBASE_E)
    {
        value = GET_BITS(VDP_Reg.Spr_Att_Adr, 0, (extendedVRAMModeEnabled) ? 8 : 7) << 9;

        if (mode4Enabled)
        {
            value = GET_BITS(VDP_Reg.Spr_Att_Adr, 1, 6) << 8;
        }
        else if (h40ModeActive)
        {
            value = GET_BITS(VDP_Reg.Spr_Att_Adr, 1, (extendedVRAMModeEnabled) ? 7 : 6) << 10;
        }

        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SPRITEBASE_E, 5, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_SPRITEPATTERNBASE)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SPRITEPATTERNBASE, 2, VDP_Reg.Reg6);
    if (currentControlFocus != IDC_VDP_REGISTERS_SPRITEPATTERNBASE_E)
    {
        if (mode4Enabled)
        {
            value = GET_BITS(VDP_Reg.Reg6, 2, 1) << 13;
        }
        else if (extendedVRAMModeEnabled)
        {
            value = GET_BITS(VDP_Reg.Reg6, 5, 1) << 16;
        }

        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SPRITEPATTERNBASE_E, 5, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_HSCROLLBASE)
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_HSCROLLBASE, 2, VDP_Reg.H_Scr_Adr);
    if (currentControlFocus != IDC_VDP_REGISTERS_HSCROLLBASE_E)
    {
        value = GET_BITS(VDP_Reg.H_Scr_Adr, 0, (extendedVRAMModeEnabled) ? 7 : 6) << 10;
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_HSCROLLBASE_E, 5, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_DMALENGTH)
    {
        value = VDP_Reg.DMA_Length_L;
        value += VDP_Reg.DMA_Length_H << 8;

        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_DMALENGTH, 4, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_DMASOURCE)
    {
        value = VDP_Reg.DMA_Src_Adr_L << 1;
        value += VDP_Reg.DMA_Src_Adr_M << 9;
        value += GET_BITS(VDP_Reg.DMA_Src_Adr_H, 0, 7) << 17;

        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_DMASOURCE, 6, value >> 1);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_DMASOURCE_E)
    {
        value = VDP_Reg.DMA_Src_Adr_L << 1;
        value += VDP_Reg.DMA_Src_Adr_M << 9;
        value += GET_BITS(VDP_Reg.DMA_Src_Adr_H, 0, 7) << 17;

        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_DMASOURCE_E, 6, value);
    }
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_DMD1, (VDP_Reg.DMA_Src_Adr_H & mask(7)) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_DMD0, (VDP_Reg.DMA_Src_Adr_H & mask(6)) ? BST_CHECKED : BST_UNCHECKED);

    if (currentControlFocus != IDC_VDP_REGISTERS_0E57)
    {
        value = GET_BITS(VDP_Reg.Reg14, 5, 3);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_0E57, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_SCROLLAPATTERNBASE)
    {
        value = VDP_Reg.Reg14 & 0x0F;
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SCROLLAPATTERNBASE, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_SCROLLAPATTERNBASE_E)
    {
        if (extendedVRAMModeEnabled)
        {
            value = GET_BITS(VDP_Reg.Reg14, 0, 1) << 16;
        }

        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SCROLLAPATTERNBASE_E, 5, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_0E13)
    {
        value = GET_BITS(VDP_Reg.Reg14, 1, 3);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_0E13, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_SCROLLBPATTERNBASE)
    {
        value = (VDP_Reg.Reg14 >> 4) & 0x0F;
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SCROLLBPATTERNBASE, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_SCROLLBPATTERNBASE_E)
    {
        if (extendedVRAMModeEnabled)
        {
            value = (GET_BITS(VDP_Reg.Reg14, 0, 1) << 16) & (GET_BITS(VDP_Reg.Reg14, 4, 1) << 16);
        }

        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_SCROLLBPATTERNBASE_E, 5, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_1067)
    {
        value = GET_BITS(VDP_Reg.Scr_Size, 6, 2);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_1067, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_VSZ)
    {
        value = GET_BITS(VDP_Reg.Scr_Size, 4, 2);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_VSZ, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_1023)
    {
        value = GET_BITS(VDP_Reg.Scr_Size, 2, 2);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_1023, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_HSZ)
    {
        value = GET_BITS(VDP_Reg.Scr_Size, 0, 2);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_HSZ, 1, value);
    }
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_WINDOWRIGHT, (VDP_Reg.Win_H_Pos & mask(7)) ? BST_CHECKED : BST_UNCHECKED);
    if (currentControlFocus != IDC_VDP_REGISTERS_1156)
    {
        value = GET_BITS(VDP_Reg.Win_H_Pos, 5, 2);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_1156, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_WINDOWBASEX)
    {
        value = GET_BITS(VDP_Reg.Win_H_Pos, 0, 5);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_WINDOWBASEX, 1, value);
    }
    CheckDlgButton(hwnd, IDC_VDP_REGISTERS_WINDOWDOWN, (VDP_Reg.Win_V_Pos & mask(7)) ? BST_CHECKED : BST_UNCHECKED);
    if (currentControlFocus != IDC_VDP_REGISTERS_1256)
    {
        value = GET_BITS(VDP_Reg.Win_V_Pos, 5, 2);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_1256, 1, value);
    }
    if (currentControlFocus != IDC_VDP_REGISTERS_WINDOWBASEY)
    {
        value = GET_BITS(VDP_Reg.Win_V_Pos, 0, 5);
        UpdateDlgItemHex(hwnd, IDC_VDP_REGISTERS_WINDOWBASEY, 1, value);
    }

    unsigned int screenSizeCellsH = 0x20 + (VDP_Reg.Scr_Size & 0x3) * 32;
    unsigned int screenSizeCellsV = 0x20 + ((VDP_Reg.Scr_Size >> 4) & 0x3) * 32;

    UpdateDlgItemBin(hwnd, IDC_VDP_REGISTERS_HSZ_E, screenSizeCellsH);
    UpdateDlgItemBin(hwnd, IDC_VDP_REGISTERS_VSZ_E, screenSizeCellsV);
}
#undef GET_BITS

#define SET_BIT(number, n, x) (number = (number & ~mask(n)) | (x << n))
#define SET_BITS(number, n, c, x) (number = (number & ~mask(n, c)) | (x << n))
//----------------------------------------------------------------------------------------
INT_PTR msgModeRegistersWM_COMMAND(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
    if (HIWORD(wparam) == BN_CLICKED)
    {
        unsigned int controlID = LOWORD(wparam);
        int chk = (IsDlgButtonChecked(hwnd, controlID) == BST_CHECKED) ? 1 : 0;
        switch (controlID)
        {
        case IDC_VDP_REGISTERS_VSI:
            SET_BIT(VDP_Reg.Set1, 7, chk);
            break;
        case IDC_VDP_REGISTERS_HSI:
            SET_BIT(VDP_Reg.Set1, 6, chk);
            break;
        case IDC_VDP_REGISTERS_LCB:
            SET_BIT(VDP_Reg.Set1, 5, chk);
            break;
        case IDC_VDP_REGISTERS_IE1:
            SET_BIT(VDP_Reg.Set1, 4, chk);
            break;
        case IDC_VDP_REGISTERS_SS:
            SET_BIT(VDP_Reg.Set1, 3, chk);
            break;
        case IDC_VDP_REGISTERS_PS:
            SET_BIT(VDP_Reg.Set1, 2, chk);
            break;
        case IDC_VDP_REGISTERS_M2:
            SET_BIT(VDP_Reg.Set1, 1, chk);
            break;
        case IDC_VDP_REGISTERS_ES:
            SET_BIT(VDP_Reg.Set1, 0, chk);
            break;
        case IDC_VDP_REGISTERS_EVRAM:
            SET_BIT(VDP_Reg.Set2, 7, chk);
            break;
        case IDC_VDP_REGISTERS_DISP:
            SET_BIT(VDP_Reg.Set2, 6, chk);
            break;
        case IDC_VDP_REGISTERS_IE0:
            SET_BIT(VDP_Reg.Set2, 5, chk);
            break;
        case IDC_VDP_REGISTERS_M1:
            SET_BIT(VDP_Reg.Set2, 4, chk);
            break;
        case IDC_VDP_REGISTERS_M3:
            SET_BIT(VDP_Reg.Set2, 3, chk);
            break;
        case IDC_VDP_REGISTERS_M5:
            SET_BIT(VDP_Reg.Set2, 2, chk);
            break;
        case IDC_VDP_REGISTERS_SZ:
            SET_BIT(VDP_Reg.Set2, 1, chk);
            break;
        case IDC_VDP_REGISTERS_MAG:
            SET_BIT(VDP_Reg.Set2, 0, chk);
            break;
        case IDC_VDP_REGISTERS_0B7:
            SET_BIT(VDP_Reg.Set3, 7, chk);
            break;
        case IDC_VDP_REGISTERS_0B6:
            SET_BIT(VDP_Reg.Set3, 6, chk);
            break;
        case IDC_VDP_REGISTERS_0B5:
            SET_BIT(VDP_Reg.Set3, 5, chk);
            break;
        case IDC_VDP_REGISTERS_0B4:
            SET_BIT(VDP_Reg.Set3, 4, chk);
            break;
        case IDC_VDP_REGISTERS_IE2:
            SET_BIT(VDP_Reg.Set3, 3, chk);
            break;
        case IDC_VDP_REGISTERS_VSCR:
            SET_BIT(VDP_Reg.Set3, 2, chk);
            break;
        case IDC_VDP_REGISTERS_HSCR:
            SET_BIT(VDP_Reg.Set3, 1, chk);
            break;
        case IDC_VDP_REGISTERS_LSCR:
            SET_BIT(VDP_Reg.Set3, 0, chk);
            break;
        case IDC_VDP_REGISTERS_RS0:
            SET_BIT(VDP_Reg.Set4, 7, chk);
            break;
        case IDC_VDP_REGISTERS_U1:
            SET_BIT(VDP_Reg.Set4, 6, chk);
            break;
        case IDC_VDP_REGISTERS_U2:
            SET_BIT(VDP_Reg.Set4, 5, chk);
            break;
        case IDC_VDP_REGISTERS_U3:
            SET_BIT(VDP_Reg.Set4, 4, chk);
            break;
        case IDC_VDP_REGISTERS_STE:
            SET_BIT(VDP_Reg.Set4, 3, chk);
            break;
        case IDC_VDP_REGISTERS_LSM1:
            SET_BIT(VDP_Reg.Set4, 2, chk);
            break;
        case IDC_VDP_REGISTERS_LSM0:
            SET_BIT(VDP_Reg.Set4, 1, chk);
            break;
        case IDC_VDP_REGISTERS_RS1:
            SET_BIT(VDP_Reg.Set4, 0, chk);
            break;
        }
    }

    return TRUE;
}

//----------------------------------------------------------------------------------------
INT_PTR msgOtherRegistersWM_COMMAND(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
    if (HIWORD(wparam) == BN_CLICKED)
    {
        unsigned int controlID = LOWORD(wparam);
        int chk = (IsDlgButtonChecked(hwnd, controlID) == BST_CHECKED) ? 1 : 0;
        switch (controlID)
        {
        case IDC_VDP_REGISTERS_077:
            SET_BIT(VDP_Reg.BG_Color, 7, chk);
            break;
        case IDC_VDP_REGISTERS_076:
            SET_BIT(VDP_Reg.BG_Color, 6, chk);
            break;
        case IDC_VDP_REGISTERS_DMD1:
            SET_BIT(VDP_Reg.DMA_Src_Adr_H, 7, chk);
            break;
        case IDC_VDP_REGISTERS_DMD0:
            SET_BIT(VDP_Reg.DMA_Src_Adr_H, 6, chk);
            break;
        case IDC_VDP_REGISTERS_WINDOWRIGHT:
            SET_BIT(VDP_Reg.Win_H_Pos, 7, chk);
            break;
        case IDC_VDP_REGISTERS_WINDOWDOWN:
            SET_BIT(VDP_Reg.Win_V_Pos, 7, chk);
            break;
        }
    }
    else if ((HIWORD(wparam) == EN_CHANGE))
    {
        return FALSE;
    }
    else if ((HIWORD(wparam) == EN_SETFOCUS))
    {
        previousText = GetDlgItemString(hwnd, LOWORD(wparam));
        currentControlFocus = LOWORD(wparam);
    }
    else if ((HIWORD(wparam) == EN_KILLFOCUS))
    {
        std::string newText = GetDlgItemString(hwnd, LOWORD(wparam));
        if (newText != previousText)
        {
            switch (LOWORD(wparam))
            {
            case IDC_VDP_REGISTERS_BACKGROUNDPALETTEROW:
                SET_BITS(VDP_Reg.BG_Color, 4, 2, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_BACKGROUNDPALETTECOLUMN:
                SET_BITS(VDP_Reg.BG_Color, 0, 4, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_BACKGROUNDSCROLLX:
                VDP_Reg.Reg8 = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_BACKGROUNDSCROLLY:
                VDP_Reg.Reg9 = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_HINTLINECOUNTER:
                VDP_Reg.H_Int = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_AUTOINCREMENT:
                VDP_Reg.Auto_Inc = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_SCROLLABASE:
                VDP_Reg.Pat_ScrA_Adr = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_SCROLLABASE_E:
                VDP_Reg.Pat_ScrA_Adr = (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 10) & 0xFF;
                break;
            case IDC_VDP_REGISTERS_WINDOWBASE:
                VDP_Reg.Pat_Win_Adr = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_WINDOWBASE_E:
                VDP_Reg.Pat_Win_Adr = (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 10) & 0xFF;
                break;
            case IDC_VDP_REGISTERS_SCROLLBBASE:
                VDP_Reg.Pat_ScrB_Adr = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_SCROLLBBASE_E:
                VDP_Reg.Pat_ScrB_Adr = (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 13) & 0xFF;
                break;
            case IDC_VDP_REGISTERS_SPRITEBASE:
                VDP_Reg.Spr_Att_Adr = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_SPRITEBASE_E:
                if (!(VDP_Reg.Set2 & mask(2)))
                {
                    VDP_Reg.Spr_Att_Adr = (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 7) & 0xFF;
                }
                else
                {
                    VDP_Reg.Spr_Att_Adr = (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 9) & 0xFF;
                }
                break;
            case IDC_VDP_REGISTERS_SPRITEPATTERNBASE:
                VDP_Reg.Reg6 = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_SPRITEPATTERNBASE_E:
                if (!(VDP_Reg.Set2 & mask(2)))
                {
                    VDP_Reg.Reg6 = (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 13) & 0xFF;
                }
                else
                {
                    VDP_Reg.Reg6 = (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 16) & 0xFF;
                }
                break;
            case IDC_VDP_REGISTERS_HSCROLLBASE:
                VDP_Reg.H_Scr_Adr = GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_HSCROLLBASE_E:
                VDP_Reg.H_Scr_Adr = (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 10) & 0xFF;
                break;
            case IDC_VDP_REGISTERS_DMALENGTH:
            {
                unsigned short w = GetDlgItemHex(hwnd, LOWORD(wparam));
                VDP_Reg.DMA_Length_L = (w & 0xFF);
                VDP_Reg.DMA_Length_H = (w >> 8) & 0xFF;
            } break;
            case IDC_VDP_REGISTERS_DMASOURCE:
            {
                unsigned int l = GetDlgItemHex(hwnd, LOWORD(wparam)) << 1;
                VDP_Reg.DMA_Src_Adr_L = (l >> 1) & 0xFF;
                VDP_Reg.DMA_Src_Adr_M = (l >> 9) & 0xFF;
                VDP_Reg.DMA_Src_Adr_H = (l >> 17) & mask(0, 7);
            } break;
            case IDC_VDP_REGISTERS_DMASOURCE_E:
            {
                unsigned int l = GetDlgItemHex(hwnd, LOWORD(wparam));
                VDP_Reg.DMA_Src_Adr_L = (l >> 1) & 0xFF;
                VDP_Reg.DMA_Src_Adr_M = (l >> 9) & 0xFF;
                VDP_Reg.DMA_Src_Adr_H = (l >> 17) & mask(0, 7);
            } break;
            case IDC_VDP_REGISTERS_0E57:
                SET_BITS(VDP_Reg.Reg14, 5, 3, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_SCROLLAPATTERNBASE:
                VDP_Reg.Reg14 = (VDP_Reg.Reg14 & 0xF0) | GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_SCROLLAPATTERNBASE_E:
                SET_BITS(VDP_Reg.Reg14, 4, 1, (GetDlgItemHex(hwnd, LOWORD(wparam)) >> 16));
                break;
            case IDC_VDP_REGISTERS_0E13:
                SET_BITS(VDP_Reg.Reg14, 1, 3, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_SCROLLBPATTERNBASE:
                VDP_Reg.Reg14 = (VDP_Reg.Reg14 & 0x0F) | GetDlgItemHex(hwnd, LOWORD(wparam));
                break;
            case IDC_VDP_REGISTERS_SCROLLBPATTERNBASE_E:
            {
                unsigned int newData = GetDlgItemHex(hwnd, LOWORD(wparam)) >> 16;
                SET_BITS(VDP_Reg.Reg14, 0, 1, newData);
                if (newData != 0)
                {
                    SET_BITS(VDP_Reg.Reg14, 4, 1, newData);
                }
            } break;
            case IDC_VDP_REGISTERS_1067:
                SET_BITS(VDP_Reg.Scr_Size, 6, 2, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_VSZ:
                SET_BITS(VDP_Reg.Scr_Size, 4, 2, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_1023:
                SET_BITS(VDP_Reg.Scr_Size, 2, 2, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_HSZ:
                SET_BITS(VDP_Reg.Scr_Size, 0, 2, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_1156:
                SET_BITS(VDP_Reg.Win_H_Pos, 5, 2, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_WINDOWBASEX:
                SET_BITS(VDP_Reg.Win_H_Pos, 0, 5, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_1256:
                SET_BITS(VDP_Reg.Win_V_Pos, 5, 2, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            case IDC_VDP_REGISTERS_WINDOWBASEY:
                SET_BITS(VDP_Reg.Win_V_Pos, 0, 5, GetDlgItemHex(hwnd, LOWORD(wparam)));
                break;
            }
        }
    }

    return TRUE;
}
#undef SET_BIT
#undef SET_BITS

//----------------------------------------------------------------------------------------
INT_PTR WndProcModeRegisters(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    WndProcDialogImplementSaveFieldWhenLostFocus(hwnd, msg, wparam, lparam);
    switch (msg)
    {
    case WM_COMMAND:
        return msgModeRegistersWM_COMMAND(hwnd, wparam, lparam);
    }
    return FALSE;
}

//----------------------------------------------------------------------------------------
INT_PTR WndProcOtherRegisters(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    WndProcDialogImplementSaveFieldWhenLostFocus(hwnd, msg, wparam, lparam);
    switch (msg)
    {
    case WM_COMMAND:
        return msgOtherRegistersWM_COMMAND(hwnd, wparam, lparam);
    }
    return FALSE;
}

INT_PTR CALLBACK WndProcModeRegistersStatic(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    //Obtain the object pointer
    int state = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    //Process the message
    switch (msg)
    {
    case WM_INITDIALOG:
        //Set the object pointer
        state = (int)lparam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(state));

        //Pass this message on to the member window procedure function
        if (state != 0)
        {
            return WndProcModeRegisters(hwnd, msg, wparam, lparam);
        }
        break;
    case WM_DESTROY:
        if (state != 0)
        {
            //Pass this message on to the member window procedure function
            INT_PTR result = WndProcModeRegisters(hwnd, msg, wparam, lparam);

            //Discard the object pointer
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)0);

            //Return the result from processing the message
            return result;
        }
        break;
    }

    //Pass this message on to the member window procedure function
    INT_PTR result = FALSE;
    if (state != 0)
    {
        result = WndProcModeRegisters(hwnd, msg, wparam, lparam);
    }
    return result;
}

INT_PTR CALLBACK WndProcOtherRegistersStatic(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    //Obtain the object pointer
    int state = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    //Process the message
    switch (msg)
    {
    case WM_INITDIALOG:
        //Set the object pointer
        state = (int)lparam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(state));

        //Pass this message on to the member window procedure function
        if (state != 0)
        {
            return WndProcOtherRegisters(hwnd, msg, wparam, lparam);
        }
        break;
    case WM_DESTROY:
        if (state != 0)
        {
            //Pass this message on to the member window procedure function
            INT_PTR result = WndProcOtherRegisters(hwnd, msg, wparam, lparam);

            //Discard the object pointer
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)0);

            //Return the result from processing the message
            return result;
        }
        break;
    }

    //Pass this message on to the member window procedure function
    INT_PTR result = FALSE;
    if (state != 0)
    {
        result = WndProcOtherRegisters(hwnd, msg, wparam, lparam);
    }
    return result;
}

INT_PTR msgRegistersWM_INITDIALOG(HWND hDlg, WPARAM wparam, LPARAM lparam)
{
    //Add our set of tab items to the list of tabs
    tabItems.clear();
    tabItems.push_back(TabInfo("Mode Registers", IDD_VDP_REGISTERS_MODEREGISTERS, WndProcModeRegistersStatic));
    tabItems.push_back(TabInfo("Other Registers", IDD_VDP_REGISTERS_OTHERREGISTERS, WndProcOtherRegistersStatic));

    //Insert our tabs into the tab control
    for (unsigned int i = 0; i < (unsigned int)tabItems.size(); ++i)
    {
        TCITEM tabItem;
        tabItem.mask = TCIF_TEXT;
        tabItem.pszText = (LPSTR)tabItems[i].tabName.c_str();
        SendMessage(GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), TCM_INSERTITEM, i, (LPARAM)&tabItem);
    }

    //Create each window associated with each tab, and calculate the required size of the
    //client area of the tab control to fit the largest tab window.
    int requiredTabClientWidth = 0;
    int requiredTabClientHeight = 0;
    for (unsigned int i = 0; i < (unsigned int)tabItems.size(); ++i)
    {
        //Create the dialog window for this tab
        DLGPROC dialogWindowProc = tabItems[i].dialogProc;
        LPCSTR dialogTemplateName = MAKEINTRESOURCE(tabItems[i].dialogID);
        tabItems[i].hwndDialog = CreateDialogParam(GetHInstance(), dialogTemplateName, GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), dialogWindowProc, (LPARAM)1);

        //Calculate the required size of the window for this tab in pixel units
        RECT rect;
        GetClientRect(tabItems[i].hwndDialog, &rect);
        int tabWidth = rect.right;
        int tabHeight = rect.bottom;

        //Increase the required size of the client area for the tab control to accommodate
        //the contents of this tab, if required.
        requiredTabClientWidth = (tabWidth > requiredTabClientWidth) ? tabWidth : requiredTabClientWidth;
        requiredTabClientHeight = (tabHeight > requiredTabClientHeight) ? tabHeight : requiredTabClientHeight;
    }

    //Save the original size of the tab control
    RECT tabControlOriginalRect;
    GetClientRect(GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), &tabControlOriginalRect);
    int tabControlOriginalSizeX = tabControlOriginalRect.right - tabControlOriginalRect.left;
    int tabControlOriginalSizeY = tabControlOriginalRect.bottom - tabControlOriginalRect.top;

    //Calculate the exact required pixel size of the tab control to fully display the
    //content in each tab
    RECT tabControlRect;
    tabControlRect.left = 0;
    tabControlRect.top = 0;
    tabControlRect.right = requiredTabClientWidth;
    tabControlRect.bottom = requiredTabClientHeight;
    SendMessage(GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), TCM_ADJUSTRECT, (WPARAM)TRUE, (LPARAM)&tabControlRect);
    int tabControlRequiredSizeX = tabControlRect.right - tabControlRect.left;
    int tabControlRequiredSizeY = tabControlRect.bottom - tabControlRect.top;

    //Resize the tab control
    SetWindowPos(GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), NULL, 0, 0, tabControlRequiredSizeX, tabControlRequiredSizeY, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);

    //Calculate the required pixel size and position of each tab window
    RECT currentTabControlRect;
    GetWindowRect(GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), &currentTabControlRect);
    SendMessage(GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), TCM_ADJUSTRECT, (WPARAM)FALSE, (LPARAM)&currentTabControlRect);
    POINT tabContentPoint;
    tabContentPoint.x = currentTabControlRect.left;
    tabContentPoint.y = currentTabControlRect.top;
    ScreenToClient(GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), &tabContentPoint);
    int tabRequiredPosX = tabContentPoint.x;
    int tabRequiredPosY = tabContentPoint.y;
    int tabRequiredSizeX = currentTabControlRect.right - currentTabControlRect.left;
    int tabRequiredSizeY = currentTabControlRect.bottom - currentTabControlRect.top;

    //Position and size each tab window
    for (unsigned int i = 0; i < (unsigned int)tabItems.size(); ++i)
    {
        SetWindowPos(tabItems[i].hwndDialog, NULL, tabRequiredPosX, tabRequiredPosY, tabRequiredSizeX, tabRequiredSizeY, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    }

    //Calculate the current size of the owning window
    RECT mainDialogRect;
    GetWindowRect(hDlg, &mainDialogRect);
    int currentMainDialogWidth = mainDialogRect.right - mainDialogRect.left;
    int currentMainDialogHeight = mainDialogRect.bottom - mainDialogRect.top;

    //Resize the owning window to the required size
    int newMainDialogWidth = currentMainDialogWidth + (tabControlRequiredSizeX - tabControlOriginalSizeX);
    int newMainDialogHeight = currentMainDialogHeight + (tabControlRequiredSizeY - tabControlOriginalSizeY);
    SetWindowPos(hDlg, NULL, 0, 0, newMainDialogWidth, newMainDialogHeight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);

    //Explicitly select and show the first tab
    activeTabWindow = tabItems[0].hwndDialog;
    ShowWindow(activeTabWindow, SW_SHOWNA);

    return TRUE;
}

extern int Show_Genesis_Screen(HWND hWnd);

void Redraw_VDP_View()
{
    if (!VDPRamHWnd) return;

    RedrawWindow(GetDlgItem(VDPRamHWnd, IDC_VDP_PALETTE), NULL, NULL, RDW_INVALIDATE);
    RedrawWindow(GetDlgItem(VDPRamHWnd, IDC_VDP_TILES), NULL, NULL, RDW_INVALIDATE);
    RedrawWindow(GetDlgItem(VDPRamHWnd, IDC_VDP_TILE_VIEW), NULL, NULL, RDW_INVALIDATE);

    msgModeRegistersUPDATE(tabItems[0].hwndDialog);
    msgOtherRegistersUPDATE(tabItems[1].hwndDialog);
}

BOOL CALLBACK MoveGroupCallback(HWND hChild, LPARAM lParam)
{
    RECT rChild;
    LPRECT r = (LPRECT)lParam;

    GetWindowRect(hChild, &rChild);
    OffsetRect(&rChild, -r->left, -r->top);
    MapWindowPoints(HWND_DESKTOP, GetParent(hChild), (LPPOINT)&rChild, 2);

    SetWindowPos(hChild, NULL,
        rChild.left,
        rChild.top,
        0,
        0,
        SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

    return TRUE;
}

LRESULT CALLBACK ButtonsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_COMMAND:
    {
        switch (wParam)
        {
        case IDC_VDP_PAL_DUMP:
        {
            char fname[2048];
            strcpy(fname, "pal.bin");
            if (Change_File_S(fname, ".", "Save Dump Pal As...", "All Files\0*.*\0\0", "*.*", hWnd))
            {
                FILE *out = fopen(fname, "wb+");
                int i;
                for (i = 0; i < sizeof(CRam); ++i)
                {
                    fname[i & 2047] = ((char*)&CRam)[i ^ 1];
                    if ((i & 2047) == 2047)
                        fwrite(fname, 1, sizeof(fname), out);
                }
                fwrite(fname, 1, i & 2047, out);
                fclose(out);
            }

            return FALSE;
        } break;
        case IDC_VDP_PAL_LOAD:
        {
            char fname[2048];
            strcpy(fname, "pal.bin");
            if (Change_File_L(fname, ".", "Load Dump Pal As...", "All Files\0*.*\0\0", "*.*", hWnd))
            {
                FILE *in = fopen(fname, "rb");
                int i;
                for (i = 0; i < sizeof(CRam); ++i)
                {
                    if (!(i & 2047))
                        fread(fname, 1, sizeof(fname), in);
                    ((char*)&CRam)[i ^ 1] = fname[i & 2047];
                }
                fclose(in);
                if (Game)
                {
                    CRam_Flag = 1;
                    Show_Genesis_Screen(HWnd);
                }
            }
            return FALSE;
        } break;
        case IDC_VDP_PAL_YY:
        {
            char fname[2048];
            strcpy(fname, "pal.pal");
            if (Change_File_S(fname, ".", "Save YY-CHR Pal As...", "All Files\0*.*\0\0", "*.*", hWnd))
            {
                FILE *out = fopen(fname, "wb+");
                int i;
                for (i = 0; i < VDP_PAL_COLORS * VDP_PAL_COUNT; ++i)
                {
                    *((DWORD*)fname) = GetPalColor(i);
                    fwrite(fname, 1, 3, out);
                }
                *((DWORD*)fname) = 0;
                for (; i < 256; ++i)
                    fwrite(fname, 1, 3, out);
                fclose(out);
            }
            return FALSE;
        } break;
        case IDC_VDP_VRAM_DUMP:
        {
            char fname[2048];
            strcpy(fname, "vram.bin");
            if (Change_File_S(fname, ".", "Save Dump VRAM As...", "All Files\0*.*\0\0", "*.*", hWnd))
            {
                FILE *out = fopen(fname, "wb+");
                int i;
                for (i = 0; i < sizeof(VRam); ++i)
                {
                    fname[i & 2047] = ((char*)&VRam)[i ^ 1];
                    if ((i & 2047) == 2047)
                        fwrite(fname, 1, sizeof(fname), out);
                }
                fwrite(fname, 1, i & 2047, out);
                fclose(out);
            }
            return FALSE;
        } break;
        case IDC_VDP_VRAM_LOAD:
        {
            char fname[2048];
            strcpy(fname, "vram.bin");
            if (Change_File_L(fname, ".", "Load Dump VRAM As...", "All Files\0*.*\0\0", "*.*", hWnd))
            {
                FILE *in = fopen(fname, "rb");
                int i;
                for (i = 0; i < sizeof(VRam); ++i)
                {
                    if (!(i & 2047))
                        fread(fname, 1, sizeof(fname), in);
                    ((char*)&VRam)[i ^ 1] = fname[i & 2047];
                }
                fclose(in);
                if (Genesis_Started || _32X_Started || SegaCD_Started)
                    Show_Genesis_Screen(HWnd);
            }
            return FALSE;
        } break;
        case IDC_VDP_VIEW_VRAM:
        {
            IsVRAM = true;
            Redraw_VDP_View();
            return FALSE;
        } break;
        case IDC_VDP_VIEW_RAM:
        {
            IsVRAM = false;
            Redraw_VDP_View();
            return FALSE;
        } break;
        }
    } break;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK VDPRamProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT r, r2, r3;
    int dx1, dy1, dx2, dy2;
    static int watchIndex = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        IsVRAM = true;
        CheckRadioButton(hDlg, IDC_VDP_VIEW_VRAM, IDC_VDP_VIEW_RAM, IDC_VDP_VIEW_VRAM);

        VDPRamHWnd = hDlg;

        if (Full_Screen)
        {
            while (ShowCursor(false) >= 0);
            while (ShowCursor(true) < 0);
        }

        GetWindowRect(HWnd, &r);
        dx1 = (r.right - r.left) / 2;
        dy1 = (r.bottom - r.top) / 2;

        GetWindowRect(hDlg, &r2);
        dx2 = (r2.right - r2.left) / 2;
        dy2 = (r2.bottom - r2.top) / 2;

        // push it away from the main window if we can
        const int width = (r.right - r.left);
        const int width2 = (r2.right - r2.left);
        if (r.left + width2 + width < GetSystemMetrics(SM_CXSCREEN))
        {
            r.right += width;
            r.left += width;
        }
        else if ((int)r.left - (int)width2 > 0)
        {
            r.right -= width2;
            r.left -= width2;
        }

        SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

        // Palette view
        HWND hPalette = GetDlgItem(hDlg, IDC_VDP_PALETTE);
        SetWindowPos(hPalette, NULL,
            5,
            5,
            VDP_PAL_COLORS * VDP_BLOCK_W,
            VDP_PAL_COUNT * VDP_BLOCK_H,
            SWP_NOZORDER | SWP_NOACTIVATE);
        // Palette view

        // Tiles view
        HWND hTiles = GetDlgItem(hDlg, IDC_VDP_TILES);
        GetWindowRect(hPalette, &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);
        SetWindowPos(hTiles, NULL,
            r.left,
            r.bottom + 5,
            VDP_TILES_IN_ROW * VDP_BLOCK_W,
            VDP_TILES_IN_COL * VDP_BLOCK_H,
            SWP_NOZORDER | SWP_NOACTIVATE);
        // Tiles view

        // Scrollbar
        GetWindowRect(hTiles, &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);
        HWND hScrollbar = GetDlgItem(hDlg, IDC_VDP_TILES_SCROLLBAR);
        SetWindowPos(hScrollbar, NULL,
            r.right + 1,
            r.top,
            VDP_BLOCK_W,
            (r.bottom - r.top),
            SWP_NOZORDER | SWP_NOACTIVATE);
        SetScrollRange(hScrollbar, SB_CTL, 0, VDP_SCROLL_MAX, TRUE);
        // Scrollbar

        // Palette group
        HWND hPalGroup = GetDlgItem(hDlg, IDC_VDP_PAL_GROUP);
        GetWindowRect(hScrollbar, &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);
        GetWindowRect(hPalette, &r2);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r2, 2);
        GetWindowRect(hPalGroup, &r3);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r3, 2);

        SetParent(GetDlgItem(hDlg, IDC_VDP_PAL_DUMP), hPalGroup);
        SetParent(GetDlgItem(hDlg, IDC_VDP_PAL_LOAD), hPalGroup);
        SetParent(GetDlgItem(hDlg, IDC_VDP_PAL_YY), hPalGroup);

        SetWindowPos(hPalGroup, NULL,
            r.right + 5,
            r2.top,
            0,
            0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        GetWindowRect(hPalGroup, &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);
        SubtractRect(&r3, &r3, &r);
        EnumChildWindows(hPalGroup, MoveGroupCallback, (LPARAM)&r3);

        SetWindowSubclass(hPalGroup, ButtonsProc, 0, 0);
        // Palette group

        // VRAM group
        HWND hVramGroup = GetDlgItem(hDlg, IDC_VDP_VRAM_GROUP);
        GetWindowRect(hVramGroup, &r3);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r3, 2);

        SetParent(GetDlgItem(hDlg, IDC_VDP_VRAM_DUMP), hVramGroup);
        SetParent(GetDlgItem(hDlg, IDC_VDP_VRAM_LOAD), hVramGroup);

        SetWindowPos(hVramGroup, NULL,
            r.left,
            r.bottom + 5,
            0,
            0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        GetWindowRect(hVramGroup, &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);
        SubtractRect(&r3, &r3, &r);
        EnumChildWindows(hVramGroup, MoveGroupCallback, (LPARAM)&r3);

        SetWindowSubclass(hVramGroup, ButtonsProc, 0, 0);
        // VRAM group

        // View mode group
        HWND hViewMode = GetDlgItem(hDlg, IDC_VDP_VIEW_MODE);
        GetWindowRect(hViewMode, &r3);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r3, 2);

        SetParent(GetDlgItem(hDlg, IDC_VDP_VIEW_VRAM), hViewMode);
        SetParent(GetDlgItem(hDlg, IDC_VDP_VIEW_RAM), hViewMode);

        SetWindowPos(hViewMode, NULL,
            r.left,
            r.bottom + 5,
            0,
            0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        GetWindowRect(hViewMode, &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);
        SubtractRect(&r3, &r3, &r);
        EnumChildWindows(hViewMode, MoveGroupCallback, (LPARAM)&r3);

        SetWindowSubclass(hViewMode, ButtonsProc, 0, 0);
        // View mode group

        // Tile view
        HWND hTileView = GetDlgItem(hDlg, IDC_VDP_TILE_VIEW);
        GetWindowRect(hViewMode, &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);
        SetWindowPos(hTileView, NULL,
            r.left,
            r.bottom + 10,
            r.right - r.left,
            r.right - r.left,
            SWP_NOZORDER | SWP_NOACTIVATE);
        // Tile view

        // Tile info
        HWND hTileInfo = GetDlgItem(hDlg, IDC_VDP_TILE_INFO);
        GetWindowRect(hTileView, &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);
        SetWindowPos(hTileInfo, NULL,
            r.left,
            r.bottom + 10,
            r.right - r.left,
            50,
            SWP_NOZORDER | SWP_NOACTIVATE);
        // Tile info

        // Exodus VDP Regs window init
        msgRegistersWM_INITDIALOG(hDlg, wParam, lParam);

        GetWindowRect(hDlg, &r);
        GetWindowRect(GetDlgItem(hDlg, IDC_VDP_REGISTERS_TABCONTROL), &r3);
        SetWindowPos(hDlg, NULL,
            0,
            0,
            r3.right - r.left + 5,
            r3.bottom - r.top + 5,
            SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
        return true;
    } break;

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT di = (LPDRAWITEMSTRUCT)lParam;

        if ((UINT)wParam == IDC_VDP_PALETTE)
        {
            BYTE* pdst;
            BITMAPINFOHEADER bmih = { sizeof(BITMAPINFOHEADER), VDP_PAL_COLORS, -VDP_PAL_COUNT, 1, 32 };

            HDC hSmallDC = CreateCompatibleDC(di->hDC);
            HBITMAP hSmallBmp = CreateDIBSection(hSmallDC, (BITMAPINFO *)&bmih, DIB_RGB_COLORS, (void **)&pdst, NULL, NULL);
            HBITMAP hOldSmallBmp = (HBITMAP)SelectObject(hSmallDC, hSmallBmp);

            for (int y = 0; y < VDP_PAL_COUNT; ++y)
                for (int x = 0; x < VDP_PAL_COLORS; ++x)
                {
                    int bytes = 4;
                    int xMul = bytes;
                    int xOff = x * xMul;
                    int yMul = (bytes * VDP_PAL_COLORS);
                    int yOff = y * yMul;

                    *(COLORREF*)(&pdst[yOff + xOff]) = GetPalColorNoSwap(VDP_PAL_COLORS * y + x);
                }

            StretchDIBits(
                di->hDC,
                0,
                0,
                di->rcItem.right - di->rcItem.left,
                di->rcItem.bottom - di->rcItem.top,
                0,
                0,
                VDP_PAL_COLORS,
                VDP_PAL_COUNT,
                pdst,
                (const BITMAPINFO *)&bmih,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            SelectObject(hSmallDC, hOldSmallBmp);
            DeleteObject(hSmallBmp);
            DeleteDC(hSmallDC);

            r.left = di->rcItem.left;
            r.right = r.left + VDP_PAL_COLORS * VDP_BLOCK_W;
            r.top = di->rcItem.top + VDPRamPal * VDP_BLOCK_H;
            r.bottom = r.top + VDP_BLOCK_H;
            DrawFocusRect(di->hDC, &r);

            return TRUE;
        }
        else if ((UINT)wParam == IDC_VDP_TILES)
        {
            int scroll = GetScrollPos(GetDlgItem(hDlg, IDC_VDP_TILES_SCROLLBAR), SB_CTL);
            int start = scroll * VDP_TILES_IN_ROW;
            int end = start + VDP_TILES_IN_ROW * VDP_TILES_IN_COL;
            int tiles = end - start;

            BYTE* pdst;
            BITMAPINFOHEADER bmih = { sizeof(BITMAPINFOHEADER), VDP_TILE_W * VDP_TILES_IN_ROW, -VDP_TILE_H * VDP_TILES_IN_COL, 1, 32 };

            HDC hSmallDC = CreateCompatibleDC(di->hDC);
            HBITMAP hSmallBmp = CreateDIBSection(hSmallDC, (BITMAPINFO *)&bmih, DIB_RGB_COLORS, (void **)&pdst, NULL, NULL);
            HBITMAP hOldSmallBmp = (HBITMAP)SelectObject(hSmallDC, hSmallBmp);

            BYTE *ptr = (BYTE *)(IsVRAM ? VRam : Ram_68k);
            for (int i = 0; i < tiles; ++i)
            {
                for (int y = 0; y < VDP_TILE_H; ++y)
                {
                    for (int x = 0; x < (VDP_TILE_W / 2); ++x)
                    {
                        int bytes = 4;
                        int xMul = bytes;
                        int _x = (i % VDP_TILES_IN_ROW) * VDP_TILE_W + x * 2 + 0;
                        int xOff = _x * xMul;
                        int yMul = (bytes * VDP_TILES_IN_ROW * VDP_TILE_W);
                        int _y = (i / VDP_TILES_IN_ROW) * VDP_TILE_H + y;
                        int yOff = _y * yMul;

                        BYTE t = ptr[(start + i) * 0x20 + y * (VDP_TILE_W / 2) + (x ^ 1)];
                        COLORREF c1 = GetPalColorNoSwap(VDP_PAL_COLORS * VDPRamPal + (t >> 4));
                        COLORREF c2 = GetPalColorNoSwap(VDP_PAL_COLORS * VDPRamPal + (t & 0xF));

                        *(COLORREF*)(&pdst[yOff + xOff]) = c1;
                        *(COLORREF*)(&pdst[yOff + xOff + bytes]) = c2;
                    }
                }
            }

            StretchDIBits(
                di->hDC,
                0,
                0,
                di->rcItem.right - di->rcItem.left,
                di->rcItem.bottom - di->rcItem.top,
                0,
                0,
                VDP_TILE_W * VDP_TILES_IN_ROW,
                VDP_TILE_H * VDP_TILES_IN_COL,
                pdst,
                (const BITMAPINFO *)&bmih,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            SelectObject(hSmallDC, hOldSmallBmp);
            DeleteObject(hSmallBmp);
            DeleteDC(hSmallDC);

            r.left = di->rcItem.left + (VDPRamTile % VDP_TILES_IN_ROW) * VDP_BLOCK_W;
            r.right = r.left + VDP_BLOCK_W;
            int row = (VDPRamTile / VDP_TILES_IN_ROW) - scroll;
            r.top = di->rcItem.top + row * VDP_BLOCK_H;
            r.bottom = r.top + VDP_BLOCK_H;

            if (row >= 0 && row < VDP_TILES_IN_COL)
                DrawFocusRect(di->hDC, &r);

            return TRUE;
        }
        else if ((UINT)wParam == IDC_VDP_TILE_VIEW)
        {
            BYTE* pdst;
            BITMAPINFOHEADER bmih = { sizeof(BITMAPINFOHEADER), VDP_TILE_W, -VDP_TILE_H, 1, 32 };

            HDC hSmallDC = CreateCompatibleDC(di->hDC);
            HBITMAP hSmallBmp = CreateDIBSection(hSmallDC, (BITMAPINFO *)&bmih, DIB_RGB_COLORS, (void **)&pdst, NULL, NULL);
            HBITMAP hOldSmallBmp = (HBITMAP)SelectObject(hSmallDC, hSmallBmp);

            BYTE *ptr = (BYTE *)(IsVRAM ? VRam : Ram_68k);
            for (int y = 0; y < VDP_TILE_H; ++y)
            {
                for (int x = 0; x < (VDP_TILE_W / 2); ++x)
                {
                    int bytes = 4;
                    int xMul = bytes;
                    int _x = x * 2 + 0;
                    int xOff = _x * xMul;
                    int yMul = (bytes * VDP_TILE_W);
                    int yOff = y * yMul;

                    BYTE t = ptr[VDPRamTile * 0x20 + y * (VDP_TILE_W / 2) + (x ^ 1)];
                    COLORREF c1 = GetPalColorNoSwap(VDP_PAL_COLORS * VDPRamPal + (t >> 4));
                    COLORREF c2 = GetPalColorNoSwap(VDP_PAL_COLORS * VDPRamPal + (t & 0xF));

                    *(COLORREF*)(&pdst[yOff + xOff]) = c1;
                    *(COLORREF*)(&pdst[yOff + xOff + bytes]) = c2;
                }
            }

            StretchDIBits(
                di->hDC,
                0,
                0,
                di->rcItem.right - di->rcItem.left,
                di->rcItem.bottom - di->rcItem.top,
                0,
                0,
                VDP_TILE_W,
                VDP_TILE_H,
                pdst,
                (const BITMAPINFO *)&bmih,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            SelectObject(hSmallDC, hOldSmallBmp);
            DeleteObject(hSmallBmp);
            DeleteDC(hSmallDC);

            char buff[30];
            sprintf(buff, "Offset: %04X\r\nId: %03X", (VDPRamTile * 0x20) | (IsVRAM ? 0x0000 : 0xFF0000), VDPRamTile);
            SetDlgItemText(hDlg, IDC_VDP_TILE_INFO, buff);

            return TRUE;
        }
    } break;

    case WM_NOTIFY:
    {
        NMHDR* nmhdr = (NMHDR*)lParam;
        if (nmhdr->idFrom == IDC_VDP_REGISTERS_TABCONTROL)
        {
            if ((nmhdr->code == TCN_SELCHANGE))
            {
                HDWP deferWindowPosSession = BeginDeferWindowPos(2);

                if (activeTabWindow != NULL)
                {
                    DeferWindowPos(deferWindowPosSession, activeTabWindow, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
                    activeTabWindow = NULL;
                }

                int currentlySelectedTab = (int)SendMessage(nmhdr->hwndFrom, TCM_GETCURSEL, 0, 0);
                if ((currentlySelectedTab < 0) || (currentlySelectedTab >= (int)tabItems.size()))
                {
                    currentlySelectedTab = 0;
                }
                activeTabWindow = tabItems[currentlySelectedTab].hwndDialog;
                DeferWindowPos(deferWindowPosSession, activeTabWindow, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);

                EndDeferWindowPos(deferWindowPosSession);
            }
        }
        return TRUE;
    } break;

    case WM_VSCROLL:
    {
        int CurPos = GetScrollPos(GetDlgItem(hDlg, IDC_VDP_TILES_SCROLLBAR), SB_CTL);
        int nSBCode = LOWORD(wParam);
        int nPos = HIWORD(wParam);
        switch (nSBCode)
        {
        case SB_LEFT:      // Scroll to far left.
            CurPos = 0;
            break;

        case SB_RIGHT:      // Scroll to far right.
            CurPos = VDP_SCROLL_MAX;
            break;

        case SB_ENDSCROLL:   // End scroll.
            break;

        case SB_LINELEFT:      // Scroll left.
            if (CurPos > 0)
                CurPos--;
            break;

        case SB_LINERIGHT:   // Scroll right.
            if (CurPos < VDP_SCROLL_MAX)
                CurPos++;
            break;

        case SB_PAGELEFT:    // Scroll one page left.
            CurPos -= VDP_TILES_IN_COL;
            if (CurPos < 0)
                CurPos = 0;
            break;

        case SB_PAGERIGHT:      // Scroll one page righ
            CurPos += VDP_TILES_IN_COL;
            if (CurPos >= VDP_SCROLL_MAX)
                CurPos = VDP_SCROLL_MAX - 1;
            break;

        case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
        case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
        {
            SCROLLINFO si;
            ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_TRACKPOS;

            // Call GetScrollInfo to get current tracking
            //    position in si.nTrackPos

            if (!GetScrollInfo(GetDlgItem(hDlg, IDC_VDP_TILES_SCROLLBAR), SB_CTL, &si))
                return 1; // GetScrollInfo failed
            CurPos = si.nTrackPos;
        } break;
        }
        SetScrollPos(GetDlgItem(hDlg, IDC_VDP_TILES_SCROLLBAR), SB_CTL, CurPos, TRUE);
        Redraw_VDP_View();
    } break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hDlg, &ps);

        RedrawWindow(GetDlgItem(VDPRamHWnd, IDC_VDP_PALETTE), NULL, NULL, RDW_INVALIDATE);
        RedrawWindow(GetDlgItem(VDPRamHWnd, IDC_VDP_TILES), NULL, NULL, RDW_INVALIDATE);
        RedrawWindow(GetDlgItem(VDPRamHWnd, IDC_VDP_TILE_VIEW), NULL, NULL, RDW_INVALIDATE);

        EndPaint(hDlg, &ps);

        msgModeRegistersUPDATE(tabItems[0].hwndDialog);
        msgOtherRegistersUPDATE(tabItems[1].hwndDialog);
        return true;
    } break;

    case WM_LBUTTONDOWN:
    {
        RECT r;
        POINT pt;

        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        GetWindowRect(GetDlgItem(hDlg, IDC_VDP_PALETTE), &r);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);

        if (PtInRect(&r, pt))
        {
            VDPRamPal = (pt.y - r.top) / VDP_BLOCK_H;
            Redraw_VDP_View();
        }
        else
        {
            GetWindowRect(GetDlgItem(hDlg, IDC_VDP_TILES), &r);
            MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&r, 2);

            if (PtInRect(&r, pt))
            {
                int scroll = GetScrollPos(GetDlgItem(hDlg, IDC_VDP_TILES_SCROLLBAR), SB_CTL);
                int row = (pt.y - r.top) / VDP_BLOCK_H + scroll;
                int col = (pt.x - r.left) / VDP_BLOCK_W;
                VDPRamTile = row * VDP_TILES_IN_ROW + col;

                Redraw_VDP_View();
            }
        }
    } break;

    case WM_CLOSE:
    {
        if (activeTabWindow != NULL)
        {
            DestroyWindow(activeTabWindow);
            activeTabWindow = NULL;
        }

        if (Full_Screen)
        {
            while (ShowCursor(true) < 0);
            while (ShowCursor(false) >= 0);
        }
        DialogsOpen--;
        VDPRamHWnd = NULL;
        EndDialog(hDlg, true);
        return true;
    } break;
    }

    return false;
}