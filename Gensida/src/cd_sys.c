#include <stdio.h>
#include <windows.h>
#include "cd_aspi.h"		// this include cd_sys.h
#include "cd_file.h"
#include "gens.h"
#include "g_dsound.h"
#include "cdda_mp3.h"
#include "lc89510.h"
#include "star_68k.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "save.h"
#include "misc.h"

int File_Add_Delay = 0;

int CDDA_Enable;

int CD_Audio_Buffer_L[8192 * 2];
int CD_Audio_Buffer_R[8192 * 2];
int CD_Audio_Buffer_Read_Pos = 0;
int CD_Audio_Buffer_Write_Pos = 2000;
int CD_Audio_Starting;

int CD_Present;
int CD_Load_System;
int CD_Timer_Counter = 0;

int CDD_Complete;

int track_number;			// Used for the Get_Track_Adr function

unsigned int CD_timer_st;	// Used for CD timer
unsigned int CD_LBA_st;		// Used for CD timer
unsigned short CDDAVol = 256;

char played_tracks_linear[105] = { 0 };

ALIGN16 _scd SCD;

#ifdef DEBUG_CD
FILE *debug_SCD_file;
#endif

#define CHECK_TRAY_OPEN				\
if (SCD.Status_CDD == TRAY_OPEN)	\
{									\
	CDD.Status = SCD.Status_CDD;	\
									\
	CDD.Minute = 0;					\
	CDD.Seconde = 0;				\
	CDD.Frame = 0;					\
	CDD.Ext = 0;					\
									\
	CDD_Complete = 1;				\
									\
	return 2;						\
}

#define CHECK_CD_PRESENT			\
if (!CD_Present)					\
{									\
	SCD.Status_CDD = NOCD;			\
	CDD.Status = SCD.Status_CDD;	\
									\
	CDD.Minute = 0;					\
	CDD.Seconde = 0;				\
	CDD.Frame = 0;					\
	CDD.Ext = 0;					\
									\
	CDD_Complete = 1;				\
									\
	return 3;						\
}

void MSB2DWORD(unsigned int *d, unsigned char *b)
{
    unsigned int  retVal;

    retVal = (unsigned int)b[0];
    retVal = (retVal << 8) + (unsigned int)b[1];
    retVal = (retVal << 8) + (unsigned int)b[2];
    retVal = (retVal << 8) + (unsigned int)b[3];

    *d = retVal;
}

int MSF_to_LBA(_msf *MSF)
{
    return (MSF->M * 60 * 75) + (MSF->S * 75) + MSF->F - 150;
}

void LBA_to_MSF(int lba, _msf *MSF)
{
    if (lba < -150) lba = 0;
    else lba += 150;
    MSF->M = lba / (60 * 75);
    MSF->S = (lba / 75) % 60;
    MSF->F = lba % 75;
}

// Modif N. -- extracted function
static int MSF_to_Ordered(_msf *MSF)
{
    //return MSF_to_LBA(MSF);
    return (MSF->M << 16) + (MSF->S << 8) + MSF->F;
}

unsigned int MSF_to_Track(_msf *MSF)
{
    // Modif N. -- changed to give better results (nothing goes silent) if the tracks are out of order
    int i, Start, Cur = 0, BestIndex = -1;
    unsigned int BestValue = ~0;

    Start = MSF_to_Ordered(MSF);

    for (i = SCD.TOC.First_Track; i <= (SCD.TOC.Last_Track + 1); i++)
    {
        if (i > SCD.TOC.First_Track)
            Cur = MSF_to_Ordered(&SCD.TOC.Tracks[i - SCD.TOC.First_Track].MSF);

#ifdef DEBUG_CD
        //	fprintf(debug_SCD_file, " i = %d  start = %.8X  cur = %.8X\n", i, Start, Cur);
#endif
        if (Start >= Cur && (unsigned int)(Start - Cur) < BestValue)
        {
            BestIndex = i;
            BestValue = Start - Cur;
        }
    }

    if (BestIndex != -1)
        i = BestIndex;
    else
        return SCD.TOC.Last_Track;

    if (i > SCD.TOC.Last_Track) return SCD.TOC.Last_Track;
    if (i < SCD.TOC.First_Track) i = SCD.TOC.First_Track;

    return (unsigned)i;
}

unsigned int LBA_to_Track(int lba)
{
    _msf MSF;

    LBA_to_MSF(lba, &MSF);
    return MSF_to_Track(&MSF);
}

void Track_to_MSF(int track, _msf *MSF)
{
    if (track < SCD.TOC.First_Track) track = SCD.TOC.First_Track;
    else if (track > SCD.TOC.Last_Track) track = SCD.TOC.Last_Track;

    MSF->M = SCD.TOC.Tracks[track - SCD.TOC.First_Track].MSF.M;
    MSF->S = SCD.TOC.Tracks[track - SCD.TOC.First_Track].MSF.S;
    MSF->F = SCD.TOC.Tracks[track - SCD.TOC.First_Track].MSF.F;
}

int Track_to_LBA(int track)
{
    _msf MSF;

    Track_to_MSF(track, &MSF);
    return MSF_to_LBA(&MSF);
}

void Flush_CD_Command(void)
{
    CDD_Complete = 0;
}

void Check_CD_Command(void)
{
#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "CHECK CD COMMAND\n");
#endif

    // Check CDD

    if (CDD_Complete)
    {
        CDD_Complete = 0;

#ifdef DEBUG_CD
        fprintf(debug_SCD_file, "CDD exporting status\n");
        fprintf(debug_SCD_file, "Status=%.4X, Minute=%.4X, Seconde=%.4X, Frame=%.4X, Ext=%.4X\n", CDD.Status, CDD.Minute, CDD.Seconde, CDD.Frame, CDD.Ext);
#endif

        CDD_Export_Status();

        if (Int_Mask_S68K & 0x10) sub68k_interrupt(4, -1);
    }

    // Check CDC

    if (SCD.Status_CDC & 1)			// CDC is reading data ...
    {
#ifdef DEBUG_CD
        fprintf(debug_SCD_file, "Send a read command\n");
#endif

        // DATA ?
        if (SCD.TOC.Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type) CDD.Control |= 0x0100;
        else CDD.Control &= ~0x0100;			// AUDIO

        if (File_Add_Delay == 0)
        {
            if (CD_Load_System == CDROM_) ASPI_Read_One_LBA_CDC();
            else FILE_Read_One_LBA_CDC();
        }
        else File_Add_Delay--;
    }

    if (SCD.Status_CDD == FAST_FOW)
    {
        SCD.Cur_LBA += 10;
        CDC_Update_Header();
    }
    else if (SCD.Status_CDD == FAST_REV)
    {
        SCD.Cur_LBA -= 10;
        if (SCD.Cur_LBA < -150) SCD.Cur_LBA = -150;
        CDC_Update_Header();
    }
}

int Init_CD_Driver(void)
{
#ifdef DEBUG_CD
    debug_SCD_file = fopen("SCD.log", "w");
#endif

    // ASPI support

    ASPI_Init();

    //	if (ASPI_Init() == 0)
    //	{
    //		MessageBox(NULL, "No CD Drive or ASPI not correctly initialised\nTry to upgrade your ASPI drivers", "Warning", MB_ICONWARNING);
    //	}

    // FILE ISO support

    FILE_Init();

    return 0;
}

void End_CD_Driver(void)
{
#ifdef DEBUG_CD
    fclose(debug_SCD_file);
#endif

    // ASPI support

    ASPI_End();

    // FILE ISO support

    FILE_End();
}

int Reset_CD(char *buf, char *iso_name)
{
    memset(CD_Audio_Buffer_L, 0, 4096 * 4);
    memset(CD_Audio_Buffer_R, 0, 4096 * 4);

    if (iso_name == NULL)
    {
        CD_Load_System = CDROM_;
        ASPI_Reset_Drive(buf);
        ASPI_Lock(1);
        return 0;
    }
    else
    {
        char* dot = strrchr(iso_name, '.');
        if (dot && !stricmp(dot, ".cue"))
        {
            CD_Load_System = FILE_CUE;
            Load_CUE(buf, iso_name);
            CD_Present = 1;
            return 0;
        }
        else
        {
            // first look for a CUE file with the same name as the ISO
            char temp_cue_name[1024], *dot;
            FILE* temp_cue_file;
            strncpy(temp_cue_name, iso_name, 1024);
            temp_cue_name[1024 - 1] = 0;
            dot = strrchr(temp_cue_name, '.');
            if (dot) *dot = 0;
            strncat(temp_cue_name, ".cue", 1024 - strlen(temp_cue_name) - 1);
            temp_cue_file = fopen(temp_cue_name, "rb");
            if (temp_cue_file)
            {
                fclose(temp_cue_file);
                CD_Load_System = FILE_CUE;
                Load_CUE(buf, temp_cue_name);
                CD_Present = 1;
                return 0;
            }
            else
            {
                // no CUE file found, so just load the ISO alone, even though we won't be able to use any CD audio that's in it
                CD_Load_System = FILE_ISO;
                Load_ISO(buf, iso_name);
                CD_Present = 1;
                return 0;
            }
        }
    }
}

void Stop_CD(void)
{
    if (CD_Load_System == CDROM_)
    {
        ASPI_Lock(0);
        ASPI_Stop_Play_Scan(0, NULL);
    }
    else
    {
        Unload_ISO();
    }
}

void Change_CD(void)
{
    if (SCD.Status_CDD == TRAY_OPEN) Close_Tray_CDD_cC();
    else Open_Tray_CDD_cD();
}

int Get_Status_CDD_c0(void)
{
#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "Status command : Cur LBA = %d\n", SCD.Cur_LBA);
#endif

    // Clear immediat status
    if ((CDD.Status & 0x0F00) == 0x0200) CDD.Status = (SCD.Status_CDD & 0xFF00) | (CDD.Status & 0x00FF);
    else if ((CDD.Status & 0x0F00) == 0x0700) CDD.Status = (SCD.Status_CDD & 0xFF00) | (CDD.Status & 0x00FF);
    else if ((CDD.Status & 0x0F00) == 0x0E00) CDD.Status = (SCD.Status_CDD & 0xFF00) | (CDD.Status & 0x00FF);

    //	CDD.Status = (SCD.Status_CDD & 0xFF00) | (CDD.Status & 0x00FF);
    //	CDD.Status = SCD.Status_CDD;

    //	CDD.Minute = 0;
    //	CDD.Seconde = 0;
    //	CDD.Frame = 0;
    //	CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int Stop_CDD_c1(void)
{
    CHECK_TRAY_OPEN

        SCD.Status_CDC &= ~1;				// Stop CDC read

    if (CD_Load_System == CDROM_)
    {
        return ASPI_Stop_Play_Scan(1, ASPI_Stop_CDD_c1_COMP);
    }
    else
    {
        if (CD_Present) SCD.Status_CDD = STOPPED;
        else SCD.Status_CDD = NOCD;
        CDD.Status = 0x0000;

        CDD.Control |= 0x0100;			// Data bit set because stopped

        CDD.Minute = 0;
        CDD.Seconde = 0;
        CDD.Frame = 0;
        CDD.Ext = 0;
    }

    CDD_Complete = 1;

    return 0;
}

int Get_Pos_CDD_c20(void)
{
    _msf MSF;

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "command 200 : Cur LBA = %d\n", SCD.Cur_LBA);
#endif

    CHECK_TRAY_OPEN

        CDD.Status &= 0xFF;
    if (!CD_Present)
    {
        SCD.Status_CDD = NOCD;
        CDD.Status |= SCD.Status_CDD;
    }
    //	else if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
    CDD.Status |= SCD.Status_CDD;

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "Status CDD = %.4X  Status = %.4X\n", SCD.Status_CDD, CDD.Status);
#endif

    LBA_to_MSF(SCD.Cur_LBA, &MSF);

    CDD.Minute = INT_TO_BCDW(MSF.M);
    CDD.Seconde = INT_TO_BCDW(MSF.S);
    CDD.Frame = INT_TO_BCDW(MSF.F);
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int Get_Track_Pos_CDD_c21(void)
{
    int elapsed_time;
    _msf MSF;

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "command 201 : Cur LBA = %d", SCD.Cur_LBA);
#endif

    CHECK_TRAY_OPEN

        CDD.Status &= 0xFF;
    if (!CD_Present)
    {
        SCD.Status_CDD = NOCD;
        CDD.Status |= SCD.Status_CDD;
    }
    //	else if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
    CDD.Status |= SCD.Status_CDD;

    elapsed_time = SCD.Cur_LBA - Track_to_LBA(LBA_to_Track(SCD.Cur_LBA));
    LBA_to_MSF(elapsed_time - 150, &MSF);

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "   elapsed = %d\n", elapsed_time);
#endif

    CDD.Minute = INT_TO_BCDW(MSF.M);
    CDD.Seconde = INT_TO_BCDW(MSF.S);
    CDD.Frame = INT_TO_BCDW(MSF.F);
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int Get_Current_Track_CDD_c22(void)
{
#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "Status CDD = %.4X  Status = %.4X\n", SCD.Status_CDD, CDD.Status);
#endif

    CHECK_TRAY_OPEN

        CDD.Status &= 0xFF;
    if (!CD_Present)
    {
        SCD.Status_CDD = NOCD;
        CDD.Status |= SCD.Status_CDD;
    }
    //	else if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
    CDD.Status |= SCD.Status_CDD;

    SCD.Cur_Track = LBA_to_Track(SCD.Cur_LBA);
    played_tracks_linear[SCD.Cur_Track - SCD.TOC.First_Track] = 1;

    if (SCD.Cur_Track == 100) CDD.Minute = 0x0A02;
    else CDD.Minute = INT_TO_BCDW(SCD.Cur_Track);
    CDD.Seconde = 0;
    CDD.Frame = 0;
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int Get_Total_Length_CDD_c23(void)
{
    CHECK_TRAY_OPEN

        CDD.Status &= 0xFF;
    if (!CD_Present)
    {
        SCD.Status_CDD = NOCD;
        CDD.Status |= SCD.Status_CDD;
    }
    //	else if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
    CDD.Status |= SCD.Status_CDD;

    CDD.Minute = INT_TO_BCDW(SCD.TOC.Tracks[SCD.TOC.Last_Track - SCD.TOC.First_Track + 1].MSF.M);
    CDD.Seconde = INT_TO_BCDW(SCD.TOC.Tracks[SCD.TOC.Last_Track - SCD.TOC.First_Track + 1].MSF.S);
    CDD.Frame = INT_TO_BCDW(SCD.TOC.Tracks[SCD.TOC.Last_Track - SCD.TOC.First_Track + 1].MSF.F);
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int Get_First_Last_Track_CDD_c24(void)
{
    CHECK_TRAY_OPEN

        CDD.Status &= 0xFF;
    if (!CD_Present)
    {
        SCD.Status_CDD = NOCD;
        CDD.Status |= SCD.Status_CDD;
    }
    //	else if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
    CDD.Status |= SCD.Status_CDD;

    CDD.Minute = INT_TO_BCDW(SCD.TOC.First_Track);
    CDD.Seconde = INT_TO_BCDW(SCD.TOC.Last_Track);
    CDD.Frame = 0;
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int Get_Track_Adr_CDD_c25(void)
{
    CHECK_TRAY_OPEN

        // track number in TC4 & TC5

        track_number = (CDD.Trans_Comm[4] & 0xF) + (CDD.Trans_Comm[5] & 0xF) * 10;

    CDD.Status &= 0xFF;
    if (!CD_Present)
    {
        SCD.Status_CDD = NOCD;
        CDD.Status |= SCD.Status_CDD;
    }
    //	else if (!(CDC.CTRL.B.B0 & 0x80)) CDD.Status |= SCD.Status_CDD;
    CDD.Status |= SCD.Status_CDD;

    if (track_number > SCD.TOC.Last_Track) track_number = SCD.TOC.Last_Track;
    else if (track_number < SCD.TOC.First_Track) track_number = SCD.TOC.First_Track;

    CDD.Minute = INT_TO_BCDW(SCD.TOC.Tracks[track_number - SCD.TOC.First_Track].MSF.M);
    CDD.Seconde = INT_TO_BCDW(SCD.TOC.Tracks[track_number - SCD.TOC.First_Track].MSF.S);
    CDD.Frame = INT_TO_BCDW(SCD.TOC.Tracks[track_number - SCD.TOC.First_Track].MSF.F);
    CDD.Ext = track_number % 10;

    if (SCD.TOC.Tracks[track_number - SCD.TOC.First_Track].Type) CDD.Frame |= 0x0800;

    CDD_Complete = 1;
    return 0;
}

int Play_CDD_c3(void)
{
    _msf MSF;
    int delay, new_lba;

    CHECK_TRAY_OPEN
        CHECK_CD_PRESENT

        if (CD_Load_System == CDROM_)
        {
            SCD.Status_CDC &= ~1;			// Stop read data with CDC
            Wait_Read_Complete();
        }
        else //if(CD_Load_System == FILE_CUE) // Modif N. -- added
            SCD.Status_CDC &= ~1;

    // MSF of the track to play in TC buffer

    MSF.M = (CDD.Trans_Comm[2] & 0xF) + (CDD.Trans_Comm[3] & 0xF) * 10;
    MSF.S = (CDD.Trans_Comm[4] & 0xF) + (CDD.Trans_Comm[5] & 0xF) * 10;
    MSF.F = (CDD.Trans_Comm[6] & 0xF) + (CDD.Trans_Comm[7] & 0xF) * 10;

    SCD.Cur_Track = MSF_to_Track(&MSF);
    played_tracks_linear[SCD.Cur_Track - SCD.TOC.First_Track] = 1;

    new_lba = MSF_to_LBA(&MSF);
    delay = 0;		//upth mod - for consistency given varying track lengths
    /*delay = new_lba - SCD.Cur_LBA;

    if (delay < 0) delay = -delay;
    delay >>= 12;*/

    SCD.Cur_LBA = new_lba;
    CDC_Update_Header();

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "Read : Cur LBA = %d, M=%d, S=%d, F=%d\n", SCD.Cur_LBA, MSF.M, MSF.S, MSF.F);
#endif

    if (CD_Load_System == CDROM_)
    {
        delay += 20;
        delay >>= 2;
        ASPI_Seek(SCD.Cur_LBA, 1, ASPI_Fast_Seek_COMP);
        ASPI_Flush_Cache_CDC();
    }
    else if (SCD.Status_CDD != PLAYING)
    {
        delay += 20;
        delay >>= 2; // Modif N. -- added for consistency
    }

    SCD.Status_CDD = PLAYING;
    CDD.Status = 0x0102;
    //	CDD.Status = COMM_OK;

    if (File_Add_Delay == 0) File_Add_Delay = delay;

    if (SCD.TOC.Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type)
    {
        CDD.Control |= 0x0100;				// DATA
    }
    else
    {
        CDD.Control &= ~0x0100;				// AUDIO
        CD_Audio_Starting = 1;
        if (!(CD_Load_System == CDROM_)) FILE_Play_CD_LBA(1);
        played_tracks_linear[SCD.Cur_Track - SCD.TOC.First_Track] = 1;
    }

    if (SCD.Cur_Track == 100) CDD.Minute = 0x0A02;
    else CDD.Minute = INT_TO_BCDW(SCD.Cur_Track);
    CDD.Seconde = 0;
    CDD.Frame = 0;
    CDD.Ext = 0;

    SCD.Status_CDC |= 1;			// Read data with CDC

    CDD_Complete = 1;
    return 0;
}

int Seek_CDD_c4(void)
{
    _msf MSF;

    CHECK_TRAY_OPEN
        CHECK_CD_PRESENT

        // MSF to seek in TC buffer

        MSF.M = (CDD.Trans_Comm[2] & 0xF) + (CDD.Trans_Comm[3] & 0xF) * 10;
    MSF.S = (CDD.Trans_Comm[4] & 0xF) + (CDD.Trans_Comm[5] & 0xF) * 10;
    MSF.F = (CDD.Trans_Comm[6] & 0xF) + (CDD.Trans_Comm[7] & 0xF) * 10;

    SCD.Cur_Track = MSF_to_Track(&MSF);
    played_tracks_linear[SCD.Cur_Track - SCD.TOC.First_Track] = 1;
    SCD.Cur_LBA = MSF_to_LBA(&MSF);
    CDC_Update_Header();

    SCD.Status_CDC &= ~1;				// Stop CDC read

    if (CD_Load_System == CDROM_)
    {
        return ASPI_Seek(SCD.Cur_LBA, 1, ASPI_Seek_CDD_c4_COMP);
    }
    else
    {
        SCD.Status_CDD = READY;
        CDD.Status = 0x0200;

        // DATA ?
        if (SCD.TOC.Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type) CDD.Control |= 0x0100;
        else CDD.Control &= 0xFEFF;		// AUDIO

        CDD.Minute = 0;
        CDD.Seconde = 0;
        CDD.Frame = 0;
        CDD.Ext = 0;
    }

    CDD_Complete = 1;

    return 0;
}

int Pause_CDD_c6(void)
{
    CHECK_TRAY_OPEN
        CHECK_CD_PRESENT

        SCD.Status_CDC &= ~1;			// Stop CDC read to start a new one if raw data

    SCD.Status_CDD = READY;
    CDD.Status = SCD.Status_CDD;

    CDD.Control |= 0x0100;			// Data bit set because stopped

    CDD.Minute = 0;
    CDD.Seconde = 0;
    CDD.Frame = 0;
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int Resume_CDD_c7(void)
{
#ifdef DEBUG_CD
    _msf MSF;
#endif

    CHECK_TRAY_OPEN
        CHECK_CD_PRESENT

        if (CD_Load_System == CDROM_)
        {
            SCD.Status_CDC &= ~1;			// Stop read data with CDC
            Wait_Read_Complete();
        }
        else //if(CD_Load_System == FILE_CUE) // Modif N. -- added
            SCD.Status_CDC &= ~1;

    SCD.Cur_Track = LBA_to_Track(SCD.Cur_LBA);
    played_tracks_linear[SCD.Cur_Track - SCD.TOC.First_Track] = 1;

#ifdef DEBUG_CD
    LBA_to_MSF(SCD.Cur_LBA, &MSF);
    fprintf(debug_SCD_file, "Resume read : Cur LBA = %d, M=%d, S=%d, F=%d\n", SCD.Cur_LBA, MSF.M, MSF.S, MSF.F);
#endif

    SCD.Status_CDD = PLAYING;
    CDD.Status = 0x0102;

    if (CD_Load_System == CDROM_)
    {
        ASPI_Seek(SCD.Cur_LBA, 1, ASPI_Fast_Seek_COMP);
        ASPI_Flush_Cache_CDC();
    }

    if (SCD.TOC.Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type)
    {
        CDD.Control |= 0x0100;				// DATA
    }
    else
    {
        CDD.Control &= ~0x0100;				// AUDIO
        CD_Audio_Starting = 1;
        if (!(CD_Load_System == CDROM_)) FILE_Play_CD_LBA(1);
    }

    if (SCD.Cur_Track == 100) CDD.Minute = 0x0A02;
    else CDD.Minute = INT_TO_BCDW(SCD.Cur_Track);
    CDD.Seconde = 0;
    CDD.Frame = 0;
    CDD.Ext = 0;

    SCD.Status_CDC |= 1;			// Read data with CDC

    CDD_Complete = 1;
    return 0;
}

int	Fast_Foward_CDD_c8(void)
{
    CHECK_TRAY_OPEN
        CHECK_CD_PRESENT

        SCD.Status_CDC &= ~1;				// Stop CDC read

    SCD.Status_CDD = FAST_FOW;
    CDD.Status = SCD.Status_CDD | 2;

    CDD.Minute = INT_TO_BCDW(SCD.Cur_Track);
    CDD.Seconde = 0;
    CDD.Frame = 0;
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int	Fast_Rewind_CDD_c9(void)
{
    CHECK_TRAY_OPEN
        CHECK_CD_PRESENT

        SCD.Status_CDC &= ~1;				// Stop CDC read

    SCD.Status_CDD = FAST_REV;
    CDD.Status = SCD.Status_CDD | 2;

    CDD.Minute = INT_TO_BCDW(SCD.Cur_Track);
    CDD.Seconde = 0;
    CDD.Frame = 0;
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int Close_Tray_CDD_cC(void)
{
    Clear_Sound_Buffer();

    SCD.Status_CDC &= ~1;			// Stop CDC read

    if (CD_Load_System == CDROM_)
    {
        ASPI_Lock(0);
        ASPI_Star_Stop_Unit(CLOSE_TRAY, 0, 0, ASPI_Close_Tray_CDD_cC_COMP);

        Reload_SegaCD(NULL);

        if (CD_Present) SCD.Status_CDD = STOPPED;
        else SCD.Status_CDD = NOCD;
        CDD.Status = 0x0000;

        CDD.Minute = 0;
        CDD.Seconde = 0;
        CDD.Frame = 0;
        CDD.Ext = 0;
    }
    else
    {
        char new_iso[1024];

        memset(new_iso, 0, 1024);

        // Modif N. -- made the action cancellable
        //if(!IsMovieActive()) // TODO: enable this check since we don't support recording of disc switches
        {
            extern HWND HWnd;
            if (Change_File_L(new_iso, Rom_Dir, "Load SegaCD image file", "SegaCD image file\0*.bin;*.cue;*.iso;*.raw\0All files\0*.*\0\0", "", HWnd))
            {
                Reload_SegaCD(new_iso);

                CD_Present = 1;

                SCD.Status_CDD = STOPPED;
            }
        }

        CDD.Status = 0x0000;

        CDD.Minute = 0;
        CDD.Seconde = 0;
        CDD.Frame = 0;
        CDD.Ext = 0;
    }

    CDD_Complete = 1;

    return 0;
}

int Open_Tray_CDD_cD(void)
{
    CHECK_TRAY_OPEN

        SCD.Status_CDC &= ~1;			// Stop CDC read

    if (CD_Load_System == CDROM_)
    {
        Clear_Sound_Buffer();

        ASPI_Lock(0);
        ASPI_Star_Stop_Unit(OPEN_TRAY, 0, 0, ASPI_Open_Tray_CDD_cD_COMP);
        ASPI_Lock(1);
        return 0;
    }
    else
    {
        Unload_ISO();
        CD_Present = 0;

        SCD.Status_CDD = TRAY_OPEN;
        CDD.Status = 0x0E00;

        CDD.Minute = 0;
        CDD.Seconde = 0;
        CDD.Frame = 0;
        CDD.Ext = 0;
    }

    CDD_Complete = 1;

    return 0;
}

int CDD_cA(void)
{
    CHECK_TRAY_OPEN
        CHECK_CD_PRESENT

        SCD.Status_CDC &= ~1;

    SCD.Status_CDD = READY;
    CDD.Status = SCD.Status_CDD;

    CDD.Minute = 0;
    CDD.Seconde = INT_TO_BCDW(1);
    CDD.Frame = INT_TO_BCDW(1);
    CDD.Ext = 0;

    CDD_Complete = 1;

    return 0;
}

int CDD_Def(void)
{
    CDD.Status = SCD.Status_CDD;

    CDD.Minute = 0;
    CDD.Seconde = 0;
    CDD.Frame = 0;
    CDD.Ext = 0;

    return 0;
}

/***************************
 *   Others CD functions   *
 **************************/

extern int disableSound; // Gens.cpp

void Write_CD_Audio(short *Buf, int rate, int channel, int _length)
{
    unsigned int _length_src, _length_dst;
    unsigned int pos_src, pas_src;

    if (disableSound)
        return;

    if (rate == 0) return;
    if (Sound_Rate == 0) return;

    if (CD_Audio_Starting)
    {
        CD_Audio_Starting = 0;
        memset(CD_Audio_Buffer_L, 0, 4096 * 4);
        memset(CD_Audio_Buffer_R, 0, 4096 * 4);
        CD_Audio_Buffer_Write_Pos = (CD_Audio_Buffer_Read_Pos + 2000) & 0xFFF;
    }

    _length_src = rate / 75;				// 75th of a second
    _length_dst = Sound_Rate / 75;		// 75th of a second

    pas_src = (_length_src << 16) / _length_dst;
    pos_src = 0;

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "\n*********  Write Pos = %d    ", CD_Audio_Buffer_Write_Pos);
#endif

    if (channel == 2)
    {
        __asm
        {
            mov edi, CD_Audio_Buffer_Write_Pos
            mov ebx, Buf
            xor esi, esi
            mov ecx, _length_dst
            xor eax, eax
            mov edx, pas_src
            dec ecx
            jmp short loop_stereo

            align 16

            loop_stereo:
            movsx eax, word ptr[ebx + esi * 4]
                mov CD_Audio_Buffer_L[edi * 4], eax
                movsx eax, word ptr[ebx + esi * 4 + 2]
                mov CD_Audio_Buffer_R[edi * 4], eax
                mov esi, dword ptr pos_src
                inc edi
                add esi, edx
                and edi, 0xFFF
                mov dword ptr pos_src, esi
                shr esi, 16
                dec ecx
                jns short loop_stereo

                mov CD_Audio_Buffer_Write_Pos, edi
        }
    }
    else
    {
        __asm
        {
            mov edi, CD_Audio_Buffer_Write_Pos
            mov ebx, Buf
            xor esi, esi
            mov ecx, _length_dst
            xor eax, eax
            mov edx, pas_src
            dec ecx
            jmp short loop_mono

            align 16

            loop_mono:
            movsx eax, word ptr[ebx + esi * 2]
                mov CD_Audio_Buffer_L[edi * 4], eax
                mov CD_Audio_Buffer_R[edi * 4], eax
                mov esi, dword ptr pos_src
                inc edi
                add esi, edx
                and edi, 0xFFF
                mov dword ptr pos_src, esi
                shr esi, 16
                dec ecx
                jns short loop_mono

                mov CD_Audio_Buffer_Write_Pos, edi
        }
    }

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "Write Pos 2 = %d\n\n", CD_Audio_Buffer_Write_Pos);
#endif
}

void Update_CD_Audio(int **buf, int _length)
{
    int *Buf_L, *Buf_R;
    int diff;

    Buf_L = buf[0];
    Buf_R = buf[1];

    if (CDD.Control & 0x0100) return;
    if (!(SCD.Status_CDC & 1)) return;
    if (CD_Audio_Starting) return;

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "\n*********  Read Pos Normal = %d     ", CD_Audio_Buffer_Read_Pos);
#endif

    if (CD_Audio_Buffer_Write_Pos < CD_Audio_Buffer_Read_Pos)
    {
        diff = CD_Audio_Buffer_Write_Pos + (4096) - CD_Audio_Buffer_Read_Pos;
    }
    else
    {
        diff = CD_Audio_Buffer_Write_Pos - CD_Audio_Buffer_Read_Pos;
    }

    if (diff < 500) CD_Audio_Buffer_Read_Pos -= 2000;
    else if (diff > 3500) CD_Audio_Buffer_Read_Pos += 2000;

#ifdef DEBUG_CD
    else fprintf(debug_SCD_file, " pas de modifs   ");
#endif

    CD_Audio_Buffer_Read_Pos &= 0xFFF;

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "Read Pos = %d   ", CD_Audio_Buffer_Read_Pos);
#endif

    if (CDDA_Enable)
    {
        int i = 0;
        for (; i <= _length; i++)
        {
            int readPos = (CD_Audio_Buffer_Read_Pos + i) & 0xFFF;
            CD_Audio_Buffer_L[readPos] = (CD_Audio_Buffer_L[readPos] * CDDAVol) >> 8;
            CD_Audio_Buffer_R[readPos] = (CD_Audio_Buffer_R[readPos] * CDDAVol) >> 8;
        }
        __asm
        {
            mov ecx, _length
            mov esi, CD_Audio_Buffer_Read_Pos
            mov edi, Buf_L
            dec ecx

            loop_L :
            mov eax, CD_Audio_Buffer_L[esi * 4]
                add[edi], eax
                inc esi
                add edi, 4
                and esi, 0xFFF
                dec ecx
                jns short loop_L

                mov ecx, _length
                mov esi, CD_Audio_Buffer_Read_Pos
                mov edi, Buf_R
                dec ecx

                loop_R :
            mov eax, CD_Audio_Buffer_R[esi * 4]
                add[edi], eax
                inc esi
                add edi, 4
                and esi, 0xFFF
                dec ecx
                jns short loop_R

                mov CD_Audio_Buffer_Read_Pos, esi
        }
    }
    else
    {
        CD_Audio_Buffer_Read_Pos += _length;
        CD_Audio_Buffer_Read_Pos &= 0xFFF;
    }

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "Read Pos 2 = %d\n\n", CD_Audio_Buffer_Read_Pos);
#endif
}