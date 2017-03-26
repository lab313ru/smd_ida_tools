/* Decode a Game Genie code into an M68000 address/data pair.
 * The Game Genie code is made of the characters
 * ABCDEFGHJKLMNPRSTVWXYZ0123456789 (notice the missing I, O, Q and U).
 * Where A = 00000, B = 00001, C = 00010, ... , on to 9 = 11111.
 *
 * These come out to a very scrambled bit pattern like this:
 * (SCRA-MBLE is just an example)
 *
 *   S     C     R     A  -  M     B     L     E
 * 01111 00010 01110 00000 01011 00001 01010 00100
 * ijklm nopIJ KLMNO PABCD EFGHd efgha bcQRS TUVWX
 *
 * Our goal is to rearrange that to this:
 *
 * 0000 0101 1001 1100 0100 0100 : 1011 0000 0111 1000
 * ABCD EFGH IJKL MNOP QRST UVWX : abcd efgh ijkl mnop
 *
 * which in Hexadecimal is 059C44:B078. Simple, huh? ;)
 *
 * So, then, we dutifully change memory location 059C44 to B078!
 * (of course, that's handled by a different source file :)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "g_main.h"
#include "ggenie.h"
#include "mem_m68k.h"
#include "cpu_68k.h"
#include "rom.h"
#include "misc.h"

struct GG_Code Liste_GG[256] = { { "\0", "\0", 0, 0, 0, 0, 0 } };
int List_GG_Max_Active_Index = 0; // fix for Fix_Codes checking all 256 codes every time, which was slowing down emulation significantly because it gets called so often
static char genie_chars[] = "AaBbCcDdEeFfGgHhJjKkLlMmNnPpRrSsTtVvWwXxYyZz0O1I2233445566778899";
char Patch_Dir[1024] = "\\";
int CheatCount = 0;
void hook_write_cheat(unsigned int address, unsigned int data, unsigned char size);
/* genie_decode
 * This function converts a Game Genie code to an address:data pair.
 * The code is given as an 8-character string, like "BJX0SA1C". It need not
 * be null terminated, since only the first 8 characters are taken. It is
 * assumed that the code is already made of valid characters, i.e. there are no
 * Q's, U's, or symbols. If such a character is
 * encountered, the function will return with a warning on stderr.
 *
 * The resulting address:data pair is returned in the struct patch pointed to
 * by result. If an error results, both the address and data will be set to -1.
 */
template <typename T>
T CheatRead(unsigned int address)
{
    T val = 0;
    for (int i = 0; i < sizeof(T); i++)
        val <<= 8, val |= (T)(M68K_RB(address + i));
    return val;
}
template <typename T>
void CheatWrite(unsigned int address, T value)
{
    for (int i = sizeof(T) - 1; i >= 0; i--)
    {
        M68K_WBC(address++, (unsigned char)(value >> (i << 3)) & 0xff);
    }
}
static void genie_decode(const char* code, struct patch* result)
{
    int i = 0, n;
    char* x;

    for (; i < 8; ++i)
    {
        /* If strchr returns NULL, we were given a bad character */
        if (!(x = strchr(genie_chars, code[i])))
        {
            result->addr = -1; result->data = -1;
            return;
        }
        n = (x - genie_chars) >> 1;
        /* Now, based on which character this is, fit it into the result */
        switch (i)
        {
        case 0:
            /* ____ ____ ____ ____ ____ ____ : ____ ____ ABCD E___ */
            result->data |= n << 3;
            break;
        case 1:
            /* ____ ____ DE__ ____ ____ ____ : ____ ____ ____ _ABC */
            result->data |= n >> 2;
            result->addr |= (n & 3) << 14;
            break;
        case 2:
            /* ____ ____ __AB CDE_ ____ ____ : ____ ____ ____ ____ */
            result->addr |= n << 9;
            break;
        case 3:
            /* BCDE ____ ____ ___A ____ ____ : ____ ____ ____ ____ */
            result->addr |= (n & 0xF) << 20 | (n >> 4) << 8;
            break;
        case 4:
            /* ____ ABCD ____ ____ ____ ____ : ___E ____ ____ ____ */
            result->data |= (n & 1) << 12;
            result->addr |= (n >> 1) << 16;
            break;
        case 5:
            /* ____ ____ ____ ____ ____ ____ : E___ ABCD ____ ____ */
            result->data |= (n & 1) << 15 | (n >> 1) << 8;
            break;
        case 6:
            /* ____ ____ ____ ____ CDE_ ____ : _AB_ ____ ____ ____ */
            result->data |= (n >> 3) << 13;
            result->addr |= (n & 7) << 5;
            break;
        case 7:
            /* ____ ____ ____ ____ ___A BCDE : ____ ____ ____ ____ */
            result->addr |= n;
            break;
        }
        /* Go around again */
    }
    return;
}

/* "Decode" an address/data pair into a structure. This is for "012345:ABCD"
 * type codes. You're more likely to find Genie codes circulating around, but
 * there's a chance you could come on to one of these. Which is nice, since
 * they're MUCH easier to implement ;) Once again, the input should be depunc-
 * tuated already. */

static char hex_chars[] = "00112233445566778899AaBbCcDdEeFf";

static void hex_decode(const char *code, struct patch *result)
{
    char *x;
    int i;
    /* 6 digits for address */
    for (i = 0; i < 6; ++i)
    {
        if (!(x = strchr(hex_chars, code[i])))
        {
            result->addr = result->data = -1;
            return;
        }
        result->addr = (result->addr << 4) | ((x - hex_chars) >> 1);
    }
    /* 4 digits for data */
    for (i = 6; i < 10; ++i)
    {
        if (!(x = strchr(hex_chars, code[i])))
        {
            result->addr = result->data = -1;
            return;
        }
        result->data = (result->data << 4) | ((x - hex_chars) >> 1);
    }
}

/* THIS is the function you call from the MegaDrive or whatever. This figures
 * out whether it's a genie or hex code, depunctuates it, and calls the proper
 * decoder. */
void decode(const char* code, struct patch* result)
{
    int len = strlen(code), i, j;
    char code_to_pass[16];
    const char *ad, *da;
    int adl, dal;

    /* Initialize the result */
    result->addr = result->data = 0;

    /* If it's 9 chars long and the 5th is a hyphen, we have a Game Genie
     * code. */

    if (len == 9 && code[4] == '-')
    {
        /* Remove the hyphen and pass to genie_decode */
        code_to_pass[0] = code[0];
        code_to_pass[1] = code[1];
        code_to_pass[2] = code[2];
        code_to_pass[3] = code[3];
        code_to_pass[4] = code[5];
        code_to_pass[5] = code[6];
        code_to_pass[6] = code[7];
        code_to_pass[7] = code[8];
        code_to_pass[8] = '\0';
        genie_decode(code_to_pass, result);
        if (result->addr >= 0xE00000) result->addr |= 0xFF0000;
        result->size = 2;
        return;
    }

    /* Otherwise, we assume it's a hex code.
     * Find the colon so we know where address ends and data starts. If there's
     * no colon, then we haven't a code at all! */
    const char *x = strchr(code, ':');
    if (!(x)) goto bad_code;
    ad = code; da = x + 1; adl = x - code; dal = len - adl - 1;

    /* If a section is empty or too long, toss it */
    if (adl == 0 || adl > 6 || dal == 0 || dal > 4) goto bad_code;

    /* Pad the address with zeros, then fill it with the value */
    for (i = 0; i < (6 - adl); ++i) code_to_pass[i] = '0';
    for (j = 0; i < 6; ++i, ++j) code_to_pass[i] = ad[j];

    /* Do the same for data */
    for (i = 6; i < (10 - dal); ++i) code_to_pass[i] = '0';
    for (j = 0; i < 10; ++i, ++j) code_to_pass[i] = da[j];

    code_to_pass[10] = '\0';

    /* Decode and goodbye */
    hex_decode(code_to_pass, result);
    if (result->addr >= 0xE00000) result->addr |= 0xFF0000;
    result->size = 2;
    return;

bad_code:

    /* AGH! Invalid code! */
    result->data = result->addr = -1;
    result->size = 0;
    return;
}
void Fix_Codes(unsigned int address, unsigned char size)
{
    int end = List_GG_Max_Active_Index;
    for (int i = 0; i < end; i++)
    {
        if ((Liste_GG[i].active))
        {
            if (address >= 0xE00000) address |= 0xFF0000;
            if (!(((address + size) < Liste_GG[i].addr) || ((Liste_GG[i].addr + Liste_GG[i].size) < address)))
            {
                for (int j = Liste_GG[i].size - 1, addr = Liste_GG[i].addr; j >= 0; addr++, j--)
                    CheatWrite<unsigned char>(addr, (Liste_GG[i].data >> (j << 3)) & 0xFF);
            }
        }
    }
}
void Patch_Codes(void)
{
    int i;

    for (i = 0; i < 256; i++)
    {
        if ((Liste_GG[i].code[0] != 0) && (Liste_GG[i].addr != 0xFFFFFFFF) && (Liste_GG[i].active))
        {
            for (int j = Liste_GG[i].size - 1, addr = Liste_GG[i].addr; j >= 0; addr++, j--)
                CheatWrite<unsigned char>(addr, (Liste_GG[i].data >> (j << 3)) & 0xFF);
        }
    }
    return;
}

int check_code(char *Code, unsigned int ind)
{
    size_t Code_len = strlen(Code);
    if ((Code_len > 11) || (Code_len < 8))
    {
        return(0);
    }

    if (Code_len == 8)
    {
        Liste_GG[ind].code[0] = Code[0];
        Liste_GG[ind].code[1] = Code[1];
        Liste_GG[ind].code[2] = Code[2];
        Liste_GG[ind].code[3] = Code[3];
        Liste_GG[ind].code[4] = '-';
        Liste_GG[ind].code[5] = Code[4];
        Liste_GG[ind].code[6] = Code[5];
        Liste_GG[ind].code[7] = Code[6];
        Liste_GG[ind].code[8] = Code[7];
        Liste_GG[ind].code[9] = 0;
    }
    else if (Code_len == 10)
    {
        Liste_GG[ind].code[0] = Code[0];
        Liste_GG[ind].code[1] = Code[1];
        Liste_GG[ind].code[2] = Code[2];
        Liste_GG[ind].code[3] = Code[3];
        Liste_GG[ind].code[4] = Code[4];
        Liste_GG[ind].code[5] = Code[5];
        Liste_GG[ind].code[6] = ':';
        Liste_GG[ind].code[7] = Code[6];
        Liste_GG[ind].code[8] = Code[7];
        Liste_GG[ind].code[9] = Code[8];
        Liste_GG[ind].code[10] = Code[9];
        Liste_GG[ind].code[11] = 0;
    }
    else strcpy(Liste_GG[ind].code, Code);

    return(1);
}

int Load_Patch_File(void)
{
    HANDLE Patch_File;
    unsigned char *Patch_String;
    char Name[2048], Code[16], Comment[256], c;
    unsigned int i_code, i_comment, Ind_GG;
    unsigned long Length, Bytes_Read, i;
    enum etat_sec { DEB_LIGNE, CODE, BLANK, COMMENT, ERR } state = DEB_LIGNE;

    SetCurrentDirectory(Gens_Path);

    for (i = 0; i < 256; i++)
    {
        Liste_GG[i].code[0] = 0;
        Liste_GG[i].name[0] = 0;
        Liste_GG[i].active = 0;
        Liste_GG[i].addr = 0xFFFFFFFF;
        Liste_GG[i].data = 0;
        Liste_GG[i].restore = 0xFFFFFFFF;
    }
    List_GG_Max_Active_Index = 0;

    strcpy(Name, Patch_Dir);
    strcat(Name, Rom_Name);
    strcat(Name, ".pat");

    Patch_File = CreateFile(Name, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (Patch_File == INVALID_HANDLE_VALUE) return(0);

    Length = GetFileSize(Patch_File, &Bytes_Read);

    Patch_String = (unsigned char *)malloc(Length);

    if (!Patch_String)
    {
        CloseHandle(Patch_File);
        return(0);
    }

    if (ReadFile(Patch_File, Patch_String, Length, &Bytes_Read, NULL))
    {
        i = 0;
        Ind_GG = 0;

        while ((i < Bytes_Read) && (Ind_GG < 256))
        {
            c = Patch_String[i++];

            switch (state)
            {
            case DEB_LIGNE:
                switch (c)
                {
                case '\n':
                case '\t':
                case ' ':
                case 13:
                    break;

                default:
                    state = CODE;
                    i_code = 0;
                    Code[i_code++] = c;
                    break;
                }
                break;

            case CODE:
                switch (c)
                {
                case '\n':
                case 13:
                    Code[i_code] = 0;
                    if (check_code(Code, Ind_GG)) Ind_GG++;
                    state = DEB_LIGNE;
                    break;

                case '\t':
                case ' ':
                    Code[i_code] = 0;
                    if (check_code(Code, Ind_GG)) state = BLANK;
                    else state = ERR;
                    break;

                default:
                    if (i_code < 14) Code[i_code++] = c;
                    break;
                }
                break;

            case BLANK:
                switch (c)
                {
                case '\n':
                    state = DEB_LIGNE;
                    Ind_GG++;
                    break;

                case '\t':
                case ' ':
                    break;

                default:
                    i_comment = 0;
                    Comment[i_comment++] = c;
                    state = COMMENT;
                    break;
                }
                break;

            case COMMENT:
                switch (c)
                {
                case 13:
                    break;

                case '\n':
                    Comment[i_comment] = 0;
                    strcpy(Liste_GG[Ind_GG].name, Comment);
                    Ind_GG++;
                    state = DEB_LIGNE;
                    break;

                default:
                    if (i_comment < 240) Comment[i_comment++] = c;
                    break;
                }
                break;

            case ERR:
                switch (c)
                {
                case '\n':
                    state = DEB_LIGNE;
                    break;

                default:
                    break;
                }
                break;
            }
        }

        switch (state)
        {
        case CODE:
            Code[i_code] = 0;
            if (check_code(Code, Ind_GG)) Ind_GG++;
            break;

        case COMMENT:
            Comment[i_comment] = 0;
            strcpy(Liste_GG[Ind_GG].name, Comment);
            Ind_GG++;
            break;
        }
    }

    CloseHandle(Patch_File);
    free(Patch_String);

    return(1);
}

int Save_Patch_File(void)
{
    FILE *Patch_File;
    char Name[2048];
    int i;

    SetCurrentDirectory(Gens_Path);

    if (Liste_GG[0].code[0] == 0) return 0;

    strcpy(Name, Patch_Dir);
    strcat(Name, Rom_Name);
    strcat(Name, ".pat");

    Patch_File = fopen(Name, "w");

    if (Patch_File == NULL) return 0;

    for (i = 0; i < 256; i++)
    {
        if (Liste_GG[i].code[0] != 0)
            fprintf(Patch_File, "%s\t%s\n", Liste_GG[i].code, Liste_GG[i].name);
    }

    fclose(Patch_File);

    return(1);
}