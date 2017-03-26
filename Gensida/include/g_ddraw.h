#ifndef G_DDRAW_H
#define G_DDRAW_H

#define DIRECTDRAW_VERSION         0x0500

#include <ddraw.h>
#include <time.h>
#include "mem_m68k.h"

extern clock_t Last_Time;
extern clock_t New_Time;
extern clock_t Used_Time;

extern int Flag_Clr_Scr;
extern int Sleep_Time;
extern int FS_VSync;
extern int Res_X; //Upth-Add - Want the resolution
extern int Res_Y; //Upth-Add - to be externally accessible
extern bool FS_No_Res_Change; //Upth-Add - as well as the No_Res_Change flag
extern bool Def_Read_Only; //Upth-Add - This is here because G_ddraw.h has bools defined in it, and I couldn't find another good place
extern int W_VSync;
extern int Stretch;
extern int Blit_Soft;
extern int Correct_256_Aspect_Ratio;
extern int Effect_Color;
extern int FPS_Style;
extern int Message_Style;
extern bool frameadvSkipLag;
extern bool justlagged;

#define ALT_X_RATIO_RES (Correct_256_Aspect_Ratio ? 320 : 256)

#define VDP_REG_SET2 (FakeVDPScreen ? VDP_Reg.Set2 : VDP_Reg_Set2_Current)
#define VDP_REG_SET4 (FakeVDPScreen ? VDP_Reg.Set4 : VDP_Reg_Set4_Current)

#define IS_FULL_X_RESOLUTION ((VDP_REG_SET4 & 0x1) || !Game || !FrameCount)
#define IS_FULL_Y_RESOLUTION (CPU_Mode && (VDP_REG_SET2 & 0x8) && !(!Game || !FrameCount))

#define FULL_X_RESOLUTION (IS_FULL_X_RESOLUTION ? 320 : 256)
#define FULL_Y_RESOLUTION (IS_FULL_Y_RESOLUTION ? 240 : 224)

extern void(*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern void(*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern int(*Update_Frame)();
extern int(*Update_Frame_Fast)();

int Init_Fail(HWND hwnd);
int Init_DDraw(HWND hWnd);
int Clear_Primary_Screen(HWND hWnd);
int Clear_Back_Screen(HWND hWnd);
int Update_Gens_Logo(HWND hWnd);
int Update_Crazy_Effect(HWND hWnd);
int Update_Emulation(HWND hWnd);
int Update_Emulation_One(HWND hWnd);
int Update_Emulation_Netplay(HWND hWnd, int player, int num_player);
int Eff_Screen(void);
int Pause_Screen(void);
extern "C" void Put_Info(char *Message, int Duration = 4000);
extern "C" void Put_Info_NonImmediate(char *Message, int Duration = 4000);
int Show_Genesis_Screen(HWND hWnd);
int Flip(HWND hWnd);
void Restore_Primary(void);
void End_DDraw(void);
int Update_Frame_Adjusted();

//void MP3_update_test();
//void MP3_init_test();

#endif
