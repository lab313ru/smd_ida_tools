#ifndef VDP_REND_H
#define VDP_REND_H

#ifdef __cplusplus
extern "C" {
#endif

    // 0ahsbbbbggggrrrr main screen
    extern unsigned short Screen_16X[336 * 240];

    // pbbbbbgggggrrrrr 32X screen
    extern unsigned short Screen_32X[336 * 240];

    extern unsigned int MD_Screen32[336 * 240];
    extern unsigned short MD_Screen[336 * 240];

    // from ttppiiii into RGB24
    // t = type {0 = normal, 1 = darkened, 2 = normal, 3 = lightened}
    // p = pal (0..3), i = index (0..F)
    //extern unsigned int MD_Palette32[256];

    // from ttppiiii into RGB555 if (Mode_555 & 1) else RGB565
    //extern unsigned short MD_Palette[256];

    // from 0ahsbbbbggggrrrr into ARGB32
    extern unsigned int Palette32[0x8000];

    // 000bbb0ggg0rrr
    extern unsigned short LockedPalette[0x40];

    // from 0ahsbbbbggggrrrr into RGB555 if (Mode_555 & 1) else RGB565
    extern unsigned short Palette[0x8000];
    extern unsigned long TAB336[240];
    extern unsigned char Bits32;

    extern struct
    {
        int Pos_X;
        int Pos_Y;
        unsigned int Size_X;
        unsigned int Size_Y;
        int Pos_X_Max;
        int Pos_Y_Max;
        unsigned int Num_Tile;
        int dirt;
    } Sprite_Struct[256];

    extern unsigned char _32X_Rend_Mode;
    extern int Mode_555;
    extern int Sprite_Over;
    extern int Sprite_Boxing;
    extern char VScrollAl; //Nitsuja added this
    extern char VScrollAh; //Nitsuja added this
    extern char VScrollBl; //Nitsuja added this
    extern char VScrollBh; //Nitsuja added this
    extern char VSpritel; //Upthorn added this
    extern char VSpriteh; //Upthorn added this
    extern char ScrollAOn;
    extern char ScrollBOn;
    extern char SpriteOn;
    extern char Sprite_Always_Top;
    extern char Swap_Scroll_PriorityA;
    extern char Swap_Scroll_PriorityB;
    extern char Swap_Sprite_Priority;
    extern char Swap_32X_Plane_Priority;
    extern char _32X_Plane_High_On;
    extern char _32X_Plane_Low_On;
    extern char _32X_Plane_On;
    extern char PalLock;
    extern bool PinkBG;

    void Render_Line();
    void Post_Line();
    void Render_Line_32X();

#ifdef __cplusplus
};
#endif

#endif
