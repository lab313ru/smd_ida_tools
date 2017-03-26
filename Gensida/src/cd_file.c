#include <stdio.h>
#include <windows.h>
#include "cd_sys.h"
#include "cd_file.h"
#include "lc89510.h"
#include "cdda_mp3.h"
#include "star_68k.h"
#include "rom.h"
#include "mem_s68k.h"
#include <string.h>

struct _file_track Tracks[100];

char cp_buf[2560];
char Track_Played;

// Modif N. -- added for cue file support
_scd_toc g_cuefile_TOC;
char g_cuefile_TOC_filenames[100][1024] = { { 0 } };
int g_cuefile_TOC_filetype[100] = { 0 };
int g_dontResetAudioCache = 0;
extern char preloaded_tracks[100], played_tracks_linear[105]; // added for synchronous MP3 code
void Delete_Preloaded_MP3s(void);

int FILE_Init(void)
{
    MP3_Init();
    Unload_ISO();

    return 0;
}

void FILE_End(void)
{
    Unload_ISO();
}

// Modif N. -- added helper function for manipulating MSF times
void AddToMSF(_msf* MSF, int lba, int m, int s, int f)
{
    // careful not to do the math directly in the MSF struct
    // because we need to be able to handle intermediate results that don't fit in an unsigned char
    m += (int)MSF->M + lba / (60 * 75);
    s += (int)MSF->S + (lba / 75) % 60;
    f += (int)MSF->F + lba % 75;

    // carry over the frames into the seconds
    while (f < 0) // negative carryover
    {
        f += 75;
        s--;
    }
    s += f / 75; // positive carryover
    f %= 75;

    // carry over the seconds into the minutes
    while (s < 0) // negative carryover
    {
        s += 60;
        m--;
    }
    m += s / 60; // positive carryover
    s %= 60;

    // output result
    MSF->M = m;
    MSF->S = s;
    MSF->F = f;
}

// Modif N. -- added helper function for making a filename in (or relative to) the same directory as some other filename we have
void MakeFilename(char* filenameWithPath, int bufferSize, const char* otherFileInDirectory, const char* desiredFilename)
{
    if (*desiredFilename && desiredFilename[1] == ':')
        filenameWithPath[0] = 0; // don't fool around if the desired filename is absolute
    else // truncate after last directory separator:
    {
        char* lastSlash;
        strncpy(filenameWithPath, otherFileInDirectory, bufferSize);
        filenameWithPath[bufferSize - 1] = '\0';
        lastSlash = strrchr(filenameWithPath, '\\');
        {
            char* lastSlash2 = strrchr(filenameWithPath, '/');
            if (lastSlash2 > lastSlash)
                lastSlash = lastSlash2;
        }
        if (lastSlash)
            lastSlash[1] = 0;
    }
    strncat(filenameWithPath, desiredFilename, bufferSize - strlen(filenameWithPath) - 1);
}

int Load_ISO(char *buf, char *iso_name)
{
    HANDLE File_Size;
    int i, num_track, Cur_LBA = 0, Max_LBA = 0, LBA_Deficit = 0; // Modif N.
    FILE *tmp_file;
    char tmp_name[1024], tmp_ext[10];
    char exts[20][16] = {
        "%02d.mp3", " %02d.mp3", "-%02d.mp3", "_%02d.mp3", " - %02d.mp3",
        "%d.mp3", " %d.mp3", "-%d.mp3", "_%d.mp3", " - %d.mp3",
        "%02d.wav", " %02d.wav", "-%02d.wav", "_%02d.wav", " - %02d.wav",
        "%d.wav", " %d.wav", "-%d.wav", "_%d.wav", " - %2d.wav" };

    Unload_ISO();

    if (Detect_Format(iso_name) == SEGACD_IMAGE + 1) Tracks[0].Type = TYPE_BIN;
    else if (Detect_Format(iso_name) == SEGACD_IMAGE) Tracks[0].Type = TYPE_ISO;
    else return -2;

    File_Size = CreateFile(iso_name, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    Tracks[0].Length = GetFileSize(File_Size, NULL);

    if (Tracks[0].Type == TYPE_ISO) Tracks[0].Length >>= 11;	// size in sectors
    else Tracks[0].Length /= 2352;								// size in sectors
    CloseHandle(File_Size);

    Tracks[0].F = fopen(iso_name, "rb");
    Tracks[0].F_decoded = NULL;

    if (Tracks[0].F == NULL)
    {
        Tracks[0].Type = 0;
        Tracks[0].Length = 0;
        return -1;
    }

    if (Tracks[0].Type == TYPE_ISO) fseek(Tracks[0].F, 0x100, SEEK_SET);
    else fseek(Tracks[0].F, 0x110, SEEK_SET);

    fread(buf, 1, 0x200, Tracks[0].F);
    fseek(Tracks[0].F, 0, SEEK_SET);

    SCD.TOC.First_Track = 1;

    SCD.TOC.Tracks[0].Num = 1;
    SCD.TOC.Tracks[0].Type = 1;				// DATA

    SCD.TOC.Tracks[0].MSF.M = 0;
    SCD.TOC.Tracks[0].MSF.S = 2;
    SCD.TOC.Tracks[0].MSF.F = 0;

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "\nTrack 0 - %02d:%02d:%02d ", SCD.TOC.Tracks[0].MSF.M, SCD.TOC.Tracks[0].MSF.S, SCD.TOC.Tracks[0].MSF.F);
    if (SCD.TOC.Tracks[0].Type) fprintf(debug_SCD_file, "DATA\n");
    else fprintf(debug_SCD_file, "AUDIO\n");
#endif

    if (!g_dontResetAudioCache)
    {
        // Modif N.
        //Cur_LBA = Tracks[0].Length;				// Size in sectors
        Max_LBA = Tracks[0].Length;
        Cur_LBA = 0;

        strcpy(tmp_name, iso_name);

        // Modif N. -- added for cue file loading support
        if (CD_Load_System == FILE_CUE)
        {
            memcpy(&SCD.TOC, &g_cuefile_TOC, sizeof(g_cuefile_TOC));

            for (i = 0; i < 100; i++)
            {
                if (*g_cuefile_TOC_filenames[i] && g_cuefile_TOC.Tracks[i].Type == 0) // audio track from some file
                {
                    Tracks[i].Type = g_cuefile_TOC_filetype[i];

                    switch (g_cuefile_TOC_filetype[i])
                    {
                    case TYPE_ISO:
                    case TYPE_BIN:
                    {
                        int j, Prev_LBA;
                        j = i ? i - 1 : j;
                        Cur_LBA = MSF_to_LBA(&SCD.TOC.Tracks[i].MSF);
                        Prev_LBA = MSF_to_LBA(&SCD.TOC.Tracks[j].MSF);
                        if (Tracks[j].Type == TYPE_ISO || Tracks[j].Type == TYPE_BIN)
                        {
                            if (Cur_LBA > Prev_LBA) // fill in length of previous audio track
                            {
                                Tracks[j].Length = Cur_LBA - Prev_LBA;
                            }
                            else // wrong order, deal with it
                            {
                                Tracks[j].Length = MSF_to_LBA(&g_cuefile_TOC.Tracks[i].MSF) - MSF_to_LBA(&g_cuefile_TOC.Tracks[j].MSF);
                                LBA_Deficit = Prev_LBA - Cur_LBA + Tracks[j].Length;
                            }
                        }
                        else // for MP3 we already know how long it should be, so correct our starting position instead
                        {
                            int Expected_LBA = Prev_LBA + Tracks[j].Length;
                            LBA_Deficit += Expected_LBA - Cur_LBA;
                        }
                        if (LBA_Deficit)
                        {
                            _scd_track* track = &SCD.TOC.Tracks[i];
                            AddToMSF(&track->MSF, LBA_Deficit, 0, 0, 0);
                            Cur_LBA += LBA_Deficit;
                        }

                        {
                            tmp_file = fopen(g_cuefile_TOC_filenames[i], "rb");
                            if (tmp_file)
                                Tracks[i].F = tmp_file;
                            else
                                Tracks[i].F = Tracks[0].F;
                            Tracks[i].F_decoded = NULL;

                            strncpy(Tracks[i].filename, g_cuefile_TOC_filenames[i], 512);
                            Tracks[i].filename[511] = 0;
                        }
                    }
                    break;
                    case TYPE_MP3:
                    case TYPE_WAV:
                    {
                        float fs;

                        tmp_file = fopen(g_cuefile_TOC_filenames[i], "rb");
                        if (tmp_file && g_cuefile_TOC_filetype[i] != TYPE_WAV) // wav is not supported yet
                        {
                            File_Size = CreateFile(g_cuefile_TOC_filenames[i], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            fs = (float)GetFileSize(File_Size, NULL);				// used to calculate length
                            CloseHandle(File_Size);

                            Tracks[i].F = tmp_file;

                            strncpy(Tracks[i].filename, g_cuefile_TOC_filenames[i], 512);
                            Tracks[i].filename[511] = 0;

                            LBA_to_MSF(Cur_LBA, &(SCD.TOC.Tracks[i].MSF));

                            // if the last track was binary, give it room by advancing to the start of the next binary track
                            if (!i || (g_cuefile_TOC_filetype[i - 1] == TYPE_ISO || g_cuefile_TOC_filetype[i - 1] == TYPE_BIN))
                            {
                                int j;
                                for (j = i + 1; g_cuefile_TOC_filetype[j] != TYPE_ISO && g_cuefile_TOC_filetype[j] != TYPE_BIN && j <= 99 && j <= SCD.TOC.Last_Track; j++);
                                if (j <= 99 && j <= SCD.TOC.Last_Track && *g_cuefile_TOC_filenames[j])
                                {
                                    SCD.TOC.Tracks[i].MSF = SCD.TOC.Tracks[j].MSF;
                                }
                                else
                                {
                                    LBA_to_MSF(Max_LBA, &SCD.TOC.Tracks[i].MSF);
                                    AddToMSF(&SCD.TOC.Tracks[i].MSF, LBA_Deficit, 0, 2, 0);
                                }
                                Cur_LBA = MSF_to_LBA(&SCD.TOC.Tracks[i].MSF);
                            }

                            //							if(g_cuefile_TOC_filetype[i] == TYPE_MP3)
                            {
                                // MP3 File
                                Tracks[i].Type = TYPE_MP3;
                                fs /= (float)(MP3_Get_Bitrate(Tracks[i].F) >> 3);
                                fs *= 75;
                                Tracks[i].Length = (int)fs - 294; // the mp3 decoder gets unstable at the very end of the song, so subtract a bit to stay on the safe side
                                Cur_LBA += Tracks[i].Length;
                            }
                            /*							else
                                                        {
                                                        // WAV File
                                                        Tracks[i].Type = TYPE_WAV;
                                                        Tracks[i].Length = 1000; // probably needs fixing if WAV is really supported...
                                                        Cur_LBA += Tracks[i].Length;
                                                        }*/
                        }
                        else
                        {
                            char errmsg[1280];
                            Tracks[i].Type = TYPE_ISO;
                            Tracks[i].Length = 0;
                            Tracks[i].F = NULL;
                            Tracks[i].F_decoded = NULL;
                            if (g_cuefile_TOC_filetype[i] == TYPE_WAV)
                            {
                                if (tmp_file)
                                {
                                    sprintf(errmsg, "Couldn't use audio file \"%s\" for track %02d because WAV files are not supported. Please use either BINARY audio tracks or MP3 files.", g_cuefile_TOC_filenames[i], SCD.TOC.Tracks[i].Num);
                                    fclose(tmp_file);
                                }
                                else
                                    sprintf(errmsg, "Couldn't find audio file \"%s\" for track %02d, and WAV files are not supported anyway. Please use either BINARY audio tracks or MP3 files.", g_cuefile_TOC_filenames[i], SCD.TOC.Tracks[i].Num);
                            }
                            else
                                sprintf(errmsg, "Couldn't find audio file \"%s\" for track %02d.", g_cuefile_TOC_filenames[i], SCD.TOC.Tracks[i].Num);
                            MessageBox(GetActiveWindow(), errmsg, "Cue File Warning", MB_OK | MB_ICONWARNING);
                        }
                    }
                    break;
                    }
                }
            }
            num_track = g_cuefile_TOC.Last_Track + 1;
            if (num_track < 2) num_track = 2;
        }
        else if (CD_Load_System == FILE_ISO) // old code, when there's no CUE file to figure out where to get the audio tracks from
        {
            // this checks for MP3s in a bunch of different possible places and registers tracks for them

            int isoNamelen = strlen(iso_name);

            Cur_LBA = Tracks[0].Length;				// Size in sectors

            for (num_track = 2, i = 0; i < 100; i++)
            {
                int jj;
                for (jj = 0; jj < 40; jj++)
                {
                    int j = jj % 20;
                    strcpy(tmp_name, iso_name);
                    if (jj < 20)
                    {
                        if (isoNamelen <= 4) break;
                        tmp_name[isoNamelen - 4] = 0;
                    }
                    else
                    {
                        if (isoNamelen <= 6) break;
                        tmp_name[isoNamelen - 6] = 0;
                    }
                    wsprintf(tmp_ext, exts[j], i);
                    strcat(tmp_name, tmp_ext);

                    tmp_file = fopen(tmp_name, "rb");

                    if (tmp_file)
                    {
                        float fs;

                        File_Size = CreateFile(tmp_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                        fs = (float)GetFileSize(File_Size, NULL);				// used to calculate length
                        CloseHandle(File_Size);

                        Tracks[num_track - SCD.TOC.First_Track].F = tmp_file;
                        Tracks[num_track - SCD.TOC.First_Track].F_decoded = NULL;

                        strncpy(Tracks[num_track - SCD.TOC.First_Track].filename, tmp_name, 512);
                        Tracks[num_track - SCD.TOC.First_Track].filename[511] = 0;

                        SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Num = num_track;
                        SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Type = 0;			// AUDIO

                        LBA_to_MSF(Cur_LBA, &(SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF));

#ifdef DEBUG_CD
                        fprintf(debug_SCD_file, "Track %i - %02d:%02d:%02d ", num_track - SCD.TOC.First_Track, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.M, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.S, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.F);
                        if (SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Type) fprintf(debug_SCD_file, "DATA\n");
                        else fprintf(debug_SCD_file, "AUDIO\n");
#endif

                        if (j < 10)
                        {
                            // MP3 File
                            Tracks[num_track - SCD.TOC.First_Track].Type = TYPE_MP3;
                            fs /= (float)(MP3_Get_Bitrate(Tracks[num_track - 1].F) >> 3);
                            fs *= 75;
                            Tracks[num_track - SCD.TOC.First_Track].Length = (int)fs;
                            Cur_LBA += Tracks[num_track - SCD.TOC.First_Track].Length;
                        }
                        else
                        {
                            // WAV File
                            Tracks[num_track - SCD.TOC.First_Track].Type = TYPE_WAV;
                            Tracks[num_track - SCD.TOC.First_Track].Length = 1000;
                            Cur_LBA += Tracks[num_track - SCD.TOC.First_Track].Length;
                        }

                        jj = 1000;
                        num_track++;
                    }
                }
            }

            /*
            //	Faking some audios tracks if no present

            if (num_track == 2)
            {
            for(; num_track < 95; num_track++)
            {
            SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Num = num_track;
            SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Type = 0;			// AUDIO

            LBA_to_MSF(Cur_LBA, &(SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF));

            Cur_LBA += 100;
            }
            }
            */

            SCD.TOC.Last_Track = num_track - 1;
        }

        SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Num = num_track;
        SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Type = 0;

        // Modif N.
        {
            _scd_track* track;
            if (!(Tracks[num_track - SCD.TOC.First_Track - 1].Type == TYPE_ISO || Tracks[num_track - SCD.TOC.First_Track - 1].Type == TYPE_BIN))
            { // if the last track was an MP3 track, move the max position after it, otherwise we'll use the ISO filesize + the current LBA deficit
                int Prev_LBA = MSF_to_LBA(&SCD.TOC.Tracks[num_track - SCD.TOC.First_Track - 1].MSF);
                int Expected_LBA = Prev_LBA + Tracks[num_track - SCD.TOC.First_Track - 1].Length;
                LBA_Deficit += Expected_LBA - Max_LBA;
            }
            track = &SCD.TOC.Tracks[num_track - SCD.TOC.First_Track];
            LBA_to_MSF(Max_LBA, &track->MSF);
            AddToMSF(&track->MSF, LBA_Deficit, 0, 2, 0);
            Tracks[g_cuefile_TOC.Last_Track - SCD.TOC.First_Track].Length = Max_LBA - Cur_LBA;
        }

#ifdef DEBUG_CD
        fprintf(debug_SCD_file, "End CD - %02d:%02d:%02d\n\n", SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.M, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.S, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.F);
#endif

        // Modif N. -- warn if the audio track is too long for the Sega CD to be able to address all of it, or fix it if possible to do so safely
        {
            _msf ninetyNine = { 99, 59, 74 };
            int ninetyNineLBA, i, firstMP3, maxLBA;
            ninetyNineLBA = MSF_to_LBA(&ninetyNine);
            maxLBA = MSF_to_LBA(&SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF);
            if (maxLBA > ninetyNineLBA)
            {
                for (i = 0; i < 100; i++)
                    if (Tracks[i].Type == TYPE_MP3 || Tracks[i].Type == TYPE_WAV)
                        break;
                firstMP3 = i;

                if (firstMP3 == 1)
                {
                    // TODO: figure out how to calculate the size of the data section
                    // when we don't know where the first audio track starts.
                    // since I don't know how, I won't risk guessing how far back we can move the first audio track
                }
                else if (firstMP3 > 1 && firstMP3 < 100)
                {
                    int lbadiff = maxLBA - ninetyNineLBA;
                    int lba2 = MSF_to_LBA(&SCD.TOC.Tracks[firstMP3].MSF);
                    int lba1 = MSF_to_LBA(&SCD.TOC.Tracks[firstMP3 - 1].MSF);
                    int baselbadiff = lba2 - lba1;
                    _msf twoMinutes = { 2, 0, 0 }; // only has to be nonzero, but I'll be courteous and give it 2 extra minutes
                    int twoMinutesLBA = MSF_to_LBA(&twoMinutes);
                    int correctionAmount;
                    if (lbadiff > baselbadiff - twoMinutesLBA)
                        correctionAmount = baselbadiff - twoMinutesLBA; // too much to safely correct for all of it
                    else
                        correctionAmount = lbadiff;
                    // move all the tracks back starting at the first MP3
                    for (i = firstMP3; i < num_track; i++)
                        AddToMSF(&SCD.TOC.Tracks[i].MSF, -correctionAmount, 0, 0, 0);
                }

                // warn if it's still too much
                maxLBA = MSF_to_LBA(&SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF);
                if (maxLBA > ninetyNineLBA)
                {
                    char errmsg[1024];
                    _msf* lastMSF = &SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF;
                    sprintf(errmsg, "Warning: The CD size including audio is %02d:%02d:%02d, which is longer than the maximum of %02d:%02d:%02d. You may encounter problems with audio tracks not playing correctly.", lastMSF->M, lastMSF->S, lastMSF->F, ninetyNine.M, ninetyNine.S, ninetyNine.F);
                    MessageBox(GetActiveWindow(), errmsg, "CD Loading Warning", MB_OK | MB_ICONWARNING);
                }
            }
        }
    }

    {
        void Preload_Used_MP3s(void);
        Preload_Used_MP3s();
    }

    return 0;
}

void Unload_ISO(void)
{
    int i;

    MP3_CancelAllPreloading();

    Track_Played = 99;

    for (i = 0; i < 100; i++)
        played_tracks_linear[i] = 0;

    if (!g_dontResetAudioCache)
    {
        for (i = 0; i < 100; i++)
        {
            if (Tracks[i].F) fclose(Tracks[i].F);
            if (Tracks[i].F_decoded)
                fclose(Tracks[i].F_decoded);
            Tracks[i].F = NULL;
            Tracks[i].F_decoded = NULL;
            Tracks[i].Length = 0;
            Tracks[i].Type = 0;
            Tracks[i].filename[0] = 0;
            if (preloaded_tracks[i] == 1) // intentionally does not clear if the value is 2
                preloaded_tracks[i] = 0;
        }

        Delete_Preloaded_MP3s();
    }
}

void Get_CUE_ISO_Filename(char *fnamebuf, int fnamebuf_size, char *cue_name)
{
    FILE* cueFile = fopen(cue_name, "r");
    if (cueFile)
    {
        char line[1024], temp[1024], filename[1024];
        char* ptr, *tempPtr;
        filename[0] = 0;
        for (;;)
        {
            fgets(line, 1024, cueFile);
            if (feof(cueFile) || ferror(cueFile))
                break;
            _strupr(line);

            // ignore comment lines
            if (!strncmp(line, "REM", 3) || line[0] == '#' || !strncmp(line, "//", 2))
                continue;

            // parse filename from the current line string
            temp[0] = 0;
            tempPtr = temp;
            ptr = line;
            while (*ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;
            if (!strncmp(ptr, "FILE", 4))
            {
                int inQuotes = 0;
                char afterFile = ptr[4];
                ptr += 4;
                while (*ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;
                if (afterFile == ' ' || afterFile == '\t' || afterFile == '\"')
                    while (*ptr && *ptr != '\n' && *ptr != '\r')
                    {
                        if ((*ptr == ' ' || *ptr == '\t') && !inQuotes)
                            break;

                        *tempPtr++ = *ptr;

                        if (*ptr == '\"')
                            if (inQuotes)
                                break;
                            else
                                inQuotes = 1;

                        ptr++;
                    }
                *tempPtr = 0;
            }

            if (temp[0]) // if a filename was found
            {
                if (strstr(line, "BINARY") || strstr(line, "ISO"))
                {
                    char* ptr;
                    for (ptr = temp; *ptr; ptr++)
                        if (*ptr == '\"')
                            *ptr = 0;
                    ptr = temp; if (!*ptr) ptr++;
                    strcpy(filename, ptr);
                    break;
                }
            }
        }

        fclose(cueFile);

        //fnamebuf[0] = 0;
        //if(fnamebuf_size) fnamebuf[fnamebuf_size-1] = 0;
        //strncpy(fnamebuf, filename, fnamebuf_size);

        MakeFilename(fnamebuf, fnamebuf_size, cue_name, filename);
        {
            FILE* isoFile = NULL;
            size_t fnamebuf_len = strlen(fnamebuf);
            if (!(fnamebuf_len > 3 && !_stricmp("CUE", &fnamebuf[fnamebuf_len - 3])))
                isoFile = fopen(fnamebuf, "rb");
            if (isoFile)
            {
                fclose(isoFile);
            }
            else
            {
                char* dot;
                strncpy(fnamebuf, cue_name, fnamebuf_size);
                fnamebuf[fnamebuf_size - 1] = 0;
                dot = strrchr(fnamebuf, '.');
                if (dot) *dot = 0;
                strncat(fnamebuf, ".iso", fnamebuf_size - strlen(fnamebuf) - 1);
                isoFile = fopen(fnamebuf, "rb");
                if (isoFile)
                {
                    fclose(isoFile);
                }
                else
                {
                    MakeFilename(fnamebuf, fnamebuf_size, cue_name, filename);
                }
            }
        }
    }
}

// Modif N. -- added Load_CUE function
// This is mainly to support loading CUE files that point to audio data on the CD,
// but they can also point to specific MP3 files on disk.

// Example of a CUE file:
/*
REM lines beginning with REM are comments
REM the following line says what file the game data is in
FILE "Some Sega CD Game.iso" BINARY
REM the first track has the non-audio data
TRACK 01 MODE1/2352
INDEX 01 00:00:00
REM all other tracks should be AUDIO
TRACK 02 AUDIO
REM the INDEX 01 says when the song starts (mm:ss:ff), and the INDEX 01 of the next BINARY track determines when it ends
INDEX 01 10:00:00
TRACK 03 AUDIO
INDEX 01 12:00:00
REM this switches to an MP3 song for track 04
REM caveat: the track before it will get longer, unless maybe you put the MP3 in a new track at the end of the CUE file and swap the track numbers
FILE "My_custom_song.mp3" MP3
TRACK 04 AUDIO
INDEX 01 00:00:00
REM switch back to CD audio for the rest of the tracks
FILE "Some Sega CD Game.iso" BINARY
TRACK 05 AUDIO
INDEX 01 16:00:00
TRACK 06 AUDIO
INDEX 01 18:00:00
TRACK 07 AUDIO
INDEX 01 20:59:74
TRACK 08 AUDIO
INDEX 01 23:00:00
*/

int Load_CUE(char *buf, char *cue_name)
{
    FILE* cueFile = fopen(cue_name, "r");
    char line[1024], temp[1024], filename[1024];
    char* ptr, *tempPtr;
    int trackIndex = -1, filetype = TYPE_ISO;

    memset(&g_cuefile_TOC, 0, sizeof(g_cuefile_TOC));
    filename[0] = 0;
    g_cuefile_TOC.First_Track = 255;
    g_cuefile_TOC.Last_Track = 0;

    for (;;)
    {
        fgets(line, 1024, cueFile);
        if (feof(cueFile) || ferror(cueFile))
            break;
        _strupr(line);

        // ignore comment lines
        if (!strncmp(line, "REM", 3) || line[0] == '#' || !strncmp(line, "//", 2))
            continue;

        // parse filename from the current line string
        temp[0] = 0;
        tempPtr = temp;
        ptr = line;
        while (*ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;
        if (!strncmp(ptr, "FILE", 4))
        {
            int inQuotes = 0;
            char afterFile = ptr[4];
            ptr += 4;
            while (*ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;
            if (afterFile == ' ' || afterFile == '\t' || afterFile == '\"')
                while (*ptr && *ptr != '\n' && *ptr != '\r')
                {
                    if ((*ptr == ' ' || *ptr == '\t') && !inQuotes)
                        break;

                    *tempPtr++ = *ptr;

                    if (*ptr == '\"')
                        if (inQuotes)
                            break;
                        else
                            inQuotes = 1;

                    ptr++;
                }
            *tempPtr = 0;
        }

        if (temp[0]) // if a filename was found
        {
            char* ptr;
            for (ptr = temp; *ptr; ptr++)
                if (*ptr == '\"')
                    *ptr = 0;
            ptr = temp; if (!*ptr) ptr++;
            strcpy(filename, ptr);
            if (strstr(line, "BINARY") || strstr(line, "ISO"))
                filetype = TYPE_ISO;
            else if (strstr(line, "MP3"))
                filetype = TYPE_MP3;
            else if (strstr(line, "WAV"))
                filetype = TYPE_WAV;
            continue;
        }

        {
            int trackNum = -1;
            if (1 == sscanf(line, " TRACK %d", &trackNum) && trackIndex < 99)
            {
                _scd_track* track = &g_cuefile_TOC.Tracks[++trackIndex];
                track->Num = trackNum;
                track->Type = strstr(line, "AUDIO") ? 0 : 1;
                if (g_cuefile_TOC.First_Track > trackNum) g_cuefile_TOC.First_Track = trackNum;
                if (g_cuefile_TOC.Last_Track < trackNum) g_cuefile_TOC.Last_Track = trackNum;
                MakeFilename(g_cuefile_TOC_filenames[trackIndex], 1024, cue_name, filename);
                g_cuefile_TOC_filetype[trackIndex] = filetype;
                continue;
            }
        }

        {
            int m, s, f;
            if (3 == sscanf(line, " INDEX 01 %d:%d:%d", &m, &s, &f) && trackIndex >= 0)
            {
                _scd_track* track = &g_cuefile_TOC.Tracks[trackIndex];
                track->MSF.M = m;
                track->MSF.S = s;
                track->MSF.F = f;
                AddToMSF(&track->MSF, 0, 0, (track->Type ? 2 : 4), 0); // not sure why I have to do this
                continue;
            }
        }
    }

    fclose(cueFile);

    if (g_cuefile_TOC.First_Track > g_cuefile_TOC.Last_Track)
        g_cuefile_TOC.First_Track = g_cuefile_TOC.Last_Track;

    Get_CUE_ISO_Filename(filename, 1024, cue_name);

    if (filename)
    {
        int retval;

        retval = Load_ISO(buf, filename);
        if (retval == -1)
        {
            MakeFilename(filename, 1024, cue_name, filename);
        }
        return retval;
    }

    return -1;
}

int FILE_Read_One_LBA_CDC(void)
{
    int where_read;

    //struct cdcstruct cdc = CDC; // enable this for stupid debuggers that don't realize CDC is a variable

    if (CDD.Control & 0x0100)					// DATA
    {
        if (Tracks[0].F == NULL) return -1;

        if (SCD.Cur_LBA < 0) where_read = 0;
        else if (SCD.Cur_LBA >= Tracks[0].Length) where_read = Tracks[0].Length - 1;
        else where_read = SCD.Cur_LBA;

        if (Tracks[0].Type == TYPE_ISO) where_read <<= 11;
        else where_read = (where_read * 2352 + 16);

        // TODO: this memset needs to happen instead of the fread when playing track 01 from the BIOS
        //       but I don't know what the condition is supposed to be (maybe !(CDC.CTRL.B.B1 & 0x20))
        //memset(cp_buf, 0, 2048);

        fseek(Tracks[0].F, where_read, SEEK_SET);
        fread(cp_buf, 1, 2048, Tracks[0].F);

#ifdef DEBUG_CD
        fprintf(debug_SCD_file, "\n\nRead file CDC 1 data sector :\n");
#endif
    }
    else										// AUDIO
    {
        int rate, channel, index;
        extern int fatal_mp3_error;
        index = SCD.Cur_Track - SCD.TOC.First_Track;

        if (Tracks[index].Type == TYPE_MP3)
        {
#ifdef _WIN32
            int forceNoDecode = 1;
#else
            int forceNoDecode = 0;
#endif

            int fileNeedsClose = 0;
            FILE* decodedFile = GetMP3TrackFile(index, &fileNeedsClose, &where_read);

            if (decodedFile || forceNoDecode)
            {
                // copy audio data to buffer
                int outRead = 0;
                if (decodedFile)
                {
                    fseek(decodedFile, where_read, SEEK_SET);
                    outRead = fread(cp_buf, 1, 588 * 4, decodedFile);
                }
                if (outRead < 588 * 4)
                    memset(cp_buf, 0, 588 * 4); // failed to read, use silence

                Write_CD_Audio((short *)cp_buf, 44100, 2, 588);

                if (fileNeedsClose)
                    fclose(decodedFile);
            }
            else // stream it
            {
                if (fatal_mp3_error) // Modif N. -- added check to prevent highly unpleasant noises if the MP3 decoder explodes (e.g. from a corrupted MP3 file)
                {
                    memset(cp_buf, 0, 588 * 4);
                    rate = 44100;
                    channel = 2;
                }
                else
                    MP3_Update(cp_buf, &rate, &channel, 0);
                Write_CD_Audio((short *)cp_buf, rate, channel, 588);
            }
        }
        else if (Tracks[index].Type == TYPE_ISO) // Modif N. -- added case to allow using music from the ISO file (cue file support)
        {
            if (Tracks[index].F)
            {
                // convert virtual LBA (Cur_LBA) to an actual read offset
                int curTrack = LBA_to_Track(SCD.Cur_LBA);
                int lbaOffset = SCD.Cur_LBA - Track_to_LBA(curTrack);
                int lbaBase = SCD.Cur_LBA;
                int lba, where_read, i;
                for (i = 0; i < 100; i++)
                {
                    int j = i ? i : curTrack - 1; // first we check where it normally is, so that we usually leave this loop on the first iteration... if that fails, then loop through the other possible tracks
                    if (g_cuefile_TOC.Tracks[j].Num == curTrack)
                    {
                        lbaBase = MSF_to_LBA(&g_cuefile_TOC.Tracks[j].MSF);
                        break;
                    }
                }
                lba = lbaBase + lbaOffset;
                where_read = (lba - 150) * 588 * 4 + 16;
                if (where_read < 0) where_read = 0;

                // copy audio data to buffer
                fseek(Tracks[index].F, where_read, SEEK_SET);
                fread(cp_buf, 1, 588 * 4, Tracks[index].F);
                Write_CD_Audio((short *)cp_buf, 44100, 2, 588);
            }
        }

#ifdef DEBUG_CD
        fprintf(debug_SCD_file, "\n\nRead file CDC 1 audio sector :\n");
#endif
    }

    // Update CDC stuff

    CDC_Update_Header();

    if (CDD.Control & 0x0100)			// DATA track
    {
        if (CDC.CTRL.B.B0 & 0x80)		// DECEN = decoding enable
        {
            if (CDC.CTRL.B.B0 & 0x04)	// WRRQ : this bit enable write to buffer
            {
                // CAUTION : lookahead bit not implemented

                SCD.Cur_LBA++;

                CDC.WA.N = (CDC.WA.N + 2352) & 0x7FFF;		// add one sector to WA
                CDC.PT.N = (CDC.PT.N + 2352) & 0x7FFF;

                memcpy(&CDC.Buffer[CDC.PT.N + 4], cp_buf, 2048);
                memcpy(&CDC.Buffer[CDC.PT.N], &CDC.HEAD, 4);

#ifdef DEBUG_CD
                fprintf(debug_SCD_file, "\nRead -> WA = %d  Buffer[%d] =\n", CDC.WA.N, CDC.PT.N & 0x3FFF);
                fprintf(debug_SCD_file, "Header 1 = %.2X %.2X %.2X %.2X\n", CDC.HEAD.B.B0, CDC.HEAD.B.B1, CDC.HEAD.B.B2, CDC.HEAD.B.B3);
                //				fwrite(Buf_Read, 1, 2048, debug_SCD_file);
                //				fprintf(debug_SCD_file, "\nCDC buffer =\n");
                //				fwrite(&CDC.Buffer[CDC.PT.N], 1, 2052, debug_SCD_file);
                fprintf(debug_SCD_file, "Header 2 = %.2X %.2X %.2X %.2X --- %.2X %.2X\n\n", CDC.Buffer[(CDC.PT.N + 0) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 1) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 2) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 3) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 4) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 5) & 0x3FFF]);
#endif
            }

            CDC.STAT.B.B0 = 0x80;

            if (CDC.CTRL.B.B0 & 0x10)		// determine form bit form sub header ?
            {
                CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x08;
            }
            else
            {
                CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x0C;
            }

            if (CDC.CTRL.B.B0 & 0x02)
                CDC.STAT.B.B3 = 0x20;	// ECC done
            else
                CDC.STAT.B.B3 = 0x00;	// ECC not done

            if (CDC.IFCTRL & 0x20)
            {
                if (Int_Mask_S68K & 0x20) sub68k_interrupt(5, -1);

#ifdef DEBUG_CD
                fprintf(debug_SCD_file, "CDC - DEC interrupt\n");
#endif

                CDC.IFSTAT &= ~0x20;		// DEC interrupt happen
                CDC_Decode_Reg_Read = 0;	// Reset read after DEC int
            }
        }
    }
    else				// AUDIO track
    {
        SCD.Cur_LBA++;		// Always increment sector if audio

        CDC.WA.N = (CDC.WA.N + 2352) & 0x7FFF;		// add one sector to WA
        CDC.PT.N = (CDC.PT.N + 2352) & 0x7FFF;

        if (CDC.CTRL.B.B0 & 0x80)		// DECEN = decoding enable
        {
            if (CDC.CTRL.B.B0 & 0x04)	// WRRQ : this bit enable write to buffer
            {
                // CAUTION : lookahead bit not implemented

                memcpy(&CDC.Buffer[CDC.PT.N], cp_buf, 2352);
            }

            CDC.STAT.B.B0 = 0x80;

            if (CDC.CTRL.B.B0 & 0x10)		// determine form bit form sub header ?
            {
                CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x08;
            }
            else
            {
                CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x0C;
            }

            if (CDC.CTRL.B.B0 & 0x02)
                CDC.STAT.B.B3 = 0x20;	// ECC done
            else
                CDC.STAT.B.B3 = 0x00;	// ECC not done

            if (CDC.IFCTRL & 0x20)
            {
                if (Int_Mask_S68K & 0x20) sub68k_interrupt(5, -1);

#ifdef DEBUG_CD
                fprintf(debug_SCD_file, "CDC - DEC interrupt\n");
#endif

                CDC.IFSTAT &= ~0x20;		// DEC interrupt happen
                CDC_Decode_Reg_Read = 0;	// Reset read after DEC int
            }
        }
    }

    return 0;
}

int FILE_Play_CD_LBA(int async)
{
    int Track_LBA_Pos;

#ifdef DEBUG_CD
    fprintf(debug_SCD_file, "Play FILE Comp\n");
#endif

    if (Tracks[SCD.Cur_Track - SCD.TOC.First_Track].F == NULL)
    {
        return 1;
    }

    Track_LBA_Pos = SCD.Cur_LBA - Track_to_LBA(SCD.Cur_Track);
    if (Track_LBA_Pos < 0) Track_LBA_Pos = 0;

    if (Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type == TYPE_MP3)
    {
        return MP3_Play(SCD.Cur_Track - SCD.TOC.First_Track, Track_LBA_Pos, async);
    }
    else if (Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type == TYPE_WAV)
    {
        return 2;
    }
    else
    {
        return 3;
    }

    return 0;
}