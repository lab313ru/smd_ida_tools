#include <stdio.h>
#include <math.h>
#include "g_ddraw.h"
#include "g_dsound.h"
#include "g_input.h"
#include "g_main.h"
#include "guidraw.h"
#include "resource.h"
#include "gens.h"
#include "mem_m68k.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "misc.h"
#include "blit.h"
#include "save.h"

#include "cdda_mp3.h"

#include "movie.h"
#include "moviegfx.h"
#include "joypads.h"
#include "drawutil.h"
#include "luascript.h"

LPDIRECTDRAW lpDD_Init;
LPDIRECTDRAW4 lpDD;
LPDIRECTDRAWSURFACE4 lpDDS_Primary;
LPDIRECTDRAWSURFACE4 lpDDS_Flip;
LPDIRECTDRAWSURFACE4 lpDDS_Back;
LPDIRECTDRAWSURFACE4 lpDDS_Blit;
LPDIRECTDRAWCLIPPER lpDDC_Clipper;

clock_t Last_Time = 0, New_Time = 0;
clock_t Used_Time = 0;

int Flag_Clr_Scr = 0;
int Sleep_Time = 1;
int FS_VSync;
int W_VSync;
int Res_X = 320 << (int)(Render_FS > 0); //Upth-Add - For the new configurable
int Res_Y = 240 << (int)(Render_FS > 0); //Upth-Add - fullscreen resolution (defaults 320 x 240)
bool FS_No_Res_Change = false; //Upth-Add - For the new fullscreen at same resolution
int Stretch;
int Blit_Soft;
int Effect_Color = 7;
int FPS_Style = EMU_MODE | WHITE;
int Message_Style = EMU_MODE | WHITE | SIZE_X2;
extern "C" int disableSound, disableSound2, disableRamSearchUpdate;

long int MovieSize;//Modif
int SlowFrame = 0; //Modif

static char Info_String[1024] = "";
static int Message_Showed = 0;
static unsigned int Info_Time = 0;

void(*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
void(*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);
int(*Update_Frame)();
int(*Update_Frame_Fast)();

int Correct_256_Aspect_Ratio = 1;

// debug string is drawn in _DEBUG
// list variables by comma and specify the string format
#define DEBUG_VARIABLES NULL
static char Debug_Format[1024] = "";
static int Debug_xPos = 0;
static int Debug_yPos = 0;

// if you have to debug something in fullscreen mode
// but the fullscreen lock prevents you from seeing the debugger
// when a breakpoint/assertion/crash/whatever happens,
// then it might help to define this.
// absolutely don't leave it enabled by default though, not even in _DEBUG
//#define DISABLE_EXCLUSIVE_FULLSCREEN_LOCK

int Update_Frame_Adjusted()
{
    if (disableVideoLatencyCompensationCount)
        disableVideoLatencyCompensationCount--;

    if (!IsVideoLatencyCompensationOn())
    {
        // normal update
        return Update_Frame();
    }
    else
    {
        // update, and render the result that's some number of frames in the (emulated) future
        // typically the video takes 2 frames to catch up with where the game really is,
        // so setting VideoLatencyCompensation to 2 can make the input more responsive
        //
        // in a way this should actually make the emulation more accurate, because
        // the delay from your computer hardware stacks with the delay from the emulated hardware,
        // so eliminating some of that delay should make it feel closer to the real system

        disableSound2 = true;
        int retval = Update_Frame_Fast();
        Update_RAM_Search();
        disableRamSearchUpdate = true;
        Save_State_To_Buffer(State_Buffer);
        for (int i = 0; i < VideoLatencyCompensation - 1; i++)
            Update_Frame_Fast();
        disableSound2 = false;
        Update_Frame();
        disableRamSearchUpdate = false;
        Load_State_From_Buffer(State_Buffer);
        return retval;
    }
}

int Update_Frame_Hook()
{
    // note: we don't handle LUACALL_BEFOREEMULATION here
    // because it needs to run immediately before SetCurrentInputCondensed() might get called
    // in order for joypad.get() and joypad.set() to work as expected

    int retval = Update_Frame_Adjusted();

    CallRegisteredLuaFunctions(LUACALL_AFTEREMULATION);

    CallRegisteredLuaFunctions(LUACALL_AFTEREMULATIONGUI);

    return retval;
}
int Update_Frame_Fast_Hook()
{
    // note: we don't handle LUACALL_BEFOREEMULATION here
    // because it needs to run immediately before SetCurrentInputCondensed() might get called
    // in order for joypad.get() and joypad.set() to work as expected

    int retval = Update_Frame_Fast();

    CallRegisteredLuaFunctions(LUACALL_AFTEREMULATION);
    Update_RAM_Search();
    return retval;
}

void Put_Info_NonImmediate(char *Message, int Duration)
{
    if (Show_Message)
    {
        strcpy(Info_String, Message);
        Info_Time = GetTickCount() + Duration;
        Message_Showed = 1;
    }
}

void Put_Info(char *Message, int Duration)
{
    if (Show_Message)
    {
        Do_VDP_Refresh();
        Put_Info_NonImmediate(Message);

        // Modif N. - in case game is paused or at extremely low speed, update screen on new message
        extern bool g_anyScriptsHighSpeed;
        if (!Setting_Render && !g_anyScriptsHighSpeed)
        {
            extern HWND HWnd;

            if (FrameCount == 0) // if frame count is 0 it's OK to clear the screen first so we can see the message better
            {
                memset(MD_Screen, 0, sizeof(MD_Screen));
                memset(MD_Screen32, 0, sizeof(MD_Screen32));
            }

            Flip(HWnd);
        }
    }
}

int Init_Fail(HWND hwnd, char *err)
{
    End_DDraw();
    MessageBox(hwnd, err, "Oups ...", MB_OK);

    return 0;
}

int Init_DDraw(HWND hWnd)
{
    int Rend;
    HRESULT rval;
    DDSURFACEDESC2 ddsd;

    int oldBits32 = Bits32;

    End_DDraw();

    if (Full_Screen) Rend = Render_FS;
    else Rend = Render_W;

    if (FAILED(DirectDrawCreate(NULL, &lpDD_Init, NULL)))
        return Init_Fail(hWnd, "Error with DirectDrawCreate !");

    if (FAILED(lpDD_Init->QueryInterface(IID_IDirectDraw4, (LPVOID *)&lpDD)))
        return Init_Fail(hWnd, "Error with QueryInterface !\nUpgrade your DirectX version.");

    lpDD_Init->Release();
    lpDD_Init = NULL;

    if (!(Mode_555 & 2))
    {
        memset(&ddsd, 0, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);

        lpDD->GetDisplayMode(&ddsd);

        if (ddsd.ddpfPixelFormat.dwGBitMask == 0x03E0) Mode_555 = 1;
        else Mode_555 = 0;

        Recalculate_Palettes();
    }

#ifdef DISABLE_EXCLUSIVE_FULLSCREEN_LOCK
    FS_VSync = 0;
    rval = lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
#else
    if (Full_Screen)
        rval = lpDD->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    else
        rval = lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
#endif

    if (FAILED(rval))
        return Init_Fail(hWnd, "Error with lpDD->SetCooperativeLevel !");

    if (Res_X < (320 << (int)(Render_FS > 0))) Res_X = 320 << (int)(Render_FS > 0); //Upth-Add - Set a floor for the resolution
    if (Res_Y < (240 << (int)(Render_FS > 0))) Res_Y = 240 << (int)(Render_FS > 0); //Upth-Add - 320x240 for single, 640x480 for double and other

    //if (Full_Screen && Render_FS >= 2) //Upth-Modif - Since software blits don't stretch right, we'll use 640x480 for those render modes
    // Modif N. removed the forced 640x480 case because it caused "windowed fullscreen mode" to be ignored and because I fixed the fullscreen software blit stretching

    if (Full_Screen && !(FS_No_Res_Change))
    {
        if (FAILED(lpDD->SetDisplayMode(Res_X, Res_Y, 16, 0, 0)))
            return Init_Fail(hWnd, "Error with lpDD->SetDisplayMode !");
    }

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);

    if ((Full_Screen) && (FS_VSync))
    {
        ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        ddsd.dwBackBufferCount = 1;
    }
    else
    {
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    }

    if (FAILED(lpDD->CreateSurface(&ddsd, &lpDDS_Primary, NULL)))
        return Init_Fail(hWnd, "Error with lpDD->CreateSurface !");

    if (Full_Screen)
    {
        if (FS_VSync)
        {
            ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

            if (FAILED(lpDDS_Primary->GetAttachedSurface(&ddsd.ddsCaps, &lpDDS_Flip)))
                return Init_Fail(hWnd, "Error with lpDDPrimary->GetAttachedSurface !");

            lpDDS_Blit = lpDDS_Flip;
        }
        else lpDDS_Blit = lpDDS_Primary;
    }
    else
    {
        if (FAILED(lpDD->CreateClipper(0, &lpDDC_Clipper, NULL)))
            return Init_Fail(hWnd, "Error with lpDD->CreateClipper !");

        if (FAILED(lpDDC_Clipper->SetHWnd(0, hWnd)))
            return Init_Fail(hWnd, "Error with lpDDC_Clipper->SetHWnd !");

        if (FAILED(lpDDS_Primary->SetClipper(lpDDC_Clipper)))
            return Init_Fail(hWnd, "Error with lpDDS_Primary->SetClipper !");
    }

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

    if (Rend < 2)
    {
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        ddsd.dwWidth = 336;
        ddsd.dwHeight = 240;
    }
    else
    {
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
        ddsd.dwWidth = 672; //Upth-Modif - was 640, but for single mode the value was 336, not 320.
        ddsd.dwHeight = 480;
    }

    if (FAILED(lpDD->CreateSurface(&ddsd, &lpDDS_Back, NULL)))
        return Init_Fail(hWnd, "Error with lpDD->CreateSurface !");

    if (Rend < 2)
    {
        memset(&ddsd, 0, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);

        if (FAILED(lpDDS_Back->GetSurfaceDesc(&ddsd)))
            return Init_Fail(hWnd, "Error with lpDD_Back->GetSurfaceDesc !");

        ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
        ddsd.dwWidth = 336;
        ddsd.dwHeight = 240;
        if (ddsd.ddpfPixelFormat.dwRGBBitCount > 16)
        {
            ddsd.lpSurface = &MD_Screen32[0];
            ddsd.lPitch = 336 * 4;
        }
        else
        {
            ddsd.lpSurface = &MD_Screen[0];
            ddsd.lPitch = 336 * 2;
        }

        if (FAILED(lpDDS_Back->SetSurfaceDesc(&ddsd, 0)))
            return Init_Fail(hWnd, "Error with lpDD_Back->SetSurfaceDesc !");
    }

    // make sure Bits32 is correct (which it could easily not be at this point)
    {
        memset(&ddsd, 0, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);

        if (FAILED(lpDDS_Back->GetSurfaceDesc(&ddsd)))
            return Init_Fail(hWnd, "Error with lpDDS_Blit->GetSurfaceDesc !");

        Bits32 = (ddsd.ddpfPixelFormat.dwRGBBitCount > 16) ? 1 : 0;

        // also prevent the colors from sometimes being messed up for 1 frame if we changed color depth

        if (Bits32 && !oldBits32)
            for (int i = 0; i < 336 * 240; i++)
                MD_Screen32[i] = DrawUtil::Pix16To32(MD_Screen[i]);

        if (!Bits32 && oldBits32)
            for (int i = 0; i < 336 * 240; i++)
                MD_Screen[i] = DrawUtil::Pix32To16(MD_Screen32[i]);
    }

    // make sure the render mode is still valid (changing options in a certain order can make it invalid at this point)
    Set_Render(hWnd, Full_Screen, -1, false);

    // make sure the menu reflects the current mode (which it generally won't yet if we changed to/from 32-bit mode in this function)
    Build_Main_Menu();

    return 1;
}

void End_DDraw()
{
    if (lpDDC_Clipper)
    {
        lpDDC_Clipper->Release();
        lpDDC_Clipper = NULL;
    }

    if (lpDDS_Back)
    {
        lpDDS_Back->Release();
        lpDDS_Back = NULL;
    }

    if (lpDDS_Flip)
    {
        lpDDS_Flip->Release();
        lpDDS_Flip = NULL;
    }

    if (lpDDS_Primary)
    {
        lpDDS_Primary->Release();
        lpDDS_Primary = NULL;
    }

    if (lpDD)
    {
        lpDD->SetCooperativeLevel(HWnd, DDSCL_NORMAL);
        lpDD->Release();
        lpDD = NULL;
    }

    lpDDS_Blit = NULL;
}

HRESULT RestoreGraphics(HWND hWnd)
{
    HRESULT rval1 = lpDDS_Primary->Restore();
    HRESULT rval2 = lpDDS_Back->Restore();

    // Modif N. -- fixes lost surface handling when the color depth has changed
    if (rval1 == DDERR_WRONGMODE || rval2 == DDERR_WRONGMODE)
        return Init_DDraw(hWnd) ? DD_OK : DDERR_GENERIC;

    return SUCCEEDED(rval2) ? rval1 : rval2;
}

int Clear_Primary_Screen(HWND hWnd)
{
    if (!lpDD)
        return 0; // bail if directdraw hasn't been initialized yet or if we're still in the middle of initializing it

    DDSURFACEDESC2 ddsd;
    DDBLTFX ddbltfx;
    RECT RD;
    POINT p;

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);

    memset(&ddbltfx, 0, sizeof(ddbltfx));
    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0;

    if (Full_Screen)
    {
        if (FS_VSync)
        {
            lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
            //lpDDS_Primary->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
        }
        else lpDDS_Primary->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
    }
    else
    {
        p.x = p.y = 0;
        GetClientRect(hWnd, &RD);
        ClientToScreen(hWnd, &p);

        RD.left = p.x;
        RD.top = p.y;
        RD.right += p.x;
        RD.bottom += p.y;

        if (RD.top < RD.bottom)
            lpDDS_Primary->Blt(&RD, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
    }

    return 1;
}

int Clear_Back_Screen(HWND hWnd)
{
    if (!lpDD)
        return 0; // bail if directdraw hasn't been initialized yet or if we're still in the middle of initializing it

    DDSURFACEDESC2 ddsd;
    DDBLTFX ddbltfx;

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);

    memset(&ddbltfx, 0, sizeof(ddbltfx));
    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0;

    lpDDS_Back->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);

    return 1;
}

void Restore_Primary(void)
{
    if (lpDD && Full_Screen && FS_VSync)
    {
        while (lpDDS_Primary->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_SURFACEBUSY);
        lpDD->FlipToGDISurface();
    }
}

// Calculate Draw Area
//  inputs: hWnd
// outputs:
//    RectDest: destination rect on screen
//    RectSrc: source rect in back buffer used for software blit
//             and for hardware blit from Back surface into destination (Primary or its Flip surface)
void CalculateDrawArea(HWND hWnd, RECT& RectDest, RECT& RectSrc)
{
    POINT p;
    float Ratio_X, Ratio_Y;

    int FS_X, FS_Y; //Upth-Add - So we can set the fullscreen resolution to the current res without changing the value that gets saved to the config
    int Render_Mode;
    if (Full_Screen)
    {
        Render_Mode = Render_FS;

        if (FS_No_Res_Change) { //Upth-Add - If we didn't change resolution when we went Full Screen
            DEVMODE temp;
            EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &temp); //Upth-Add - Gets the current screen resolution
            FS_X = temp.dmPelsWidth;
            FS_Y = temp.dmPelsHeight;
        }
        else { //Upth-Add - Otherwise use the configured resolution values
            FS_X = Res_X;
            FS_Y = Res_Y;
        }
    }
    else
    {
        Render_Mode = Render_W;

        GetClientRect(hWnd, &RectDest);
        FS_X = RectDest.right;
        FS_Y = RectDest.bottom;
    }

    Ratio_X = (float)FS_X / 320.0f; //Upth-Add - Find the current size-ratio on the x-axis
    Ratio_Y = (float)FS_Y / 240.0f; //Upth-Add - Find the current size-ratio on the y-axis
    Ratio_X = Ratio_Y = (Ratio_X < Ratio_Y) ? Ratio_X : Ratio_Y; //Upth-Add - Floor them to the smaller value for correct ratio display

    int Screen_X = FULL_X_RESOLUTION; // Genesis screen width
    int Screen_Y = VDP_Num_Vis_Lines; // Genesis screen height

    RectSrc.left = 0 + 8;
    RectSrc.right = Screen_X + 8;
    RectSrc.top = 0;
    RectSrc.bottom = Screen_Y;

    if (Correct_256_Aspect_Ratio)
    {
        RectDest.left = (int)((FS_X - (320 * Ratio_X)) / 2); //Upth-Modif - Centering the screen left-right
        RectDest.right = (int)(320 * Ratio_X + RectDest.left); //Upth-modif - again
    }
    else
    {
        RectDest.left = (int)((FS_X - (Screen_X * Ratio_X)) / 2); //Upth-Add - Centering left-right
        RectDest.right = (int)(Screen_X * Ratio_X + RectDest.left); //Upth-modif - again
    }

    RectDest.top = (int)((FS_Y - (Screen_Y * Ratio_Y)) / 2); //Upth-Modif - centering top-bottom under other circumstances
    RectDest.bottom = (int)(Screen_Y * Ratio_Y) + RectDest.top; //Upth-Modif - using the same method

    if (Stretch)
    {
        RectDest.left = 0;
        RectDest.right = FS_X; //Upth-Modif - use the user configured value
        RectDest.top = 0;      //Upth-Add - also, if we have stretch enabled
        RectDest.bottom = FS_Y;//Upth-Add - we don't correct the screen ratio
    }

    if (Render_Mode < 2)
    {
        /* r57shell: I don't think this is needed, but I leave it
        if (Render_Mode == 0 && Ratio_X >= 1) //Upth-Add - If the render is "single" we don't stretch it
        {
        // If screen not smaller than 320x240
        if (Stretch)
        {
        RectDest.top = (int) ((FS_Y - 240)/2); //Upth-Add - But we still
        RectDest.bottom = 240 + RectDest.top;  //Upth-Add - center the screen
        }
        else
        {
        RectDest.top = (int) ((FS_Y - Screen_Y)/2); //Upth-Add - for both of the
        RectDest.bottom = Screen_Y + RectDest.top;  //Upth-Add - predefined conditions
        }
        if (Correct_256_Aspect_Ratio || Stretch)
        {
        RectDest.left = (int) ((FS_X - 320)/2); //Upth-Add - and along the
        RectDest.right = 320  + RectDest.left;  //Upth-Add - x axis, also
        }
        else
        {
        RectDest.left = (int) ((FS_X - Screen_X)/2); //Upth-Modif - Centering the screen left-right
        RectDest.right = Screen_X + RectDest.left;   //Upth-modif - again
        }
        }*/
    }
    else
    {
        RectSrc.left *= 2;
        RectSrc.top *= 2;
        RectSrc.right *= 2;
        RectSrc.bottom *= 2;
    }

    if (!Full_Screen)
    {
        p.x = p.y = 0;
        ClientToScreen(hWnd, &p);

        RectDest.top += p.y; //Upth-Modif - this part moves the picture into the window
        RectDest.bottom += p.y; //Upth-Modif - I had to move it after all of the centering
        RectDest.left += p.x;   //Upth-Modif - because it modifies the values
        RectDest.right += p.x;  //Upth-Modif - that I use to find the center
    }
}

extern "C" { extern unsigned int LED_Status; }
void Draw_SegaCD_LED()
{
    if (LED_Status & 2)
    {
        MD_Screen[336 * 220 + 12] = 0x03E0;
        MD_Screen[336 * 220 + 13] = 0x03E0;
        MD_Screen[336 * 220 + 14] = 0x03E0;
        MD_Screen[336 * 220 + 15] = 0x03E0;
        MD_Screen[336 * 222 + 12] = 0x03E0;
        MD_Screen[336 * 222 + 13] = 0x03E0;
        MD_Screen[336 * 222 + 14] = 0x03E0;
        MD_Screen[336 * 222 + 15] = 0x03E0;
        MD_Screen32[336 * 220 + 12] = 0x00FF00;
        MD_Screen32[336 * 220 + 13] = 0x00FF00;
        MD_Screen32[336 * 220 + 14] = 0x00FF00;
        MD_Screen32[336 * 220 + 15] = 0x00FF00;
        MD_Screen32[336 * 222 + 12] = 0x00FF00;
        MD_Screen32[336 * 222 + 13] = 0x00FF00;
        MD_Screen32[336 * 222 + 14] = 0x00FF00;
        MD_Screen32[336 * 222 + 15] = 0x00FF00;
    }

    if (LED_Status & 1)
    {
        MD_Screen[336 * 220 + 12 + 8] = 0xF800;
        MD_Screen[336 * 220 + 13 + 8] = 0xF800;
        MD_Screen[336 * 220 + 14 + 8] = 0xF800;
        MD_Screen[336 * 220 + 15 + 8] = 0xF800;
        MD_Screen[336 * 222 + 12 + 8] = 0xF800;
        MD_Screen[336 * 222 + 13 + 8] = 0xF800;
        MD_Screen[336 * 222 + 14 + 8] = 0xF800;
        MD_Screen[336 * 222 + 15 + 8] = 0xF800;
        MD_Screen32[336 * 220 + 12 + 8] = 0xFF0000;
        MD_Screen32[336 * 220 + 13 + 8] = 0xFF0000;
        MD_Screen32[336 * 220 + 14 + 8] = 0xFF0000;
        MD_Screen32[336 * 220 + 15 + 8] = 0xFF0000;
        MD_Screen32[336 * 222 + 12 + 8] = 0xFF0000;
        MD_Screen32[336 * 222 + 13 + 8] = 0xFF0000;
        MD_Screen32[336 * 222 + 14 + 8] = 0xFF0000;
        MD_Screen32[336 * 222 + 15 + 8] = 0xFF0000;
    }
}

// text, recording status, etc.
void DrawInformationOnTheScreen()
{
    int i;
    int n, j, m, l[3]; //UpthModif - Added l[3] for use when displaying input
    char FCTemp[256], pos;
    static float FPS = 0.0f, frames[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static unsigned int old_time = 0, view_fps = 0, index_fps = 0, freq_cpu[2] = { 0, 0 };
    unsigned int new_time[2];

    if (Show_LED && SegaCD_Started)
        Draw_SegaCD_LED();

    if (MainMovie.Status == MOVIE_RECORDING)
    {
        static unsigned int CircleFrameCount = 0;
        if ((CircleFrameCount++) % 64 < 32)
        {
            m = 0;
            n = 70560 + 309;
            for (j = 0; j < 9; j++, n += 327)
            {
                for (i = 0; i < 9; i++, m++, n++)
                {
                    if (RCircle[m] > 0) MD_Screen[n] = RCircle[m];
                    if (RCircle32[m] > 0) MD_Screen32[n] = RCircle32[m];
                }
            }
        }
    }
#ifdef SONICSPEEDHACK
#include "sonichacksuite.h"
    // SONIC THE HEDGEHOG SPEEDOMETER HACK
    SonicSpeedTicker();
#endif
    //Framecounter
    if (FrameCounterEnabled)
    {
        if (FrameCounterFrames) //Nitsuja added this option, raw integer frame number display
        {
            if (!MainMovie.File || MainMovie.Status == MOVIE_RECORDING)
                sprintf(FCTemp, "%d", FrameCount);
            else
                sprintf(FCTemp, "%d/%d", FrameCount, MainMovie.LastFrame);
        }
        else
        {
            const int fps = CPU_Mode ? 50 : 60;
            if (!MainMovie.File || MainMovie.Status == MOVIE_RECORDING)
                sprintf(FCTemp, "%d:%02d:%02d", (FrameCount / (fps * 60)), (FrameCount / fps) % 60, FrameCount%fps); //Nitsuja modified this to use the FPS value instead of 60 | Upth-Modif - Nitsuja forgot that even in PAL mode, it's 60 seconds per minute
            else
                sprintf(FCTemp, "%d:%02d:%02d/%d:%02d:%02d", (FrameCount / (fps * 60)), (FrameCount / fps) % 60, FrameCount%fps, (MainMovie.LastFrame / (fps * 60)), (MainMovie.LastFrame / fps) % 60, MainMovie.LastFrame%fps);
        }

        unsigned int slashPos = ~0;
        const char* slash = strchr(FCTemp, '/');
        if (slash)
            slashPos = slash - FCTemp;

        if (FCTemp[0] != '\0')
        {
            n = FrameCounterPosition;
            if (FrameCounterPosition == FRAME_COUNTER_TOP_RIGHT || FrameCounterPosition == FRAME_COUNTER_BOTTOM_RIGHT)
            {
                if (!IS_FULL_X_RESOLUTION)
                    n -= 64;
                int off = strlen(FCTemp) - 8;
                if (off > 0) n -= off * 6;
            }
            BOOL drawRed = Lag_Frame;
            size_t FCTemp_len = strlen(FCTemp);
            for (pos = 0; (unsigned int)pos < FCTemp_len; pos++)
            {
                if ((unsigned int)pos == slashPos)
                    drawRed = (MainMovie.Status != MOVIE_PLAYING);

                m = (FCTemp[pos] - '-') * 30;

                for (j = 0; j < 7; j++, n += 330)
                {
                    for (i = 0; i < 6; i++, n++)
                    {
                        if (j > 0 && j < 6)
                        {
                            if (Bits32) MD_Screen32[n] = FCDigit32[m++] & (drawRed ? 0xFF0000 : 0xFFFFFF);
                            else MD_Screen[n] = FCDigit[m++] & (drawRed ? 0xF800 : 0xFFFF);
                        }
                        else
                        {
                            if (Bits32) MD_Screen32[n] = 0x0000;
                            else MD_Screen[n] = 0x0000;
                        }
                        MD_Screen32[n] |= 0xFF000000; // alpha added (opaque)
                    }
                }
                n -= 336 * 7 - 6;
            }
        }
    }
    //Lagcounter
    if (LagCounterEnabled)
    {
        if (LagCounterFrames) //Nitsuja added this option, raw integer Lag number display
        {
            sprintf(FCTemp, "%d", LagCount);
        }
        else
        {
            const int fps = CPU_Mode ? 50 : 60;
            sprintf(FCTemp, "%d:%02d:%02d", (LagCount / (fps * 60)), (LagCount / fps) % 60, LagCount%fps); //Nitsuja modified this to use the FPS value instead of 60 | Upth-Modif - Nitsuja forgot that even in PAL mode, it's 60 seconds per minute
        }

        if (FCTemp[0] != '\0')
        {
            n = FrameCounterPosition + 2352;
            if (!IS_FULL_X_RESOLUTION && (FrameCounterPosition == FRAME_COUNTER_TOP_RIGHT || FrameCounterPosition == FRAME_COUNTER_BOTTOM_RIGHT))
                n -= 64;
            size_t FCTemp_len = strlen(FCTemp);
            for (pos = 0; (unsigned int)pos < FCTemp_len; pos++)
            {
                m = (FCTemp[pos] - '-') * 30;

                for (j = 0; j < 7; j++, n += 330)
                {
                    for (i = 0; i < 6; i++, n++)
                    {
                        if (j > 0 && j < 6)
                        {
                            if (Bits32) MD_Screen32[n] = FCDigit32[m++] & 0xFF0000;
                            else MD_Screen[n] = FCDigit[m++] & 0xF800;
                        }
                        else
                        {
                            if (Bits32) MD_Screen32[n] = 0x0000;
                            else MD_Screen[n] = 0x0000;
                        }
                        MD_Screen32[n] |= 0xFF000000; // alpha added (opaque)
                    }
                }
                n -= 336 * 7 - 6;
            }
        }
    }
    //Show Input
    if (ShowInputEnabled)
    {
        strcpy(FCTemp, "[\\]^ABCSXYZM[\\]^ABCSXYZM[\\]^ABCSXYZM"); //upthmodif

        if (Controller_1_Up) FCTemp[0] = '@';
        if (Controller_1_Down) FCTemp[1] = '@';
        if (Controller_1_Left) FCTemp[2] = '@';
        if (Controller_1_Right) FCTemp[3] = '@';
        if (Controller_1_A) FCTemp[4] = '@';
        if (Controller_1_B) FCTemp[5] = '@';
        if (Controller_1_C) FCTemp[6] = '@';
        if (Controller_1_Start) FCTemp[7] = '@';
        if (Controller_1_X) FCTemp[8] = '@';
        if (Controller_1_Y) FCTemp[9] = '@';
        if (Controller_1_Z) FCTemp[10] = '@';
        if (Controller_1_Mode) FCTemp[11] = '@';
        //Upth - added 3 controller display support
        if (Controller_1_Type & 0x10) {
            if (Controller_1B_Up) FCTemp[12 + 0] = '@';
            if (Controller_1B_Down) FCTemp[12 + 1] = '@';
            if (Controller_1B_Left) FCTemp[12 + 2] = '@';
            if (Controller_1B_Right) FCTemp[12 + 3] = '@';
            if (Controller_1B_A) FCTemp[12 + 4] = '@';
            if (Controller_1B_B) FCTemp[12 + 5] = '@';
            if (Controller_1B_C) FCTemp[12 + 6] = '@';
            if (Controller_1B_Start) FCTemp[12 + 7] = '@';
            if (Controller_1B_X) FCTemp[12 + 8] = '@';
            if (Controller_1B_Y) FCTemp[12 + 9] = '@';
            if (Controller_1B_Z) FCTemp[12 + 10] = '@';
            if (Controller_1B_Mode) FCTemp[12 + 11] = '@';
        }
        else {
            if (Controller_2_Up) FCTemp[12 + 0] = '@';
            if (Controller_2_Down) FCTemp[12 + 1] = '@';
            if (Controller_2_Left) FCTemp[12 + 2] = '@';
            if (Controller_2_Right) FCTemp[12 + 3] = '@';
            if (Controller_2_A) FCTemp[12 + 4] = '@';
            if (Controller_2_B) FCTemp[12 + 5] = '@';
            if (Controller_2_C) FCTemp[12 + 6] = '@';
            if (Controller_2_Start) FCTemp[12 + 7] = '@';
            if (Controller_2_X) FCTemp[12 + 8] = '@';
            if (Controller_2_Y) FCTemp[12 + 9] = '@';
            if (Controller_2_Z) FCTemp[12 + 10] = '@';
            if (Controller_2_Mode) FCTemp[12 + 11] = '@';
        }
        if (Controller_1C_Up) FCTemp[12 + 12 + 0] = '@';
        if (Controller_1C_Down) FCTemp[12 + 12 + 1] = '@';
        if (Controller_1C_Left) FCTemp[12 + 12 + 2] = '@';
        if (Controller_1C_Right) FCTemp[12 + 12 + 3] = '@';
        if (Controller_1C_A) FCTemp[12 + 12 + 4] = '@';
        if (Controller_1C_B) FCTemp[12 + 12 + 5] = '@';
        if (Controller_1C_C) FCTemp[12 + 12 + 6] = '@';
        if (Controller_1C_Start) FCTemp[12 + 12 + 7] = '@';
        if (Controller_1C_X) FCTemp[12 + 12 + 8] = '@';
        if (Controller_1C_Y) FCTemp[12 + 12 + 9] = '@';
        if (Controller_1C_Z) FCTemp[12 + 12 + 10] = '@';
        if (Controller_1C_Mode) FCTemp[12 + 12 + 11] = '@';
        n = FrameCounterPosition - 2352;
        if (!IS_FULL_X_RESOLUTION && (FrameCounterPosition == FRAME_COUNTER_TOP_RIGHT || FrameCounterPosition == FRAME_COUNTER_BOTTOM_RIGHT))
            n -= 64;
        for (pos = 0; pos < 12; pos++) //upthmodif
        {
            l[0] = (FCTemp[pos] - '@') * 20 + 420;
            l[1] = (FCTemp[pos + 12] - '@') * 20 + 420;
            l[2] = (FCTemp[pos + 12 + 12] - '@') * 20 + 420;

            for (j = 0; j < 7; j++, n += 332)
            {
                for (i = 0; i < 4; i++, n++)
                {
                    if (j > 0 && j < 6) {
                        if (Controller_1_Type & 0x10)
                        {
                            if (Bits32) MD_Screen32[n] = (FCDigit32[l[0]++] & FCColor32[0]) | (FCDigit32[l[1]++] & FCColor32[1]) | (FCDigit32[l[2]++] & FCColor32[2]); //Upthmodif - three player display support - three player mode
                            else MD_Screen[n] = (FCDigit[l[0]++] & FCColor[Mode_555 & 1][0]) | (FCDigit[l[1]++] & FCColor[Mode_555 & 1][1]) | (FCDigit[l[2]++] & FCColor[Mode_555 & 1][2]); //Upthmodif - three player display support - three player mode
                        }
                        else
                        {
                            if (Bits32) MD_Screen32[n] = (FCDigit32[l[0]++] & FCColor32[3]) | (FCDigit32[l[1]++] & FCColor32[4]); //Upthmodif - three player display support - three player mode
                            else MD_Screen[n] = (FCDigit[l[0]++] & FCColor[Mode_555 & 1][3]) | (FCDigit[l[1]++] & FCColor[Mode_555 & 1][4]); //Upthmodif - three player display support - two player mode
                        }
                    }
                    else
                    {
                        if (Bits32) MD_Screen32[n] = 0x0000;
                        else MD_Screen[n] = 0x0000;
                    }
                    MD_Screen32[n] |= 0xFF000000; // alpha added (opaque)
                }
            }
            n -= 336 * 7 - 4;
        }
    }

    //// original SONIC THE HEDGEHOG 3 SPEEDOMETER HACK
    //{
    //	char temp [128];
    //	extern unsigned char Ram_68k[64 * 1024];
    //	// sonic 1: D010 ... sonic 2: B010 ... sonic 3: B018
    //	short xvel = (short)(Ram_68k[0xB018] | (Ram_68k[0xB018 + 1] << 8));
    //	short yvel = (short)(Ram_68k[0xB01A] | (Ram_68k[0xB01A + 1] << 8));
    //	short xvel2 = (short)(Ram_68k[0xB062] | (Ram_68k[0xB062 + 1] << 8)); // tails
    //	short yvel2 = (short)(Ram_68k[0xB064] | (Ram_68k[0xB064 + 1] << 8));
    //	sprintf(temp, "(%d, %d) [%d, %d]", xvel,yvel, xvel2,yvel2);
    //	Message_Showed = 1;
    //	if(GetTickCount() > Info_Time)
    //	{
    //		Info_Time = 0x7fffffff;
    //		strcpy(Info_String, "");
    //	}
    //	char temp2 [1024];
    //	char* paren = strchr(Info_String, ']');
    //	strcpy(temp2, paren ? (paren+2) : Info_String);
    //	sprintf(Info_String, "%s %s", temp, temp2);
    //}

    if (Message_Showed)
    {
        if (GetTickCount() > Info_Time)
        {
            Message_Showed = 0;
            strcpy(Info_String, "");
        }
        else //Modif N - outlined message text, so it isn't unreadable on any background color:
        {
            int backColor = (((Message_Style & (BLUE | GREEN | RED)) == BLUE) ? RED : BLUE) | (Message_Style & SIZE_X2) | (Message_Style & TRANS);
            const static int xOffset[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
            const static int yOffset[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
            for (int i = 0; i < 8; i++)
                Print_Text(Info_String, strlen(Info_String), 10 + xOffset[i], 210 + yOffset[i], backColor);
            Print_Text(Info_String, strlen(Info_String), 10, 210, Message_Style);
        }
    }
    else if (Show_FPS)
    {
        if (freq_cpu[0] > 1)				// accurate timer ok
        {
            if (++view_fps >= 16)
            {
                QueryPerformanceCounter((union _LARGE_INTEGER *) new_time);

                if (new_time[0] != old_time)
                {
                    FPS = (float)(freq_cpu[0]) * 16.0f / (float)(new_time[0] - old_time);
                    sprintf(Info_String, "%.1f", FPS);
                }
                else
                {
                    sprintf(Info_String, "too much...");
                }

                old_time = new_time[0];
                view_fps = 0;
            }
        }
        else if (freq_cpu[0] == 1)			// accurate timer not supported
        {
            if (++view_fps >= 10)
            {
                new_time[0] = GetTickCount();

                if (new_time[0] != old_time) frames[index_fps] = 10000 / (float)(new_time[0] - old_time);
                else frames[index_fps] = 2000;

                index_fps++;
                index_fps &= 7;
                FPS = 0.0f;

                for (i = 0; i < 8; i++) FPS += frames[i];

                FPS /= 8.0f;
                old_time = new_time[0];
                view_fps = 0;
            }

            sprintf(Info_String, "%.1f", FPS);
        }
        else
        {
            QueryPerformanceFrequency((union _LARGE_INTEGER *) freq_cpu);
            if (freq_cpu[0] == 0) freq_cpu[0] = 1;

            sprintf(Info_String, "%.1f", FPS);
        }
        int backColor = (((FPS_Style & (BLUE | GREEN | RED)) == BLUE) ? RED : BLUE) | (FPS_Style & SIZE_X2) | (FPS_Style & TRANS);
        const static int xOffset[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
        const static int yOffset[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
        for (int i = 0; i < 8; i++)
            Print_Text(Info_String, strlen(Info_String), 10 + xOffset[i], 210 + yOffset[i], backColor);
        Print_Text(Info_String, strlen(Info_String), 10, 210, FPS_Style);
    }
#ifdef _DEBUG
    char str[1024];
    sprintf(str, Debug_Format, DEBUG_VARIABLES);
    PutText2(str, Debug_xPos, Debug_yPos);
#endif
}

int Flip(HWND hWnd)
{
    if (!lpDD)
        return 0; // bail if directdraw hasn't been initialized yet or if we're still in the middle of initializing it

    HRESULT rval = DD_OK;
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    RECT RectDest, RectSrc;
    int bpp = Bits32 ? 4 : 2; // Modif N. -- added: bytes per pixel

    DrawInformationOnTheScreen(); // Modif N. -- moved all this stuff out to its own function

    if (Fast_Blur) Half_Blur();

    CalculateDrawArea(hWnd, RectDest, RectSrc);

    int Src_X = (RectSrc.right - RectSrc.left);
    int Src_Y = (RectSrc.bottom - RectSrc.top);

    int Clr_Cmp_Val = IS_FULL_X_RESOLUTION ? 4 : 8;
    Clr_Cmp_Val |= IS_FULL_Y_RESOLUTION ? 16 : 32;

    if (Flag_Clr_Scr & 0x100) // need clear second buffer
    {
        Clear_Primary_Screen(hWnd);
        Flag_Clr_Scr ^= 0x300; // already cleared
    }

    if ((Flag_Clr_Scr & 0xFF) != (Clr_Cmp_Val & 0xFF))
    {
        if (!Full_Screen && W_VSync)
            lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
        if (!(Flag_Clr_Scr & 0x200))
            Clear_Primary_Screen(hWnd); // already cleared
        if ((!Full_Screen && Render_W >= 2)
            || (Full_Screen && Render_FS >= 2))
            Clear_Back_Screen(hWnd);

        if (Full_Screen && FS_VSync)
            Flag_Clr_Scr = Clr_Cmp_Val | 0x100; // need to clear second buffer
        else
            Flag_Clr_Scr = Clr_Cmp_Val;
    }

    Flag_Clr_Scr &= 0x1FF; // remove "already cleared"

    if (Full_Screen)
    {
        if (Render_FS < 2)
        {
            if (FS_VSync)
            {
                lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
                lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
            }
            else
            {
                lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
                //				lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
            }
        }
        else
        {
            // save Primary or Flip surface
            LPDIRECTDRAWSURFACE4 curBlit = lpDDS_Blit;

            // check equal size
            if ((RectDest.right - RectDest.left) != Src_X
                || (RectDest.bottom - RectDest.top) != Src_Y)
                curBlit = lpDDS_Back; // replace with Back

            rval = curBlit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

            if (FAILED(rval)) goto cleanup_flip;

            if (curBlit == lpDDS_Back) // note: this can happen in windowed fullscreen, or if Correct_256_Aspect_Ratio is defined and the current display mode is 256 pixels across
            {
                // blit into lpDDS_Back first
                Blit_FS((unsigned char *)ddsd.lpSurface + ddsd.lPitch * RectSrc.top + RectSrc.left * bpp, ddsd.lPitch, Src_X / 2, Src_Y / 2, (16 + 320 - Src_X / 2) * bpp);

                curBlit->Unlock(NULL);

                if (FS_VSync)
                {
                    lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
                    lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
                }
                else
                    // blit into Primary
                    lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
            }
            else
            {
                // blit direct on screen (or flip if VSync)
                Blit_FS((unsigned char *)ddsd.lpSurface + ddsd.lPitch * RectDest.top + RectDest.left * bpp, ddsd.lPitch, Src_X / 2, Src_Y / 2, (16 + 320 - Src_X / 2) * bpp);

                curBlit->Unlock(NULL);

                if (FS_VSync)
                {
                    lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
                }
            }
        }
    }
    else
    {
        // Window
        lpDDS_Blit = lpDDS_Back;

        if (Render_W >= 2)
        {
            rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

            if (FAILED(rval)) goto cleanup_flip;

            Blit_W((unsigned char *)ddsd.lpSurface + ddsd.lPitch * RectSrc.top + RectSrc.left * bpp, ddsd.lPitch, Src_X / 2, Src_Y / 2, (16 + 320 - Src_X / 2) * bpp);

            lpDDS_Blit->Unlock(NULL);
        }

        if (RectDest.top < RectDest.bottom)
        {
            if (W_VSync)
            {
                // wait if it is clearing primary, wait for VBlank only after this
                while (lpDDS_Primary->GetBltStatus(DDGBS_ISBLTDONE) == DDERR_WASSTILLDRAWING);
                /*int vb;
                lpDD->GetVerticalBlankStatus(&vb);
                if (!vb)*/ lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
            }

            rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
            //			rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
        }
    }

cleanup_flip:
    if (rval == DDERR_SURFACELOST)
        rval = RestoreGraphics(hWnd);

    return 1;
}

int Update_Gens_Logo(HWND hWnd)
{
    int i, j, m, n;
    static short tab[64000], Init = 0;
    static float renv = 0, ang = 0, zoom_x = 0, zoom_y = 0, pas;
    unsigned short c;

    if (!Init)
    {
        HBITMAP Logo;

        Logo = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_LOGO_BIG));
        GetBitmapBits(Logo, 64000 * 2, tab);
        pas = 0.05f;
        Init = 1;
    }

    renv += pas;
    zoom_x = (float)sin(renv);
    if (zoom_x == 0.0f) zoom_x = 0.0000001f;
    zoom_x = (1 / zoom_x) * 1;
    zoom_y = 1;

    if (IS_FULL_X_RESOLUTION)
    {
        for (j = 0; j < 240; j++)
        {
            for (i = 0; i < 320; i++)
            {
                m = (int)((float)(i - 160) * zoom_x);
                n = (int)((float)(j - 120) * zoom_y);

                if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
                {
                    c = tab[m + 130 + (n + 90) * 260];
                    if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
                }
            }
        }
    }
    else
    {
        for (j = 0; j < 240; j++)
        {
            for (i = 0; i < 256; i++)
            {
                m = (int)((float)(i - 128) * zoom_x);
                n = (int)((float)(j - 120) * zoom_y);

                if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
                {
                    c = tab[m + 130 + (n + 90) * 260];
                    if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
                }
            }
        }
    }

    Half_Blur();
    Flip(hWnd);

    return 1;
}

int Update_Crazy_Effect(HWND hWnd)
{
    int i, j, offset;
    int r = 0, v = 0, b = 0, prev_l, prev_p;
    int RB, G;

    for (offset = 336 * 240, j = 0; j < 240; j++)
    {
        for (i = 0; i < 336; i++, offset--)
        {
            prev_l = MD_Screen32[offset - 336];
            prev_p = MD_Screen32[offset - 1];

            RB = ((prev_l & 0xFF00FF) + (prev_p & 0xFF00FF)) >> 1;
            G = ((prev_l & 0x00FF00) + (prev_p & 0x00FF00)) >> 1;

            if (Effect_Color & 0x4)
            {
                r = RB & 0xFF0000;
                if (rand() > 0x1600) r += 0x010000;
                else r -= 0x010000;
                if (r > 0xFF0000) r = 0xFF0000;
                else if (r < 0x00FFFF) r = 0;
            }

            if (Effect_Color & 0x2)
            {
                v = G & 0x00FF00;
                if (rand() > 0x1600) v += 0x000100;
                else v -= 0x000100;
                if (v > 0xFF00) v = 0xFF00;
                else if (v < 0x00FF) v = 0;
            }

            if (Effect_Color & 0x1)
            {
                b = RB & 0xFF;
                if (rand() > 0x1600) b++;
                else b--;
                if (b > 0xFF) b = 0xFF;
                else if (b < 0) b = 0;
            }
            MD_Screen32[offset] = r + v + b;
            prev_l = MD_Screen[offset - 336];
            prev_p = MD_Screen[offset - 1];

            if (Mode_555 & 1)
            {
                RB = ((prev_l & 0x7C1F) + (prev_p & 0x7C1F)) >> 1;
                G = ((prev_l & 0x03E0) + (prev_p & 0x03E0)) >> 1;

                if (Effect_Color & 0x4)
                {
                    r = RB & 0x7C00;
                    if (rand() > 0x2C00) r += 0x0400;
                    else r -= 0x0400;
                    if (r > 0x7C00) r = 0x7C00;
                    else if (r < 0x0400) r = 0;
                }

                if (Effect_Color & 0x2)
                {
                    v = G & 0x03E0;
                    if (rand() > 0x2C00) v += 0x0020;
                    else v -= 0x0020;
                    if (v > 0x03E0) v = 0x03E0;
                    else if (v < 0x0020) v = 0;
                }

                if (Effect_Color & 0x1)
                {
                    b = RB & 0x001F;
                    if (rand() > 0x2C00) b++;
                    else b--;
                    if (b > 0x1F) b = 0x1F;
                    else if (b < 0) b = 0;
                }
            }
            else
            {
                RB = ((prev_l & 0xF81F) + (prev_p & 0xF81F)) >> 1;
                G = ((prev_l & 0x07C0) + (prev_p & 0x07C0)) >> 1;

                if (Effect_Color & 0x4)
                {
                    r = RB & 0xF800;
                    if (rand() > 0x2C00) r += 0x0800;
                    else r -= 0x0800;
                    if (r > 0xF800) r = 0xF800;
                    else if (r < 0x0800) r = 0;
                }

                if (Effect_Color & 0x2)
                {
                    v = G & 0x07C0;
                    if (rand() > 0x2C00) v += 0x0040;
                    else v -= 0x0040;
                    if (v > 0x07C0) v = 0x07C0;
                    else if (v < 0x0040) v = 0;
                }

                if (Effect_Color & 0x1)
                {
                    b = RB & 0x001F;
                    if (rand() > 0x2C00) b++;
                    else b--;
                    if (b > 0x1F) b = 0x1F;
                    else if (b < 0) b = 0;
                }
            }

            MD_Screen[offset] = r + v + b;
        }
    }

    Flip(hWnd);

    return 1;
}

void UpdateInput()
{
    if (MainMovie.Status == MOVIE_PLAYING)
        MoviePlayingStuff();
    else
        Update_Controllers();

    //// XXX - probably most people won't need to use this?
    ////Modif N - playback one player while recording another player
    //if(MainMovie.Status==MOVIE_RECORDING)
    //{
    //	if(GetKeyState(VK_CAPITAL) != 0)
    //	{
    //		extern void MoviePlayPlayer2();
    //		MoviePlayPlayer2();
    //	}
    //	if(GetKeyState(VK_NUMLOCK) != 0)
    //	{
    //		extern void MoviePlayPlayer1();
    //		MoviePlayPlayer1();
    //	}
    //}
}

void UpdateLagCount()
{
    if (Lag_Frame)
    {
        LagCount++;
        LagCountPersistent++;
    }

    // catch-all to fix problem with sound stuttered when paused during frame skipping
    // looks out of place but maybe this function should be renamed
    extern bool soundCleared;
    soundCleared = false;
}

int Dont_Skip_Next_Frame = 0;
void Prevent_Next_Frame_Skipping()
{
    Dont_Skip_Next_Frame = 8;
}

int Update_Emulation(HWND hWnd)
{
    int prevFrameCount = FrameCount;

    static int Over_Time = 0;
    int current_div;

    int Temp_Frame_Skip = Frame_Skip; //Modif N - part of a quick hack to make Tab the fast-forward key
    if (TurboMode)
        Temp_Frame_Skip = 8;
    if (SeekFrame) // If we've set a frame to seek to
    {
        if (FrameCount < (SeekFrame - 5000))      // we fast forward
            Temp_Frame_Skip = 10;
        else if (FrameCount < (SeekFrame - 100)) // at a variable rate
            Temp_Frame_Skip = 0;
        else 						     // based on distance to target
            Temp_Frame_Skip = -1;
        if (((SeekFrame - FrameCount) == 1))
        {
            Paused = 1; //then pause when we arrive
            SeekFrame = 0; //and clear the seek target
            Put_Info("Seek complete. Paused");
            Clear_Sound_Buffer();
            MustUpdateMenu = 1;
        }
    }
    if (Temp_Frame_Skip != -1)
    {
        if (Sound_Enable)
        {
            WP = (WP + 1) & (Sound_Segs - 1);

            if (WP == Get_Current_Seg())
                WP = (WP + Sound_Segs - 1) & (Sound_Segs - 1);

            Write_Sound_Buffer(NULL);
        }
        UpdateInput(); //Modif N
        if (MainMovie.Status == MOVIE_RECORDING)	//Modif
            MovieRecordingStuff();
        FrameCount++; //Modif

        if (Frame_Number++ < Temp_Frame_Skip) //Modif N - part of a quick hack to make Tab the fast-forward key
        {
            Lag_Frame = 1;
            Update_Frame_Fast_Hook();
            UpdateLagCount();
        }
        else
        {
            Frame_Number = 0;
            Lag_Frame = 1;
            Update_Frame_Hook();
            UpdateLagCount();
            Flip(hWnd);
        }
    }
    else
    {
        if (Sound_Enable)
        {
            while (WP == Get_Current_Seg()) Sleep(Sleep_Time);
            RP = Get_Current_Seg();
            while (WP != RP)
            {
                Write_Sound_Buffer(NULL);
                WP = (WP + 1) & (Sound_Segs - 1);
                if (SlowDownMode)
                {
                    if (SlowFrame)
                        SlowFrame--;
                    else
                    {
                        SlowFrame = SlowDownSpeed;
                        UpdateInput(); //Modif N
                        if (MainMovie.Status == MOVIE_RECORDING)	//Modif
                            MovieRecordingStuff();
                        FrameCount++; //Modif

                        // note: we check for RamSearchHWnd because if it's open then it's likely causing most of any slowdown we get, in which case skipping renders will only make the slowdown appear worse
                        if (WP != RP && Never_Skip_Frame == 0 && !Dont_Skip_Next_Frame && !(RamSearchHWnd || RamWatchHWnd))
                        {
                            Lag_Frame = 1;
                            Update_Frame_Fast_Hook();
                            UpdateLagCount();
                        }
                        else
                        {
                            Lag_Frame = 1;
                            Update_Frame_Hook();
                            UpdateLagCount();
                            Flip(hWnd);
                        }
                    }
                }
                else
                {
                    UpdateInput(); //Modif N
                    if (MainMovie.Status == MOVIE_RECORDING)	//Modif
                        MovieRecordingStuff();
                    FrameCount++; //Modif

                    if (WP != RP && Never_Skip_Frame == 0 && !Dont_Skip_Next_Frame && !(RamSearchHWnd || RamWatchHWnd))
                    {
                        Lag_Frame = 1;
                        Update_Frame_Fast_Hook();
                        UpdateLagCount();
                    }
                    else
                    {
                        Lag_Frame = 1;
                        Update_Frame_Hook();
                        UpdateLagCount();
                        Flip(hWnd);
                    }
                }
                if (Never_Skip_Frame || Dont_Skip_Next_Frame)
                    WP = RP;
            }
        }
        else
        {
            if (CPU_Mode) current_div = 20;
            else current_div = 16 + (Over_Time ^= 1);

            if (SlowDownMode)
                current_div *= (SlowDownSpeed + 1);

            New_Time = GetTickCount();
            Used_Time += (New_Time - Last_Time);
            Frame_Number = Used_Time / current_div;
            Used_Time %= current_div;
            Last_Time = New_Time;

            if (Frame_Number > 8) Frame_Number = 8;
            if ((Never_Skip_Frame != 0 || Dont_Skip_Next_Frame) && Frame_Number > 0)
                Frame_Number = 1;
            /*if(SlowDownMode)
            {
            if(SlowFrame)
            {
            Frame_Number>>=1; //Modif
            SlowFrame--;
            }
            else
            SlowFrame=SlowDownSpeed;
            }*/

            for (; Frame_Number > 1; Frame_Number--)
            {
                UpdateInput(); //Modif N
                if (MainMovie.Status == MOVIE_RECORDING)	//Modif
                    MovieRecordingStuff();
                FrameCount++; //Modif

                if (Never_Skip_Frame == 0 && !Dont_Skip_Next_Frame)
                {
                    Lag_Frame = 1;
                    Update_Frame_Fast_Hook();
                    UpdateLagCount();
                }
                else
                {
                    Lag_Frame = 1;
                    Update_Frame_Hook();
                    UpdateLagCount();
                    Flip(hWnd);
                }
            }

            if (Frame_Number)
            {
                UpdateInput(); //Modif N
                if (MainMovie.Status == MOVIE_RECORDING)	//Modif
                    MovieRecordingStuff();
                FrameCount++; //Modif
                Lag_Frame = 1;
                Update_Frame_Hook();
                UpdateLagCount();
                Flip(hWnd);
            }
            else Sleep(Sleep_Time);
        }
    }

    if (Dont_Skip_Next_Frame)
        Dont_Skip_Next_Frame--;

    return prevFrameCount != FrameCount;
}

void Update_Emulation_One_Before(HWND hWnd)
{
    if (Sound_Enable)
    {
        WP = (WP + 1) & (Sound_Segs - 1);

        if (WP == Get_Current_Seg())
            WP = (WP + Sound_Segs - 1) & (Sound_Segs - 1);

        Write_Sound_Buffer(NULL);
    }
    UpdateInput(); //Modif N
    if (MainMovie.Status == MOVIE_RECORDING)	//Modif
        MovieRecordingStuff();
    FrameCount++; //Modif
    Lag_Frame = 1;
}

void Update_Emulation_One_Before_Minimal()
{
    UpdateInput();
    if (MainMovie.Status == MOVIE_RECORDING)
        MovieRecordingStuff();
    FrameCount++;
    Lag_Frame = 1;
}

void Update_Emulation_One_After(HWND hWnd)
{
    UpdateLagCount();
    Flip(hWnd);
}

void Update_Emulation_After_Fast(HWND hWnd)
{
    UpdateLagCount();

    int Temp_Frame_Skip = Frame_Skip;
    if (TurboMode)
        Temp_Frame_Skip = 8;
    if (Frame_Number > 8) Frame_Number = 8;
    if (Frame_Number++ < Temp_Frame_Skip)
        return;
    Frame_Number = 0;

    Flip(hWnd);
}

void Update_Emulation_After_Controlled(HWND hWnd, bool flip)
{
    UpdateLagCount();

    if (flip)
    {
        Flip(hWnd);
    }
}

int Update_Emulation_One(HWND hWnd)
{
    Update_Emulation_One_Before(hWnd);
    Update_Frame_Hook();
    Update_Emulation_One_After(hWnd);
    return 1;
}

int Eff_Screen(void)
{
    int i;

    for (i = 0; i < 336 * 240; i++)
    {
        MD_Screen[i] = 0;
        MD_Screen32[i] = 0;
    }

    return 1;
}

int Pause_Screen(void)
{
    int i, j, offset;
    int r, v, b, nr, nv, nb;

    if (Disable_Blue_Screen) return 1;

    r = v = b = nr = nv = nb = 0;

    for (offset = j = 0; j < 240; j++)
    {
        for (i = 0; i < 336; i++, offset++)
        {
            r = (MD_Screen32[offset] & 0xFF0000) >> 16;
            v = (MD_Screen32[offset] & 0x00FF00) >> 8;
            b = (MD_Screen32[offset] & 0x0000FF);

            nr = nv = nb = (r + v + b) / 3;

            if ((nb <<= 1) > 255) nb = 255;

            nr &= 0xFE;
            nv &= 0xFE;
            nb &= 0xFE;

            MD_Screen32[offset] = (nr << 16) + (nv << 8) + nb;

            if (Mode_555 & 1)
            {
                r = (MD_Screen[offset] & 0x7C00) >> 10;
                v = (MD_Screen[offset] & 0x03E0) >> 5;
                b = (MD_Screen[offset] & 0x001F);
            }
            else
            {
                r = (MD_Screen[offset] & 0xF800) >> 11;
                v = (MD_Screen[offset] & 0x07C0) >> 6;
                b = (MD_Screen[offset] & 0x001F);
            }

            nr = nv = nb = (r + v + b) / 3;

            if ((nb <<= 1) > 31) nb = 31;

            nr &= 0x1E;
            nv &= 0x1E;
            nb &= 0x1E;

            if (Mode_555 & 1)
                MD_Screen[offset] = (nr << 10) + (nv << 5) + nb;
            else
                MD_Screen[offset] = (nr << 11) + (nv << 6) + nb;
        }
    }

    return 1;
}

int Show_Genesis_Screen(HWND hWnd)
{
    Do_VDP_Refresh();
    Flip(hWnd);

    return 1;
}
int Show_Genesis_Screen()
{
    return Show_Genesis_Screen(HWnd);
}

/*
void MP3_init_test()
{
FILE *f;
f = fopen("\\vc\\gens\\release\\test.mp3", "rb");
if (f == NULL) return;
MP3_Test(f);
Play_Sound();
}

void MP3_update_test()
{
int *buf[2];
buf[0] = Seg_L;
buf[1] = Seg_R;
while (WP == Get_Current_Seg());
RP = Get_Current_Seg();
while (WP != RP)
{
Write_Sound_Buffer(NULL);
WP = (WP + 1) & (Sound_Segs - 1);
memset(Seg_L, 0, (Seg_Length << 2));
memset(Seg_R, 0, (Seg_Length << 2));
MP3_Update(buf, Seg_Length);
}
}
*/