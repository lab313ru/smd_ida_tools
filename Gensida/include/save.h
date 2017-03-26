#ifndef SAVE_H_
#define SAVE_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GENESIS_V9_LENGTH  0x34380
#define GENESIS_V8_LENGTH  0x34353
#define GENESIS_V7_LENGTH  0x3434E
#define GENESIS_V6_LENGTH  0x22C68
#define SEGACD_V9_LENGTH_EX  0xF3D40
#define SEGACD_V8_LENGTH_EX  0xF3D2F
#define G32X_V9_LENGTH_EX  0x849E0
#define G32X_V8_LENGTH_EX  0x849BF

#define LATEST_SAVESTATE_VERSION        9
#define GENESIS_STATE_LENGTH   GENESIS_V9_LENGTH
#define SEGACD_LENGTH_EX        SEGACD_V9_LENGTH_EX
#define G32X_LENGTH_EX            G32X_V9_LENGTH_EX

#define MAX_STATE_FILE_LENGTH (GENESIS_STATE_LENGTH + SEGACD_LENGTH_EX + G32X_LENGTH_EX)

    extern char State_Dir[1024];
    extern char SRAM_Dir[1024];
    extern char BRAM_Dir[1024];
    extern unsigned short FrameBuffer[336 * 240];
    extern unsigned int FrameBuffer32[336 * 240];

#ifdef _MSC_VER
#define ALIGN16 __declspec(align(16)) // 16-byte alignment speeds up memcpy for size >= 0x100 (as of VS2005, if SSE2 is supported at runtime)
#else
#define ALIGN16 // __attribute__((aligned(16)))
#endif

    extern ALIGN16 unsigned char State_Buffer[MAX_STATE_FILE_LENGTH];

    int Change_File_S(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext, HWND hwnd);
    int Change_File_L(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext, HWND hwnd);
    int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext, HWND hwnd);
    FILE *Get_State_File();
    void Get_State_File_Name(char *name);
    int Load_State_From_Buffer(unsigned char *buf);
    int Save_State_To_Buffer(unsigned char *buf);
    int Load_State(char *Name);
    int Save_State(char *Name);
    int Import_Genesis(unsigned char *Data);
    void Export_Genesis(unsigned char *Data);
    int Import_SegaCD(unsigned char *Data);
    void Export_SegaCD(unsigned char *Data);
    int Import_32X(unsigned char *Data);
    void Export_32X(unsigned char *Data);
    int Save_Config(char *File_Name);
    int Save_As_Config(HWND hWnd);
    int Load_Config(char *File_Name, void *Game_Active);
    int Load_As_Config(HWND hWnd, void *Game_Active);
    int Load_SRAM(void);
    int Load_SRAMFromBuf(char *buf);
    int Save_SRAM(void);
    int Save_SRAMToBuf(char *buf);
    int Load_BRAM(void);
    int Save_BRAM(void);
    void Format_Backup_Ram(void);

#ifdef __cplusplus
};
#endif

#endif
