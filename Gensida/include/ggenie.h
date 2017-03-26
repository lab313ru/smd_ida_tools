/* Header file for decode functions */
#ifndef _GENIE_DECODE_H__
#define _GENIE_DECODE_H__

struct patch { unsigned int addr, data; unsigned char size; };

#ifdef __cplusplus
extern "C" {
#endif

    struct GG_Code
    {
        char code[16];
        char name[240];
        unsigned int active;
        unsigned int restore;
        unsigned int addr;
        unsigned int data;
        unsigned char size;
        unsigned char Type;
        unsigned char mode;
        char oper;
    };

    extern struct GG_Code Liste_GG[256];
    extern int List_GG_Max_Active_Index;
    extern char Patch_Dir[1024];
    extern int CheatCount;

    int Load_Patch_File(void);
    int Save_Patch_File(void);
    void decode(const char* code, struct patch *result);
    void Fix_Codes(unsigned int address, unsigned char size);
    void Patch_Codes(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _GENIE_DECODE_H__
