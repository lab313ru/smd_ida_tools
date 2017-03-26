#ifndef _MEM_M68K_H
#define _MEM_M68K_H

#ifdef __cplusplus
extern "C" {
#endif

    extern unsigned char Ram_68k[64 * 1024];
    extern unsigned char Rom_Data[8 * 1024 * 1024];
    extern unsigned char SRAM[64 * 1024];
    extern unsigned char Ram_Backup_Ex[64 * 1024];
    extern unsigned char Genesis_Rom[2 * 1024];
    extern unsigned char _32X_Genesis_Rom[256];

    extern unsigned int _32X_M68K_Read_Byte_Table[0x20];
    extern unsigned int _32X_M68K_Read_Word_Table[0x20];
    extern unsigned int _32X_M68K_Write_Byte_Table[0x10];
    extern unsigned int _32X_M68K_Write_Word_Table[0x10];

    extern unsigned int M68K_Read_Byte_Table[0x20];
    extern unsigned int M68K_Read_Word_Table[0x20];
    extern unsigned int M68K_Write_Byte_Table[0x10];
    extern unsigned int M68K_Write_Word_Table[0x10];

    extern unsigned int Rom_Size;

    extern int SRAM_Start;
    extern int SRAM_End;
    extern int SRAM_ON;
    extern int SRAM_Write;
    extern int SRAM_Custom;

    extern int BRAM_Ex_State;
    extern int BRAM_Ex_Size;

    extern int Z80_M68K_Cycle_Tab[512];

    extern int S68K_State;
    extern int Z80_State;
    extern int Last_BUS_REQ_Cnt;
    extern int Last_BUS_REQ_St;
    extern int Bank_M68K;
    extern int Bank_SH2;
    extern int Fake_Fetch;

    extern int CPL_M68K;
    extern int CPL_S68K;
    extern int CPL_Z80;
    extern int Cycles_M68K;
    extern int Cycles_S68K;
    extern int Cycles_Z80;

    extern int Game_Mode;
    extern int CPU_Mode;
    extern int Gen_Mode;
    extern int Gen_Version;

    void Init_Memory_M68K(int System_ID);
    unsigned char M68K_RB(unsigned int Adr);
    unsigned short M68K_RW(unsigned int Adr);
    void M68K_WBR(unsigned int Adr, unsigned char Data);
    void M68K_WBC(unsigned int Adr, unsigned char Data);
    void M68K_WB(unsigned int Adr, unsigned char Data);
    void M68K_WW(unsigned int Adr, unsigned short Data);
    void Update_SegaCD_Timer(void);

#ifdef __cplusplus
};
#endif

#endif
