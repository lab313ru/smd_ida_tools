#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
    struct Reg_VDP_Type
    {
        union
        {
            struct {
                unsigned int Set1; // 0
                unsigned int Set2; // 1
                unsigned int Pat_ScrA_Adr; // 2
                unsigned int Pat_Win_Adr; // 3
                unsigned int Pat_ScrB_Adr; // 4
                unsigned int Spr_Att_Adr; // 5
                unsigned int Reg6; // 6
                unsigned int BG_Color; // 7
                unsigned int Reg8; // 8
                unsigned int Reg9; // 9
                unsigned int H_Int; // 10
                unsigned int Set3; // 11
                unsigned int Set4; // 12
                unsigned int H_Scr_Adr; // 13
                unsigned int Reg14; // 14
                unsigned int Auto_Inc; // 15
                unsigned int Scr_Size; // 16
                unsigned int Win_H_Pos; // 17
                unsigned int Win_V_Pos; // 18
                unsigned int DMA_Length_L; // 19
                unsigned int DMA_Length_H; // 20
                unsigned int DMA_Src_Adr_L; // 21
                unsigned int DMA_Src_Adr_M; // 22
                unsigned int DMA_Src_Adr_H; // 23
            };
            unsigned int regs[24];
        };
        unsigned int DMA_Length;
        unsigned int DMA_Address;
    };
#pragma pack(pop)

    // XXX: these array sizes appear to be completely different from what they are declared as in the ASM files
    extern unsigned char VRam[65536];
    extern unsigned short CRam[256];
    extern unsigned char VSRam[256];
    extern unsigned char H_Counter_Table[512 * 2];
    //extern unsigned int Spr_Link[256]; // not really
    extern int H_Pix_Begin;
    extern int Genesis_Started;
    extern int SegaCD_Started;
    extern int _32X_Started;

    extern struct Reg_VDP_Type VDP_Reg;
    extern int VDP_Current_Line;
    extern int VDP_Num_Lines;
    extern int VDP_Num_Vis_Lines;
    extern int CRam_Flag;
    extern int VRam_Flag;
    extern int VDP_Int;
    extern int VDP_Status;
    extern int DMAT_Length;
    extern int DMAT_Type;
    extern int DMAT_Tmp;
    extern struct {
        int Flag;
        int Data;
        int Write;
        int Access;
        int Address;
        int DMA_Mode;
        int DMA;
    } Ctrl;

    void Reset_VDP(void);
    unsigned int Update_DMA(void);
    unsigned short Read_VDP_Data(void);
    unsigned short Read_VDP_Status(void);
    unsigned char Read_VDP_H_Counter(void);
    unsigned char Read_VDP_V_Counter(void);
    int Write_Low_Byte_VDP_Data(unsigned char Data);
    int Write_High_Byte_VDP_Data(unsigned char Data);
    int Write_Word_VDP_Data(unsigned short Data);
    int Write_VDP_Ctrl(unsigned short Data);
    int Set_VDP_Reg(int Num_Reg, int Val);
    void Update_IRQ_Line(void);

#ifdef __cplusplus
};
#endif
