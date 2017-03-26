#include <stdio.h>
#include <vector>
#include <map>
#include "resource.h"
#include "g_input.h"
#include "joypads.h"
#include "g_main.h"
#include "movie.h"
#include "save.h" //Modif
#include "luascript.h"

#define KEYDOWN(key) (Keys[key] & 0x80)
#define MAX_JOYS 8

LPDIRECTINPUT lpDI;
LPDIRECTINPUTDEVICE lpDIDKeyboard;
LPDIRECTINPUTDEVICE lpDIDMouse;
char Phrase[1024];
int Nb_Joys = 0;
static IDirectInputDevice2 *Joy_ID[MAX_JOYS] = { NULL };
static DIJOYSTATE Joy_State[MAX_JOYS] = { { 0 } };
long MouseX, MouseY;
unsigned char Keys[256];
int Cur_Player; //Upth-Add - For the new key-redefinition dialogs
unsigned int DelayFactor = 5;
bool BackgroundInput = false;

struct ControlPadData
{
    // flags indicating controller type
    unsigned int& Type;

    // which buttons are pressed
    // 0 means "pressed" and 1 "not pressed"
    unsigned int& Up;
    unsigned int& Down;
    unsigned int& Left;
    unsigned int& Right;
    unsigned int& Start;
    unsigned int& Mode;
    unsigned int& A;
    unsigned int& B;
    unsigned int& C;
    unsigned int& X;
    unsigned int& Y;
    unsigned int& Z;

    // extra state for L+R/U+D logic
    unsigned int UpReally;
    unsigned int DownReally;
    unsigned int LeftReally;
    unsigned int RightReally;
}
controllerVariables[8] =
{
    { Controller_1_Type, Controller_1_Up, Controller_1_Down, Controller_1_Left, Controller_1_Right, Controller_1_Start, Controller_1_Mode, Controller_1_A, Controller_1_B, Controller_1_C, Controller_1_X, Controller_1_Y, Controller_1_Z, },
    { Controller_2_Type, Controller_2_Up, Controller_2_Down, Controller_2_Left, Controller_2_Right, Controller_2_Start, Controller_2_Mode, Controller_2_A, Controller_2_B, Controller_2_C, Controller_2_X, Controller_2_Y, Controller_2_Z, },
    { Controller_1B_Type, Controller_1B_Up, Controller_1B_Down, Controller_1B_Left, Controller_1B_Right, Controller_1B_Start, Controller_1B_Mode, Controller_1B_A, Controller_1B_B, Controller_1B_C, Controller_1B_X, Controller_1B_Y, Controller_1B_Z, },
    { Controller_1C_Type, Controller_1C_Up, Controller_1C_Down, Controller_1C_Left, Controller_1C_Right, Controller_1C_Start, Controller_1C_Mode, Controller_1C_A, Controller_1C_B, Controller_1C_C, Controller_1C_X, Controller_1C_Y, Controller_1C_Z, },
    { Controller_1D_Type, Controller_1D_Up, Controller_1D_Down, Controller_1D_Left, Controller_1D_Right, Controller_1D_Start, Controller_1D_Mode, Controller_1D_A, Controller_1D_B, Controller_1D_C, Controller_1D_X, Controller_1D_Y, Controller_1D_Z, },
    { Controller_2B_Type, Controller_2B_Up, Controller_2B_Down, Controller_2B_Left, Controller_2B_Right, Controller_2B_Start, Controller_2B_Mode, Controller_2B_A, Controller_2B_B, Controller_2B_C, Controller_2B_X, Controller_2B_Y, Controller_2B_Z, },
    { Controller_2C_Type, Controller_2C_Up, Controller_2C_Down, Controller_2C_Left, Controller_2C_Right, Controller_2C_Start, Controller_2C_Mode, Controller_2C_A, Controller_2C_B, Controller_2C_C, Controller_2C_X, Controller_2C_Y, Controller_2C_Z, },
    { Controller_2D_Type, Controller_2D_Up, Controller_2D_Down, Controller_2D_Left, Controller_2D_Right, Controller_2D_Start, Controller_2D_Mode, Controller_2D_A, Controller_2D_B, Controller_2D_C, Controller_2D_X, Controller_2D_Y, Controller_2D_Z, },
};

struct K_Def Keys_Def[8] = {
    { DIK_RETURN, DIK_RSHIFT,
    DIK_A, DIK_S, DIK_D,
    DIK_Z, DIK_X, DIK_C,
    DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT },
    { DIK_U, DIK_T,
    DIK_K, DIK_L, DIK_M,
    DIK_I, DIK_O, DIK_P,
    DIK_Y, DIK_H, DIK_G, DIK_J }
};
K_Def Temp_Keys; //Upth-Add - Stores the key definitions so that the user can cancel redefinition without changing them
static bool autoAlternator = false; //Nitsuja did this part, it's part of his AutoHold and AutoFire mod
#define MAKE_AUTO_KEY_VAR(x) unsigned int x##_Last = 1; bool x##_Just = 0; bool x##_Autofire = 0; bool x##_Autofire2 = 0; bool x##_Autohold = 0;
MAKE_AUTO_KEY_VAR(Controller_1_Up);
MAKE_AUTO_KEY_VAR(Controller_1_Down);
MAKE_AUTO_KEY_VAR(Controller_1_Left);
MAKE_AUTO_KEY_VAR(Controller_1_Right);
MAKE_AUTO_KEY_VAR(Controller_1_Start);
MAKE_AUTO_KEY_VAR(Controller_1_Mode);
MAKE_AUTO_KEY_VAR(Controller_1_A);
MAKE_AUTO_KEY_VAR(Controller_1_B);
MAKE_AUTO_KEY_VAR(Controller_1_C);
MAKE_AUTO_KEY_VAR(Controller_1_X);
MAKE_AUTO_KEY_VAR(Controller_1_Y);
MAKE_AUTO_KEY_VAR(Controller_1_Z);

MAKE_AUTO_KEY_VAR(Controller_1B_Up);
MAKE_AUTO_KEY_VAR(Controller_1B_Down);
MAKE_AUTO_KEY_VAR(Controller_1B_Left);
MAKE_AUTO_KEY_VAR(Controller_1B_Right);
MAKE_AUTO_KEY_VAR(Controller_1B_Start);
MAKE_AUTO_KEY_VAR(Controller_1B_Mode);
MAKE_AUTO_KEY_VAR(Controller_1B_A);
MAKE_AUTO_KEY_VAR(Controller_1B_B);
MAKE_AUTO_KEY_VAR(Controller_1B_C);
MAKE_AUTO_KEY_VAR(Controller_1B_X);
MAKE_AUTO_KEY_VAR(Controller_1B_Y);
MAKE_AUTO_KEY_VAR(Controller_1B_Z);

MAKE_AUTO_KEY_VAR(Controller_1C_Up);
MAKE_AUTO_KEY_VAR(Controller_1C_Down);
MAKE_AUTO_KEY_VAR(Controller_1C_Left);
MAKE_AUTO_KEY_VAR(Controller_1C_Right);
MAKE_AUTO_KEY_VAR(Controller_1C_Start);
MAKE_AUTO_KEY_VAR(Controller_1C_Mode);
MAKE_AUTO_KEY_VAR(Controller_1C_A);
MAKE_AUTO_KEY_VAR(Controller_1C_B);
MAKE_AUTO_KEY_VAR(Controller_1C_C);
MAKE_AUTO_KEY_VAR(Controller_1C_X);
MAKE_AUTO_KEY_VAR(Controller_1C_Y);
MAKE_AUTO_KEY_VAR(Controller_1C_Z);

MAKE_AUTO_KEY_VAR(Controller_1D_Up);
MAKE_AUTO_KEY_VAR(Controller_1D_Down);
MAKE_AUTO_KEY_VAR(Controller_1D_Left);
MAKE_AUTO_KEY_VAR(Controller_1D_Right);
MAKE_AUTO_KEY_VAR(Controller_1D_Start);
MAKE_AUTO_KEY_VAR(Controller_1D_Mode);
MAKE_AUTO_KEY_VAR(Controller_1D_A);
MAKE_AUTO_KEY_VAR(Controller_1D_B);
MAKE_AUTO_KEY_VAR(Controller_1D_C);
MAKE_AUTO_KEY_VAR(Controller_1D_X);
MAKE_AUTO_KEY_VAR(Controller_1D_Y);
MAKE_AUTO_KEY_VAR(Controller_1D_Z);

MAKE_AUTO_KEY_VAR(Controller_2_Up);
MAKE_AUTO_KEY_VAR(Controller_2_Down);
MAKE_AUTO_KEY_VAR(Controller_2_Left);
MAKE_AUTO_KEY_VAR(Controller_2_Right);
MAKE_AUTO_KEY_VAR(Controller_2_Start);
MAKE_AUTO_KEY_VAR(Controller_2_Mode);
MAKE_AUTO_KEY_VAR(Controller_2_A);
MAKE_AUTO_KEY_VAR(Controller_2_B);
MAKE_AUTO_KEY_VAR(Controller_2_C);
MAKE_AUTO_KEY_VAR(Controller_2_X);
MAKE_AUTO_KEY_VAR(Controller_2_Y);
MAKE_AUTO_KEY_VAR(Controller_2_Z);

MAKE_AUTO_KEY_VAR(Controller_2B_Up);
MAKE_AUTO_KEY_VAR(Controller_2B_Down);
MAKE_AUTO_KEY_VAR(Controller_2B_Left);
MAKE_AUTO_KEY_VAR(Controller_2B_Right);
MAKE_AUTO_KEY_VAR(Controller_2B_Start);
MAKE_AUTO_KEY_VAR(Controller_2B_Mode);
MAKE_AUTO_KEY_VAR(Controller_2B_A);
MAKE_AUTO_KEY_VAR(Controller_2B_B);
MAKE_AUTO_KEY_VAR(Controller_2B_C);
MAKE_AUTO_KEY_VAR(Controller_2B_X);
MAKE_AUTO_KEY_VAR(Controller_2B_Y);
MAKE_AUTO_KEY_VAR(Controller_2B_Z);

MAKE_AUTO_KEY_VAR(Controller_2C_Up);
MAKE_AUTO_KEY_VAR(Controller_2C_Down);
MAKE_AUTO_KEY_VAR(Controller_2C_Left);
MAKE_AUTO_KEY_VAR(Controller_2C_Right);
MAKE_AUTO_KEY_VAR(Controller_2C_Start);
MAKE_AUTO_KEY_VAR(Controller_2C_Mode);
MAKE_AUTO_KEY_VAR(Controller_2C_A);
MAKE_AUTO_KEY_VAR(Controller_2C_B);
MAKE_AUTO_KEY_VAR(Controller_2C_C);
MAKE_AUTO_KEY_VAR(Controller_2C_X);
MAKE_AUTO_KEY_VAR(Controller_2C_Y);
MAKE_AUTO_KEY_VAR(Controller_2C_Z);

MAKE_AUTO_KEY_VAR(Controller_2D_Up);
MAKE_AUTO_KEY_VAR(Controller_2D_Down);
MAKE_AUTO_KEY_VAR(Controller_2D_Left);
MAKE_AUTO_KEY_VAR(Controller_2D_Right);
MAKE_AUTO_KEY_VAR(Controller_2D_Start);
MAKE_AUTO_KEY_VAR(Controller_2D_Mode);
MAKE_AUTO_KEY_VAR(Controller_2D_A);
MAKE_AUTO_KEY_VAR(Controller_2D_B);
MAKE_AUTO_KEY_VAR(Controller_2D_C);
MAKE_AUTO_KEY_VAR(Controller_2D_X);
MAKE_AUTO_KEY_VAR(Controller_2D_Y);
MAKE_AUTO_KEY_VAR(Controller_2D_Z);

#define MOD_NONE 0
#define VK_NONE 0
#define ID_NONE 0

// TODO: add a "category" field and a way to filter the hotkey list by category

struct InputButton
{
    int modifiers; // ex: MOD_ALT | MOD_CONTROL | MOD_SHIFT

    int virtKey; // ex: VK_ESCAPE or 'O'
    WORD eventID; // send message on press

    int diKey; // ex: DIK_ESCAPE
    BOOL* alias; // set value = held

    const char* description; // for user display... feel free to change it
    const char* saveIDString; // for config file... please do not ever change these names or you will break backward compatibility

    BOOL heldNow;

    bool ShouldUseAccelerator() {
        return eventID && (virtKey > 0x07) && !(modifiers & MOD_WIN);
    }

    void CopyConfigurablePartsTo(InputButton& button) {
        button.modifiers = modifiers;
        button.virtKey = virtKey;
        button.diKey = diKey;
    }
    void SetAsDIK(int dik, int mods = 0) {
        modifiers = mods;
        virtKey = VK_NONE;
        diKey = dik;
    }
    void SetAsVirt(int virt, int mods = 0) {
        modifiers = mods;
        virtKey = virt;
        diKey = 0;
    }
};

static InputButton s_inputButtons[] =
{
    { MOD_CONTROL, 'O', ID_FILES_OPENROM, 0, NULL, "Open ROM", "OpenROMKey" },
    { MOD_CONTROL, 'C', ID_FILES_CLOSEROM, 0, NULL, "Close ROM", "CloseROMKey" },
    { MOD_NONE, VK_ESCAPE, ID_EMULATION_PAUSED, 0, NULL, "Pause/Unpause", "PauseKey0" },
    { MOD_NONE, VK_PAUSE, ID_EMULATION_PAUSED, 0, NULL, "Pause/Unpause (key 2)", "PauseKey" },
    { MOD_CONTROL | MOD_SHIFT, 'R', ID_CPU_RESET, 0, NULL, "Reset Game", "ResetKey" },

    //	{MOD_CONTROL|MOD_SHIFT, 'S',       ID_CPU_SOFTRESET,         0, NULL, "Soft Reset CPU", "SoftResetKey"},
    //	{MOD_CONTROL|MOD_SHIFT, 'P',       ID_CPU_ACCURATE_SYNCHRO,  0, NULL, "Accurate Sega CD CPU Sync On/Off", "AccurateSyncKey"},

    { MOD_CONTROL, 'B', ID_FILES_BOOTCD, 0, NULL, "Boot CD", "BootCDKey" },
    { MOD_CONTROL, 'V', ID_FILES_OPENCLOSECD, 0, NULL, "Open/Close CD", "OpenCloseCDKey" },

    { MOD_NONE, VK_NONE, ID_FILES_OPENRECENTROM0, 0, NULL, "Open Most Recent ROM", "OpenMostRecentROM" },
    { MOD_NONE, VK_NONE, ID_TOOLS_OPENRECENTMOVIE0, 0, NULL, "Open Most Recent Movie", "OpenMostRecentMovie" },
    { MOD_NONE, VK_NONE, ID_LUA_OPENRECENTSCRIPT0, 0, NULL, "Open Most Recent Lua Script", "OpenMostRecentLua" },

    { MOD_NONE, VK_TAB, ID_NONE, 0, &FastForwardKeyDown, "Fast Forward", "FastForwardKey" },
    { MOD_NONE, VK_OEM_5, ID_NONE, 0, &FrameAdvanceKeyDown, "Frame Advance", "SkipFrameKey" },
    { MOD_NONE, VK_NONE, ID_NONE, 0, &AutoFireKeyDown, "Auto-Fire Modifier", "AutoFireKey" },
    { MOD_NONE, VK_NONE, ID_NONE, 0, &AutoHoldKeyDown, "Auto-Hold Modifier", "AutoHoldKey" },
    { MOD_NONE, VK_OEM_3, ID_NONE, 0, &AutoClearKeyDown, "Clear Auto-Fire and Auto-Hold", "AutoClearKey" },

    { MOD_SHIFT, VK_TAB, ID_TOGGLE_TURBO, 0, NULL, "Toggle Fast Forward", "ToggleFastForwardKey" },
    { MOD_NONE, VK_OEM_COMMA, ID_TOGGLE_SHOWINPUT, 0, NULL, "Show Input", "ShowInputKey" },
    { MOD_NONE, VK_OEM_PERIOD, ID_TOGGLE_SHOWFRAMEANDLAGCOUNT, 0, NULL, "Show Frame/Lag Counter", "ShowFrameAndLagCounterKey" },
    { MOD_NONE, VK_NONE, ID_TOGGLE_TIMEUNIT, 0, NULL, "Toggle Counter Time Unit", "ToggleCounterTimeUnitKey" },
    { MOD_NONE, VK_NONE, ID_TOGGLE_SHOWFRAMECOUNT, 0, NULL, "Show Frame Counter", "ShowFrameCounterKey" },
    { MOD_NONE, VK_NONE, ID_TOGGLE_SHOWLAGCOUNT, 0, NULL, "Show Lag Counter", "ShowLagCounterKey" },
    { MOD_NONE, VK_NONE, ID_TOGGLE_SHOWFPS, 0, NULL, "Show Framerate", "ShowFPSKey" },
    { MOD_NONE, VK_NONE, ID_TOGGLE_SHOWLED, 0, NULL, "Show SegaCD LED", "ShowLEDKey" },

    { MOD_CONTROL, 'G', ID_FILES_GAMEGENIE, 0, NULL, "Game Genie Setup", "GameGenieKey" },

    { MOD_NONE, '1', ID_FILES_LOADSTATE_1, 0, NULL, "Load State 1", "Load1Key" },
    { MOD_NONE, '2', ID_FILES_LOADSTATE_2, 0, NULL, "Load State 2", "Load2Key" },
    { MOD_NONE, '3', ID_FILES_LOADSTATE_3, 0, NULL, "Load State 3", "Load3Key" },
    { MOD_NONE, '4', ID_FILES_LOADSTATE_4, 0, NULL, "Load State 4", "Load4Key" },
    { MOD_NONE, '5', ID_FILES_LOADSTATE_5, 0, NULL, "Load State 5", "Load5Key" },
    { MOD_NONE, '6', ID_FILES_LOADSTATE_6, 0, NULL, "Load State 6", "Load6Key" },
    { MOD_NONE, '7', ID_FILES_LOADSTATE_7, 0, NULL, "Load State 7", "Load7Key" },
    { MOD_NONE, '8', ID_FILES_LOADSTATE_8, 0, NULL, "Load State 8", "Load8Key" },
    { MOD_NONE, '9', ID_FILES_LOADSTATE_9, 0, NULL, "Load State 9", "Load9Key" },
    { MOD_NONE, '0', ID_FILES_LOADSTATE_0, 0, NULL, "Load State 0", "Load0Key" },

    { MOD_SHIFT, '1', ID_FILES_SAVESTATE_1, 0, NULL, "Save State 1", "Save1Key" },
    { MOD_SHIFT, '2', ID_FILES_SAVESTATE_2, 0, NULL, "Save State 2", "Save2Key" },
    { MOD_SHIFT, '3', ID_FILES_SAVESTATE_3, 0, NULL, "Save State 3", "Save3Key" },
    { MOD_SHIFT, '4', ID_FILES_SAVESTATE_4, 0, NULL, "Save State 4", "Save4Key" },
    { MOD_SHIFT, '5', ID_FILES_SAVESTATE_5, 0, NULL, "Save State 5", "Save5Key" },
    { MOD_SHIFT, '6', ID_FILES_SAVESTATE_6, 0, NULL, "Save State 6", "Save6Key" },
    { MOD_SHIFT, '7', ID_FILES_SAVESTATE_7, 0, NULL, "Save State 7", "Save7Key" },
    { MOD_SHIFT, '8', ID_FILES_SAVESTATE_8, 0, NULL, "Save State 8", "Save8Key" },
    { MOD_SHIFT, '9', ID_FILES_SAVESTATE_9, 0, NULL, "Save State 9", "Save9Key" },
    { MOD_SHIFT, '0', ID_FILES_SAVESTATE_0, 0, NULL, "Save State 0", "Save0Key" },

    { MOD_CONTROL, '1', ID_FILES_SETSTATE_1, 0, NULL, "Set State 1", "Set1Key" },
    { MOD_CONTROL, '2', ID_FILES_SETSTATE_2, 0, NULL, "Set State 2", "Set2Key" },
    { MOD_CONTROL, '3', ID_FILES_SETSTATE_3, 0, NULL, "Set State 3", "Set3Key" },
    { MOD_CONTROL, '4', ID_FILES_SETSTATE_4, 0, NULL, "Set State 4", "Set4Key" },
    { MOD_CONTROL, '5', ID_FILES_SETSTATE_5, 0, NULL, "Set State 5", "Set5Key" },
    { MOD_CONTROL, '6', ID_FILES_SETSTATE_6, 0, NULL, "Set State 6", "Set6Key" },
    { MOD_CONTROL, '7', ID_FILES_SETSTATE_7, 0, NULL, "Set State 7", "Set7Key" },
    { MOD_CONTROL, '8', ID_FILES_SETSTATE_8, 0, NULL, "Set State 8", "Set8Key" },
    { MOD_CONTROL, '9', ID_FILES_SETSTATE_9, 0, NULL, "Set State 9", "Set9Key" },
    { MOD_CONTROL, '0', ID_FILES_SETSTATE_0, 0, NULL, "Set State 0", "Set0Key" },

    { MOD_NONE, VK_F7, ID_FILES_NEXTSTATE, 0, NULL, "Set Next State", "SetNextKey" },
    { MOD_NONE, VK_F6, ID_FILES_PREVIOUSSTATE, 0, NULL, "Set Previous State", "SetPrevKey" },
    { MOD_NONE, VK_F8, ID_FILES_LOADSTATE, 0, NULL, "Load Current Savestate", "SaveCurrentKey" },
    { MOD_NONE, VK_NONE, ID_FILES_LOADSTATE, 0, NULL, "Load Current Savestate (key2)", "QuickLoadKey" },
    { MOD_NONE, VK_F5, ID_FILES_SAVESTATE, 0, NULL, "Save Current Savestate", "LoadCurrentKey" },
    { MOD_NONE, VK_NONE, ID_FILES_SAVESTATE, 0, NULL, "Save Current Savestate (key2)", "QuickSaveKey" },
    { MOD_NONE, VK_NONE, ID_FILES_SAVESTATE_10, 0, NULL, "Save Backup State", "Save11Key" },
    { MOD_SHIFT, VK_F8, ID_FILES_LOADSTATEAS, 0, NULL, "Load State From...", "LoadFromKey" },
    { MOD_SHIFT, VK_F5, ID_FILES_SAVESTATEAS, 0, NULL, "Save State As...", "SaveAsKey" },

    { MOD_NONE, VK_NONE, ID_FRAME_SEARCH_NEXT, 0, NULL, "Frame Search", "FrameSearchNextKey" },
    { MOD_NONE, VK_NONE, ID_FRAME_SEARCH_PREV, 0, NULL, "Frame Search Prev", "FrameSearchPrevKey" },
    { MOD_NONE, VK_NONE, ID_FRAME_SEARCH_END, 0, NULL, "Frame Search Result", "FrameSearchEndKey" },

    { MOD_CONTROL, VK_F1, ID_GRAPHICS_TOGGLEA, 0, NULL, "Toggle Scroll A", "ScrollAKey" },
    { MOD_CONTROL, VK_F2, ID_GRAPHICS_TOGGLEB, 0, NULL, "Toggle Scroll B", "ScrollBKey" },
    { MOD_CONTROL, VK_F3, ID_GRAPHICS_TOGGLES, 0, NULL, "Toggle Sprites", "SpritesKey" },
    { MOD_CONTROL, VK_F4, ID_GRAPHICS_PINKBG, 0, NULL, "Toggle Pink BG", "PinkBGKey" },

    { MOD_NONE, VK_F2, ID_GRAPHICS_FRAMESKIP_AUTO, 0, NULL, "Set Auto Frameskip", "AutoFrameskipKey" },
    { MOD_NONE, VK_F3, ID_GRAPHICS_FRAMESKIP_DECREASE, 0, NULL, "Decrease Frameskip", "PrevFrameskipKey" },
    { MOD_NONE, VK_F4, ID_GRAPHICS_FRAMESKIP_INCREASE, 0, NULL, "Increase Frameskip", "NextFrameskipKey" },

    { MOD_NONE, VK_F12, ID_GRAPHICS_NEXT_RENDER, 0, NULL, "Next Render Mode", "NextRenderKey" },
    { MOD_NONE, VK_F11, ID_GRAPHICS_PREVIOUS_RENDER, 0, NULL, "Previous Render Mode", "PrevRenderKey" },
    { MOD_ALT, VK_RETURN, ID_GRAPHICS_SWITCH_MODE, 0, NULL, "Fullscreen Mode On/Off", "FullscreenKey" },
    { MOD_SHIFT, VK_F2, ID_GRAPHICS_STRETCH, 0, NULL, "Stretch Graphics On/Off", "StretchKey" },
    { MOD_SHIFT, VK_F3, ID_GRAPHICS_VSYNC, 0, NULL, "VSync On/Off", "VSyncKey" },
    { MOD_SHIFT, VK_F9, ID_GRAPHICS_FORCESOFT, 0, NULL, "Force Software Blit On/Off", "SoftwareBlitKey" },
    { MOD_NONE, VK_F9, ID_OPTIONS_FASTBLUR, 0, NULL, "Motion Blur On/Off", "MotionBlurKey" },
    { MOD_NONE, VK_F10, ID_OPTIONS_SHOWFPS, 0, NULL, "Show Framerate On/Off", "ShowFPSKey" },

    { MOD_NONE, VK_OEM_MINUS, ID_SLOW_SPEED_MINUS, 0, NULL, "Decrease Speed", "SlowDownKey" },
    { MOD_NONE, VK_OEM_PLUS, ID_SLOW_SPEED_PLUS, 0, NULL, "Increase Speed", "SpeedUpKey" },
    { MOD_NONE, VK_NONE, ID_SLOW_MODE, 0, NULL, "Toggle Slow Mode", "ToggleSlowKey" },

    { MOD_NONE, VK_NONE, ID_LATENCY_COMPENSATION_0, 0, NULL, "Latency Compensation 0", "LatencyComp0" },
    { MOD_NONE, VK_NONE, ID_LATENCY_COMPENSATION_1, 0, NULL, "Latency Compensation 1", "LatencyComp1" },
    { MOD_NONE, VK_NONE, ID_LATENCY_COMPENSATION_2, 0, NULL, "Latency Compensation 2", "LatencyComp2" },
    { MOD_NONE, VK_NONE, ID_LATENCY_COMPENSATION_3, 0, NULL, "Latency Compensation 3", "LatencyComp3" },
    { MOD_NONE, VK_NONE, ID_LATENCY_COMPENSATION_4, 0, NULL, "Latency Compensation 4", "LatencyComp4" },
    //	{MOD_NONE,              VK_NONE,  ID_LATENCY_COMPENSATION_5, 0, NULL, "Latency Compensation 5", "LatencyComp5"},

    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_1, 0, NULL, "Lua Custom Hotkey 1", "LuaHotkey1" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_2, 0, NULL, "Lua Custom Hotkey 2", "LuaHotkey2" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_3, 0, NULL, "Lua Custom Hotkey 3", "LuaHotkey3" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_4, 0, NULL, "Lua Custom Hotkey 4", "LuaHotkey4" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_5, 0, NULL, "Lua Custom Hotkey 5", "LuaHotkey5" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_6, 0, NULL, "Lua Custom Hotkey 6", "LuaHotkey6" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_7, 0, NULL, "Lua Custom Hotkey 7", "LuaHotkey7" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_8, 0, NULL, "Lua Custom Hotkey 8", "LuaHotkey8" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_9, 0, NULL, "Lua Custom Hotkey 9", "LuaHotkey9" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_10, 0, NULL, "Lua Custom Hotkey 10", "LuaHotkey10" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_11, 0, NULL, "Lua Custom Hotkey 11", "LuaHotkey11" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_12, 0, NULL, "Lua Custom Hotkey 12", "LuaHotkey12" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_13, 0, NULL, "Lua Custom Hotkey 13", "LuaHotkey13" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_14, 0, NULL, "Lua Custom Hotkey 14", "LuaHotkey14" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_15, 0, NULL, "Lua Custom Hotkey 15", "LuaHotkey15" },
    { MOD_NONE, VK_NONE, IDC_LUA_SCRIPT_HOTKEY_16, 0, NULL, "Lua Custom Hotkey 16", "LuaHotkey16" },

    { MOD_NONE, VK_NONE, ID_GRAPHICS_SPRITEALWAYS, 0, NULL, "Sprites On Top On/Off", "SpritesOnTopKey" },
    { MOD_NONE, VK_NONE, ID_CHANGE_PALLOCK, 0, NULL, "Lock/Unlock Palette", "LockPaletteKey" },

    { MOD_NONE, VK_NONE, ID_SOUND_DACIMPROV, 0, NULL, "Improved DAC On/Off", "ImpDACKey" },
    //	{MOD_NONE,             VK_NONE,    ID_SOUND_PSGIMPROV,       0, NULL, "Improved PSG On/Off", "ImpPSGKey"},
    { MOD_NONE, VK_NONE, ID_SOUND_YMIMPROV, 0, NULL, "Improved YM On/Off", "ImpYMKey" },
    { MOD_NONE, VK_NONE, ID_SOUND_SOFTEN, 0, NULL, "Sound Soften Filter On/Off", "SoundSoftenKey" },

    { MOD_NONE, VK_NONE, ID_OPTIONS_JOYPADSETTING, 0, NULL, "Configure Input", "ConfigInputKey" },
    { MOD_NONE, VK_NONE, ID_OPTIONS_GENERAL, 0, NULL, "Configure General", "ConfigGeneralKey" },
    { MOD_NONE, VK_NONE, ID_OPTIONS_CHANGEDIR, 0, NULL, "Configure Directories", "ConfigDirKey" },
    { MOD_NONE, VK_NONE, ID_OPTIONS_CHANGEFILES, 0, NULL, "Configure BIOS/Misc. Files", "ConfigFilesKey" },

    { MOD_CONTROL | MOD_SHIFT, '0', ID_MOVIE_CHANGETRACK_ALL, 0, NULL, "Enable All Tracks", "AllTracksKey" },
    { MOD_CONTROL | MOD_SHIFT, '1', ID_MOVIE_CHANGETRACK_1, 0, NULL, "Toggle Player Track 1", "Track1Key" },
    { MOD_CONTROL | MOD_SHIFT, '2', ID_MOVIE_CHANGETRACK_2, 0, NULL, "Toggle Player Track 2", "Track2Key" },
    { MOD_CONTROL | MOD_SHIFT, '3', ID_MOVIE_CHANGETRACK_3, 0, NULL, "Toggle Player Track 3", "Track3Key" },
    { MOD_SHIFT, VK_OEM_COMMA, ID_PREV_TRACK, 0, NULL, "Previous Player Track", "PrevTrackKey" },
    { MOD_SHIFT, VK_OEM_PERIOD, ID_NEXT_TRACK, 0, NULL, "Next Player Track", "NextTrackKey" },

    { MOD_NONE, VK_NONE, IDC_NEW_LUA_SCRIPT, 0, NULL, "New Lua Script Window", "OpenNewLuaScriptWindow" },
    { MOD_NONE, VK_NONE, IDC_CLOSE_LUA_SCRIPTS, 0, NULL, "Close Lua Script Windows", "CloseLuaScriptWindows" },
    { MOD_NONE, VK_NONE, ID_RAM_SEARCH, 0, NULL, "Ram Search", "RamSearchKey" },
    { MOD_NONE, VK_NONE, ID_RAM_WATCH, 0, NULL, "Ram Watch", "RamWatchKey" },
    { MOD_NONE, VK_NONE, ID_PLAY_MOVIE, 0, NULL, "Play Movie", "PlayMovieKey" },
    { MOD_NONE, VK_NONE, ID_RECORD_MOVIE, 0, NULL, "Record Movie", "RecordMovieKey" },
    { MOD_NONE, VK_NONE, ID_PLAY_FROM_START, 0, NULL, "Watch Movie From Beginning", "WatchFromBeginningKey" },
    { MOD_NONE, VK_NONE, ID_RESUME_RECORD, 0, NULL, "Resume Record Movie", "Resume RecordMovieKey" },
    { MOD_NONE, VK_NONE, ID_STOP_MOVIE, 0, NULL, "Stop Movie", "StopMovieKey" },
    { MOD_CONTROL, 'T', ID_TOGGLE_MOVIE_READONLY, 0, NULL, "Toggle Movie Read-Only", "ToggleReadOnlyKey" },
    { MOD_CONTROL, 'R', ID_LAG_RESET, 0, NULL, "Reset Lag Counter", "LagResetKey" },
    { MOD_SHIFT, 'S', ID_SPLICE, 0, NULL, "Splice Input", "SpliceInputKey" },
    { MOD_NONE, VK_NONE, IDC_SEEK_FRAME, 0, NULL, "Seek To Frame", "SeekToFrameKey" },
};

static inline int GetNumHotkeys(void)
{
    return sizeof(s_inputButtons) / sizeof(s_inputButtons[0]);
}

static std::map<WORD, int> s_reverseEventLookup;

static InputButton s_defaultInputButtons[sizeof(s_inputButtons) / sizeof(s_inputButtons[0])];
static bool defaultInputButtonsStored = false;
static void StoreDefaultInputButtons()
{
    if (!defaultInputButtonsStored)
    {
        memcpy(s_defaultInputButtons, s_inputButtons, sizeof(s_defaultInputButtons));
        defaultInputButtonsStored = true;
    }
}
static InputButton s_initialInputButtons[sizeof(s_inputButtons) / sizeof(s_inputButtons[0])];
static bool initialInputButtonsStored = false;
static void StoreInitialInputButtons()
{
    if (!initialInputButtonsStored)
    {
        memcpy(s_initialInputButtons, s_inputButtons, sizeof(s_initialInputButtons));
        initialInputButtonsStored = true;
    }
}

void BuildAccelerators(HACCEL& hAccelTable)
{
    if (hAccelTable)
        DestroyAcceleratorTable(hAccelTable);

    std::vector<ACCEL> accels;
    int numInputButtons = GetNumHotkeys();
    for (int i = 0; i < numInputButtons; i++)
    {
        InputButton& button = s_inputButtons[i];
        if (button.ShouldUseAccelerator())
        {
            // button can be expressed as a Windows accelerator
            ACCEL accel;

            accel.cmd = button.eventID;
            accel.key = button.virtKey;
            accel.fVirt = FVIRTKEY | FNOINVERT;
            if (button.modifiers & MOD_ALT)
                accel.fVirt |= FALT;
            if (button.modifiers & MOD_SHIFT)
                accel.fVirt |= FSHIFT;
            if (button.modifiers & MOD_CONTROL)
                accel.fVirt |= FCONTROL;

            accels.push_back(accel);
        }
        else
        {
            // button can't be expressed as a Windows accelerator
            // handle it in Update_Input()
        }
    }

    s_reverseEventLookup.clear();
    for (int i = 0; i < numInputButtons; i++)
        if (s_inputButtons[i].eventID && (s_inputButtons[i].diKey || s_inputButtons[i].virtKey || s_inputButtons[i].modifiers || !s_reverseEventLookup[s_inputButtons[i].eventID]))
            s_reverseEventLookup[s_inputButtons[i].eventID] = i + 1;

    if (!accels.empty())
        hAccelTable = CreateAcceleratorTable(&accels[0], accels.size());
    else
        hAccelTable = NULL;
}

static void SetButtonToOldDIKey(InputButton& button, int key)
{
    if (key)
    {
        if (button.diKey == DIK_PAUSE)
        {
            // special case this one since VK_PAUSE works much more reliably than DIK_PAUSE
            button.virtKey = VK_PAUSE;
            button.diKey = 0;
        }
        else
        {
            button.virtKey = VK_NONE;
            button.diKey = key;
        }
        button.modifiers = MOD_NONE;
    }
}

void SaveAccelerators(char *File_Name)
{
    char Str_Tmp[1024];
    int numInputButtons = GetNumHotkeys();
    for (int i = 0; i < numInputButtons; i++)
    {
        InputButton& button = s_inputButtons[i];
        wsprintf(Str_Tmp, "%d,%d,%d", button.diKey, button.modifiers, button.virtKey); // it's important that diKey comes first, since older versions only had that one
        WritePrivateProfileString("Input", button.saveIDString, Str_Tmp, File_Name);
    }
}

void LoadAccelerators(char *File_Name)
{
    StoreDefaultInputButtons();
    char Str_Tmp[1024];
    int numInputButtons = GetNumHotkeys();
    for (int i = 0; i < numInputButtons; i++)
    {
        InputButton& button = s_inputButtons[i];

        GetPrivateProfileString("Input", button.saveIDString, "", Str_Tmp, 1024, File_Name);
        if (Str_Tmp[0] != 0)
        {
            // found
            InputButton temp;
            int read = sscanf(Str_Tmp, "%d, %d, %d", &temp.diKey, &temp.modifiers, &temp.virtKey);
            if (read == 3)
            {
                temp.CopyConfigurablePartsTo(button);
            }
            else if (read == 1)
            {
                // must be from an older version of Gens, convert it
                SetButtonToOldDIKey(button, temp.diKey);
            }
        }
        else
        {
            // not found, if it's a savestate hotkey then make it honor the StateSelectCfg

            switch (button.eventID)
            {
            case ID_FILES_SAVESTATE_1:
            case ID_FILES_SAVESTATE_2:
            case ID_FILES_SAVESTATE_3:
            case ID_FILES_SAVESTATE_4:
            case ID_FILES_SAVESTATE_5:
            case ID_FILES_SAVESTATE_6:
            case ID_FILES_SAVESTATE_7:
            case ID_FILES_SAVESTATE_8:
            case ID_FILES_SAVESTATE_9:
            case ID_FILES_SAVESTATE_0:
                switch (StateSelectCfg)
                {
                case 0: case 5: button.modifiers = MOD_SHIFT; break;
                case 1: case 3: button.modifiers = MOD_NONE; break;
                case 2: case 4: button.modifiers = MOD_CONTROL; break;
                }
                break;

            case ID_FILES_LOADSTATE_1:
            case ID_FILES_LOADSTATE_2:
            case ID_FILES_LOADSTATE_3:
            case ID_FILES_LOADSTATE_4:
            case ID_FILES_LOADSTATE_5:
            case ID_FILES_LOADSTATE_6:
            case ID_FILES_LOADSTATE_7:
            case ID_FILES_LOADSTATE_8:
            case ID_FILES_LOADSTATE_9:
            case ID_FILES_LOADSTATE_0:
                switch (StateSelectCfg)
                {
                case 0: case 1: button.modifiers = MOD_CONTROL; break;
                case 2: case 3: button.modifiers = MOD_SHIFT; break;
                case 4: case 5: button.modifiers = MOD_NONE; break;
                }
                break;

            case ID_FILES_SETSTATE_1:
            case ID_FILES_SETSTATE_2:
            case ID_FILES_SETSTATE_3:
            case ID_FILES_SETSTATE_4:
            case ID_FILES_SETSTATE_5:
            case ID_FILES_SETSTATE_6:
            case ID_FILES_SETSTATE_7:
            case ID_FILES_SETSTATE_8:
            case ID_FILES_SETSTATE_9:
            case ID_FILES_SETSTATE_0:
                switch (StateSelectCfg)
                {
                case 0: case 2: button.modifiers = MOD_NONE; break;
                case 1: case 4: button.modifiers = MOD_SHIFT; break;
                case 3: case 5: button.modifiers = MOD_CONTROL; break;
                }
                break;
            }
        }
    }
}

static const char* alphabet = "A\0B\0C\0D\0E\0F\0G\0H\0I\0J\0K\0L\0M\0N\0O\0P\0Q\0R\0S\0T\0U\0V\0W\0X\0Y\0Z";
static const char* digits = "0" "\0" "1" "\0" "2" "\0" "3" "\0" "4" "\0" "5" "\0" "6" "\0" "7" "\0" "8" "\0" "9";

const char* GetVirtualKeyName(int key)
{
    if (key >= 'A' && key <= 'Z')
        return alphabet + 2 * (key - 'A');
    if (key >= '0' && key <= '9')
        return digits + 2 * (key - '0');

    switch (key)
    {
    case 0: return "None";
    case VK_LBUTTON: return "LeftClick";
    case VK_RBUTTON: return "RightClick";
    case VK_CANCEL: return "Cancel";
    case VK_MBUTTON: return "MiddleClick";
    case VK_BACK: return "Backspace";
    case VK_TAB: return "Tab";
    case VK_CLEAR: return "Clear";
    case VK_RETURN: return "Enter";
    case VK_SHIFT: return "Shift";
    case VK_CONTROL: return "Control";
    case VK_MENU: return "Alt";
    case VK_PAUSE: return "Pause";
    case VK_CAPITAL: return "CapsLock";
    case VK_KANA: return "Kana/Hangul";
    case VK_JUNJA: return "Junja";
    case VK_FINAL: return "Final";
    case VK_HANJA: return "Hanja/Kanji";
    case VK_ESCAPE: return "Escape";
    case VK_CONVERT: return "Convert";
    case VK_NONCONVERT: return "NoConvert";
    case VK_ACCEPT: return "Accept";
    case VK_MODECHANGE: return "Modechange";
    case VK_SPACE: return "Space";
    case VK_PRIOR: return "PageUp";
    case VK_NEXT: return "PageDown";
    case VK_END: return "End";
    case VK_HOME: return "Home";
    case VK_LEFT: return "Left";
    case VK_UP: return "Up";
    case VK_RIGHT: return "Right";
    case VK_DOWN: return "Down";
    case VK_SELECT: return "Select";
    case VK_PRINT: return "Print";
    case VK_EXECUTE: return "Execute";
    case VK_SNAPSHOT: return "PrintScreen";
    case VK_INSERT: return "Insert";
    case VK_DELETE: return "Delete";
    case VK_HELP: return "Help";
    case VK_LWIN: return "LWin";
    case VK_RWIN: return "RWin";
    case VK_APPS: return "Apps";
    case VK_SLEEP: return "Sleep";
    case VK_NUMPAD0: return "Numpad0";
    case VK_NUMPAD1: return "Numpad1";
    case VK_NUMPAD2: return "Numpad2";
    case VK_NUMPAD3: return "Numpad3";
    case VK_NUMPAD4: return "Numpad4";
    case VK_NUMPAD5: return "Numpad5";
    case VK_NUMPAD6: return "Numpad6";
    case VK_NUMPAD7: return "Numpad7";
    case VK_NUMPAD8: return "Numpad8";
    case VK_NUMPAD9: return "Numpad9";
    case VK_MULTIPLY: return "Numpad*";
    case VK_ADD: return "Numpad+";
    case VK_SEPARATOR: return "Separator";
    case VK_SUBTRACT: return "Numpad-";
    case VK_DECIMAL: return "Numpad.";
    case VK_DIVIDE: return "Numpad/";
    case VK_F1: return "F1";
    case VK_F2: return "F2";
    case VK_F3: return "F3";
    case VK_F4: return "F4";
    case VK_F5: return "F5";
    case VK_F6: return "F6";
    case VK_F7: return "F7";
    case VK_F8: return "F8";
    case VK_F9: return "F9";
    case VK_F10: return "F10";
    case VK_F11: return "F11";
    case VK_F12: return "F12";
    case VK_F13: return "F13";
    case VK_F14: return "F14";
    case VK_F15: return "F15";
    case VK_F16: return "F16";
    case VK_F17: return "F17";
    case VK_F18: return "F18";
    case VK_F19: return "F19";
    case VK_F20: return "F20";
    case VK_F21: return "F21";
    case VK_F22: return "F22";
    case VK_F23: return "F23";
    case VK_F24: return "F24";
    case VK_NUMLOCK: return "NumLock";
    case VK_SCROLL: return "ScrollLock";
    case VK_OEM_1: return ";:";
    case VK_OEM_PLUS: return "=+";
    case VK_OEM_COMMA: return ",<";
    case VK_OEM_MINUS: return "-_";
    case VK_OEM_PERIOD: return ".>";
    case VK_OEM_2: return "/?";
    case VK_OEM_3: return "`~";
    case VK_OEM_4: return "[{";
    case VK_OEM_5: return "\\|";
    case VK_OEM_6: return "]}";
    case VK_OEM_7: return "'\"";
    case VK_OEM_8: return "OEM_8";
    case VK_LSHIFT: return "LShift";
    case VK_RSHIFT: return "RShift";
    case VK_LCONTROL: return "LControl";
    case VK_RCONTROL: return "RControl";
    case VK_LMENU: return "LAlt";
    case VK_RMENU: return "RAlt";

    default:
        static char unk[8];
        sprintf(unk, "0x%X", key);
        return unk;
    }
}

static const char* GetDirectInputKeyName(int key)
{
    switch (key)
    {
    case 0: return "None";
    case DIK_ESCAPE: return "Escape";
    case DIK_1: return "1";
    case DIK_2: return "2";
    case DIK_3: return "3";
    case DIK_4: return "4";
    case DIK_5: return "5";
    case DIK_6: return "6";
    case DIK_7: return "7";
    case DIK_8: return "8";
    case DIK_9: return "9";
    case DIK_0: return "0";
    case DIK_MINUS: return "-_";
    case DIK_EQUALS: return "=+";
    case DIK_BACK: return "Backspace";
    case DIK_TAB: return "Tab";
    case DIK_Q: return "Q";
    case DIK_W: return "W";
    case DIK_E: return "E";
    case DIK_R: return "R";
    case DIK_T: return "T";
    case DIK_Y: return "Y";
    case DIK_U: return "U";
    case DIK_I: return "I";
    case DIK_O: return "O";
    case DIK_P: return "P";
    case DIK_LBRACKET: return "[{";
    case DIK_RBRACKET: return "]}";
    case DIK_RETURN: return "Enter";
    case DIK_LCONTROL: return "LControl";
    case DIK_A: return "A";
    case DIK_S: return "S";
    case DIK_D: return "D";
    case DIK_F: return "F";
    case DIK_G: return "G";
    case DIK_H: return "H";
    case DIK_J: return "J";
    case DIK_K: return "K";
    case DIK_L: return "L";
    case DIK_SEMICOLON: return ";:";
    case DIK_APOSTROPHE: return "'\"";
    case DIK_GRAVE: return "`~";
    case DIK_LSHIFT: return "LShift";
    case DIK_BACKSLASH: return "\\|";
    case DIK_Z: return "Z";
    case DIK_X: return "X";
    case DIK_C: return "C";
    case DIK_V: return "V";
    case DIK_B: return "B";
    case DIK_N: return "N";
    case DIK_M: return "M";
    case DIK_COMMA: return ",<";
    case DIK_PERIOD: return ".>";
    case DIK_SLASH: return "/?";
    case DIK_RSHIFT: return "RShift";
    case DIK_MULTIPLY: return "Numpad*";
    case DIK_LMENU: return "LAlt";
    case DIK_SPACE: return "Space";
    case DIK_CAPITAL: return "CapsLock";
    case DIK_F1: return "F1";
    case DIK_F2: return "F2";
    case DIK_F3: return "F3";
    case DIK_F4: return "F4";
    case DIK_F5: return "F5";
    case DIK_F6: return "F6";
    case DIK_F7: return "F7";
    case DIK_F8: return "F8";
    case DIK_F9: return "F9";
    case DIK_F10: return "F10";
    case DIK_NUMLOCK: return "NumLock";
    case DIK_SCROLL: return "ScrollLock";
    case DIK_NUMPAD7: return "Numpad7";
    case DIK_NUMPAD8: return "Numpad8";
    case DIK_NUMPAD9: return "Numpad9";
    case DIK_SUBTRACT: return "Numpad-";
    case DIK_NUMPAD4: return "Numpad4";
    case DIK_NUMPAD5: return "Numpad5";
    case DIK_NUMPAD6: return "Numpad6";
    case DIK_ADD: return "Numpad+";
    case DIK_NUMPAD1: return "Numpad1";
    case DIK_NUMPAD2: return "Numpad2";
    case DIK_NUMPAD3: return "Numpad3";
    case DIK_NUMPAD0: return "Numpad0";
    case DIK_DECIMAL: return "Numpad.";
    case DIK_OEM_102: return "<>\\|";
    case DIK_F11: return "F11";
    case DIK_F12: return "F12";
    case DIK_F13: return "F13";
    case DIK_F14: return "F14";
    case DIK_F15: return "F15";
    case DIK_KANA: return "Kana";
    case DIK_ABNT_C1: return "/?";
    case DIK_CONVERT: return "Convert";
    case DIK_NOCONVERT: return "NoConvert";
    case DIK_YEN: return "Yen";
    case DIK_ABNT_C2: return "Numpad.";
    case DIK_NUMPADEQUALS: return "Numpad=";
    case DIK_PREVTRACK: return "Prevtrack";
    case DIK_AT: return "@";
    case DIK_COLON: return ":";
    case DIK_UNDERLINE: return "_";
    case DIK_KANJI: return "Kanji";
    case DIK_STOP: return "Stop";
    case DIK_AX: return "AX";
    case DIK_UNLABELED: return "Unlabeled";
    case DIK_NEXTTRACK: return "Nexttrack";
    case DIK_NUMPADENTER: return "NumpadEnter";
    case DIK_RCONTROL: return "RControl";
    case DIK_MUTE: return "Mute";
    case DIK_CALCULATOR: return "Calculator";
    case DIK_PLAYPAUSE: return "PlayPause";
    case DIK_MEDIASTOP: return "MediaStop";
    case DIK_VOLUMEDOWN: return "VolumeDown";
    case DIK_VOLUMEUP: return "VolumeUp";
    case DIK_WEBHOME: return "WebHome";
    case DIK_NUMPADCOMMA: return "Numpad,";
    case DIK_DIVIDE: return "Numpad/";
    case DIK_SYSRQ: return "Sysrq";
    case DIK_RMENU: return "RAlt";
    case DIK_PAUSE: return "Pause";
    case DIK_HOME: return "Home";
    case DIK_UP: return "Up";
    case DIK_PRIOR: return "PageUp";
    case DIK_LEFT: return "Left";
    case DIK_RIGHT: return "Right";
    case DIK_END: return "End";
    case DIK_DOWN: return "Down";
    case DIK_NEXT: return "PageDown";
    case DIK_INSERT: return "Insert";
    case DIK_DELETE: return "Delete";
    case DIK_LWIN: return "LWin";
    case DIK_RWIN: return "RWin";
    case DIK_APPS: return "Apps";
    case DIK_POWER: return "Power";
    case DIK_SLEEP: return "Sleep";
    case DIK_WAKE: return "Wake";
    case DIK_WEBSEARCH: return "WebSearch";
    case DIK_WEBFAVORITES: return "WebFavorites";
    case DIK_WEBREFRESH: return "WebRefresh";
    case DIK_WEBSTOP: return "WebStop";
    case DIK_WEBFORWARD: return "WebForward";
    case DIK_WEBBACK: return "WebBack";
    case DIK_MYCOMPUTER: return "MyComputer";
    case DIK_MAIL: return "Mail";
    case DIK_MEDIASELECT: return "MediaSelect";

    default:
        if (key > 0x100)
        {
            static char joy[64];
            sprintf(joy, "Pad%d", ((key >> 8) & 0xF) + 1);
            char Key[32];
            if (key & 0x80)
            {
                switch (key & 0xF)
                {
                case 1:
                    sprintf(Key, "Povhat%dUp", ((key >> 4) & 0x3) + 1);
                    break;
                case 2:
                    sprintf(Key, "Povhat%dRight", ((key >> 4) & 0x3) + 1);
                    break;
                case 3:
                    sprintf(Key, "Povhat%dDown", ((key >> 4) & 0x3) + 1);
                    break;
                case 4:
                    sprintf(Key, "Povhat%dLeft", ((key >> 4) & 0x3) + 1);
                    break;
                default:
                    sprintf(Key, "Povhat%dunknown 0x%X", ((key >> 4) & 0x3) + 1, key & 0xF);
                    break;
                }
            }
            else if (key & 0x70)
                sprintf(Key, "Button%d", (key & 0xFF) - 0xF);
            else
            {
                switch (key & 0xF)
                {
                case 1:
                    sprintf(Key, "Up");
                    break;
                case 2:
                    sprintf(Key, "Down");
                    break;
                case 3:
                    sprintf(Key, "Left");
                    break;
                case 4:
                    sprintf(Key, "Right");
                    break;
                case 5:
                    sprintf(Key, "RLeft");
                    break;
                case 6:
                    sprintf(Key, "RRight");
                    break;
                case 7:
                    sprintf(Key, "RUp");
                    break;
                case 8:
                    sprintf(Key, "RDown");
                    break;
                case 9:
                    sprintf(Key, "ZRight");
                    break;
                case 0xA:
                    sprintf(Key, "ZLeft");
                    break;

                default:
                    sprintf(Key, "undefined 0x%X", key & 0xF);
                }
            }
            strcat(joy, Key);
            return joy;
        }
        static char unk[8];
        sprintf(unk, "0x%X", key);
        return unk;
    }
}

void AddHotkeySuffix(char* str, InputButton& button)
{
    if (!button.modifiers && !button.virtKey && !button.diKey)
        return;

    strcat(str, "\t");

    //#define MODIFIER_SEPARATOR "+"
#define MODIFIER_SEPARATOR " "

    if (button.modifiers & MOD_CONTROL)
        strcat(str, "Ctrl" MODIFIER_SEPARATOR);
    if (button.modifiers & MOD_SHIFT)
        strcat(str, "Shift" MODIFIER_SEPARATOR);
    if (button.modifiers & MOD_ALT)
        strcat(str, "Alt" MODIFIER_SEPARATOR);
    if (button.modifiers & MOD_WIN)
        strcat(str, "Win" MODIFIER_SEPARATOR);

    if (button.virtKey)
        strcat(str, GetVirtualKeyName(button.virtKey));
    else if (button.diKey)
        strcat(str, GetDirectInputKeyName(button.diKey));
}

void AddHotkeySuffix(char* str, int id, const char* defaultSuffix)
{
    if (id & 0xFFFF0000)
        return; // ignore non-WORD menu IDs

    int index = s_reverseEventLookup[id] - 1;
    if (index < 0)
    {
        strcat(str, "\t");
        if (defaultSuffix)
            strcat(str, defaultSuffix);
        return;
    }

    AddHotkeySuffix(str, s_inputButtons[index]);
}

void PopulateHotkeyListbox(HWND listbox)
{
    StoreDefaultInputButtons();
    int numInputButtons = GetNumHotkeys();
    for (int i = 0; i < numInputButtons; i++)
    {
        InputButton& button = s_inputButtons[i];

        char str[1024];
        strcpy(str, button.description);
        AddHotkeySuffix(str, button);

        SendMessage((HWND)listbox, (UINT)LB_ADDSTRING, (WPARAM)0, (LPARAM)str);
    }
}

void Get_Key_2(InputButton& button, bool allowVirtual);
static void SetKey(char* message, InputButton& button, HWND hset)
{
    if (!lpDI && !Init_Input(ghInstance, hset))
        MessageBox(NULL, "I failed to initialize the input.", "Notice", MB_OK);

    for (int i = 0; i < 256; i++)
        Keys[i] &= ~0x80;

    Get_Key_2(button, true);

    MSG m;
    while (PeekMessage(&m, hset, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE));
    while (PeekMessage(&m, hset, WM_LBUTTONDOWN, WM_MBUTTONDBLCLK, PM_REMOVE));
}

void ModifyHotkeyFromListbox(HWND listbox, WORD command, HWND statusText, HWND parentWindow)
{
    StoreInitialInputButtons();

    bool rebuildAccelerators = false;

    int numHotkeys = GetNumHotkeys();
    for (int i = 0; i < numHotkeys; i++)
    {
        int selected = SendMessage((HWND)listbox, (UINT)LB_GETSEL, (WPARAM)i, (LPARAM)0);
        if (selected <= 0)
            continue;

        InputButton& button = s_inputButtons[i];

        if (button.ShouldUseAccelerator())
            rebuildAccelerators = true;

        switch (command)
        {
        case IDC_REASSIGNKEY:
        {
            char str[256];
            sprintf(str, "SETTING KEY: %s", button.description);
            SetWindowText(statusText, str);
            SetKey(str, button, parentWindow);
            SetWindowText(statusText, "");

            // for convenience, set all similar savestate buttons together when the first one is set to certain keys
            if (button.virtKey == '1' || button.virtKey == VK_F1)
            {
                if (button.eventID == ID_FILES_SAVESTATE_1 || button.eventID == ID_FILES_LOADSTATE_1 || button.eventID == ID_FILES_SETSTATE_1)
                {
                    for (int j = 1; j <= 9; j++)
                    {
                        int index2 = s_reverseEventLookup[button.eventID + j] - 1;
                        if (index2 >= 0)
                        {
                            InputButton& otherButton = s_inputButtons[index2];

                            otherButton.diKey = 0;
                            otherButton.modifiers = button.modifiers;
                            int vk = button.virtKey + j; if (vk == '1' + 9) vk = '0';
                            otherButton.virtKey = vk;

                            char str[1024];
                            strcpy(str, otherButton.description);
                            AddHotkeySuffix(str, otherButton);

                            SendMessage(listbox, LB_DELETESTRING, index2, 0);
                            SendMessage(listbox, LB_INSERTSTRING, index2, (LPARAM)str);
                        }
                    }
                }
            }
        }
        break;
        case IDC_REVERTKEY:
            s_initialInputButtons[i].CopyConfigurablePartsTo(button);
            break;
        case IDC_USEDEFAULTKEY:
            s_defaultInputButtons[i].CopyConfigurablePartsTo(button);
            break;
        case IDC_DISABLEKEY:
            button.modifiers = MOD_NONE;
            button.virtKey = VK_NONE;
            button.diKey = 0;
            break;
        }

        if (button.ShouldUseAccelerator())
            rebuildAccelerators = true;

        char str[1024];
        strcpy(str, button.description);
        AddHotkeySuffix(str, button);

        SendMessage(listbox, LB_DELETESTRING, i, 0);
        SendMessage(listbox, LB_INSERTSTRING, i, (LPARAM)str);
        SendMessage(listbox, LB_SETSEL, (WPARAM)TRUE, (LPARAM)i);
    }

    if (rebuildAccelerators)
    {
        extern HACCEL hAccelTable;
        BuildAccelerators(hAccelTable);
    }
}

void End_Input()
{
    int i;

    if (lpDI)
    {
        if (lpDIDMouse)
        {
            lpDIDMouse->Release();
            lpDIDMouse = NULL;
        }

        if (lpDIDKeyboard)
        {
            lpDIDKeyboard->Release();
            lpDIDKeyboard = NULL;
        }

        for (i = 0; i < MAX_JOYS; i++)
        {
            if (Joy_ID[i])
            {
                Joy_ID[i]->Unacquire();
                Joy_ID[i]->Release();
            }
        }

        Nb_Joys = 0;
        lpDI->Release();
        lpDI = NULL;
    }
}

BOOL CALLBACK InitJoystick(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef)
{
    HRESULT rval;
    LPDIRECTINPUTDEVICE	lpDIJoy;
    DIPROPRANGE diprg;
    int i;

    if (Nb_Joys >= MAX_JOYS) return(DIENUM_STOP);

    Joy_ID[Nb_Joys] = NULL;

    rval = lpDI->CreateDevice(lpDIIJoy->guidInstance, &lpDIJoy, NULL);
    if (rval != DI_OK)
    {
        MessageBox(HWnd, "IDirectInput::CreateDevice FAILED", "erreur joystick", MB_OK);
        return(DIENUM_CONTINUE);
    }

    rval = lpDIJoy->QueryInterface(IID_IDirectInputDevice2, (void **)&Joy_ID[Nb_Joys]);
    lpDIJoy->Release();
    if (rval != DI_OK)
    {
        MessageBox(HWnd, "IDirectInputDevice2::QueryInterface FAILED", "erreur joystick", MB_OK);
        Joy_ID[Nb_Joys] = NULL;
        return(DIENUM_CONTINUE);
    }

    rval = Joy_ID[Nb_Joys]->SetDataFormat(&c_dfDIJoystick);
    if (rval != DI_OK)
    {
        MessageBox(HWnd, "IDirectInputDevice::SetDataFormat FAILED", "erreur joystick", MB_OK);
        Joy_ID[Nb_Joys]->Release();
        Joy_ID[Nb_Joys] = NULL;
        return(DIENUM_CONTINUE);
    }

    rval = Joy_ID[Nb_Joys]->SetCooperativeLevel((HWND)pvRef, DISCL_NONEXCLUSIVE | (BackgroundInput ? DISCL_BACKGROUND : DISCL_FOREGROUND));

    if (rval != DI_OK)
    {
        MessageBox(HWnd, "IDirectInputDevice::SetCooperativeLevel FAILED", "erreur joystick", MB_OK);
        Joy_ID[Nb_Joys]->Release();
        Joy_ID[Nb_Joys] = NULL;
        return(DIENUM_CONTINUE);
    }

    diprg.diph.dwSize = sizeof(diprg);
    diprg.diph.dwHeaderSize = sizeof(diprg.diph);
    diprg.diph.dwObj = DIJOFS_X;
    diprg.diph.dwHow = DIPH_BYOFFSET;
    diprg.lMin = -1000;
    diprg.lMax = +1000;

    rval = Joy_ID[Nb_Joys]->SetProperty(DIPROP_RANGE, &diprg.diph);
    if ((rval != DI_OK) && (rval != DI_PROPNOEFFECT))
        MessageBox(HWnd, "IDirectInputDevice::SetProperty() (X-Axis) FAILED", "erreur joystick", MB_OK);

    diprg.diph.dwSize = sizeof(diprg);
    diprg.diph.dwHeaderSize = sizeof(diprg.diph);
    diprg.diph.dwObj = DIJOFS_Y;
    diprg.diph.dwHow = DIPH_BYOFFSET;
    diprg.lMin = -1000;
    diprg.lMax = +1000;

    rval = Joy_ID[Nb_Joys]->SetProperty(DIPROP_RANGE, &diprg.diph);
    if ((rval != DI_OK) && (rval != DI_PROPNOEFFECT))
        MessageBox(HWnd, "IDirectInputDevice::SetProperty() (Y-Axis) FAILED", "erreur joystick", MB_OK);

    for (i = 0; i < 10; i++)
    {
        rval = Joy_ID[Nb_Joys]->Acquire();
        if (rval == DI_OK) break;
        Sleep(10);
    }

    Nb_Joys++;

    return(DIENUM_CONTINUE);
}

int Init_Input(HINSTANCE hInst, HWND hWnd)
{
    int i;
    HRESULT rval;

    End_Input();

    StoreDefaultInputButtons();

    rval = DirectInputCreate(hInst, DIRECTINPUT_VERSION, &lpDI, NULL);
    if (rval != DI_OK)
    {
        MessageBox(hWnd, "DirectInput failed ...You must have DirectX 5", "Error", MB_OK);
        return 0;
    }

    Nb_Joys = 0;

    for (i = 0; i < MAX_JOYS; i++) Joy_ID[i] = NULL;

    rval = lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, &InitJoystick, hWnd, DIEDFL_ATTACHEDONLY);
    if (rval != DI_OK) return 0;

    //	rval = lpDI->CreateDevice(GUID_SysMouse, &lpDIDMouse, NULL);
    rval = lpDI->CreateDevice(GUID_SysKeyboard, &lpDIDKeyboard, NULL);
    if (rval != DI_OK) return 0;

    //	rval = lpDIDMouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    rval = lpDIDKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | (BackgroundInput ? DISCL_BACKGROUND : DISCL_FOREGROUND));
    if (rval != DI_OK) return 0;

    //	rval = lpDIDMouse->SetDataFormat(&c_dfDIMouse);
    rval = lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
    if (rval != DI_OK) return 0;

    //	rval = lpDIDMouse->Acquire();
    for (i = 0; i < 10; i++)
    {
        rval = lpDIDKeyboard->Acquire();
        if (rval == DI_OK) break;
        Sleep(10);
    }

    return 1;
}

void Restore_Input()
{
    //	lpDIDMouse->Acquire();
    lpDIDKeyboard->Acquire();
}

void Update_Input()
{
    //	DIMOUSESTATE MouseState;
    HRESULT rval;
    int i;

    rval = lpDIDKeyboard->GetDeviceState(256, &Keys);

    if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
    {
        Restore_Input();

        rval = lpDIDKeyboard->GetDeviceState(256, &Keys);
        if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
        {
            memset(Keys, 0, sizeof(Keys));
        }
    }

    for (i = 0; i < Nb_Joys; i++)
    {
        if (Joy_ID[i])
        {
            Joy_ID[i]->Poll();
            rval = Joy_ID[i]->GetDeviceState(sizeof(Joy_State[i]), &Joy_State[i]);
            if (rval != DI_OK) Joy_ID[i]->Acquire();
        }
    }

    //	rval = lpDIDMouse->GetDeviceState(sizeof(MouseState), &MouseState);

    //	if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
    //		Restore_Input();

    //  MouseX = MouseState.lX;
    //  MouseY = MouseState.lY;

    int numInputButtons = GetNumHotkeys();
    for (int i = 0; i < numInputButtons; i++)
    {
        InputButton& button = s_inputButtons[i];

        BOOL pressed = button.diKey ? Check_Key_Pressed(button.diKey) : FALSE;

        if (button.virtKey || button.modifiers)
        {
            bool pressed2 = button.virtKey ? !!(GetAsyncKeyState(button.virtKey) & 0x8000) : true;

            pressed2 &= !(button.modifiers & MOD_CONTROL) == !(GetAsyncKeyState(VK_CONTROL) & 0x8000);
            pressed2 &= !(button.modifiers & MOD_SHIFT) == !(GetAsyncKeyState(VK_SHIFT) & 0x8000);
            pressed2 &= !(button.modifiers & MOD_ALT) == !(GetAsyncKeyState(VK_MENU) & 0x8000);
            pressed2 &= !(button.modifiers & MOD_WIN) == !((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000);

            if (!button.diKey)
                pressed = TRUE;

            if (!pressed2)
                pressed = FALSE;
        }

        if (button.alias)
            *button.alias = pressed;

        BOOL oldPressed = button.heldNow;
        button.heldNow = pressed;

        if (pressed && !oldPressed && button.eventID && ((BackgroundInput && !(GetActiveWindow() == HWnd)) || !button.ShouldUseAccelerator()))
            SendMessage(HWnd, WM_COMMAND, button.eventID, 0);
    }
}

int Check_Key_Pressed(unsigned int key)
{
    int Num_Joy;

    if (key < 0x100)
    {
        if KEYDOWN(key)
            return(1);
    }
    else
    {
        Num_Joy = ((key >> 8) & 0xF);

        if (Joy_ID[Num_Joy])
        {
            if (key & 0x80)			// Test POV Joys
            {
                int value = Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3];
                if (value == -1) return (0);
                switch (key & 0xF)
                {
                case 1:
                    if ((value >= 29250) || (value <= 6750))
                        return(1);
                    break;
                case 2:
                    if ((value >= 2250) && (value <= 15750))
                        return(1);
                    break;
                case 3:
                    if ((value >= 11250) && (value <= 24750))
                        return(1);
                    break;
                case 4:
                    if ((value >= 20250) && (value <= 33750))
                        return(1);
                    break;
                }
            }
            else if (key & 0x70)		// Test Button Joys
            {
                if (Joy_State[Num_Joy].rgbButtons[(key & 0xFF) - 0x10])
                    return(1);
            }
            else
            {
                switch (key & 0xF)
                {
                case 1:
                    if (Joy_State[Num_Joy].lY < -500)
                        return(1);
                    break;

                case 2:
                    if (Joy_State[Num_Joy].lY > +500)
                        return(1);
                    break;

                case 3:
                    if (Joy_State[Num_Joy].lX < -500)
                        return(1);
                    break;

                case 4:
                    if (Joy_State[Num_Joy].lX > +500)
                        return(1);
                    break;

                case 5:
                    if (Joy_State[Num_Joy].lRx < 0x3FFF)
                        return(1);
                    break;
                case 6:
                    if (Joy_State[Num_Joy].lRx > 0xBFFF)
                        return(1);
                    break;
                case 7:
                    if (Joy_State[Num_Joy].lRy < 0x3FFF)
                        return(1);
                    break;
                case 8:
                    if (Joy_State[Num_Joy].lRy > 0xBFFF)
                        return(1);
                    break;
                case 9:
                    if (Joy_State[Num_Joy].lZ < 0x3FFF)
                        return(1);
                    break;
                case 0xA:
                    if (Joy_State[Num_Joy].lZ > 0xBFFF)
                        return(1);
                    break;
                }
            }
        }
    }

    return 0;
}

void Get_Key_2(InputButton& button, bool allowVirtual)
{
    int i, j, joyIndex;

    bool prevReady = false;
    int curAlt, curCtrl, curSft, curWin = curSft = curCtrl = curAlt = 0;
    int prevAlt, prevCtrl, prevSft, prevWin = prevSft = prevCtrl = prevAlt = 0;

    int prevMod;
    BOOL prevDiKeys[256];
    BOOL prevVirtKeys[256];
    BOOL prevJoyKeys[256];

    int curMod;
    BOOL curDiKeys[256];
    BOOL curVirtKeys[256];
    BOOL curJoyKeys[256];

    while (1)
    {
        // compute the current state of all buttons
        {
            Update_Input();

            // current state of virtual windows keys
            for (i = 0; i < 256; i++)
                curVirtKeys[i] = (GetAsyncKeyState(i) & 0x8000);

            // current state of direct input keys
            for (i = 0; i < 256; i++)
                curDiKeys[i] = KEYDOWN(i);

            // current state of modifier keys
            curMod = 0;
            prevCtrl = curCtrl;
            prevSft = curSft;
            prevAlt = curAlt;
            prevWin = curWin;
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
                curMod |= MOD_CONTROL, curCtrl = (curDiKeys[DIK_LCONTROL] ? 1 : 0) | (curDiKeys[DIK_RCONTROL] ? 2 : 0);
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                curMod |= MOD_SHIFT, curSft = (curDiKeys[DIK_LSHIFT] ? 1 : 0) | (curDiKeys[DIK_RSHIFT] ? 2 : 0);
            if (GetAsyncKeyState(VK_MENU) & 0x8000)
                curMod |= MOD_ALT, curAlt = (curDiKeys[DIK_LMENU] ? 1 : 0) | (curDiKeys[DIK_RMENU] ? 2 : 0);
            if ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000)
                curMod |= MOD_WIN, curWin = (curDiKeys[DIK_LWIN] ? 1 : 0) | (curDiKeys[DIK_RWIN] ? 2 : 0);

            // current state of recognized buttons on joypad
            joyIndex = 0;
            for (i = 0; i < Nb_Joys; i++)
            {
                if (Joy_ID[i])
                {
                    curJoyKeys[joyIndex++] = (Joy_State[i].lY < -500);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lY > +500);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lX < -500);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lX > +500);
                    for (j = 0; j < 4; j++) {
                        curJoyKeys[joyIndex++] = (Joy_State[i].rgdwPOV[j] == 0);
                        curJoyKeys[joyIndex++] = (Joy_State[i].rgdwPOV[j] == 9000);
                        curJoyKeys[joyIndex++] = (Joy_State[i].rgdwPOV[j] == 18000);
                        curJoyKeys[joyIndex++] = (Joy_State[i].rgdwPOV[j] == 27000);
                    }
                    for (j = 0; j < 32; j++)
                        curJoyKeys[joyIndex++] = (Joy_State[i].rgbButtons[j]);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lRx < 0x3FFF);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lRx > 0xBFFF);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lRy < 0x3FFF);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lRy > 0xBFFF);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lZ < 0x3FFF);
                    curJoyKeys[joyIndex++] = (Joy_State[i].lZ > 0xBFFF);
                }
            }
        }

        // compare buttons against the previous state
        // to determine what is now pressed that wasn't already pressed before
        if (prevReady)
        {
            // check for new virtual key presses
            for (i = 1; i < 255; i++)
            {
                if (curVirtKeys[i] && !prevVirtKeys[i])
                {
                    if (allowVirtual)
                    {
                        if (i == VK_CONTROL || i == VK_SHIFT || i == VK_MENU || i == VK_LWIN || i == VK_RWIN || i == VK_LSHIFT || i == VK_RSHIFT || i == VK_LCONTROL || i == VK_RCONTROL || i == VK_LMENU || i == VK_RMENU)
                            continue;
                        button.SetAsVirt(i, curMod);
                        return;
                    }
                    else if (i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON || i == VK_CANCEL)
                    {
                        button.SetAsDIK(0);
                        return;
                    }
                }
            }

            // check for new direct input key presses
            for (i = 1; i < 255; i++)
            {
                if (curDiKeys[i] && !prevDiKeys[i])
                {
                    if (allowVirtual && (i == DIK_LWIN || i == DIK_RWIN || i == DIK_LSHIFT || i == DIK_RSHIFT || i == DIK_LCONTROL || i == DIK_RCONTROL || i == DIK_LMENU || i == DIK_RMENU))
                        continue;
                    button.SetAsDIK(i, curMod);
                    return;
                }
            }

            // check for modifier key releases
            // this allows a modifier key to be used as a hotkey on its own, as some people like to do
            // Upth-mod: fixed to bind only the specific key that was pressed, instead of either of the two.
            if (!curMod && prevMod)
            {
                switch (prevMod)
                {
                case MOD_ALT:
                    button.SetAsDIK((prevAlt == 2) ? DIK_RMENU : DIK_LMENU, curMod);	//for some reason, it won't read these if set as VirtKeys.
                    break;
                case MOD_CONTROL:
                    button.SetAsDIK((prevCtrl == 2) ? DIK_RCONTROL : DIK_LCONTROL, curMod);
                    break;
                case MOD_SHIFT:
                    button.SetAsDIK((prevSft == 2) ? DIK_RSHIFT : DIK_LSHIFT, curMod);
                    break;
                case MOD_WIN:
                    button.SetAsDIK((prevWin == 2) ? DIK_RWIN : DIK_LWIN, curMod);
                    break;
                }
                return;
            }
            if (curSft == 3) // both LSHIFT + RSHIFT held = assign them both to the hotkey
            {
                button.SetAsVirt(0, MOD_SHIFT); return;
            }
            if (curAlt == 3)
            {
                button.SetAsVirt(0, MOD_ALT); return;
            }
            if (curCtrl == 3)
            {
                button.SetAsVirt(0, MOD_CONTROL); return;
            }
            if (curWin == 3)
            {
                button.SetAsVirt(0, MOD_WIN); return;
            }

            // check for new recognized joypad button presses
            for (int index = 0; index < joyIndex; index++)
            {
                if (curJoyKeys[index] && !prevJoyKeys[index])
                {
                    int joyIndex2 = 0;
                    for (i = 0; i < Nb_Joys; i++)
                    {
                        if (Joy_ID[i])
                        {
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x1, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x2, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x3, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x4, curMod); return; }
                            for (j = 0; j < 4; j++) {
                                if (index == joyIndex2++) { button.SetAsDIK(0x1080 + (0x100 * i) + (0x10 * j) + 0x1, curMod); return; }
                                if (index == joyIndex2++) { button.SetAsDIK(0x1080 + (0x100 * i) + (0x10 * j) + 0x2, curMod); return; }
                                if (index == joyIndex2++) { button.SetAsDIK(0x1080 + (0x100 * i) + (0x10 * j) + 0x3, curMod); return; }
                                if (index == joyIndex2++) { button.SetAsDIK(0x1080 + (0x100 * i) + (0x10 * j) + 0x4, curMod); return; }
                            }
                            for (j = 0; j < 32; j++)
                                if (index == joyIndex2++) { button.SetAsDIK(0x1010 + (0x100 * i) + j, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x5, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x6, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x7, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x8, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0x9, curMod); return; }
                            if (index == joyIndex2++) { button.SetAsDIK(0x1000 + (0x100 * i) + 0xA, curMod); return; }
                        }
                    }
                }
            }
        }

        // update previous state
        memcpy(prevVirtKeys, curVirtKeys, sizeof(prevVirtKeys));
        memcpy(prevDiKeys, curDiKeys, sizeof(curDiKeys));
        memcpy(prevJoyKeys, curJoyKeys, sizeof(curJoyKeys));
        prevMod = curMod;
        prevReady = true;
    }
}

unsigned int Get_Key(void)
{
    InputButton tempButton;
    tempButton.diKey = 0;
    Get_Key_2(tempButton, false);
    return tempButton.diKey;
}

long long GetCurrentInputCondensed()
{
    long long a = Controller_1_Up | (Controller_1_Down << 1) | (Controller_1_Left << 2) | (Controller_1_Right << 3) | (Controller_1_A << 4) | (Controller_1_B << 5) | (Controller_1_C << 6) | (Controller_1_Start << 7);
    long long b = Controller_1B_Up | (Controller_1B_Down << 1) | (Controller_1B_Left << 2) | (Controller_1B_Right << 3) | (Controller_1B_A << 4) | (Controller_1B_B << 5) | (Controller_1B_C << 6) | (Controller_1B_Start << 7);
    long long c = Controller_1C_Up | (Controller_1C_Down << 1) | (Controller_1C_Left << 2) | (Controller_1C_Right << 3) | (Controller_1C_A << 4) | (Controller_1C_B << 5) | (Controller_1C_C << 6) | (Controller_1C_Start << 7);
    long long d = Controller_2_Up | (Controller_2_Down << 1) | (Controller_2_Left << 2) | (Controller_2_Right << 3) | (Controller_2_A << 4) | (Controller_2_B << 5) | (Controller_2_C << 6) | (Controller_2_Start << 7);
    long long e = Controller_1_X | (Controller_1_Y << 1) | (Controller_1_Z << 2) | (Controller_1_Mode << 3) | (Controller_2_X << 4) | (Controller_2_Y << 5) | (Controller_2_Z << 6) | (Controller_2_Mode << 7);
    return a | (b << 8) | (c << 16) | (d << 24) | (e << 32) | (0xFFFFFF0000000000);
}
static void SetCurrentInputCondensed(long long input)
{
    Controller_1_Up = (input & (1LL << 0)) ? 1 : 0;
    Controller_1_Down = (input & (1LL << 1)) ? 1 : 0;
    Controller_1_Left = (input & (1LL << 2)) ? 1 : 0;
    Controller_1_Right = (input & (1LL << 3)) ? 1 : 0;
    Controller_1_A = (input & (1LL << 4)) ? 1 : 0;
    Controller_1_B = (input & (1LL << 5)) ? 1 : 0;
    Controller_1_C = (input & (1LL << 6)) ? 1 : 0;
    Controller_1_Start = (input & (1LL << 7)) ? 1 : 0;
    Controller_1B_Up = (input & (1LL << (8 + 0))) ? 1 : 0;
    Controller_1B_Down = (input & (1LL << (8 + 1))) ? 1 : 0;
    Controller_1B_Left = (input & (1LL << (8 + 2))) ? 1 : 0;
    Controller_1B_Right = (input & (1LL << (8 + 3))) ? 1 : 0;
    Controller_1B_A = (input & (1LL << (8 + 4))) ? 1 : 0;
    Controller_1B_B = (input & (1LL << (8 + 5))) ? 1 : 0;
    Controller_1B_C = (input & (1LL << (8 + 6))) ? 1 : 0;
    Controller_1B_Start = (input & (1LL << (8 + 7))) ? 1 : 0;
    Controller_1C_Up = (input & (1LL << (16 + 0))) ? 1 : 0;
    Controller_1C_Down = (input & (1LL << (16 + 1))) ? 1 : 0;
    Controller_1C_Left = (input & (1LL << (16 + 2))) ? 1 : 0;
    Controller_1C_Right = (input & (1LL << (16 + 3))) ? 1 : 0;
    Controller_1C_A = (input & (1LL << (16 + 4))) ? 1 : 0;
    Controller_1C_B = (input & (1LL << (16 + 5))) ? 1 : 0;
    Controller_1C_C = (input & (1LL << (16 + 6))) ? 1 : 0;
    Controller_1C_Start = (input & (1LL << (16 + 7))) ? 1 : 0;
    Controller_2_Up = (input & (1LL << (24 + 0))) ? 1 : 0;
    Controller_2_Down = (input & (1LL << (24 + 1))) ? 1 : 0;
    Controller_2_Left = (input & (1LL << (24 + 2))) ? 1 : 0;
    Controller_2_Right = (input & (1LL << (24 + 3))) ? 1 : 0;
    Controller_2_A = (input & (1LL << (24 + 4))) ? 1 : 0;
    Controller_2_B = (input & (1LL << (24 + 5))) ? 1 : 0;
    Controller_2_C = (input & (1LL << (24 + 6))) ? 1 : 0;
    Controller_2_Start = (input & (1LL << (24 + 7))) ? 1 : 0;
    Controller_1_X = (input & (1LL << (32 + 0))) ? 1 : 0;
    Controller_1_Y = (input & (1LL << (32 + 1))) ? 1 : 0;
    Controller_1_Z = (input & (1LL << (32 + 2))) ? 1 : 0;
    Controller_1_Mode = (input & (1LL << (32 + 3))) ? 1 : 0;
    Controller_2_X = (input & (1LL << (32 + 4))) ? 1 : 0;
    Controller_2_Y = (input & (1LL << (32 + 5))) ? 1 : 0;
    Controller_2_Z = (input & (1LL << (32 + 6))) ? 1 : 0;
    Controller_2_Mode = (input & (1LL << (32 + 7))) ? 1 : 0;
}

long long s_lastInputCondensed = ~0;
long long s_nextInputCondensed = ~0;
long long s_nextInputMask = ~0;
bool s_nextInputCondensedSet = false;

long long GetLastInputCondensed()
{
    return s_lastInputCondensed;
}
void SetNextInputCondensed(long long input, long long mask)
{
    if (!s_nextInputCondensedSet)
    {
        s_nextInputCondensed = input;
        s_nextInputMask = mask;
        s_nextInputCondensedSet = true;
    }
    else
    {
        s_nextInputCondensed = (input & mask) | (s_nextInputCondensed & ~mask);
        s_nextInputMask |= mask;
    }
}

void Update_Controllers()
{
    Update_Input();

    for (int i = 0; i < 8; i++) // for each controller port
    {
        if ((i == 2 || i == 3 || i == 4) && !(Controller_1_Type & 0x10)) // TEAMPLAYER PORT 1
            continue; // not enabled
        if ((i == 5 || i == 6 || i == 7) && !(Controller_2_Type & 0x10)) // TEAMPLAYER PORT 2
            continue; // not enabled

        controllerVariables[i].Start = !Check_Key_Pressed(Keys_Def[i].Start);
        controllerVariables[i].A = !Check_Key_Pressed(Keys_Def[i].A);
        controllerVariables[i].B = !Check_Key_Pressed(Keys_Def[i].B);
        controllerVariables[i].C = !Check_Key_Pressed(Keys_Def[i].C);

        int threeButtonController = !(controllerVariables[i].Type & 1);
        controllerVariables[i].Mode = threeButtonController || !Check_Key_Pressed(Keys_Def[i].Mode);
        controllerVariables[i].X = threeButtonController || !Check_Key_Pressed(Keys_Def[i].X);
        controllerVariables[i].Y = threeButtonController || !Check_Key_Pressed(Keys_Def[i].Y);
        controllerVariables[i].Z = threeButtonController || !Check_Key_Pressed(Keys_Def[i].Z);

        if (LeftRightEnabled)
        {
            controllerVariables[i].Up = !Check_Key_Pressed(Keys_Def[i].Up);
            controllerVariables[i].Down = !Check_Key_Pressed(Keys_Def[i].Down);
            controllerVariables[i].Left = !Check_Key_Pressed(Keys_Def[i].Left);
            controllerVariables[i].Right = !Check_Key_Pressed(Keys_Def[i].Right);
        }
        else //if(!LeftRightEnabled)
        {
            // prevent Left+Right or Up+Down from being pressed at the same time,
            // but make the most-recently-held one stay pressed if conflicting

            int up = !Check_Key_Pressed(Keys_Def[i].Up);
            int down = !Check_Key_Pressed(Keys_Def[i].Down);
            int left = !Check_Key_Pressed(Keys_Def[i].Left);
            int right = !Check_Key_Pressed(Keys_Def[i].Right);
            int upNext = up, downNext = down, leftNext = left, rightNext = right;
            if (!upNext && !downNext)
                upNext = controllerVariables[i].DownReally,
                downNext = controllerVariables[i].UpReally;
            if (!leftNext && !rightNext)
                leftNext = controllerVariables[i].RightReally,
                rightNext = controllerVariables[i].LeftReally;
            if (upNext || downNext)
                controllerVariables[i].Up = upNext,
                controllerVariables[i].Down = downNext;
            if (leftNext || rightNext)
                controllerVariables[i].Left = leftNext,
                controllerVariables[i].Right = rightNext;
            controllerVariables[i].UpReally = up;
            controllerVariables[i].DownReally = down;
            controllerVariables[i].LeftReally = left;
            controllerVariables[i].RightReally = right;
        } // (!LeftRightEnabled)
    } // for loop

    // autofire / autohold
    {
        extern long unsigned int FrameCount;
        extern long unsigned int LagCountPersistent;
        autoAlternator = ((FrameCount - LagCountPersistent) % 2) == 0;

#define APPLY_AUTOS(x) APPLY_AUTOS_I(Controller_1_##x); APPLY_AUTOS_I(Controller_1B_##x);	\
		                       APPLY_AUTOS_I(Controller_1C_##x); APPLY_AUTOS_I(Controller_1D_##x);	\
							   APPLY_AUTOS_I(Controller_2_##x); APPLY_AUTOS_I(Controller_2B_##x);	\
							   APPLY_AUTOS_I(Controller_2C_##x); APPLY_AUTOS_I(Controller_2D_##x);
#define APPLY_AUTOS_I(x)\
                		{\
			const bool autoFireFrame = autoAlternator ? x##_Autofire : x##_Autofire2;\
			if(autoFireFrame != x##_Autohold)\
				x = !x;\
                		}

        APPLY_AUTOS(Up);
        APPLY_AUTOS(Down);
        APPLY_AUTOS(Left);
        APPLY_AUTOS(Right);
        APPLY_AUTOS(Start);
        APPLY_AUTOS(Mode);
        APPLY_AUTOS(A);
        APPLY_AUTOS(B);
        APPLY_AUTOS(C);
        APPLY_AUTOS(X);
        APPLY_AUTOS(Y);
        APPLY_AUTOS(Z);
    }

    CallRegisteredLuaFunctions(LUACALL_BEFOREEMULATION);

    if (s_nextInputCondensedSet)
    {
        if (s_nextInputMask == ~0)
            SetCurrentInputCondensed(s_nextInputCondensed);
        else
        {
            long long newInput = s_nextInputCondensed & s_nextInputMask;
            long long keptInput = GetCurrentInputCondensed() & ~s_nextInputMask;
            SetCurrentInputCondensed(newInput | keptInput);
        }
        s_nextInputCondensedSet = false;
    }
    s_lastInputCondensed = GetCurrentInputCondensed();
}

long long PeekInputCondensed()
{
    Update_Input();
#define IMKEY(n,x) (!Check_Key_Pressed(Keys_Def[n].x))
    long long a = IMKEY(0, Up) | (IMKEY(0, Down) << 1) | (IMKEY(0, Left) << 2) | (IMKEY(0, Right) << 3) | (IMKEY(0, A) << 4) | (IMKEY(0, B) << 5) | (IMKEY(0, C) << 6) | (IMKEY(0, Start) << 7);
    long long b = IMKEY(2, Up) | (IMKEY(2, Down) << 1) | (IMKEY(2, Left) << 2) | (IMKEY(2, Right) << 3) | (IMKEY(2, A) << 4) | (IMKEY(2, B) << 5) | (IMKEY(2, C) << 6) | (IMKEY(2, Start) << 7);
    long long c = IMKEY(3, Up) | (IMKEY(3, Down) << 1) | (IMKEY(3, Left) << 2) | (IMKEY(3, Right) << 3) | (IMKEY(3, A) << 4) | (IMKEY(3, B) << 5) | (IMKEY(3, C) << 6) | (IMKEY(3, Start) << 7);
    long long d = IMKEY(1, Up) | (IMKEY(1, Down) << 1) | (IMKEY(1, Left) << 2) | (IMKEY(1, Right) << 3) | (IMKEY(1, A) << 4) | (IMKEY(1, B) << 5) | (IMKEY(1, C) << 6) | (IMKEY(1, Start) << 7);
    long long e = IMKEY(0, X) | (IMKEY(0, Y) << 1) | (IMKEY(0, Z) << 2) | (IMKEY(0, Mode) << 3) | (IMKEY(1, X) << 4) | (IMKEY(1, Y) << 5) | (IMKEY(1, Z) << 6) | (IMKEY(1, Mode) << 7);
    return a | (b << 8) | (c << 16) | (d << 24) | (e << 32) | (0xFFFFFF0000000000);
#undef IMKEY
}

unsigned int& KeyDialogButtonIDToDIKey(int controlID)
{
    switch (controlID)
    {
    case IDC_BUTTON_REDEFINE_UP_KEY: return Temp_Keys.Up;
    case IDC_BUTTON_REDEFINE_DOWN_KEY: return Temp_Keys.Down;
    case IDC_BUTTON_REDEFINE_LEFT_KEY: return Temp_Keys.Left;
    case IDC_BUTTON_REDEFINE_RIGHT_KEY: return Temp_Keys.Right;
    case IDC_BUTTON_REDEFINE_A_KEY: return Temp_Keys.A;
    case IDC_BUTTON_REDEFINE_B_KEY: return Temp_Keys.B;
    case IDC_BUTTON_REDEFINE_C_KEY: return Temp_Keys.C;
    case IDC_BUTTON_REDEFINE_START_KEY: return Temp_Keys.Start;
    case IDC_BUTTON_REDEFINE_X_KEY: return Temp_Keys.X;
    case IDC_BUTTON_REDEFINE_Y_KEY: return Temp_Keys.Y;
    case IDC_BUTTON_REDEFINE_Z_KEY: return Temp_Keys.Z;
    case IDC_BUTTON_REDEFINE_MODE_KEY: return Temp_Keys.Mode;
    default: static unsigned int none = 0; return none;
    }
}

const char* KeyDialogButtonIDToName(int controlID)
{
    switch (controlID)
    {
    case IDC_BUTTON_REDEFINE_UP_KEY: return "Up";
    case IDC_BUTTON_REDEFINE_DOWN_KEY: return "Down";
    case IDC_BUTTON_REDEFINE_LEFT_KEY: return "Left";
    case IDC_BUTTON_REDEFINE_RIGHT_KEY: return "Right";
    case IDC_BUTTON_REDEFINE_A_KEY: return "A";
    case IDC_BUTTON_REDEFINE_B_KEY: return "B";
    case IDC_BUTTON_REDEFINE_C_KEY: return "C";
    case IDC_BUTTON_REDEFINE_START_KEY: return "Start";
    case IDC_BUTTON_REDEFINE_X_KEY: return "X";
    case IDC_BUTTON_REDEFINE_Y_KEY: return "Y";
    case IDC_BUTTON_REDEFINE_Z_KEY: return "Z";
    case IDC_BUTTON_REDEFINE_MODE_KEY: return "Mode";
    default: return NULL; // important: must return NULL instead of any description
    }
}

const char* GetPlayerName(int curPlayer)
{
    switch (curPlayer)
    {
    case 0: return "Player 1";
    case 1: return "Player 2";
    case 2: return "Player 1-B";
    case 3: return "Player 1-C";
    case 4: return "Player 1-D";
    case 5: return "Player 2-B";
    case 6: return "Player 2-C";
    case 7: return "Player 2-D";
    default: return "Invalid Player";
    }
}

LRESULT CALLBACK KeyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) //Upth-Add - This is the new controller key redefinition dialog box
{
    RECT r;
    RECT r2;
    int dx1, dy1, dx2, dy2;
    int Player = Cur_Player;
    MSG m;

    static HWND Tex0 = NULL;
    static HWND Tex1 = NULL;
    static POINT prevCursorPos;

    switch (uMsg)
    {
    case WM_INITDIALOG:
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

        SetWindowPos(hDlg, NULL, r.left + (dx1 - dx2), r.top + (dy1 - dy2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

        Tex0 = GetDlgItem(hDlg, IDC_STATIC_TEXT1);
        Tex1 = GetDlgItem(hDlg, IDC_STATIC_TEXT2);

        //Upth-Add - This part shows what player we're redefining keys for.
        SetWindowText(Tex0, GetPlayerName(Cur_Player));

        if (!Init_Input(ghInstance, hDlg)) return false;

        SetTimer(hDlg, NULL, 50, NULL);

        return true;
        break;

    case WM_TIMER: // because WM_MOUSEMOVE is too much of a pain
        if (GetActiveWindow() == hDlg)
        {
            POINT point;
            GetCursorPos(&point);
            if (point.x != prevCursorPos.x || point.y != prevCursorPos.y)
            {
                int controlID = GetDlgCtrlID(WindowFromPoint(point));
                int diKey = KeyDialogButtonIDToDIKey(controlID);
                const char* buttonName = KeyDialogButtonIDToName(controlID);
                if (buttonName)
                {
                    const char* buttonKey = GetDirectInputKeyName(diKey);
                    sprintf(Str_Tmp, "Current Key for %s Button is:    %s", buttonName, buttonKey);
                    SetWindowText(Tex1, Str_Tmp);
                }
                prevCursorPos = point;
            }
        }	break;

    case WM_COMMAND:
    {	int controlID = LOWORD(wParam);
    switch (controlID)
    {
    case IDOK:
        Keys_Def[Cur_Player] = Temp_Keys;
        SetWindowText(Tex0, "");
        End_Input();
        DialogsOpen--;
        EndDialog(hDlg, false);
        return true;
        break;
    case ID_CANCEL:
        SetWindowText(Tex0, "");
        End_Input();
        DialogsOpen--;
        EndDialog(hDlg, false);
        return true;
        break;
    }

    const char* name = KeyDialogButtonIDToName(controlID);
    if (name)
    {
        sprintf(Str_Tmp, "Please Input Key for: %s Button", name);
        SetWindowText(Tex1, Str_Tmp);
        int diKey = Get_Key();
        if (diKey)
            KeyDialogButtonIDToDIKey(controlID) = diKey;
        while (PeekMessage(&m, hDlg, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE)); // prevent the outer dialog from receiving the key that was pressed for the button
        //while (PeekMessage(&m, hDlg, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)); // prevent the outer dialog from receiving extra clicks that were made while waiting for the button
        SetWindowText(Tex1, "");
        prevCursorPos.x = -1; // makes the button name refresh
        return true;
    }
    }	break;

    case WM_CLOSE:
        KillTimer(hDlg, NULL);
        SetWindowText(Tex0, "");
        End_Input();
        DialogsOpen--;
        EndDialog(hDlg, false);
        return true;
        break;
    }

    return false;
}

int Setting_Keys(HWND hset, int Player, int TypeP) //Upth-Add - Opens the new dialog box
{
    //	HWND Tex1, Txt2;
    Cur_Player = Player; //Upth-Add - Tells it what player we're redefining keys for
    Temp_Keys = Keys_Def[Player]; //Upth-Add - So that any keys that aren't redefined don't get unset when the user hits "OK"

    DialogsOpen++;
    DialogBox(ghInstance, MAKEINTRESOURCE(TypeP ? IDD_SETJOYKEYS_6 : IDD_SETJOYKEYS_3), hset, (DLGPROC)KeyProc); //Upth-Add - Opens the dialog

    return 1;
}

/*int Check_Pause_Key()
{
static bool PauseKeyHeld = 0;
Update_Input();
if(PauseKeyDown && !PauseKeyHeld)
{
PauseKeyHeld=1;
return 1;
}
if(PauseKeyHeld && !PauseKeyDown)
PauseKeyHeld=0;
return 0;
}*/

int Check_Skip_Key_Released()
{
    static int FrameAdvanceKeyWasDown = 0;
    int rv = FrameAdvanceKeyWasDown && !FrameAdvanceKeyDown;
    FrameAdvanceKeyWasDown = FrameAdvanceKeyDown;
    return rv;
}
int Check_Skip_Key_Pressed()
{
    static int FrameAdvanceKeyWasDown = 0;
    int rv = !FrameAdvanceKeyWasDown && FrameAdvanceKeyDown;
    FrameAdvanceKeyWasDown = FrameAdvanceKeyDown;
    return rv;
}

//Modif N - changed to make frame advance key continuous, after a delay:
int Check_Skip_Key()
{
    static time_t lastSkipTime = 0;
    const int skipPressedNew = FrameAdvanceKeyDown;

    static int checks = 0;
    if (skipPressedNew && timeGetTime() - lastSkipTime >= 5)
    {
        checks++;
        if (checks > 8000 + 60)
            checks -= 8000;
        lastSkipTime = timeGetTime();
    }

    if (skipPressedNew && (!SkipKeyIsPressed || ((checks > 60) && ((checks % DelayFactor) == 0))))
    {
        SkipKeyIsPressed = 1;
        return 1;
    }
    else {
        if (!skipPressedNew && SkipKeyIsPressed)
        {
            SkipKeyIsPressed = 0;
            checks = 0;
        }
        return 0;
    }
}

void Check_Misc_Key()
{
    // N - checks for enabling/disabling the autofire and autohold toggles
    // (this first part needs to always run even if no autofire/autohold/autoclear keys are held)
    {
#define TRANSFER_PRESSED(x)	TRANSFER_PRESSED_I(Controller_1_##x,0,x)	\
		                            TRANSFER_PRESSED_I(Controller_1B_##x,2,x)	\
		                            TRANSFER_PRESSED_I(Controller_1C_##x,3,x)	\
		                            TRANSFER_PRESSED_I(Controller_1D_##x,4,x)	\
		                            TRANSFER_PRESSED_I(Controller_2_##x,1,x)	\
		                            TRANSFER_PRESSED_I(Controller_2B_##x,5,x)	\
		                            TRANSFER_PRESSED_I(Controller_2C_##x,6,x)	\
		                            TRANSFER_PRESSED_I(Controller_2D_##x,7,x)
#define TRANSFER_PRESSED_I(x,y,z)	{	const int now = !Check_Key_Pressed(Keys_Def[y].##z);	\
				x##_Just = x##_Last && !now; x##_Last = now;	}
        TRANSFER_PRESSED(Up);
        TRANSFER_PRESSED(Down);
        TRANSFER_PRESSED(Left);
        TRANSFER_PRESSED(Right);
        TRANSFER_PRESSED(Start);
        TRANSFER_PRESSED(Mode);
        TRANSFER_PRESSED(A);
        TRANSFER_PRESSED(B);
        TRANSFER_PRESSED(C);
        TRANSFER_PRESSED(X);
        TRANSFER_PRESSED(Y);
        TRANSFER_PRESSED(Z);

        if (AutoFireKeyDown || AutoHoldKeyDown)
        {
            extern long unsigned int FrameCount;
            extern long unsigned int LagCountPersistent;
            autoAlternator = ((FrameCount - LagCountPersistent) % 2) == 0;

#define CHECK_TOGGLE_AUTO(x) CHECK_TOGGLE_AUTO_I(Controller_1_##x);	\
			                             CHECK_TOGGLE_AUTO_I(Controller_1B_##x);	\
										 CHECK_TOGGLE_AUTO_I(Controller_1C_##x);	\
										 CHECK_TOGGLE_AUTO_I(Controller_1D_##x);	\
										 CHECK_TOGGLE_AUTO_I(Controller_2_##x);	\
										 CHECK_TOGGLE_AUTO_I(Controller_2B_##x);	\
										 CHECK_TOGGLE_AUTO_I(Controller_2C_##x);	\
										 CHECK_TOGGLE_AUTO_I(Controller_2D_##x);
#define CHECK_TOGGLE_AUTO_I(x)\
			if(x##_Just)\
                        			{\
				if(AutoHoldKeyDown)\
                                				{\
					x##_Autohold = !x##_Autohold;\
					x##_Autofire = x##_Autofire2 = false;\
                                				}\
				if(AutoFireKeyDown)\
                                				{\
					const bool autoFired = x##_Autofire || x##_Autofire2;\
					x##_Autohold = x##_Autofire = x##_Autofire2 = false;\
					if(autoAlternator)\
						x##_Autofire = !autoFired;\
                					else\
						x##_Autofire2 = !autoFired;\
                                				}\
                        			}

            CHECK_TOGGLE_AUTO(Up);
            CHECK_TOGGLE_AUTO(Down);
            CHECK_TOGGLE_AUTO(Left);
            CHECK_TOGGLE_AUTO(Right);
            CHECK_TOGGLE_AUTO(Start);
            CHECK_TOGGLE_AUTO(Mode);
            CHECK_TOGGLE_AUTO(A);
            CHECK_TOGGLE_AUTO(B);
            CHECK_TOGGLE_AUTO(C);
            CHECK_TOGGLE_AUTO(X);
            CHECK_TOGGLE_AUTO(Y);
            CHECK_TOGGLE_AUTO(Z);
        }
        if (AutoClearKeyDown)
        {
#define CLEAR_AUTO(x) CLEAR_AUTO_I(Controller_1_##x); CLEAR_AUTO_I(Controller_1B_##x);	\
			                      CLEAR_AUTO_I(Controller_1C_##x); CLEAR_AUTO_I(Controller_1D_##x);	\
								  CLEAR_AUTO_I(Controller_2_##x); CLEAR_AUTO_I(Controller_2B_##x);	\
								  CLEAR_AUTO_I(Controller_2C_##x); CLEAR_AUTO_I(Controller_2D_##x);
#define CLEAR_AUTO_I(x) x##_Autofire = 0; x##_Autofire2 = 0; x##_Autohold = 0;

            CLEAR_AUTO(Up);
            CLEAR_AUTO(Down);
            CLEAR_AUTO(Left);
            CLEAR_AUTO(Right);
            CLEAR_AUTO(Start);
            CLEAR_AUTO(Mode);
            CLEAR_AUTO(A);
            CLEAR_AUTO(B);
            CLEAR_AUTO(C);
            CLEAR_AUTO(X);
            CLEAR_AUTO(Y);
            CLEAR_AUTO(Z);
        }
    }
}