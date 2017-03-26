#include <stdio.h>
#include "gens.h"
#include "g_main.h"
#include "g_ddraw.h"
#include "g_dsound.h"
#include "g_input.h"
#include "rom.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "ym2612.h"
#include "psg.h"
#include "cpu_68k.h"
#include "cpu_z80.h"
#include "cpu_sh2.h"
#include "z80.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32x.h"
#include "joypads.h"
#include "misc.h"
#include "save.h"
#include "ggenie.h"
#include "cd_sys.h"
#include "lc89510.h"
#include "gfx_cd.h"
#include "wave.h"
#include "pcm.h"
#include "pwm.h"
#include "cd_sys.h"
#include "cd_file.h"
#include "movie.h"
#include "ram_search.h"
#include "luascript.h"

// uncomment this to run a simple test every frame for potential desyncs
// you do not need to start any movie for the test to work, just play
// expect it to be pretty slow, though
//#define TEST_GENESIS_FOR_DESYNCS

// uncomment this to test the sega-cd specific parts of the savestates for desyncs
// (you should still use TEST_GENESIS_FOR_DESYNCS to test for desyncs in sega cd games,
//  the only reason to use this instead is to do an extra-finicky check that could fail earlier)
// IMPORTANT: in Visual Studio you'll need to increase the Stack Reserve Size
// to something like 4194304 in your Project Settings > Linker > System tab
// otherwise this will give you a stack overflow error
//#define TEST_SEGACD_FOR_DESYNCS

// uncomment this to test the 32x-specific parts of the savestates for desyncs
//#define TEST_32X_FOR_DESYNCS

// uncomment this in addition to one of the above to allow for
// calling DesyncDetection from elsewhere without forcing it on all frame updates
//#define MANUAL_DESYNC_CHECKS_ONLY

#if defined(TEST_GENESIS_FOR_DESYNCS) || defined(TEST_SEGACD_FOR_DESYNCS) || defined(TEST_32X_FOR_DESYNCS)
#define TEST_FOR_DESYNCS
#include <assert.h>
#include <map>

#ifdef TEST_GENESIS_FOR_DESYNCS
static const int CHECKED_STATE_LENGTH = GENESIS_STATE_LENGTH;
#define Export_Func Export_Genesis
#if defined(TEST_SEGACD_FOR_DESYNCS) || defined(TEST_32X_FOR_DESYNCS)
#error Sorry, only one of TEST_GENESIS_FOR_DESYNCS, TEST_SEGACD_FOR_DESYNCS, TEST_32X_FOR_DESYNCS can be defined at a time
#endif
#elif defined(TEST_SEGACD_FOR_DESYNCS)
static const int CHECKED_STATE_LENGTH = SEGACD_LENGTH_EX;
#define Export_Func Export_SegaCD
#if defined(TEST_32X_FOR_DESYNCS)
#error Sorry, only one of TEST_GENESIS_FOR_DESYNCS, TEST_SEGACD_FOR_DESYNCS, TEST_32X_FOR_DESYNCS can be defined at a time
#endif
#elif defined(TEST_32X_FOR_DESYNCS)
static const int CHECKED_STATE_LENGTH = G32X_LENGTH_EX;
#define Export_Func Export_32X
#endif

struct SaveStateData
{
    unsigned char State_Buffer[CHECKED_STATE_LENGTH];
};
std::map<int, SaveStateData> saveStateDataMap;

int firstFailureByte = -1;
bool CompareSaveStates(SaveStateData& data1, SaveStateData& data2)
{
    SaveStateData difference;
    bool ok = true;
    firstFailureByte = -1;
    static const int maxFailures = 4000; // print at most this many failure at once
    int numFailures = 0;
    for (int i = 0; i < CHECKED_STATE_LENGTH; i++)
    {
        unsigned char diff = data2.State_Buffer[i] - data1.State_Buffer[i];
        difference.State_Buffer[i] = diff;
        if (diff)
        {
            char desc[128];
            sprintf(desc, "byte %d (0x%x) was 0x%x is 0x%x\n", i, i, data1.State_Buffer[i], data2.State_Buffer[i]);
            OutputDebugString(desc);
            if (ok)
            {
                ok = false;
                firstFailureByte = i;
            }
            numFailures++;
            if (numFailures > maxFailures)
                break;
        }
    }
    return ok;
}

static SaveStateData tempData;

void DesyncDetection(bool forceCheckingDesync = false, bool forcePart = false)
{
#ifdef TEST_GENESIS_FOR_DESYNCS
    if (!Game) return;
#endif
#ifdef TEST_SEGACD_FOR_DESYNCS
    if (!SegaCD_Started) return;
#endif
#ifdef TEST_32X_FOR_DESYNCS
    if (!_32X_Started) return;
#endif
    // (only if forceCheckingDesync is false)
    // hold control to save a savestate for frames in a movie for later checking
    // then turn on scroll lock when replaying those frames later to check them
    bool checkingDesync = (GetKeyState(VK_LCONTROL) & 0x8000) != 0;
    int checkingDesyncPart = GetKeyState(VK_SCROLL) ? 1 : 0;
    if (!forceCheckingDesync)
        checkingDesync |= !!checkingDesyncPart;
    else
    {
        checkingDesync = true;
        checkingDesyncPart = forcePart ? 1 : 0;
    }

    if (checkingDesync)
    {
        if (checkingDesyncPart == 0)
        {
            // first part: just save states
            static SaveStateData data;

            //Save_State_To_Buffer(data.State_Buffer);
            memset(data.State_Buffer, 0, sizeof(data.State_Buffer));
            Export_Func(data.State_Buffer);

            saveStateDataMap[FrameCount] = data;
        }
        else
        {
            // second part: compare to saved states
            static SaveStateData dataNow;

            //Save_State_To_Buffer(dataNow.State_Buffer);
            memset(dataNow.State_Buffer, 0, sizeof(dataNow.State_Buffer));
            Export_Func(dataNow.State_Buffer);

            if (saveStateDataMap.find(FrameCount) != saveStateDataMap.end())
            {
                SaveStateData& dataThen = saveStateDataMap[FrameCount];
                if (!CompareSaveStates(dataThen, dataNow))
                {
                    memset(tempData.State_Buffer, tempData.State_Buffer[firstFailureByte] ? 0 : 0xFF, sizeof(tempData.State_Buffer));

                    // if this assert fails, congratulations, you got a desync!
                    assert(0);
                    // check your output window for details.
                    // now you might want to know which part of the savestate went wrong.
                    // to do that, go to the Breakpoints window and add a new Data breakpoint and
                    // set it to break when the data at &tempData.State_Buffer[firstFailureByte] changes,
                    // then continue running and it should break inside the export function where the desyncing data gets saved.

                    Export_Func(tempData.State_Buffer);
                }
            }
        }
    }
}

#ifndef MANUAL_DESYNC_CHECKS_ONLY

int Do_Genesis_Frame_Real();
int Do_Genesis_Frame_No_VDP_Real();
int Do_SegaCD_Frame_Real();
int Do_SegaCD_Frame_No_VDP_Real();
int Do_SegaCD_Frame_Cycle_Accurate_Real();
int Do_SegaCD_Frame_No_VDP_Cycle_Accurate_Real();
int Do_32X_Frame_Real();
int Do_32X_Frame_No_VDP_Real();

#define DO_FRAME_HEADER(name, fastname) \
	int name() \
	{ \
		if(!((TurboMode))) { \
			Save_State_To_Buffer(State_Buffer); /* save */ \
			disableSound = true; \
			fastname##_Real(); /* run 2 frames */ \
			fastname##_Real(); \
			DesyncDetection(1,0); /* save */ \
			Load_State_From_Buffer(State_Buffer); /* load */ \
			fastname##_Real(); /* run 2 frames */ \
			fastname##_Real(); \
			DesyncDetection(1,1); /* check that it's identical to last save ... this is the slow part */ \
			Load_State_From_Buffer(State_Buffer); /* load */ \
			saveStateDataMap.clear(); \
			disableSound = false; \
                		} \
		return name##_Real(); /* run the frame for real */ \
	} \
	int name##_Real()
#else // MANUAL_DESYNC_CHECKS_ONLY:
#define DO_FRAME_HEADER(name, fastname) int name()
#endif

BOOL IsAsyncAllowed(void)
{
    // no asynchronous stuff allowed when testing for desyncs
    return false;
}

#else // !TEST_FOR_DESYNCS:

#define DO_FRAME_HEADER(name, fastname) int name()

BOOL IsAsyncAllowed(void)
{
    // no asynchronous stuff allowed when playng or recording a movie
    if (MainMovie.Status == MOVIE_RECORDING)
        return false;
    if (MainMovie.Status == MOVIE_PLAYING)
        return false;
    return true;
}

#endif // TEST_FOR_DESYNCS

int Frame_Skip;
int Frame_Number;
int Inside_Frame = 0;
int DAC_Improv;
int RMax_Level;
int GMax_Level;
int BMax_Level;
int Contrast_Level;
int Brightness_Level;
int Greyscale;
int Invert_Color;
int FakeVDPScreen = true;
int VDP_Reg_Set2_Current;
int VDP_Reg_Set4_Current;

unsigned char CD_Data[1024];		// Used for hard reset to know the game name
int SRAM_Was_On = 0;

int Round_Double(double val)
{
    if ((val - (double)(int)val) > 0.5) return (int)(val + 1);
    else return (int)val;
}

void Init_Tab(void)
{
    int x, y, dep;

    for (x = 0; x < 1024; x++)
    {
        for (y = 0; y < 64; y++)
        {
            dep = (x & 3) + (((x & 0x3FC) >> 2) << 8);
            dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
            dep >>= 1;
            Cell_Conv_Tab[(x >> 1) + (y << 9)] = (unsigned short)dep;
        }
    }

    for (x = 0; x < 512; x++)
    {
        for (y = 0; y < 64; y++)
        {
            dep = (x & 3) + (((x & 0x1FC) >> 2) << 8);
            dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
            dep >>= 1;
            Cell_Conv_Tab[(x >> 1) + (y << 8) + 0x8000] = (unsigned short)(dep + 0x8000);
        }
    }

    for (x = 0; x < 256; x++)
    {
        for (y = 0; y < 64; y++)
        {
            dep = (x & 3) + (((x & 0xFC) >> 2) << 8);
            dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
            dep >>= 1;
            Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xC000] = (unsigned short)(dep + 0xC000);
        }
    }

    for (x = 0; x < 256; x++)
    {
        for (y = 0; y < 32; y++)
        {
            dep = (x & 3) + (((x & 0xFC) >> 2) << 7);
            dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
            dep >>= 1;
            Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xE000] = (unsigned short)(dep + 0xE000);
            Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xF000] = (unsigned short)(dep + 0xF000);
        }
    }

    for (x = 0; x < 512; x++) Z80_M68K_Cycle_Tab[x] = (int)((double)x * 7.0 / 15.0);
}

void Recalculate_Palettes(void)
{
    int i;
    int r, g, b;
    int rf, gf, bf;
    int bright, cont;

    for (r = 0; r < 0x10; r++)
    {
        for (g = 0; g < 0x10; g++)
        {
            for (b = 0; b < 0x10; b++)
            {
                rf = (r & 0xF) << 2;
                gf = (g & 0xF) << 2;
                bf = (b & 0xF) << 2;

                rf = (int)((double)(rf)* ((double)(RMax_Level) / 224.0));
                gf = (int)((double)(gf)* ((double)(GMax_Level) / 224.0));
                bf = (int)((double)(bf)* ((double)(BMax_Level) / 224.0));

                // Compute colors here (64 levels)

                bright = Brightness_Level;
                bright -= 100;
                bright *= 32;
                bright /= 100;

                rf += bright;
                gf += bright;
                bf += bright;

                if (rf < 0) rf = 0;
                else if (rf > 0x3F) rf = 0x3F;
                if (gf < 0) gf = 0;
                else if (gf > 0x3F) gf = 0x3F;
                if (bf < 0) bf = 0;
                else if (bf > 0x3F) bf = 0x3F;

                cont = Contrast_Level;

                rf = (rf * cont) / 100;
                gf = (gf * cont) / 100;
                bf = (bf * cont) / 100;

                if (rf < 0) rf = 0;
                else if (rf > 0x3F) rf = 0x3F;
                if (gf < 0) gf = 0;
                else if (gf > 0x3F) gf = 0x3F;
                if (bf < 0) bf = 0;
                else if (bf > 0x3F) bf = 0x3F;

                rf <<= 18;
                gf <<= 10;
                bf <<= 2;

                Palette32[(b << 8) | (g << 4) | r] = 0xFF000000 | rf | gf | bf; // alpha added

                rf >>= 18;
                gf >>= 10;
                bf >>= 2;

                if (Mode_555 & 1)
                {
                    rf = (rf >> 1) << 10;
                    gf = (gf >> 1) << 5;
                }
                else
                {
                    rf = (rf >> 1) << 11;
                    gf = (gf >> 0) << 5;
                }
                bf = (bf >> 1) << 0;

                Palette[(b << 8) | (g << 4) | r] = rf | gf | bf;
            }
        }
    }

    for (i = 0; i < 0x10000; i++)
    {
        b = ((i >> 10) & 0x1F) << 1;
        g = ((i >> 5) & 0x1F) << 1;
        r = ((i >> 0) & 0x1F) << 1;

        r = (int)((double)(r)* ((double)(RMax_Level) / 248.0));
        g = (int)((double)(g)* ((double)(GMax_Level) / 248.0));
        b = (int)((double)(b)* ((double)(BMax_Level) / 248.0));

        // Compute colors here (64 levels)

        bright = Brightness_Level;
        bright -= 100;
        bright *= 32;
        bright /= 100;

        r += bright;
        g += bright;
        b += bright;

        if (r < 0) r = 0;
        else if (r > 0x3F) r = 0x3F;
        if (g < 0) g = 0;
        else if (g > 0x3F) g = 0x3F;
        if (b < 0) b = 0;
        else if (b > 0x3F) b = 0x3F;

        cont = Contrast_Level;

        r = (r * cont) / 100;
        g = (g * cont) / 100;
        b = (b * cont) / 100;

        if (r < 0) r = 0;
        else if (r > 0x3F) r = 0x3F;
        if (g < 0) g = 0;
        else if (g > 0x3F) g = 0x3F;
        if (b < 0) b = 0;
        else if (b > 0x3F) b = 0x3F;

        r <<= 18;
        g <<= 10;
        b <<= 2;

        _32X_Palette_32B[i] = 0xFF000000 | r | g | b; // alpha added

        r >>= 18;
        g >>= 10;
        b >>= 2;

        if (Mode_555 & 1)
        {
            r = (r >> 1) << 10;
            g = (g >> 1) << 5;
        }
        else
        {
            r = (r >> 1) << 11;
            g = (g >> 0) << 5;
        }
        b = (b >> 1) << 0;

        _32X_Palette_16B[i] = r | g | b;
    }

    if (Greyscale)
    {
        for (i = 0; i < 0x1000; i++)
        {
            r = (Palette32[i] >> 16) & 0xFF;
            g = (Palette32[i] >> 8) & 0xFF;
            b = Palette32[i] & 0xFF;

            r = (r * unsigned int(0.30 * 65536.0)) >> 16;
            g = (g * unsigned int(0.59 * 65536.0)) >> 16;
            b = (b * unsigned int(0.11 * 65536.0)) >> 16;

            r = g = b = r + g + b;

            r <<= 16;
            g <<= 8;

            Palette32[i] = 0xFF000000 | r | g | b; // alpha added

            if (Mode_555 & 1)
            {
                r = ((Palette[i] >> 10) & 0x1F) << 1;
                g = ((Palette[i] >> 5) & 0x1F) << 1;
            }
            else
            {
                r = ((Palette[i] >> 11) & 0x1F) << 1;
                g = (Palette[i] >> 5) & 0x3F;
            }

            b = ((Palette[i] >> 0) & 0x1F) << 1;

            r = (r * unsigned int(0.30 * 65536.0)) >> 16;
            g = (g * unsigned int(0.59 * 65536.0)) >> 16;
            b = (b * unsigned int(0.11 * 65536.0)) >> 16;

            r = g = b = r + g + b;

            if (Mode_555 & 1)
            {
                r = (r >> 1) << 10;
                g = (g >> 1) << 5;
            }
            else
            {
                r = (r >> 1) << 11;
                g = (g >> 0) << 5;
            }

            b = (b >> 1) << 0;

            Palette[i] = r | g | b;
        }

        for (i = 0; i < 0x10000; i++)
        {
            r = (_32X_Palette_32B[i] >> 16) & 0xFF;
            g = (_32X_Palette_32B[i] >> 8) & 0xFF;
            b = _32X_Palette_32B[i] & 0xFF;

            r = (r * unsigned int(0.30 * 65536.0)) >> 16;
            g = (g * unsigned int(0.59 * 65536.0)) >> 16;
            b = (b * unsigned int(0.11 * 65536.0)) >> 16;

            r = g = b = r + g + b;

            r <<= 16;
            g <<= 8;

            _32X_Palette_32B[i] = 0xFF000000 | r | g | b; // alpha added

            if (Mode_555 & 1)
            {
                r = ((_32X_Palette_16B[i] >> 10) & 0x1F) << 1;
                g = ((_32X_Palette_16B[i] >> 5) & 0x1F) << 1;
            }
            else
            {
                r = ((_32X_Palette_16B[i] >> 11) & 0x1F) << 1;
                g = (_32X_Palette_16B[i] >> 5) & 0x3F;
            }

            b = ((_32X_Palette_16B[i] >> 0) & 0x1F) << 1;

            r = (r * unsigned int(0.30 * 65536.0)) >> 16;
            g = (g * unsigned int(0.59 * 65536.0)) >> 16;
            b = (b * unsigned int(0.11 * 65536.0)) >> 16;

            r = g = b = r + g + b;

            if (Mode_555 & 1)
            {
                r = (r >> 1) << 10;
                g = (g >> 1) << 5;
            }
            else
            {
                r = (r >> 1) << 11;
                g = (g >> 0) << 5;
            }

            b = (b >> 1) << 0;

            _32X_Palette_16B[i] = r | g | b;
        }
    }

    if (Invert_Color)
    {
        for (i = 0; i < 0x1000; i++)
        {
            Palette[i] ^= 0xFFFF;
            Palette32[i] ^= 0xFFFFFF;
        }

        for (i = 0; i < 0x10000; i++)
        {
            _32X_Palette_16B[i] ^= 0xFFFF;
            _32X_Palette_32B[i] ^= 0xFFFFFF;
        }
    }

    for (i = 0; i < 0x1000; i++)
    {
        Palette32[0x1000 | i] = Palette32[i >> 1];  // shadow
        Palette32[0x2000 | i] = Palette32[(i >> 1) + 0x777];	// highlight
        Palette32[0x3000 | i] = Palette32[i];	// normal
        Palette[0x1000 | i] = Palette[i >> 1];		// shadow
        Palette[0x2000 | i] = Palette[(i >> 1) + 0x777];		// highlight
        Palette[0x3000 | i] = Palette[i];		// normal
    }

    // colors for alpha = 1
    for (i = 0; i < 0x4000; i++)
    {
        Palette32[0x4000 | i] = Palette32[i];
        Palette[0x4000 | i] = Palette[i];
    }

    unsigned short pink_555 = (Mode_555 & 1) ? 0x7C1F : 0xF81F;
    if (PinkBG)
        for (i = 0; i < 0x4000; i++)
        {
            Palette32[i] = 0xFF00FF;
            Palette[i] = pink_555;
        }

    for (i = 0; i < 0x100; i++)
    {
        _32X_VDP_CRam_Ajusted[i] = _32X_Palette_16B[_32X_VDP_CRam[i]];
        _32X_VDP_CRam_Ajusted32[i] = _32X_Palette_32B[_32X_VDP_CRam[i]];
    }
}

void Check_Country_Order(void)
{
    int i, j;
    bool bad = false;

    for (i = 0; i < 3; ++i)
        for (j = i + 1; j < 3; ++j)
            if (Country_Order[i] == Country_Order[j])
                bad = true;

    for (i = 0; i < 3; ++i)
        if (Country_Order[i] > 2
            || Country_Order[i] < 0)
            bad = true;

    if (bad)
        for (i = 0; i < 3; ++i)
            Country_Order[i] = i;
}

char *Detect_Country_SegaCD(void)
{
    if (CD_Data[0x10B] == 0x64)
    {
        Game_Mode = 1;
        CPU_Mode = 1;
        return EU_CD_Bios;
    }
    else if (CD_Data[0x10B] == 0xA1)
    {
        Game_Mode = 0;
        CPU_Mode = 0;
        return JA_CD_Bios;
    }
    else
    {
        Game_Mode = 1;
        CPU_Mode = 0;
        return US_CD_Bios;
    }
}

void Detect_Country_Genesis(void)
{
    int c_tab[3] = { 4, 1, 8 };
    int gm_tab[3] = { 1, 0, 1 };
    int cm_tab[3] = { 0, 0, 1 };
    int i, coun = 0;
    char c;

    if (!strnicmp((char *)&Rom_Data[0x1F0], "eur", 3)) coun |= 8;
    else if (!strnicmp((char *)&Rom_Data[0x1F0], "usa", 3)) coun |= 4;
    else if (!strnicmp((char *)&Rom_Data[0x1F0], "jap", 3)) coun |= 1;
    else for (i = 0; i < 4; i++)
    {
        c = toupper(Rom_Data[0x1F0 + i]);

        if (c == 'U') coun |= 4;
        else if (c == 'J') coun |= 1;
        else if (c == 'E') coun |= 8;
        else if (c < 16) coun |= c;
        else if ((c >= '0') && (c <= '9')) coun |= c - '0';
        else if ((c >= 'A') && (c <= 'F')) coun |= c - 'A' + 10;
    }

    if (coun & c_tab[Country_Order[0]])
    {
        Game_Mode = gm_tab[Country_Order[0]];
        CPU_Mode = cm_tab[Country_Order[0]];
    }
    else if (coun & c_tab[Country_Order[1]])
    {
        Game_Mode = gm_tab[Country_Order[1]];
        CPU_Mode = cm_tab[Country_Order[1]];
    }
    else if (coun & c_tab[Country_Order[2]])
    {
        Game_Mode = gm_tab[Country_Order[2]];
        CPU_Mode = cm_tab[Country_Order[2]];
    }
    else if (coun & 2)
    {
        Game_Mode = 0;
        CPU_Mode = 1;
    }
    else
    {
        Game_Mode = 1;
        CPU_Mode = 0;
    }

    if (Game_Mode)
    {
        if (CPU_Mode) Put_Info("Europe system (50 FPS)");
        else Put_Info("USA system (60 FPS)");
    }
    else
    {
        if (CPU_Mode) Put_Info("Japan system (50 FPS)");
        else Put_Info("Japan system (60 FPS)");
    }

    if (CPU_Mode)
    {
        VDP_Status |= 0x0001;
        _32X_VDP.Mode &= ~0x8000;
    }
    else
    {
        _32X_VDP.Mode |= 0x8000;
        VDP_Status &= 0xFFFE;
    }
}

// a few things previously uninitialized by one or more systems,
// causing even savestates made on frame 0 to differ.
// the init functions should call this first so that the actual initialization
// can overwrite what this does (in the cases where that system does initialize it).
// the reset functions shouldn't call this at all,
// it's correct that those don't fully reset the emulation state.
static void Misc_Genesis_Init()
{
    //M_Z80.CycleCnt = 0;
    memset(&M_Z80.AF, 0, sizeof(M_Z80.CycleSup) + ((char*)&M_Z80.CycleSup - (char*)&M_Z80.AF));
    M_Z80.RetIC = 0;
    M_Z80.IntAckC = 0;
    Controller_1_Delay = 0;
    Controller_2_Delay = 0;
    Ctrl.DMA_Mode = 0;
    VDP_Reg.DMA_Address = 0;
    VDP_Current_Line = 0;
    Cycles_S68K = 0;
    Cycles_M68K = 0;
    Cycles_Z80 = 0;
    S68K_State = 0;
    CPL_S68K = 0;
    Bank_M68K = 0;
    Lag_Frame = 0;
    main68k_context.cycles_leftover = 0;
    main68k_context.xflag = 0;
    main68k_context.io_cycle_counter = -1;
    main68k_context.io_fetchbase = 0;
    main68k_context.io_fetchbased_pc = 0;
    main68k_context.access_address = 0;
    main68k_context.odometer = 0;
    sub68k_context.cycles_leftover = 0;
    sub68k_context.xflag = 0;
    sub68k_context.io_cycle_counter = -1;
    sub68k_context.io_fetchbase = 0;
    sub68k_context.io_fetchbased_pc = 0;
    sub68k_context.access_address = 0;
    sub68k_context.odometer = 0;
}

/*************************************/
/*              GENESIS              */
/*************************************/

void Init_Genesis_Bios(void)
{
    Misc_Genesis_Init();

    FILE *f;

    if (f = fopen(Genesis_Bios, "rb"))
    {
        fread(&Genesis_Rom[0], 1, 2 * 1024, f);
        Byte_Swap(&Genesis_Rom[0], 2 * 1024);
        fclose(f);
    }
    else memset(Genesis_Rom, 0, 2 * 1024);

    Rom_Size = 2 * 1024;
    memcpy(Rom_Data, Genesis_Rom, 2 * 1024);
    Game_Mode = 0;
    CPU_Mode = 0;
    VDP_Num_Vis_Lines = 224;
    M68K_Reset(0, 1);
    Z80_Reset();
    Reset_VDP();
    FakeVDPScreen = true;
    CPL_Z80 = Round_Double((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
    CPL_M68K = Round_Double((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
    VDP_Num_Lines = 262;
    VDP_Status &= 0xFFFE;
    YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
    PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
}

int Init_Genesis(struct Rom *MD_Rom)
{
    Misc_Genesis_Init();

    char Str_Err[256];

    Flag_Clr_Scr = 1;
    Paused = Frame_Number = 0;
    SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
    Controller_1_COM = Controller_2_COM = 0;

    if ((MD_Rom->Ram_Infos[8] == 'R') && (MD_Rom->Ram_Infos[9] == 'A') && (MD_Rom->Ram_Infos[10] & 0x40))
    {
        SRAM_Start = MD_Rom->Ram_Start_Address & 0x0F80000;		// multiple de 0x080000
        SRAM_End = MD_Rom->Ram_End_Address;
    }
    else
    {
        SRAM_Start = 0x200000;
        SRAM_End = 0x20FFFF;
    }

    if ((SRAM_Start > SRAM_End) || ((SRAM_End - SRAM_Start) >= (64 * 1024)))
        SRAM_End = SRAM_Start + (64 * 1024) - 1;

    if (Rom_Size <= (2 * 1024 * 1024))
    {
        SRAM_ON = 1;
        SRAM_Write = 1;
    }

    SRAM_Start &= 0xFFFFFFFE;
    SRAM_End |= 0x00000001;

    //		sprintf(Str_Err, "deb = %.8X end = %.8X", SRAM_Start, SRAM_End);
    //		MessageBox(NULL, Str_Err, "", MB_OK);

    if ((SRAM_End - SRAM_Start) <= 2) SRAM_Custom = 1;
    else SRAM_Custom = 0;

    Load_SRAM();

    switch (Country)
    {
    default:
    case -1:
        Detect_Country_Genesis();
        break;

    case 0:
        Game_Mode = 0;
        CPU_Mode = 0;

        break;

    case 1:
        Game_Mode = 1;
        CPU_Mode = 0;
        break;

    case 2:
        Game_Mode = 1;
        CPU_Mode = 1;
        break;

    case 3:
        Game_Mode = 0;
        CPU_Mode = 1;
        break;
    }

    if ((CPU_Mode == 1) || (Game_Mode == 0))
        sprintf(Str_Err, GENS_NAME " - Megadrive : %s", MD_Rom->Rom_Name_W);
    else
        sprintf(Str_Err, GENS_NAME " - Genesis : %s", MD_Rom->Rom_Name_W);

    // Modif N. - remove double-spaces from title bar
    for (int i = 0; i < (int)strlen(Str_Err) - 1; i++)
        if (Str_Err[i] == Str_Err[i + 1] && Str_Err[i] == ' ')
            strcpy(Str_Err + i, Str_Err + i + 1), i--;

    SetWindowText(HWnd, Str_Err);

    VDP_Num_Vis_Lines = 224;
    Gen_Version = 0x20 + 0x0;	 	// Version de la megadrive (0x0 - 0xF)

    Byte_Swap(Rom_Data, Rom_Size);

    M68K_Init(); // Modif N. -- added for symmetry, maybe it helps something
    M68K_Reset(0, 1);
    Z80_Reset();
    Reset_VDP();
    FakeVDPScreen = true;

    if (CPU_Mode)
    {
        CPL_Z80 = Round_Double((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
        CPL_M68K = Round_Double((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
        CPL_MSH2 = Round_Double(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double)MSH2_Speed) / 100.0);
        CPL_SSH2 = Round_Double(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double)SSH2_Speed) / 100.0);

        VDP_Num_Lines = 312;
        VDP_Status |= 0x0001;

        YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
        PSG_Init(CLOCK_PAL / 15, Sound_Rate);
    }
    else
    {
        CPL_Z80 = Round_Double((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
        CPL_M68K = Round_Double((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
        CPL_MSH2 = Round_Double(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double)MSH2_Speed) / 100.0);
        CPL_SSH2 = Round_Double(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double)SSH2_Speed) / 100.0);

        VDP_Num_Lines = 262;
        VDP_Status &= 0xFFFE;

        YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
        PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
    }

    if (Auto_Fix_CS) Fix_Checksum();

    if (Sound_Enable)
    {
        End_Sound();

        if (!Init_Sound(HWnd)) Sound_Enable = 0;
        else Play_Sound();
    }

    Load_Patch_File();
    Build_Main_Menu();

    Last_Time = GetTickCount();
    New_Time = 0;
    Used_Time = 0;

    Update_Frame = Do_Genesis_Frame;
    Update_Frame_Fast = Do_Genesis_Frame_No_VDP;

    Genesis_Started = 1; // used inside reset_address_info
    SegaCD_Started = 0;
    _32X_Started = 0;
    reset_address_info();
    RestartAllLuaScripts();

    return 1;
}

void Reset_Genesis()
{
    Controller_1_COM = Controller_2_COM = 0;
    //Paused = 0;

    if (Rom_Size <= (2 * 1024 * 1024))
    {
        SRAM_ON = 1;
        SRAM_Write = 1;
    }
    else
    {
        SRAM_ON = 0;
        SRAM_Write = 0;
    }

    M68K_Reset(0, 1);
    Z80_Reset();
    Reset_VDP();
    FakeVDPScreen = true;
    YM2612_Reset();

    if (CPU_Mode) VDP_Status |= 1;
    else VDP_Status &= ~1;

    if (Auto_Fix_CS) Fix_Checksum();
}

extern "C"
{
    int disableSound = false;
    int disableSound2 = false; // slower but more reversible way of disabling sound generation
    int disableRamSearchUpdate = false;
    int Seg_Junk[882];
}

inline static int* LeftAudioBuffer() { return disableSound2 ? Seg_Junk : Seg_L; }
inline static int* RightAudioBuffer() { return disableSound2 ? Seg_Junk : Seg_R; }

// to make two different compiled functions
template<int bits>
void Render_MD_Screen_()
{
    int Line;
    for (Line = 0; Line < VDP_Num_Vis_Lines; Line++)
    {
        for (unsigned long Pixel = TAB336[Line] + 8; Pixel < TAB336[Line] + 336; Pixel++)
        {
            if (bits == 32)
                MD_Screen32[Pixel] = Palette32[Screen_16X[Pixel]];
            if (bits == 16)
                MD_Screen[Pixel] = Palette[Screen_16X[Pixel]];
        }
    }
    // fixes for filters
    // bottom row
    for (int Pixel = 336 * Line + 8; Pixel < 336 * Line + 336; Pixel++)
    {
        if (bits == 32)
            MD_Screen32[Pixel] = 0;
        if (bits == 16)
            MD_Screen[Pixel] = 0;
    }
    // right row
    if (VDP_REG_SET4 & 0x1)
        for (Line = 0; Line < VDP_Num_Vis_Lines; Line++)
        {
            int Pixel = TAB336[Line] + 8 + 320;
            if (bits == 32)
                MD_Screen32[Pixel] = 0;
            if (bits == 16)
                MD_Screen[Pixel] = 0;
        }
    else
        for (Line = 0; Line < VDP_Num_Vis_Lines; Line++)
        {
            int Pixel = TAB336[Line] + 8 + 256;
            if (bits == 32)
                MD_Screen32[Pixel] = 0;
            if (bits == 16)
                MD_Screen[Pixel] = 0;
        }
}

void Render_MD_Screen()
{
    if (Bits32)
        Render_MD_Screen_<32>();
    else
        Render_MD_Screen_<16>();
}

// it is so simple :) and so awesome!
template<int bits, int swap, int high, int low>
void Render_MD_Screen32X_()
{
    for (int Line = 0; Line < VDP_Num_Vis_Lines; Line++)
    {
        if (Screen_32X[TAB336[Line] + 7])
            for (unsigned long Pixel = TAB336[Line] + 8; Pixel < TAB336[Line] + 336; Pixel++)
            {
                unsigned short pix = Screen_32X[Pixel];
                if (!(Screen_16X[Pixel] & 0x4000))
                {
                    if (swap)
                    {
                        if ((pix & 0x8000))
                        {
                            if (low)
                                goto _32x;
                        }
                        else if (high)
                            goto _32x;
                    }
                    else
                    {
                        if (!(pix & 0x8000))
                        {
                            if (low)
                                goto _32x;
                        }
                        else if (high)
                            goto _32x;
                    }

                    goto _gen;
                }
                else
                {
                    if (swap)
                    {
                        if ((pix & 0x8000))
                            goto _gen;
                    }
                    else
                        if (!(pix & 0x8000))
                            goto _gen;

                    if (high)
                        goto _32x;
                    else
                        goto _gen;
                }

            _gen: // image from genesis
                if (bits == 32)
                    MD_Screen32[Pixel] = Palette32[Screen_16X[Pixel]];
                if (bits == 16)
                    MD_Screen[Pixel] = Palette[Screen_16X[Pixel]];
                continue;
            _32x: // image from 32X
                if (bits == 32)
                    MD_Screen32[Pixel] = _32X_Palette_32B[pix];
                if (bits == 16)
                    MD_Screen[Pixel] = _32X_Palette_16B[pix];
            }
        else
            for (unsigned long Pixel = TAB336[Line] + 8; Pixel < TAB336[Line] + 336; Pixel++)
            {
                if (bits == 32)
                    MD_Screen32[Pixel] = Palette32[Screen_16X[Pixel]];
                if (bits == 16)
                    MD_Screen[Pixel] = Palette[Screen_16X[Pixel]];
            }
    }
}

template<int bits, int swap>
void Render_MD_Screen32X_1()
{
    char High;
    char Low;
    if (swap == 1)
    {
        High = _32X_Plane_Low_On;
        Low = _32X_Plane_High_On;
    }
    else
    {
        High = _32X_Plane_High_On;
        Low = _32X_Plane_Low_On;
    }
    if (High)
    {
        if (Low)
            Render_MD_Screen32X_<bits, swap, 1, 1>();
        else
            Render_MD_Screen32X_<bits, swap, 1, 0>();
    }
    else
    {
        if (Low)
            Render_MD_Screen32X_<bits, swap, 0, 1>();
        else
            Render_MD_Screen32X_<bits, swap, 0, 0>();
    }
}

template<int bits>
void Render_MD_Screen32X_2()
{
    if (Swap_32X_Plane_Priority)
        Render_MD_Screen32X_1<bits, 1>();
    else
        Render_MD_Screen32X_1<bits, 0>();
}

void Render_MD_Screen32X()
{
    if (!_32X_Plane_On)
        Render_MD_Screen();
    else if (Bits32)
        Render_MD_Screen32X_2<32>();
    else
        Render_MD_Screen32X_2<16>();
}

#ifdef SONICCAMHACK

#else

int Do_Genesis_Frame(bool fast)
{
    struct Scope { Scope() { Inside_Frame = 1; } ~Scope() { Inside_Frame = 0; } } scope;

    int *buf[2];
    int HInt_Counter;

    if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
    else VDP_Num_Vis_Lines = 224;

    if (!disableSound)
    {
        YM_Buf[0] = PSG_Buf[0] = LeftAudioBuffer();
        YM_Buf[1] = PSG_Buf[1] = RightAudioBuffer();
    }
    YM_Len = PSG_Len = 0;

    Cycles_M68K = Cycles_Z80 = 0;
    Last_BUS_REQ_Cnt = -1000;
    main68k_tripOdometer();
    z80_Clear_Odo(&M_Z80);

    Patch_Codes();

    VRam_Flag = 1;

    VDP_Status &= 0xFFF7;							// Clear V Blank
    if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;
    VDP_Reg_Set2_Current = VDP_Reg.Set2;
    VDP_Reg_Set4_Current = VDP_Reg.Set4;

    HInt_Counter = VDP_Reg.H_Int;					// Hint_Counter = step d'interruption H

    if (fast)
        FakeVDPScreen = true;

    for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
    {
        if (!disableSound)
        {
            buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM_Len += Sound_Extrapol[VDP_Current_Line][1];
            PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
        }

        Fix_Controllers();
        Cycles_M68K += CPL_M68K;
        Cycles_Z80 += CPL_Z80;
        if (DMAT_Length) main68k_addCycles(Update_DMA());
        VDP_Status |= 0x0004;			// HBlank = 1
        //		main68k_exec(Cycles_M68K - 436);
        main68k_exec(Cycles_M68K - 404);
        VDP_Status &= 0xFFFB;			// HBlank = 0

        if (--HInt_Counter < 0)
        {
            HInt_Counter = VDP_Reg.H_Int;
            VDP_Int |= 0x4;
            Update_IRQ_Line();
        }

        if (!fast)
            Render_Line();

        main68k_exec(Cycles_M68K);
        if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
        else z80_Set_Odo(&M_Z80, Cycles_Z80);
    }

    if (!fast)
    {
        FakeVDPScreen = false;
        Render_MD_Screen();
    }

    if (!disableSound)
    {
        buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
        buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
        YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
        YM_Len += Sound_Extrapol[VDP_Current_Line][1];
        PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
    }

    Fix_Controllers();
    Cycles_M68K += CPL_M68K;
    Cycles_Z80 += CPL_Z80;
    if (DMAT_Length) main68k_addCycles(Update_DMA());
    if (--HInt_Counter < 0)
    {
        VDP_Int |= 0x4;
        Update_IRQ_Line();
    }

    VDP_Status |= 0x000C;			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
    main68k_exec(Cycles_M68K - 360);
    if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
    else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

    VDP_Status &= 0xFFFB;			// HBlank = 0
    VDP_Status |= 0x0080;			// V Int happened

    VDP_Int |= 0x8;
    Update_IRQ_Line();
    z80_Interrupt(&M_Z80, 0xFF);

    main68k_exec(Cycles_M68K);
    if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
    else z80_Set_Odo(&M_Z80, Cycles_Z80);

    for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
    {
        if (!disableSound)
        {
            buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM_Len += Sound_Extrapol[VDP_Current_Line][1];
            PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
        }

        Fix_Controllers();
        Cycles_M68K += CPL_M68K;
        Cycles_Z80 += CPL_Z80;
        if (DMAT_Length) main68k_addCycles(Update_DMA());
        VDP_Status |= 0x0004;					// HBlank = 1
        //		main68k_exec(Cycles_M68K - 436);
        main68k_exec(Cycles_M68K - 404);
        VDP_Status &= 0xFFFB;					// HBlank = 0

        main68k_exec(Cycles_M68K);
        if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
        else z80_Set_Odo(&M_Z80, Cycles_Z80);
    }

    if (!disableSound)
    {
        PSG_Special_Update();
        YM2612_Special_Update();
    }
#ifdef RKABOXHACK
    CamX = CheatRead<short>(0xB158);
    CamY = CheatRead<short>(0xB1D6);
#endif
    if (!fast)
        Update_RAM_Search();
    //	if (SRAM_ON != SRAM_Was_On)
    //	{
    //		SRAM_Was_On = SRAM_ON;
    //		if (SRAM_ON) sprintf(Str_Tmp,"SRAM enabled");
    //		else		 sprintf(Str_Tmp,"SRAM disabled");;
    //		Put_Info(Str_Tmp);
    //	}
    return(1);
}

DO_FRAME_HEADER(Do_Genesis_Frame, Do_Genesis_Frame_No_VDP)
{
    return Do_Genesis_Frame(false);
}

DO_FRAME_HEADER(Do_Genesis_Frame_No_VDP, Do_Genesis_Frame_No_VDP)
{
    return Do_Genesis_Frame(true);
}

#endif

int Do_VDP_Refresh()
{
    int VDP_Current_Line_bak = VDP_Current_Line;

    if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
    else VDP_Num_Vis_Lines = 224;

    if (!_32X_Started)
    {
        if (Genesis_Started || SegaCD_Started)
        {
            if (FakeVDPScreen)
                for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
                    Render_Line();
            Render_MD_Screen();
        }
        else // emulation hasn't started so just set all pixels to black
        {
            memset(MD_Screen, 0, sizeof(MD_Screen));
            memset(MD_Screen32, 0, sizeof(MD_Screen32));
        }
    }
    else
    {
        if (FakeVDPScreen)
            for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
                Render_Line_32X();
        Render_MD_Screen32X();
    }

    VDP_Current_Line = VDP_Current_Line_bak;

    Update_RAM_Search();
#ifdef RKABOXHACK
    CamX = CheatRead<short>(0xB158);
    CamY = CheatRead<short>(0xB1D6);
    DrawBoxes();
#endif
#ifdef SONICCAMHACK
    CamX = CheatRead<short>(CAMOFFSET1);
    CamY = CheatRead<short>(CAMOFFSET1 + 4);
    DrawBoxes();
#endif

    CallRegisteredLuaFunctions(LUACALL_AFTEREMULATIONGUI);

    return(0);
}
/*************************************/
/*                32X                */
/*************************************/

int Init_32X(struct Rom *MD_Rom)
{
    char Str_Err[256];
    FILE *f;
    int i;

    if (f = fopen(_32X_Genesis_Bios, "rb"))
    {
        fread(&_32X_Genesis_Rom[0], 1, 256, f);
        Byte_Swap(&_32X_Genesis_Rom[0], 256);
        fclose(f);
    }
    else
    {
        MessageBox(HWnd, "Your 32X bios files aren't correctly configured :\nGenesis 32X bios not found.\nGo to menu 'Option -> Bios/Misc Files' to set up them", "Error", MB_OK);
        return 0;
    }

    if (f = fopen(_32X_Master_Bios, "rb"))
    {
        fread(&_32X_MSH2_Rom[0], 1, 2 * 1024, f);
        fclose(f);
    }
    else
    {
        MessageBox(HWnd, "Your 32X bios files aren't correctly configured :\nMaster SH2 bios not found.\nGo to menu 'Option -> Bios/Misc Files' to set up them", "Error", MB_OK);
        return 0;
    }

    if (f = fopen(_32X_Slave_Bios, "rb"))
    {
        fread(&_32X_SSH2_Rom[0], 1, 1 * 1024, f);
        fclose(f);
    }
    else
    {
        MessageBox(HWnd, "Your 32X bios files aren't correctly configured :\nSlave SH2 bios not found.\nGo to menu 'Option -> Bios/Misc Files' to set up them", "Error", MB_OK);
        return 0;
    }

    Misc_Genesis_Init();

    Flag_Clr_Scr = 1;
    Paused = Frame_Number = 0;
    SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
    Controller_1_COM = Controller_2_COM = 0;

    if ((MD_Rom->Ram_Infos[8] == 'R') && (MD_Rom->Ram_Infos[9] == 'A') && (MD_Rom->Ram_Infos[10] & 0x40))
    {
        SRAM_Start = MD_Rom->Ram_Start_Address & 0x0F80000;		// multiple de 0x080000
        SRAM_End = MD_Rom->Ram_End_Address;
    }
    else
    {
        SRAM_Start = 0x200000;
        SRAM_End = 0x20FFFF;
    }

    if ((SRAM_Start > SRAM_End) || ((SRAM_End - SRAM_Start) >= (64 * 1024)))
        SRAM_End = SRAM_Start + (64 * 1024) - 1;

    if (Rom_Size <= (2 * 1024 * 1024))
    {
        SRAM_ON = 1;
        SRAM_Write = 1;
    }

    SRAM_Start &= 0xFFFFFFFE;
    SRAM_End |= 0x00000001;

    //		sprintf(Str_Err, "deb = %.8X end = %.8X", SRAM_Start, SRAM_End);
    //		MessageBox(HWnd, Str_Err, "", MB_OK);

    if ((SRAM_End - SRAM_Start) <= 2) SRAM_Custom = 1;
    else SRAM_Custom = 0;

    Load_SRAM();

    switch (Country)
    {
    default:
    case -1:
        Detect_Country_Genesis();
        break;

    case 0:
        Game_Mode = 0;
        CPU_Mode = 0;

        break;

    case 1:
        Game_Mode = 1;
        CPU_Mode = 0;
        break;

    case 2:
        Game_Mode = 1;
        CPU_Mode = 1;
        break;

    case 3:
        Game_Mode = 0;
        CPU_Mode = 1;
        break;
    }

    if (CPU_Mode == 1)
        sprintf(Str_Err, GENS_NAME " - 32X (PAL) : %s", MD_Rom->Rom_Name_W);
    else
        sprintf(Str_Err, GENS_NAME " - 32X (NTSC) : %s", MD_Rom->Rom_Name_W);

    // Modif N. - remove double-spaces from title bar
    for (int i = 0; i < (int)strlen(Str_Err) - 1; i++)
        if (Str_Err[i] == Str_Err[i + 1] && Str_Err[i] == ' ')
            strcpy(Str_Err + i, Str_Err + i + 1), i--;

    SetWindowText(HWnd, Str_Err);

    VDP_Num_Vis_Lines = 224;
    Gen_Version = 0x20 + 0x0;	 	// Version de la megadrive (0x0 - 0xF)

    memcpy(_32X_Rom, Rom_Data, 4 * 1024 * 1024);	// no byteswapped image (for SH2)
    Byte_Swap(Rom_Data, Rom_Size);					// byteswapped image (for 68000)

    // a bunch of 32x stuff that was left uninitialized before (most of it, at least)
    {
        for (int contextNum = 0; contextNum < 2; contextNum++)
        {
            SH2_CONTEXT* context = (contextNum == 0) ? &M_SH2 : &S_SH2;
            memset(&context->IO_Reg, 0, sizeof(context->IO_Reg));
            memset(&context->DVCR, 0, sizeof(context->BCR1) + ((char*)&context->BCR1 - (char*)&context->DVCR));
        }
        memset(_32X_Ram, 0, sizeof(_32X_Ram));
        memset(_MSH2_Reg, 0, sizeof(_MSH2_Reg));
        memset(_SSH2_Reg, 0, sizeof(_SSH2_Reg));
        memset(_SH2_VDP_Reg, 0, sizeof(_SH2_VDP_Reg));
        memset(_32X_VDP_CRam, 0, sizeof(_32X_VDP_CRam));
        memset(_32X_Comm, 0, sizeof(_32X_Comm));
        _32X_ADEN = 0;
        _32X_RES = 0;
        _32X_FM = 0;
        _32X_RV = 0;
        Cycles_MSH2 = 0;
        Cycles_SSH2 = 0;
        int pwmEnabled = PWM_Enable;
        memset(&PWM_FIFO_R, 0, sizeof(PWM_Out_R_Tmp) + ((char*)&PWM_Out_R_Tmp - (char*)&PWM_FIFO_R));
        PWM_Enable = pwmEnabled;
    }

    MSH2_Reset();
    SSH2_Reset();
    M68K_Reset(1, 1);
    Z80_Reset();
    Reset_VDP();
    FakeVDPScreen = true;
    _32X_VDP_Reset();
    _32X_Set_FB();
    PWM_Init();

    if (CPU_Mode)
    {
        CPL_Z80 = Round_Double((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
        CPL_M68K = Round_Double((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
        CPL_MSH2 = Round_Double(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double)MSH2_Speed) / 100.0);
        CPL_SSH2 = Round_Double(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double)SSH2_Speed) / 100.0);

        VDP_Num_Lines = 312;
        VDP_Status |= 0x0001;
        _32X_VDP.Mode &= ~0x8000;

        YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
        PSG_Init(CLOCK_PAL / 15, Sound_Rate);
    }
    else
    {
        CPL_Z80 = Round_Double((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
        CPL_M68K = Round_Double((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
        CPL_MSH2 = Round_Double(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double)MSH2_Speed) / 100.0);
        CPL_SSH2 = Round_Double(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double)SSH2_Speed) / 100.0);

        VDP_Num_Lines = 262;
        VDP_Status &= 0xFFFE;
        _32X_VDP.Mode |= 0x8000;

        YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
        PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
    }

    _32X_VDP.State |= 0x2000;

    if (Auto_Fix_CS) Fix_Checksum();

    if (Sound_Enable)
    {
        End_Sound();

        if (!Init_Sound(HWnd)) Sound_Enable = 0;
        else Play_Sound();
    }

    Load_Patch_File();
    Build_Main_Menu();

    Last_Time = GetTickCount();
    New_Time = 0;
    Used_Time = 0;

    Update_Frame = Do_32X_Frame;
    Update_Frame_Fast = Do_32X_Frame_No_VDP;

    // We patch the Master SH2 bios with ROM bios
    // this permit 32X games with older BIOS version to run correctly
    // Ecco 32X demo needs it

    for (i = 0; i < 0x400; i++) _32X_MSH2_Rom[i + 0x36C] = _32X_Rom[i + 0x400];

    _32X_Started = 1; // used inside reset_address_info
    SegaCD_Started = 0;
    reset_address_info();
    RestartAllLuaScripts();

    return 1;
}

void Reset_32X()
{
    int i;

    //Paused = 0;
    Controller_1_COM = Controller_2_COM = 0;
    _32X_ADEN = _32X_RES = _32X_FM = _32X_RV = 0;

    if (Rom_Size <= (2 * 1024 * 1024))
    {
        SRAM_ON = 1;
        SRAM_Write = 1;
    }
    else
    {
        SRAM_ON = 0;
        SRAM_Write = 0;
    }

    MSH2_Reset();
    SSH2_Reset();
    M68K_Reset(1, 1);
    Z80_Reset();
    Reset_VDP();
    FakeVDPScreen = true;
    _32X_VDP_Reset();
    _32X_Set_FB();
    YM2612_Reset();
    PWM_Init();

    if (CPU_Mode)
    {
        VDP_Status |= 1;
        _32X_VDP.Mode &= ~0x8000;
    }
    else
    {
        VDP_Status &= ~1;
        _32X_VDP.Mode |= 0x8000;
    }

    _32X_VDP.State |= 0x2000;

    if (Auto_Fix_CS) Fix_Checksum();

    // We patch the Master SH2 bios with ROM bios
    // this permit 32X games with older BIOS version to run correctly
    // Ecco 32X demo needs it

    for (i = 0; i < 0x400; i++) _32X_MSH2_Rom[i + 0x36C] = _32X_Rom[i + 0x400];
}

int Do_32X_Frame(bool fast)
{
    struct Scope { Scope() { Inside_Frame = 1; } ~Scope() { Inside_Frame = 0; } } scope;

    int i, j, k, l, p_i, p_j, p_k, p_l, *buf[2];
    int HInt_Counter, HInt_Counter_32X;
    int CPL_PWM;

    if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
    else VDP_Num_Vis_Lines = 224;

    if (!disableSound)
    {
        YM_Buf[0] = PSG_Buf[0] = LeftAudioBuffer();
        YM_Buf[1] = PSG_Buf[1] = RightAudioBuffer();
    }
    YM_Len = PSG_Len = 0;

    CPL_PWM = CPL_M68K * 3;

    PWM_Cycles = Cycles_SSH2 = Cycles_MSH2 = Cycles_M68K = Cycles_Z80 = 0;
    Last_BUS_REQ_Cnt = -1000;

    main68k_tripOdometer();
    z80_Clear_Odo(&M_Z80);
    SH2_Clear_Odo(&M_SH2);
    SH2_Clear_Odo(&S_SH2);
    PWM_Clear_Timer();

    Patch_Codes();

    VRam_Flag = 1;

    VDP_Status &= 0xFFF7;							// Clear V Blank
    if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;
    _32X_VDP.State &= ~0x8000;
    VDP_Reg_Set2_Current = VDP_Reg.Set2;
    VDP_Reg_Set4_Current = VDP_Reg.Set4;

    HInt_Counter = VDP_Reg.H_Int;					// Hint_Counter = step d'interruption H
    HInt_Counter_32X = _32X_HIC;

    p_i = 84;
    p_j = (p_i * CPL_MSH2) / CPL_M68K;
    p_k = (p_i * CPL_SSH2) / CPL_M68K;
    p_l = p_i * 3;

    if (fast)
        FakeVDPScreen = true;

    for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
    {
        if (!disableSound)
        {
            buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM_Len += Sound_Extrapol[VDP_Current_Line][1];
            PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
        }

        i = Cycles_M68K + (p_i * 2);
        j = Cycles_MSH2 + (p_j * 2);
        k = Cycles_SSH2 + (p_k * 2);
        l = PWM_Cycles + (p_l * 2);

        Fix_Controllers();
        Cycles_M68K += CPL_M68K;
        Cycles_MSH2 += CPL_MSH2;
        Cycles_SSH2 += CPL_SSH2;
        Cycles_Z80 += CPL_Z80;
        PWM_Cycles += CPL_PWM;
        if (DMAT_Length) main68k_addCycles(Update_DMA());
        VDP_Status |= 0x0004;			// HBlank = 1
        _32X_VDP.State |= 0x6000;

        main68k_exec(i - p_i);
        SH2_Exec(&M_SH2, j - p_j);
        SH2_Exec(&S_SH2, k - p_k);
        PWM_Update_Timer(l - p_l);

        VDP_Status &= ~0x0004;			// HBlank = 0
        _32X_VDP.State &= ~0x6000;

        if (--HInt_Counter < 0)
        {
            HInt_Counter = VDP_Reg.H_Int;
            VDP_Int |= 0x4;
            Update_IRQ_Line();
        }

        if (--HInt_Counter_32X < 0)
        {
            HInt_Counter_32X = _32X_HIC;
            if (_32X_MINT & 0x04) SH2_Interrupt(&M_SH2, 10);
            if (_32X_SINT & 0x04) SH2_Interrupt(&S_SH2, 10);
        }

        if (!fast)
            Render_Line_32X();

#ifdef _DEBUG
        static int _32X_Prev_Rend_Mode = -1;
        if (_32X_Rend_Mode != _32X_Prev_Rend_Mode)
        {
            char temp[20];
            sprintf(temp, "32X Video Mode: %d", _32X_Rend_Mode);
            SetWindowText(HWnd, temp);
            _32X_Prev_Rend_Mode = _32X_Rend_Mode;
        }
#endif

        /* instruction by instruction execution */

        while (i < Cycles_M68K)
        {
            main68k_exec(i);
            SH2_Exec(&M_SH2, j);
            SH2_Exec(&S_SH2, k);
            PWM_Update_Timer(l);
            i += p_i;
            j += p_j;
            k += p_k;
            l += p_l;
        }

        main68k_exec(Cycles_M68K);
        SH2_Exec(&M_SH2, Cycles_MSH2);
        SH2_Exec(&S_SH2, Cycles_SSH2);
        PWM_Update_Timer(PWM_Cycles);
        if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
        else z80_Set_Odo(&M_Z80, Cycles_Z80);
    }

    if (!fast)
    {
        FakeVDPScreen = false;
        Render_MD_Screen32X();
    }

    if (!disableSound)
    {
        buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
        buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
        YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
        PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
        YM_Len += Sound_Extrapol[VDP_Current_Line][1];
        PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
    }

    i = Cycles_M68K + p_i;
    j = Cycles_MSH2 + p_j;
    k = Cycles_SSH2 + p_k;
    l = PWM_Cycles + p_l;

    Fix_Controllers();
    Cycles_M68K += CPL_M68K;
    Cycles_MSH2 += CPL_MSH2;
    Cycles_SSH2 += CPL_SSH2;
    Cycles_Z80 += CPL_Z80;
    PWM_Cycles += CPL_PWM;
    if (DMAT_Length) main68k_addCycles(Update_DMA());
    if (--HInt_Counter < 0)
    {
        VDP_Int |= 0x4;
        Update_IRQ_Line();
    }

    if (--HInt_Counter_32X < 0)
    {
        HInt_Counter_32X = _32X_HIC;
        if (_32X_MINT & 0x04) SH2_Interrupt(&M_SH2, 10);
        if (_32X_SINT & 0x04) SH2_Interrupt(&S_SH2, 10);
    }

    VDP_Status |= 0x000C;			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
    _32X_VDP.State |= 0xE000;		// VBlank = 1, HBlank = 1, PEN = 1

    if (_32X_VDP.State & 0x10000) _32X_VDP.State |= 1;
    else _32X_VDP.State &= ~1;

    _32X_Set_FB();

    while (i < (Cycles_M68K - 360))
    {
        main68k_exec(i);
        SH2_Exec(&M_SH2, j);
        SH2_Exec(&S_SH2, k);
        PWM_Update_Timer(l);
        i += p_i;
        j += p_j;
        k += p_k;
        l += p_l;
    }

    main68k_exec(Cycles_M68K - 360);
    if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
    else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

    VDP_Status &= ~0x0004;			// HBlank = 0
    _32X_VDP.State &= ~0x4000;
    VDP_Status |= 0x0080;			// V Int happened

    VDP_Int |= 0x8;
    Update_IRQ_Line();
    if (_32X_MINT & 0x08) SH2_Interrupt(&M_SH2, 12);
    if (_32X_SINT & 0x08) SH2_Interrupt(&S_SH2, 12);
    z80_Interrupt(&M_Z80, 0xFF);

    while (i < Cycles_M68K)
    {
        main68k_exec(i);
        SH2_Exec(&M_SH2, j);
        SH2_Exec(&S_SH2, k);
        PWM_Update_Timer(l);
        i += p_i;
        j += p_j;
        k += p_k;
        l += p_l;
    }

    main68k_exec(Cycles_M68K);
    SH2_Exec(&M_SH2, Cycles_MSH2);
    SH2_Exec(&S_SH2, Cycles_SSH2);
    PWM_Update_Timer(PWM_Cycles);
    if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
    else z80_Set_Odo(&M_Z80, Cycles_Z80);

    for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
    {
        if (!disableSound)
        {
            buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM_Len += Sound_Extrapol[VDP_Current_Line][1];
            PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
        }

        i = Cycles_M68K + (p_i * 2);
        j = Cycles_MSH2 + (p_j * 2);
        k = Cycles_SSH2 + (p_k * 2);
        l = PWM_Cycles + (p_l * 2);

        Fix_Controllers();
        Cycles_M68K += CPL_M68K;
        Cycles_MSH2 += CPL_MSH2;
        Cycles_SSH2 += CPL_SSH2;
        Cycles_Z80 += CPL_Z80;
        PWM_Cycles += CPL_PWM;
        if (DMAT_Length) main68k_addCycles(Update_DMA());
        VDP_Status |= 0x0004;			// HBlank = 1
        _32X_VDP.State |= 0x6000;

        main68k_exec(i - p_i);
        SH2_Exec(&M_SH2, j - p_j);
        SH2_Exec(&S_SH2, k - p_k);
        PWM_Update_Timer(l - p_l);

        VDP_Status &= ~0x0004;			// HBlank = 0
        _32X_VDP.State &= ~0x6000;

        if (--HInt_Counter_32X < 0)
        {
            HInt_Counter_32X = _32X_HIC;
            if ((_32X_MINT & 0x04) && (_32X_MINT & 0x80)) SH2_Interrupt(&M_SH2, 10);
            if ((_32X_SINT & 0x04) && (_32X_SINT & 0x80)) SH2_Interrupt(&S_SH2, 10);
        }

        /* instruction by instruction execution */

        while (i < Cycles_M68K)
        {
            main68k_exec(i);
            SH2_Exec(&M_SH2, j);
            SH2_Exec(&S_SH2, k);
            PWM_Update_Timer(l);
            i += p_i;
            j += p_j;
            k += p_k;
            l += p_l;
        }

        main68k_exec(Cycles_M68K);
        SH2_Exec(&M_SH2, Cycles_MSH2);
        SH2_Exec(&S_SH2, Cycles_SSH2);
        PWM_Update_Timer(PWM_Cycles);
        if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
        else z80_Set_Odo(&M_Z80, Cycles_Z80);
    }

    if (!disableSound)
    {
        PSG_Special_Update();
        YM2612_Special_Update();
    }

    if (!fast)
        Update_RAM_Search();

    return 1;
}

DO_FRAME_HEADER(Do_32X_Frame, Do_32X_Frame_No_VDP)
{
    return Do_32X_Frame(false);
}

DO_FRAME_HEADER(Do_32X_Frame_No_VDP, Do_32X_Frame_No_VDP)
{
    return Do_32X_Frame(true);
}

/*************************************/
/*              SEGA CD              */
/*************************************/

int Init_SegaCD(char *iso_name)
{
    char Str_Err[256], *Bios_To_Use;

    SetWindowText(HWnd, GENS_NAME " - Sega CD : initializing, please wait ...");

    if (Reset_CD((char *)CD_Data, iso_name))
    {
        SetWindowText(HWnd, GENS_NAME " - Idle");
        return 0;
    }

    switch (Country)
    {
    default:
    case -1:
        Bios_To_Use = Detect_Country_SegaCD();
        break;

    case 0:
        Game_Mode = 0;
        CPU_Mode = 0;
        Bios_To_Use = JA_CD_Bios;
        break;

    case 1:
        Game_Mode = 1;
        CPU_Mode = 0;
        Bios_To_Use = US_CD_Bios;
        break;

    case 2:
        Game_Mode = 1;
        CPU_Mode = 1;
        Bios_To_Use = EU_CD_Bios;
        break;

    case 3:
        Game_Mode = 0;
        CPU_Mode = 1;
        Bios_To_Use = JA_CD_Bios;
        break;
    }

    if (Load_Bios(HWnd, Bios_To_Use) == NULL)
    {
        MessageBox(HWnd, "Your BIOS files aren't correctly configured, do it with 'Option -> Directories/Files...' menu.", "Warning", MB_OK | MB_ICONEXCLAMATION);
        SetWindowText(HWnd, GENS_NAME " - Idle");
        return 0;
    }

    Misc_Genesis_Init();

    Update_CD_Rom_Name((char *)&CD_Data[32]);

    if ((CPU_Mode == 1) || (Game_Mode == 0))
        sprintf(Str_Err, GENS_NAME " - MegaCD : %s", Rom_Name);
    else
        sprintf(Str_Err, GENS_NAME " - SegaCD : %s", Rom_Name);

    // Modif N. - remove double-spaces from title bar
    for (int i = 0; i < (int)strlen(Str_Err) - 1; i++)
        if (Str_Err[i] == Str_Err[i + 1] && Str_Err[i] == ' ')
            strcpy(Str_Err + i, Str_Err + i + 1), i--;

    SetWindowText(HWnd, Str_Err);

    Flag_Clr_Scr = 1;
    Paused = Frame_Number = 0;
    SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
    BRAM_Ex_State &= 0x100;
    Controller_1_COM = Controller_2_COM = 0;

    if (CPU_Mode)
    {
        CPL_Z80 = Round_Double((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
        CPL_M68K = Round_Double((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
        CPL_MSH2 = Round_Double(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double)MSH2_Speed) / 100.0);
        CPL_SSH2 = Round_Double(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double)SSH2_Speed) / 100.0);

        VDP_Num_Lines = 312;
        VDP_Status |= 0x0001;

        CD_Access_Timer = 2080;
        Timer_Step = 136752;
    }
    else
    {
        CPL_Z80 = Round_Double((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
        CPL_M68K = Round_Double((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
        CPL_MSH2 = Round_Double(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double)MSH2_Speed) / 100.0);
        CPL_SSH2 = Round_Double(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double)SSH2_Speed) / 100.0);

        VDP_Num_Lines = 262;
        VDP_Status &= 0xFFFE;

        CD_Access_Timer = 2096;
        Timer_Step = 135708;
    }

    VDP_Num_Vis_Lines = 224;
    Gen_Version = 0x20 + 0x0;	 	// Version de la megadrive (0x0 - 0xF)

    Rom_Data[0x72] = 0xFF;
    Rom_Data[0x73] = 0xFF;
    Byte_Swap(Rom_Data, Rom_Size);

    M68K_Init(); // Modif N. -- added for symmetry, maybe it helps something
    M68K_Reset(2, 1);
    S68K_Init(); // Modif N. -- added, needed to fully reset the sub68k
    S68K_Reset();
    Z80_Reset();
    Reset_VDP();
    FakeVDPScreen = true;
    Init_RS_GFX();
    LC89510_Reset();

    if (CPU_Mode)
    {
        YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
        PSG_Init(CLOCK_PAL / 15, Sound_Rate);
    }
    else
    {
        YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
        PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
    }

    Init_PCM(Sound_Rate);

    if (Sound_Enable)
    {
        End_Sound();

        if (!Init_Sound(HWnd)) Sound_Enable = 0;
        else Play_Sound();
    }

    Load_BRAM();				// Initialise BRAM
    Load_Patch_File();			// Only used to reset Patch structure
    Build_Main_Menu();

    Last_Time = GetTickCount();
    New_Time = 0;
    Used_Time = 0;

    if (SegaCD_Accurate)
    {
        Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
        Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
    }
    else
    {
        Update_Frame = Do_SegaCD_Frame;
        Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
    }

    // Modif N. -- added: (copied from Reset_SegaCD())
    Reset_PCM();
    YM2612_Reset();
    File_Add_Delay = 0;
    CD_Audio_Buffer_Read_Pos = 0;
    CD_Audio_Buffer_Write_Pos = 2000;
    CD_Timer_Counter = 0;

    track_number = 0; // Modif N. -- added, was never initialized before

    SegaCD_Started = 1; // used inside reset_address_info
    _32X_Started = 0;
    reset_address_info();
    RestartAllLuaScripts();

    return 1;
}

int Reload_SegaCD(char *iso_name)
{
    char Str_Err[256];

    Save_BRAM();

    SetWindowText(HWnd, GENS_NAME " - Sega CD : re-initializing, please wait ...");

    Reset_CD((char *)CD_Data, iso_name);
    Update_CD_Rom_Name((char *)&CD_Data[32]);

    if ((CPU_Mode == 1) || (Game_Mode == 0)) sprintf(Str_Err, GENS_NAME " - MegaCD : %s", Rom_Name);
    else sprintf(Str_Err, GENS_NAME " - SegaCD : %s", Rom_Name);

    SetWindowText(HWnd, Str_Err);

    Load_BRAM();

    return 1;
}

void Reset_SegaCD()
{
    char *Bios_To_Use;

    if (CPU_Mode) Bios_To_Use = EU_CD_Bios;
    else if (Game_Mode) Bios_To_Use = US_CD_Bios;
    else Bios_To_Use = JA_CD_Bios;

    SetCurrentDirectory(Gens_Path);

    if (Detect_Format(Bios_To_Use) == -1)
    {
        MessageBox(HWnd, "Some bios files are missing !\nConfigure them in the option menu.\n", "Warning", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    Controller_1_COM = Controller_2_COM = 0;
    SRAM_ON = 0;
    SRAM_Write = 0;
    //Paused = 0; //Modif-Upthorn: Commented out the auto-unpause. It can sometimes be useful to input on the first frame.
    BRAM_Ex_State &= 0x100;

    //if (!stricmp("ZIP", &Bios_To_Use[strlen(Bios_To_Use) - 3]))
    //{
    //	Game = Load_Rom_Zipped(HWnd, Bios_To_Use, 0);
    //}
    //else
    {
        Game = Load_Rom(HWnd, Bios_To_Use, 0);
    }

    Update_CD_Rom_Name((char *)&CD_Data[32]);

    Rom_Data[0x72] = 0xFF;
    Rom_Data[0x73] = 0xFF;

    Byte_Swap(Rom_Data, Rom_Size);

    M68K_Reset(2, 1);
    S68K_Reset();
    Z80_Reset();
    LC89510_Reset();
    Reset_VDP();
    FakeVDPScreen = true;
    Init_RS_GFX();
    Reset_PCM();
    YM2612_Reset();
    File_Add_Delay = 0;
    CD_Audio_Buffer_Read_Pos = 0;
    CD_Audio_Buffer_Write_Pos = 2000;
    CD_Timer_Counter = 0;

    if (CPU_Mode) VDP_Status |= 1;
    else VDP_Status &= ~1;
}

int Do_SegaCD_Frame(bool fast)
{
    struct Scope { Scope() { Inside_Frame = 1; } ~Scope() { Inside_Frame = 0; } } scope;

    int *buf[2];
    int HInt_Counter;

    if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
    else VDP_Num_Vis_Lines = 224;

    CPL_S68K = 795;

    if (!disableSound)
    {
        YM_Buf[0] = PSG_Buf[0] = LeftAudioBuffer();
        YM_Buf[1] = PSG_Buf[1] = RightAudioBuffer();
    }
    YM_Len = PSG_Len = 0;

    Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
    Last_BUS_REQ_Cnt = -1000;
    main68k_tripOdometer();
    sub68k_tripOdometer();
    z80_Clear_Odo(&M_Z80);

    Patch_Codes();

    VRam_Flag = 1;

    VDP_Status &= 0xFFF7;
    if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;
    VDP_Reg_Set2_Current = VDP_Reg.Set2;
    VDP_Reg_Set4_Current = VDP_Reg.Set4;

    HInt_Counter = VDP_Reg.H_Int;		// Hint_Counter = step d'interruption H

    if (fast)
        FakeVDPScreen = true;

    for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
    {
        if (!disableSound)
        {
            buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM_Len += Sound_Extrapol[VDP_Current_Line][1];
            PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
        }
        Update_CDC_TRansfert();

        Fix_Controllers();
        Cycles_M68K += CPL_M68K;
        Cycles_Z80 += CPL_Z80;
        if (S68K_State == 1) Cycles_S68K += CPL_S68K;
        if (DMAT_Length) main68k_addCycles(Update_DMA());
        VDP_Status |= 0x0004;			// HBlank = 1
        main68k_exec(Cycles_M68K - 404);
        VDP_Status &= 0xFFFB;			// HBlank = 0

        if (--HInt_Counter < 0)
        {
            HInt_Counter = VDP_Reg.H_Int;
            VDP_Int |= 0x4;
            Update_IRQ_Line();
        }

        if (!fast)
            Render_Line();

        main68k_exec(Cycles_M68K);
        sub68k_exec(Cycles_S68K);
        if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
        else z80_Set_Odo(&M_Z80, Cycles_Z80);

        Update_SegaCD_Timer();
    }

    if (!fast)
    {
        FakeVDPScreen = false;
        Render_MD_Screen();
    }

    if (!disableSound)
    {
        buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
        buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
        if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
        YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
        YM_Len += Sound_Extrapol[VDP_Current_Line][1];
        PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
    }
    Update_CDC_TRansfert();

    Fix_Controllers();
    Cycles_M68K += CPL_M68K;
    Cycles_Z80 += CPL_Z80;
    if (S68K_State == 1) Cycles_S68K += CPL_S68K;
    if (DMAT_Length) main68k_addCycles(Update_DMA());
    if (--HInt_Counter < 0)
    {
        VDP_Int |= 0x4;
        Update_IRQ_Line();
    }

    VDP_Status |= 0x000C;				// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
    main68k_exec(Cycles_M68K - 360);
    sub68k_exec(Cycles_S68K - 586);
    if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
    else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

    VDP_Status &= 0xFFFB;				// HBlank = 0
    VDP_Status |= 0x0080;				// V Int happened
    VDP_Int |= 0x8;
    Update_IRQ_Line();
    z80_Interrupt(&M_Z80, 0xFF);

    main68k_exec(Cycles_M68K);
    sub68k_exec(Cycles_S68K);
    if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
    else z80_Set_Odo(&M_Z80, Cycles_Z80);

    Update_SegaCD_Timer();

    for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
    {
        if (!disableSound)
        {
            buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM_Len += Sound_Extrapol[VDP_Current_Line][1];
            PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
        }
        Update_CDC_TRansfert();

        Fix_Controllers();
        Cycles_M68K += CPL_M68K;
        Cycles_Z80 += CPL_Z80;
        if (S68K_State == 1) Cycles_S68K += CPL_S68K;
        if (DMAT_Length) main68k_addCycles(Update_DMA());
        VDP_Status |= 0x0004;					// HBlank = 1
        main68k_exec(Cycles_M68K - 404);
        VDP_Status &= 0xFFFB;					// HBlank = 0

        main68k_exec(Cycles_M68K);
        sub68k_exec(Cycles_S68K);
        if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
        else z80_Set_Odo(&M_Z80, Cycles_Z80);

        Update_SegaCD_Timer();
    }

    if (!disableSound)
    {
        buf[0] = LeftAudioBuffer();
        buf[1] = RightAudioBuffer();

        PSG_Special_Update();
        YM2612_Special_Update();
        Update_CD_Audio(buf, Seg_Length);
    }

    if (!fast)
        Update_RAM_Search();

    return(1);
}

DO_FRAME_HEADER(Do_SegaCD_Frame, Do_SegaCD_Frame_No_VDP)
{
    return Do_SegaCD_Frame(false);
}

DO_FRAME_HEADER(Do_SegaCD_Frame_No_VDP, Do_SegaCD_Frame_No_VDP)
{
    return Do_SegaCD_Frame(true);
}

int Do_SegaCD_Frame_Cycle_Accurate(bool fast)
{
    struct Scope { Scope() { Inside_Frame = 1; } ~Scope() { Inside_Frame = 0; } } scope;

    int *buf[2], i, j;
    int HInt_Counter;

    if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
    else VDP_Num_Vis_Lines = 224;

    CPL_S68K = 795;

    if (!disableSound)
    {
        YM_Buf[0] = PSG_Buf[0] = LeftAudioBuffer();
        YM_Buf[1] = PSG_Buf[1] = RightAudioBuffer();
    }
    YM_Len = PSG_Len = 0;

    Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
    Last_BUS_REQ_Cnt = -1000;
    main68k_tripOdometer();
    sub68k_tripOdometer();
    z80_Clear_Odo(&M_Z80);

    Patch_Codes();

    VRam_Flag = 1;

    VDP_Status &= 0xFFF7;
    if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;
    VDP_Reg_Set2_Current = VDP_Reg.Set2;
    VDP_Reg_Set4_Current = VDP_Reg.Set4;

    HInt_Counter = VDP_Reg.H_Int;		// Hint_Counter = step d'interruption H

    if (fast)
        FakeVDPScreen = true;

    for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
    {
        if (!disableSound)
        {
            buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM_Len += Sound_Extrapol[VDP_Current_Line][1];
            PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
        }
        Update_CDC_TRansfert();

        i = Cycles_M68K + 24;
        j = Cycles_S68K + 39;

        Fix_Controllers();
        Cycles_M68K += CPL_M68K;
        Cycles_Z80 += CPL_Z80;
        if (S68K_State == 1) Cycles_S68K += CPL_S68K;
        if (DMAT_Length) main68k_addCycles(Update_DMA());
        VDP_Status |= 0x0004;			// HBlank = 1

        /* instruction by instruction execution */

        while (i < (Cycles_M68K - 404))
        {
            main68k_exec(i);
            i += 24;

            if (j < (Cycles_S68K - 658))
            {
                sub68k_exec(j);
                j += 39;
            }
        }

        main68k_exec(Cycles_M68K - 404);
        sub68k_exec(Cycles_S68K - 658);

        /* end instruction by instruction execution */

        VDP_Status &= 0xFFFB;			// HBlank = 0

        if (--HInt_Counter < 0)
        {
            HInt_Counter = VDP_Reg.H_Int;
            VDP_Int |= 0x4;
            Update_IRQ_Line();
        }

        if (!fast)
            Render_Line();

        /* instruction by instruction execution */

        while (i < Cycles_M68K)
        {
            main68k_exec(i);
            i += 24;

            if (j < Cycles_S68K)
            {
                sub68k_exec(j);
                j += 39;
            }
        }

        main68k_exec(Cycles_M68K);
        sub68k_exec(Cycles_S68K);

        /* end instruction by instruction execution */

        if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
        else z80_Set_Odo(&M_Z80, Cycles_Z80);

        Update_SegaCD_Timer();
    }

    if (!fast)
    {
        FakeVDPScreen = false;
        Render_MD_Screen();
    }

    if (!disableSound)
    {
        buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
        buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
        if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
        YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
        YM_Len += Sound_Extrapol[VDP_Current_Line][1];
        PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
    }
    Update_CDC_TRansfert();

    i = Cycles_M68K + 24;
    j = Cycles_S68K + 39;

    Fix_Controllers();
    Cycles_M68K += CPL_M68K;
    Cycles_Z80 += CPL_Z80;
    if (S68K_State == 1) Cycles_S68K += CPL_S68K;
    if (DMAT_Length) main68k_addCycles(Update_DMA());
    if (--HInt_Counter < 0)
    {
        VDP_Int |= 0x4;
        Update_IRQ_Line();
    }

    VDP_Status |= 0x000C;				// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)

    /* instruction by instruction execution */

    while (i < (Cycles_M68K - 360))
    {
        main68k_exec(i);
        i += 24;

        if (j < (Cycles_S68K - 586))
        {
            sub68k_exec(j);
            j += 39;
        }
    }

    main68k_exec(Cycles_M68K - 360);
    sub68k_exec(Cycles_S68K - 586);

    /* end instruction by instruction execution */

    if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
    else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

    VDP_Status &= 0xFFFB;				// HBlank = 0
    VDP_Status |= 0x0080;				// V Int happened
    VDP_Int |= 0x8;
    Update_IRQ_Line();
    z80_Interrupt(&M_Z80, 0xFF);

    /* instruction by instruction execution */

    while (i < Cycles_M68K)
    {
        main68k_exec(i);
        i += 24;

        if (j < Cycles_S68K)
        {
            sub68k_exec(j);
            j += 39;
        }
    }

    main68k_exec(Cycles_M68K);
    sub68k_exec(Cycles_S68K);

    /* end instruction by instruction execution */

    if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
    else z80_Set_Odo(&M_Z80, Cycles_Z80);

    Update_SegaCD_Timer();

    for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
    {
        if (!disableSound)
        {
            buf[0] = LeftAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            buf[1] = RightAudioBuffer() + Sound_Extrapol[VDP_Current_Line][0];
            if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
            YM_Len += Sound_Extrapol[VDP_Current_Line][1];
            PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
        }
        Update_CDC_TRansfert();

        i = Cycles_M68K + 24;
        j = Cycles_S68K + 39;

        Fix_Controllers();
        Cycles_M68K += CPL_M68K;
        Cycles_Z80 += CPL_Z80;
        if (S68K_State == 1) Cycles_S68K += CPL_S68K;
        if (DMAT_Length) main68k_addCycles(Update_DMA());
        VDP_Status |= 0x0004;					// HBlank = 1

        /* instruction by instruction execution */

        while (i < (Cycles_M68K - 404))
        {
            main68k_exec(i);
            i += 24;

            if (j < (Cycles_S68K - 658))
            {
                sub68k_exec(j);
                j += 39;
            }
        }

        main68k_exec(Cycles_M68K - 404);
        sub68k_exec(Cycles_S68K - 658);

        /* end instruction by instruction execution */

        VDP_Status &= 0xFFFB;					// HBlank = 0

        /* instruction by instruction execution */

        while (i < Cycles_M68K)
        {
            main68k_exec(i);
            i += 24;					// Chuck Rock intro need faster timing ... strange.

            if (j < Cycles_S68K)
            {
                sub68k_exec(j);
                j += 39;
            }
        }

        main68k_exec(Cycles_M68K);
        sub68k_exec(Cycles_S68K);

        /* end instruction by instruction execution */

        if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
        else z80_Set_Odo(&M_Z80, Cycles_Z80);

        Update_SegaCD_Timer();
    }

    if (!disableSound)
    {
        buf[0] = LeftAudioBuffer();
        buf[1] = RightAudioBuffer();

        PSG_Special_Update();
        YM2612_Special_Update();
        Update_CD_Audio(buf, Seg_Length);
    }

    if (!fast)
        Update_RAM_Search();

    return 1;
}

DO_FRAME_HEADER(Do_SegaCD_Frame_Cycle_Accurate, Do_SegaCD_Frame_No_VDP_Cycle_Accurate)
{
    return Do_SegaCD_Frame_Cycle_Accurate(false);
}

DO_FRAME_HEADER(Do_SegaCD_Frame_No_VDP_Cycle_Accurate, Do_SegaCD_Frame_No_VDP_Cycle_Accurate)
{
    return Do_SegaCD_Frame_Cycle_Accurate(true);
}