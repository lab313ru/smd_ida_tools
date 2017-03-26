#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include "rom.h"
#include "g_dsound.h"
#include "g_main.h"
#include "gens.h"
#include "ggenie.h"
#include "cpu_68k.h"
#include "cd_sys.h"
#include "mem_m68k.h"
#include "mem_sh2.h"
#include "vdp_io.h"
#include "save.h"
#include "misc.h"
#include "unzip.h"
#include "wave.h"
#include "cd_file.h"
#include "luascript.h"
#include <assert.h>

int File_Type_Index;
Rom *My_Rom = NULL;
char Rom_Name[512];
char Rom_Dir[1024];
char IPS_Dir[1024];
char Recent_Rom[MAX_RECENT_ROMS][1024];
char US_CD_Bios[1024];
char EU_CD_Bios[1024];
char JA_CD_Bios[1024];
char _32X_Genesis_Bios[1024];
char _32X_Master_Bios[1024];
char _32X_Slave_Bios[1024];
char Genesis_Bios[1024];

void Get_Name_From_Path(char *Full_Path, char *Name)
{
    int i = 0;

    i = strlen(Full_Path) - 1;

    while ((i >= 0) && (Full_Path[i] != '\\') && (Full_Path[i] != '/') && (Full_Path[i] != '|')) i--;

    if (i <= 0)
    {
        Name[0] = 0;
    }
    else
    {
        strcpy(Name, &Full_Path[++i]);
    }
}

void Get_Dir_From_Path(char *Full_Path, char *Dir)
{
    int i = 0;

    i = strlen(Full_Path) - 1;

    while ((i >= 0) && (Full_Path[i] != '\\') && (Full_Path[i] != '/') /*&& (Full_Path[i] != '|')*/) i--;

    if (i <= 0)
    {
        Dir[0] = 0;
    }
    else
    {
        strncpy(Dir, Full_Path, ++i);
        Dir[i] = 0;
    }
}

void Update_Recent_Rom(const char *Path)
{
    int i;
    for (i = 0; i < MAX_RECENT_ROMS; i++)
    {
        if (!(strcmp(Recent_Rom[i], Path)))
        {
            // move recent item to the top of the list
            if (i == 0)
                return;
            char temp[1024];
            strcpy(temp, Recent_Rom[i]);
            int j;
            for (j = i; j > 0; j--)
                strcpy(Recent_Rom[j], Recent_Rom[j - 1]);
            strcpy(Recent_Rom[0], temp);
            return;
        }
    }

    for (i = MAX_RECENT_ROMS - 1; i > 0; i--)
        strcpy(Recent_Rom[i], Recent_Rom[i - 1]);

    strcpy(Recent_Rom[0], Path);
}

void Update_Rom_Dir(char *Path)
{
    Get_Dir_From_Path(Path, Rom_Dir);
}

void Update_Rom_Name(char *Name)
{
    if (Name == Rom_Name) // do nothing if pointers are the same
        return;

    int i, leng;

    leng = strlen(Name) - 1;

    while ((leng >= 0) && (Name[leng] != '\\') && (Name[leng] != '/') && (Name[leng] != '|')) leng--;

    leng++; i = 0;

    while ((Name[leng]) && (Name[leng] != '.'))
        Rom_Name[i++] = Name[leng++];

    Rom_Name[i] = 0;
}

void Update_CD_Rom_Name(char *Name)
{
    int i, j;

    memcpy(Rom_Name, Name, 48);

    for (i = 0; i < 48; i++)
    {
        if ((Rom_Name[i] >= '0') && (Rom_Name[i] <= '9')) continue;
        if (Rom_Name[i] == ' ') continue;
        if ((Rom_Name[i] >= 'A') && (Rom_Name[i] <= 'Z')) continue;
        if ((Rom_Name[i] >= 'a') && (Rom_Name[i] <= 'z')) continue;
        Rom_Name[i] = ' ';
    }

    for (i = 0; i < 48; i++)
    {
        if (Rom_Name[i] != ' ') i = 100;
    }

    if (i < 100) strcpy(Rom_Name, "no name");

    for (i = 47, j = 48; i >= 0; i--, j--)
    {
        if (Rom_Name[i] != ' ') i = -1;
    }

    Rom_Name[j + 1] = 0;
}

int Detect_Format(char *FileName)
{
    int i;

    char Name[1024];
    strncpy(Name, FileName, 1024);
    Name[1023] = '\0';
    char* bar = strchr(Name, '|');
    if (bar)
        *bar = '\0';

    SetCurrentDirectory(Gens_Path);

    size_t Name_len = strlen(Name);
    if (Name_len > 3 && (!stricmp("CUE", &Name[Name_len - 3])))
    {
        char isoname[1024];
        isoname[0] = 0;
        Get_CUE_ISO_Filename(isoname, 1024, Name);
        size_t isoname_len = strlen(isoname);
        if (isoname[0] && !(isoname_len > 3 && (!stricmp("CUE", &isoname[isoname_len - 3]))))
            return Detect_Format(isoname);
    }

    char buf[1024] = { 0 };
    {
        FILE *f = *Name ? fopen(Name, "rb") : NULL;
        if (f == NULL) return -1;
        fread(buf, 1, 1024, f);
        fclose(f);
    }

    if (!strnicmp("SEGADISCSYSTEM", &buf[0x00], 14)) return SEGACD_IMAGE;		// Sega CD (ISO)
    if (!strnicmp("SEGADISCSYSTEM", &buf[0x10], 14)) return SEGACD_IMAGE + 1;	// Sega CD (BIN)

    i = 0;

    if (strnicmp("SEGA", &buf[0x100], 4))
    {
        // Maybe interleaved

        if (!strnicmp("EA", &buf[0x200 + (0x100 / 2)], 2)) i = 1;
        if ((buf[0x08] == 0xAA) && (buf[0x09] == 0xBB) && (buf[0x0A] == 0x06)) i = 1;
    }

    if (i)		// interleaved
    {
        if ((!strnicmp("32X", &Name[strlen(Name) - 3], 3)) && (buf[0x200 / 2] == 0x4E)) return _32X_ROM + 1;
        if (!strnicmp("3X", &buf[0x200 + (0x105 / 2)], 2)) return _32X_ROM + 1;
    }
    else
    {
        if ((!strnicmp("32X", &Name[strlen(Name) - 3], 3)) && (buf[0x200] == 0x4E)) return _32X_ROM;
        if (!strnicmp("32X", &buf[0x105], 3)) return _32X_ROM;
    }

    return GENESIS_ROM + i;
}

void De_Interleave(void)
{
    unsigned char buf[16384];
    unsigned char *Src;
    int i, j, Nb_Blocks, ptr;

    Src = &Rom_Data[0x200];

    Rom_Size -= 512;

    Nb_Blocks = Rom_Size / 16384;

    for (ptr = 0, i = 0; i < Nb_Blocks; i++, ptr += 16384)
    {
        memcpy(buf, &Src[ptr], 16384);

        for (j = 0; j < 8192; j++)
        {
            Rom_Data[ptr + (j << 1) + 1] = buf[j];
            Rom_Data[ptr + (j << 1)] = buf[j + 8192];
        }
    }
}

void Fill_Infos(void)
{
    int i;

    // Finally we do the IPS patch here, we can have the translated game name

    IPS_Patching();

    for (i = 0; i < 16; i++)
        My_Rom->Console_Name[i] = Rom_Data[i + 256];

    for (i = 0; i < 16; i++)
        My_Rom->Copyright[i] = Rom_Data[i + 272];

    for (i = 0; i < 48; i++)
        My_Rom->Rom_Name[i] = Rom_Data[i + 288];

    for (i = 0; i < 48; i++)
        My_Rom->Rom_Name_W[i] = Rom_Data[i + 336];

    My_Rom->Type[0] = Rom_Data[384];
    My_Rom->Type[1] = Rom_Data[385];

    for (i = 0; i < 12; i++)
        My_Rom->Version[i] = Rom_Data[i + 386];

    My_Rom->Checksum = (Rom_Data[398] << 8) | Rom_Data[399];

    for (i = 0; i < 16; i++)
        My_Rom->IO_Support[i] = Rom_Data[i + 400];

    My_Rom->Rom_Start_Address = Rom_Data[416] << 24;
    My_Rom->Rom_Start_Address |= Rom_Data[417] << 16;
    My_Rom->Rom_Start_Address |= Rom_Data[418] << 8;
    My_Rom->Rom_Start_Address |= Rom_Data[419];

    My_Rom->Rom_End_Address = Rom_Data[420] << 24;
    My_Rom->Rom_End_Address |= Rom_Data[421] << 16;
    My_Rom->Rom_End_Address |= Rom_Data[422] << 8;
    My_Rom->Rom_End_Address |= Rom_Data[423];

    My_Rom->R_Size = My_Rom->Rom_End_Address - My_Rom->Rom_Start_Address + 1;

    for (i = 0; i < 12; i++)
        My_Rom->Ram_Infos[i] = Rom_Data[i + 424];

    My_Rom->Ram_Start_Address = Rom_Data[436] << 24;
    My_Rom->Ram_Start_Address |= Rom_Data[437] << 16;
    My_Rom->Ram_Start_Address |= Rom_Data[438] << 8;
    My_Rom->Ram_Start_Address |= Rom_Data[439];

    My_Rom->Ram_End_Address = Rom_Data[440] << 24;
    My_Rom->Ram_End_Address |= Rom_Data[441] << 16;
    My_Rom->Ram_End_Address |= Rom_Data[442] << 8;
    My_Rom->Ram_End_Address |= Rom_Data[443];

    for (i = 0; i < 12; i++)
        My_Rom->Modem_Infos[i] = Rom_Data[i + 444];

    for (i = 0; i < 40; i++)
        My_Rom->Description[i] = Rom_Data[i + 456];

    for (i = 0; i < 3; i++)
        My_Rom->Countries[i] = Rom_Data[i + 496];

    My_Rom->Console_Name[16] = 0;
    My_Rom->Copyright[16] = 0;
    My_Rom->Rom_Name[48] = 0;
    My_Rom->Rom_Name_W[48] = 0;
    My_Rom->Type[2] = 0;
    My_Rom->Version[12] = 0;
    My_Rom->IO_Support[12] = 0;
    My_Rom->Ram_Infos[12] = 0;
    My_Rom->Modem_Infos[12] = 0;
    My_Rom->Description[40] = 0;
    My_Rom->Countries[3] = 0;
}

// some extensions that might commonly be near ROM files that almost certainly aren't ROM files.
static const char* s_nonRomExtensions[] = { "txt", "nfo", "htm", "html", "jpg", "jpeg", "png", "bmp", "gif", "mp3", "wav", "lnk", "exe", "bat", "gmv", "gm2", "lua", "luasav", "sav", "srm", "brm", "cfg", "wch", "gs*" };
// question: why use exclusion instead of inclusion?
// answer: because filename extensions aren't that reliable.
// if it's one of these extensions then it's probably safe to assume it's not a ROM (and doing so makes things simpler and more convenient for the user),
// but if it isn't one of these then it's best to ask the user or check the file contents,
// in case it's a valid ROM or a valid ROM-containing archive with an unknown extension.

int Get_Rom(HWND hWnd)
{
    char Name[1024];
    OPENFILENAME ofn;
    int sys;

    SetCurrentDirectory(Gens_Path);

    memset(Name, 0, 1024);
    memset(&ofn, 0, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = ghInstance;
    ofn.lpstrFile = Name;
    ofn.nMaxFile = 1023;
    ofn.lpstrTitle = "Open ROM";

    const char* supportedArchives = "";

    char filterString[2048];
    char* filterPtr = filterString;
#define APPEND_FILTER(x) filterPtr += 1 + sprintf(filterPtr, "%s", x)
#define EXTEND_FILTER(x) filterPtr--; APPEND_FILTER(x)

    APPEND_FILTER("Sega CD / 32X / Genesis files");
    APPEND_FILTER("*.bin;*.smd;*.gen;*.32x;*.cue;*.iso;*.raw");
    EXTEND_FILTER(supportedArchives);
    APPEND_FILTER("Genesis roms (*.smd *.bin *.gen)");
    APPEND_FILTER("*.smd;*.bin;*.gen");
    EXTEND_FILTER(supportedArchives);
    APPEND_FILTER("32X roms (*.32x)");
    APPEND_FILTER("*.32x;*32x*.bin;*32x*.gen;*32x*.smd");
    //EXTEND_FILTER(supportedArchives); // same as this but replace "*" with "*32x*" :
    filterPtr--; const char* arcptr = supportedArchives; while (*arcptr) { if (*arcptr == '*') { *filterPtr++ = '*'; *filterPtr++ = '3'; *filterPtr++ = '2'; *filterPtr++ = 'x'; } *filterPtr++ = *arcptr++; } *filterPtr++ = 0;
    APPEND_FILTER("Sega CD images (*.cue *.iso *.bin *.raw)");
    APPEND_FILTER("*.cue;*.iso;*.bin;*.raw");
    EXTEND_FILTER(supportedArchives); // not recommended for SegaCD but it should work to some degree...
    APPEND_FILTER("Compressed only");
    APPEND_FILTER(supportedArchives + 1);
    APPEND_FILTER("Uncompressed only");
    APPEND_FILTER("*.bin;*.smd;*.gen;*.32x;*.cue;*.iso;*.raw");
    APPEND_FILTER("Tagged with [!]");
    APPEND_FILTER("*[!]*.bin;*[!]*.smd;*[!]*.gen;*[!]*.32x;*[!]*.cue;*[!]*.iso;*[!]*.raw;*[!]*.zip;*[!]*.rar;*[!]*.7z");
    APPEND_FILTER("All Files");
    APPEND_FILTER("*.*");
    APPEND_FILTER("");

#undef APPEND_FILTER
#undef EXTEND_FILTER
    assert((filterPtr - filterString) < sizeof(filterString));
    ofn.lpstrFilter = filterString;

    ofn.nFilterIndex = File_Type_Index;
    ofn.lpstrInitialDir = Rom_Dir;
    ofn.lpstrDefExt = "smd";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileName(&ofn) == NULL) return 0;

    char LogicalName[1024], PhysicalName[1024];
    strcpy(LogicalName, Name);
    strcpy(PhysicalName, Name);

    Free_Rom(Game);

    sys = Detect_Format(PhysicalName);

    if (sys < 1) return -1;

    File_Type_Index = ofn.nFilterIndex;

    if ((File_Type_Index > 1) && (File_Type_Index < 5))
    {
        sys &= 1;
        sys |= (File_Type_Index - 1) << 1;
    }

    Update_Recent_Rom(LogicalName);
    Update_Rom_Dir(LogicalName);
    Update_Rom_Name(LogicalName);

    if ((sys >> 1) < 3)		// Have to load a rom
    {
        Game = Load_Rom(hWnd, PhysicalName, sys & 1);
    }

    switch (sys >> 1)
    {
    default:
    case 1:		// Genesis rom
        if (Game) Genesis_Started = Init_Genesis(Game);
        Build_Main_Menu();
        return Genesis_Started ? 1 : -1;
        break;

    case 2:		// 32X rom
        if (Game) _32X_Started = Init_32X(Game);
        Build_Main_Menu();
        return _32X_Started ? 1 : -1;
        break;

    case 3:		// Sega CD image
        SegaCD_Started = Init_SegaCD(PhysicalName);
        Build_Main_Menu();
        return SegaCD_Started ? 1 : -1;
        break;

    case 4:		// Sega CD 32X image
        break;
    }

    return -1;
}

int Pre_Load_Rom(HWND hWnd, const char *NameTemp)
{
    char Name[1024];
    strncpy(Name, NameTemp, 1024);

    int sys;

    SetCurrentDirectory(Gens_Path);

    char LogicalName[1024], PhysicalName[1024];
    strcpy(LogicalName, Name);
    strcpy(PhysicalName, Name);

    Free_Rom(Game);

    sys = Detect_Format(PhysicalName);

    if (sys < 1) return -1;

    Update_Recent_Rom(LogicalName);
    Update_Rom_Dir(LogicalName);
    Update_Rom_Name(LogicalName);

    if ((sys >> 1) < 3)		// Have to load a rom
    {
        Game = Load_Rom(hWnd, PhysicalName, sys & 1);
    }

    switch (sys >> 1)
    {
    default:
    case 1:		// Genesis rom
        if (Game) Genesis_Started = Init_Genesis(Game);
        Build_Main_Menu();
        return Genesis_Started ? 1 : -1;
        break;

    case 2:		// 32X rom
        if (Game) _32X_Started = Init_32X(Game);
        Build_Main_Menu();
        return _32X_Started ? 1 : -1;
        break;

    case 3:		// Sega CD image
        SegaCD_Started = Init_SegaCD(PhysicalName);
        Build_Main_Menu();
        return SegaCD_Started ? 1 : -1;
        break;

    case 4:		// Sega CD 32X image
        break;
    }

    return -1;
}

// Rom is already in buffer, we just need to fill rom structure
// and do init stuff...
int Load_Rom_CC(char *Name, int Size)
{
    My_Rom = (Rom*)malloc(sizeof(Rom));

    if (!My_Rom)
    {
        Game = NULL;
        return NULL;
    }

    Update_Rom_Name(Name);
    Rom_Size = Size;
    Fill_Infos();

    Game = My_Rom;

    Genesis_Started = Init_Genesis(Game);
    return Genesis_Started;
}

Rom *Load_Bios(HWND hWnd, char *Name)
{
    SetCurrentDirectory(Gens_Path);

    char LogicalName[1024], PhysicalName[1024];
    strcpy(LogicalName, Name);
    strcpy(PhysicalName, Name);

    Free_Rom(Game);

    Game = Load_Rom(hWnd, PhysicalName, 0);

    return Game;
}

Rom *Load_Rom(HWND hWnd, char *Name, int inter)
{
    SetCurrentDirectory(Gens_Path);

    memset(Rom_Data, 0, sizeof(Rom_Data));
    Rom_Size = 0;

    FILE* file = fopen(Name, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        int len = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (len <= sizeof(Rom_Data))
            Rom_Size = fread(Rom_Data, 1, len, file);
        fclose(file);
    }

    My_Rom = (Rom*)malloc(sizeof(Rom));
    // freed later in Free_Rom

    if (!Rom_Size || !My_Rom)
        return NULL;

    if (inter) De_Interleave();

    Fill_Infos();

    return My_Rom;
}

unsigned short Calculate_Checksum(void)
{
    unsigned short checksum = 0;
    unsigned int i;

    if (!Game) return(0);

    for (i = 512; i < Rom_Size; i += 2)
    {
        checksum = (checksum + Rom_Data[i + 0]) & 0xFFFF;
        checksum = (checksum + (Rom_Data[i + 1] << 8)) & 0xFFFF;
    }

    return checksum;
}

void Fix_Checksum(void)
{
    unsigned short checks;

    if (!Game) return;

    checks = Calculate_Checksum();

    if (Rom_Size)
    {
        Rom_Data[0x18E] = checks & 0xFF;
        Rom_Data[0x18F] = checks >> 8;
        _32X_Rom[0x18E] = checks >> 8;;
        _32X_Rom[0x18F] = checks & 0xFF;
    }
}

unsigned int Calculate_CRC32(void)
{
    unsigned int crc = 0;

    Byte_Swap(Rom_Data, Rom_Size);
    crc = crc32(0, Rom_Data, Rom_Size);
    Byte_Swap(Rom_Data, Rom_Size);

    return crc;
}

int IPS_Patching(void)
{
    FILE *IPS_File;
    char Name[1024];
    unsigned char buf[16];
    unsigned int adr, len, i;

    SetCurrentDirectory(Gens_Path);

    strcpy(Name, IPS_Dir);
    strcat(Name, Rom_Name);
    strcat(Name, ".ips");

    IPS_File = fopen(Name, "rb");

    if (IPS_File == NULL) return 1;

    fseek(IPS_File, 0, SEEK_SET);

    fread(buf, 1, 5, IPS_File);
    buf[5] = 0;

    if (stricmp((char *)buf, "patch"))
    {
        fclose(IPS_File);
        return 2;
    }

    fread(buf, 1, 3, IPS_File);
    buf[3] = 0;

    while (stricmp((char *)buf, "eof"))
    {
        adr = (unsigned int)buf[2];
        adr += (unsigned int)(buf[1] << 8);
        adr += (unsigned int)(buf[0] << 16);

        if (fread(buf, 1, 2, IPS_File) == 0)
        {
            fclose(IPS_File);
            return 3;
        }

        len = (unsigned int)buf[1];
        len += (unsigned int)(buf[0] << 8);

        for (i = 0; i < len; i++)
        {
            if (fread(buf, 1, 1, IPS_File) == 0)
            {
                fclose(IPS_File);
                return 3;
            }

            if (adr < Rom_Size) Rom_Data[adr++] = buf[0];
        }

        if (fread(buf, 1, 3, IPS_File) == 0)
        {
            fclose(IPS_File);
            return 3;
        }

        buf[3] = 0;
    }

    fclose(IPS_File);

    return 0;
}

void Free_Rom(Rom *Rom_MD)
{
    if (Game == NULL) return;
    //CloseRamWindows();

    StopAllLuaScripts();

#ifdef CC_SUPPORT
    CC_Close();
#endif

    if (SegaCD_Started) Save_BRAM();
    Save_SRAM();
    Save_Patch_File();
    if (SegaCD_Started) Stop_CD();
    Genesis_Started = 0;
    _32X_Started = 0;
    SegaCD_Started = 0;
    Game = NULL;

    if (Rom_MD)
    {
        free(Rom_MD);
        Rom_MD = NULL;
    }

    if (Intro_Style == 3) Init_Genesis_Bios();

    SetWindowText(HWnd, GENS_NAME " - Idle");
}