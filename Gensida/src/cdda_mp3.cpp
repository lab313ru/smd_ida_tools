#include <windows.h>
#include <direct.h>

extern "C" {
#include "mp3_dec\mpg123.h"		// include <stdio.h>
#include "mp3_dec\mpglib.h"
#include "cd_file.h"
#include "cd_aspi.h"
#include "cdda_mp3.h"

    struct mpstr mp;

    extern int Sound_Rate;		// in G_dsound.h...

    unsigned int Current_IN_Pos;
    unsigned int Current_OUT_Pos;
    unsigned int Current_OUT_Size;

    char buf_out[8 * 1024];

    int freqs_mp3[9] = { 44100, 48000, 32000,
        22050, 24000, 16000,
        11025, 12000, 8000 };

    // Modif N. -- added variable. Set when MP3 decoding fails, reset when attempting to play a new MP3.
    // The point of this is to turn what was previously a crash
    // into an error that only stops the MP3 playback part of the program for the particular MP3 that couldn't play
    int fatal_mp3_error = 0;

    // Modif N. -- added
    int allowContinueToNextTrack = 1;
    char preloaded_tracks[100] = { 0 };
    extern BOOL IsAsyncAllowed(void);
    extern void Put_Info(char *Message, int Duration);
    extern void Put_Info_NonImmediate(char *Message, int Duration);
    extern char Gens_Path[1024];
    extern char played_tracks_linear[105];
#define PRELOADED_MP3_DIRECTORY "temp"
#define PRELOADED_MP3_FILENAME "\\track%02d.pcm"

    int MP3_Init(void)
    {
        InitMP3(&mp);

        MP3_Reset();

        return 0;
    }

    void MP3_Reset(void)
    {
        Current_IN_Pos = 0;
        Current_OUT_Pos = 0;
        Current_OUT_Size = 0;

        memset(buf_out, 0, 8 * 1024);
    }

    int MP3_Get_Bitrate(FILE *f)
    {
        unsigned int header, br;
        struct frame fr;

        fseek(f, 0, SEEK_SET);

        br = fread(&header, 1, 4, f);
        fseek(f, -3, SEEK_CUR);

        while (br == 4)
        {
            if ((header & 0x0000E0FF) == 0x0000E0FF)
            {
                return (1000 * decode_header_bitrate(&fr, header));
            }

            br = fread(&header, 1, 4, f);
            fseek(f, -3, SEEK_CUR);
        }

        return -1;
    }

    int MP3_Length_LBA(FILE *f)
    {
        float len;
        unsigned int header, br;
        struct frame fr;

        fseek(f, 0, SEEK_SET);

        len = 0;

        br = fread(&header, 1, 4, f);
        fseek(f, -3, SEEK_CUR);

        while (br == 4)
        {
            if ((header & 0x0000E0FF) == 0x0000E0FF)
            {
                float decode_header_res = decode_header_gens(&fr, header);
                if (!decode_header_res)
                {
                    //fatal_mp3_error = 1;
                    break;
                }
                len += decode_header_res;

#ifdef DEBUG_CD
                fprintf(debug_SCD_file, "mp3 length update = %f\n", len);
#endif

                fseek(f, fr.framesize, SEEK_CUR);
            }

            br = fread(&header, 1, 4, f);
            fseek(f, -3, SEEK_CUR);
        }

        len *= (float) 0.075;

        return (int)len;
    }

    int MP3_Find_Frame(FILE *f, int pos_wanted)
    {
        unsigned int header;
        float cur_pos;
        int br, prev_pos;
        struct frame fr;

        fseek(f, 0, SEEK_SET);

        //fprintf(debug_SCD_file, "		****** pos wanted = %d\n", pos_wanted);

        cur_pos = (float)0;
        prev_pos = 0;

        br = fread(&header, 1, 4, f);
        fseek(f, -3, SEEK_CUR);

        while ((int)cur_pos <= pos_wanted)
        {
            if (br < 4) break;

            if ((header & 0x0000E0FF) == 0x0000E0FF)
            {
                float decode_header_res;

                prev_pos = ftell(f) - 1;

                decode_header_res = decode_header_gens(&fr, header);
                if (!decode_header_res)
                {
                    fatal_mp3_error = 1;
                    break;
                }
                cur_pos += (float)decode_header_res * (float) 0.075;

                //fprintf(debug_SCD_file, "		hearder find at= %d	= %.8X\n", ftell(f) - 1, header);
                //fprintf(debug_SCD_file, "		current time = %g\n", cur_pos);

                if (fr.framesize < 0) fr.framesize = 0;

                fseek(f, fr.framesize + 3, SEEK_CUR);
            }

            br = fread(&header, 1, 4, f);

            //fprintf(debug_SCD_file, "		next header at= %d	= %.8X\n", ftell(f) - 4, header);

            fseek(f, -3, SEEK_CUR);
        }

        //fprintf(debug_SCD_file, "		pos returned = %d\n", prev_pos);

        return prev_pos;
    }

    int MP3_Update_IN(void)
    {
        char buf_in[8 * 1024];
        int size_read;

        if (Tracks[Track_Played].F == NULL) return -1;

        fseek(Tracks[Track_Played].F, Current_IN_Pos, SEEK_SET);
        size_read = fread(buf_in, 1, 8 * 1024, Tracks[Track_Played].F);
        Current_IN_Pos += size_read;

        if (size_read <= 0 || fatal_mp3_error)
        {
            int rv;
            if (!allowContinueToNextTrack)
                return 6;

            // go to the next track

            SCD.Cur_Track = ++Track_Played + 1; // because Cur_Track may or may not have already been incremented, and it's 1-based whereas Track_Played is 0-based
            played_tracks_linear[SCD.Cur_Track - SCD.TOC.First_Track] = 1;

            rv = FILE_Play_CD_LBA(1);
            if (rv)
                return rv;

            ResetMP3_Gens(&mp);

            if (Tracks[Track_Played].Type == TYPE_WAV)
            {
                // WAV_Play();
                return 4;
            }
            else if (Tracks[Track_Played].Type != TYPE_MP3)
            {
                return 5;
            }

            Current_IN_Pos = MP3_Find_Frame(Tracks[Track_Played].F, 0);
            fseek(Tracks[Track_Played].F, Current_IN_Pos, SEEK_SET);
            size_read = fread(buf_in, 1, 8 * 1024, Tracks[Track_Played].F);
            Current_IN_Pos += size_read;
        }

        if (fatal_mp3_error)
            return 1;

        if (decodeMP3(&mp, buf_in, size_read, buf_out, 8 * 1024, (int*)&Current_OUT_Size) != MP3_OK)
        {
            fseek(Tracks[Track_Played].F, Current_IN_Pos, SEEK_SET);
            size_read = fread(buf_in, 1, 8 * 1024, Tracks[Track_Played].F);
            Current_IN_Pos += size_read;

            if (decodeMP3(&mp, buf_in, size_read, buf_out, 8 * 1024, (int*)&Current_OUT_Size) != MP3_OK)
            {
                fatal_mp3_error = 1;
                return 1;
            }
        }

        return 0;
    }

    int MP3_Update_OUT(void)
    {
        Current_OUT_Pos = 0;

        if (fatal_mp3_error)
            return 1;

        if (decodeMP3(&mp, NULL, 0, buf_out, 8 * 1024, (int*)&Current_OUT_Size) != MP3_OK)
            return MP3_Update_IN();

        return 0;
    }

    void Delete_Preloaded_MP3(int track)
    {
        char str[128];
        SetCurrentDirectory(Gens_Path);
        _mkdir(PRELOADED_MP3_DIRECTORY);
        sprintf(str, PRELOADED_MP3_DIRECTORY PRELOADED_MP3_FILENAME, track + 1);
        _unlink(str);
    }

    void Delete_Preloaded_MP3s(void)
    {
        int i;
        for (i = 0; i < 100; i++)
            Delete_Preloaded_MP3(i);

        _rmdir(PRELOADED_MP3_DIRECTORY); // this will only delete the directory if it's empty, which is good. but it won't delete the directory if Windows Explorer has acquired and leaked the directory handle as it loves to do, which is bad. oh well.
    }

    int noTracksQueued = 1;

    bool Preload_MP3_Synchronous_Cancel;
    int Preload_MP3_Synchronous_Cancel_Exception = -1;
    bool Waiting_For_Preload_MP3_Synchronous = false;
    void Preload_MP3_Synchronous(FILE** filePtr, int track)
    {
        if (filePtr && track >= 0 && track < 100)
        {
            if (!*filePtr && Tracks[track].Type == TYPE_MP3)
            {
                char str[1024], msg[256];
                int prevTrack = Track_Played;
                FILE* file = NULL;

                SetCurrentDirectory(Gens_Path);
                _mkdir(PRELOADED_MP3_DIRECTORY);

                sprintf(str, PRELOADED_MP3_DIRECTORY PRELOADED_MP3_FILENAME, track + 1);

#ifdef _WIN32
                sprintf(msg, "Loading track %02d MP3", track + 1);
                Put_Info_NonImmediate(msg, 100);
#else
                sprintf(msg, "Preloading track %02d MP3", track + 1);
                Put_Info(msg, 100);
#endif

                if (Preload_MP3_Synchronous_Cancel)
                    return;

                //Tracks[track].F_decoded = fopen(str, "wb+");
                if (preloaded_tracks[track] == 3)
                    file = fopen(str, "r+b");
                else
                    file = fopen(str, "w+b");

                if (Preload_MP3_Synchronous_Cancel && track != Preload_MP3_Synchronous_Cancel_Exception)
                {
                    if (file)
                        fclose(file);
                    return;
                }

                preloaded_tracks[track] = 3;

                if (file)
                {
                    // decode mp3 to the temporary file

                    static const int inSize = 588 * 4, outSize = 8192;
                    char temp_in_buf[inSize], temp_out_buf[outSize];
                    int inRead = 0, outRead = 0;
                    int ok = MP3_OK;
                    mpstr temp_mp;
                    InitMP3(&temp_mp);
                    int inStartPos = MP3_Find_Frame(Tracks[track].F, 0);
                    fseek(file, 0, SEEK_END);
                    int outStartPos = ftell(file);
                    int outPos = 0;
                    fseek(Tracks[track].F, inStartPos, SEEK_SET);
                    int iter = 0;

                    while (ok == MP3_OK && !(Preload_MP3_Synchronous_Cancel && track != Preload_MP3_Synchronous_Cancel_Exception))
                    {
                        inRead = fread(temp_in_buf, 1, inSize, Tracks[track].F);
                        ok = decodeMP3(&temp_mp, temp_in_buf, inRead, temp_out_buf, outSize, (int*)&outRead);
                        if (outPos >= outStartPos)
                            fwrite(temp_out_buf, outRead, 1, file);
                        outPos += outRead;

                        ++iter;

#ifdef _WIN32
                        // even with "lowest priority" set on this thread, on win32,
                        // it still prevents other threads from doing processing for long enough
                        // to cause stuttering problems, even with multiple CPU cores present,
                        // so voluntarily give up control of the thread by sleeping every few decoding iterations
                        if (!Waiting_For_Preload_MP3_Synchronous)
                            if (iter % 16 == 0)
                                Sleep(10);
                            else
                                Sleep(0);
#endif
                    }

                    if ((Preload_MP3_Synchronous_Cancel && track != Preload_MP3_Synchronous_Cancel_Exception)
                        || ferror(file)) // if we got some error (out of disk space?) give up on pre-loading the MP3
                    {
                        fclose(file);
                        file = NULL;
                    }
                }
                else
                {
                    // couldn't open, maybe another instance of Gens has it open? if so use that
                    file = fopen(str, "rb");
                }

                *filePtr = file;
            }
        }
        if (!(Preload_MP3_Synchronous_Cancel && track != Preload_MP3_Synchronous_Cancel_Exception))
            preloaded_tracks[track] = (filePtr && *filePtr) ? 1 : 0;
    }

#ifdef _WIN32
}
#include <vector>
#include <algorithm>
extern "C" {
    class CriticalSection
    {
        CRITICAL_SECTION m_cs;
    public:
        CriticalSection() {
            ::InitializeCriticalSection(&m_cs);
        }
        ~CriticalSection() {
            ::DeleteCriticalSection(&m_cs);
        }
        void Lock() {
            ::EnterCriticalSection(&m_cs);
        }
        void Unlock() {
            ::LeaveCriticalSection(&m_cs);
        }
    };

    class AutoCriticalSection
    {
        CriticalSection* m_pCS;
    public:
        AutoCriticalSection(CriticalSection& pCS) : m_pCS(&pCS) {
            m_pCS->Lock();
        }
        ~AutoCriticalSection() {
            m_pCS->Unlock();
        }
    };

    CriticalSection preloadingCriticalSection;
#define ENTER_CRIT_SECT do{ AutoCriticalSection acs (preloadingCriticalSection);
#define EXIT_CRIT_SECT } while(0);

    struct PreloadMP3ThreadArg
    {
        FILE** filePtr;
        int track;
        int sortPriority;

        bool operator < (const PreloadMP3ThreadArg& other) {
            return other.sortPriority < sortPriority;
        }
    };
    std::vector<PreloadMP3ThreadArg> preloadMP3ThreadArgs;
    PreloadMP3ThreadArg curThreadArgs;

    static HANDLE s_preloadingMP3Thread = NULL;
    DWORD Preload_MP3_Thread(LPVOID lpThreadParameter)
    {
        for (;;)
        {
            ENTER_CRIT_SECT
                if (preloadMP3ThreadArgs.empty())
                {
                    s_preloadingMP3Thread = NULL;
                    noTracksQueued = 1;
                    return 0;
                }
            curThreadArgs = preloadMP3ThreadArgs.back();
            preloadMP3ThreadArgs.pop_back();
            EXIT_CRIT_SECT

                Preload_MP3_Synchronous(curThreadArgs.filePtr, curThreadArgs.track);

            ENTER_CRIT_SECT
                if (Preload_MP3_Synchronous_Cancel)
                {
                    if (!preloadMP3ThreadArgs.empty()
                        && preloadMP3ThreadArgs.back().track != curThreadArgs.track
                        && Preload_MP3_Synchronous_Cancel_Exception >= 0)
                        preloadMP3ThreadArgs.insert(preloadMP3ThreadArgs.end() - 1, curThreadArgs);
                    Preload_MP3_Synchronous_Cancel = false;
                }
            curThreadArgs.filePtr = NULL;
            curThreadArgs.track = -1;
            EXIT_CRIT_SECT
        }
    }

    void Preload_MP3(FILE** filePtr, int track)
    {
        if (!filePtr || *filePtr || Tracks[track].Type != TYPE_MP3)
            return;

        ENTER_CRIT_SECT
            if (!noTracksQueued)
            {
                if (track == curThreadArgs.track)
                    return;
                Preload_MP3_Synchronous_Cancel_Exception = track;
                Preload_MP3_Synchronous_Cancel = true; // start loading this one immediately
            }
        if (preloadMP3ThreadArgs.empty() || track != preloadMP3ThreadArgs.back().track)
        {
            PreloadMP3ThreadArg args = { filePtr, track };
            preloadMP3ThreadArgs.push_back(args);
        }
        noTracksQueued = 0;
        EXIT_CRIT_SECT

            if (!s_preloadingMP3Thread)
            {
                s_preloadingMP3Thread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Preload_MP3_Thread, (LPVOID)NULL, CREATE_SUSPENDED, NULL);
                ::SetThreadPriority(s_preloadingMP3Thread, THREAD_PRIORITY_LOWEST);
                ::ResumeThread(s_preloadingMP3Thread);
            }
    }

    void Preload_Used_MP3s(void)
    {
        int track;
        for (track = 0; track < 100; track++)
            if (preloaded_tracks[track] > 1)
                Preload_MP3(&Tracks[track].F_decoded, track);

        ENTER_CRIT_SECT

            // sort so that the smallest MP3 files load first

            for (unsigned int i = 0; i < preloadMP3ThreadArgs.size(); i++)
            {
                const char* filename = Tracks[preloadMP3ThreadArgs[i].track].filename;
                FILE* file;
                if (filename && filename[0] && (file = fopen(filename, "rb")))
                {
                    fseek(file, 0, SEEK_END);
                    preloadMP3ThreadArgs[i].sortPriority = ftell(file);
                    fclose(file);
                }
                else
                {
                    preloadMP3ThreadArgs[i].sortPriority = 0;
                }
            }
        std::sort(preloadMP3ThreadArgs.begin(), preloadMP3ThreadArgs.end());

        EXIT_CRIT_SECT
    }

#else // no support for threaded MP3 loading on this platform

    void Preload_MP3(FILE** filePtr, int track)
    {
        Preload_MP3_Synchronous(filePtr, track);
    }

    void Preload_Used_MP3s(void)
    {
        int track;
        for (track = 0; track < 100; track++)
            if (preloaded_tracks[track])
                Preload_MP3(&Tracks[track].F_decoded, track);
    }

#endif

    void MP3_CancelAllPreloading(void)
    {
        while (!noTracksQueued)
        {
            Preload_MP3_Synchronous_Cancel_Exception = -1;
            Preload_MP3_Synchronous_Cancel = true;
#ifdef _WIN32
            ENTER_CRIT_SECT
                preloadMP3ThreadArgs.clear();
            EXIT_CRIT_SECT
                Sleep(10);
#endif
        }
    }

    FILE* GetMP3TrackFile(int trackIndex, int* pDontLeaveFileOpen, int* pWhere_read)
    {
        FILE* decodedFile = Tracks[trackIndex].F_decoded;
        int usePartiallyDecodedFile = 0;
        char str[1024];

        int curTrack = LBA_to_Track(SCD.Cur_LBA);
        int lbaOffset = SCD.Cur_LBA - Track_to_LBA(curTrack);
        int lba = lbaOffset;
        int outRead = 0;
        int forceNoDecode = 0;

        int& where_read = *pWhere_read;
        where_read = (lba) * 588 * 4 + 16;
        if (where_read < 0) where_read = 0;

#ifdef _WIN32
        if (!decodedFile)
        {
            Waiting_For_Preload_MP3_Synchronous = true;
            Preload_MP3(&Tracks[trackIndex].F_decoded, trackIndex);

            DWORD tgtime = timeGetTime(); //Modif N - give frame advance sound:
            bool soundCleared = false;

            sprintf(str, "%s/" PRELOADED_MP3_DIRECTORY PRELOADED_MP3_FILENAME, Gens_Path, trackIndex + 1);
            while (!usePartiallyDecodedFile)
            {
                if (!decodedFile)
                    decodedFile = fopen(str, "rb");
                if (decodedFile)
                {
                    int decodedSize;
                    fseek(decodedFile, 0, SEEK_END);
                    decodedSize = ftell(decodedFile);
                    if (decodedSize >= where_read + 588 * 4)
                        usePartiallyDecodedFile = 1;
                }
                if (!usePartiallyDecodedFile && noTracksQueued)
                {
                    if (decodedFile)
                        fclose(decodedFile);
                    decodedFile = NULL;
                    break;
                }
#ifdef _WIN32
                if (!usePartiallyDecodedFile)
                    Sleep(5);
#endif // win32

                if (!soundCleared && timeGetTime() - tgtime >= 125) //eliminate stutter
                {
                    int Clear_Sound_Buffer(void);
                    Clear_Sound_Buffer();
                    soundCleared = true;
                }
            }
            if (!decodedFile)
                decodedFile = Tracks[trackIndex].F_decoded;
            Waiting_For_Preload_MP3_Synchronous = false;
        }
#endif // threaded

        *pDontLeaveFileOpen = usePartiallyDecodedFile;
        return decodedFile;
    }

    int MP3_Play(int track, int lba_pos, int async)
    {
        if (track < 0 || track > 99)
            return -1;

        Track_Played = track;

        if (Tracks[Track_Played].F == NULL)
        {
            Current_IN_Pos = 0;
            return -1;
        }

        if (!IsAsyncAllowed())
            async = 0;

        if (!async)
        {
            Preload_MP3(&Tracks[Track_Played].F_decoded, Track_Played);
        }

        if (async && !Tracks[Track_Played].F_decoded)
        {
            // start playing MP3 "asynchronously", decoding on the fly... but it won't reliably produce the same sound samples under the same circumstances
            Current_IN_Pos = MP3_Find_Frame(Tracks[Track_Played].F, lba_pos);
            ResetMP3_Gens(&mp);
            MP3_Update_IN();
        }

        return 0;
    }

    int MP3_Update(char *buf, int *rate, int *channel, unsigned int length_dest)
    {
        unsigned int length_src, size;
        char *buf_mp3;

        int updateSize = 0;

        if (Current_OUT_Size == 0) if (MP3_Update_IN()) return updateSize;
        if (Current_OUT_Size == 0) return updateSize;

        *rate = freqs_mp3[mp.fr.sampling_frequency];

        if (mp.fr.stereo == 2) *channel = 2;
        else *channel = 1;

        length_src = (*rate / 75) << *channel;

        size = Current_OUT_Size - Current_OUT_Pos;

        //	fprintf(debug_SCD_file, "\n*********  rate = %d chan = %d size = %d len = %d\n", *rate, *channel, size, length_src);

        while (length_src > size)
        {
            buf_mp3 = (char *)&buf_out[Current_OUT_Pos];

            memcpy(buf, buf_mp3, size);
            updateSize += size;

            length_src -= size;
            buf += size;

            //		fprintf(debug_SCD_file, "size = %d len = %d\n", size, length_src);

            if (MP3_Update_OUT()) return updateSize;
            size = Current_OUT_Size - Current_OUT_Pos;
        }

        buf_mp3 = (char *)&buf_out[Current_OUT_Pos];

        memcpy(buf, buf_mp3, length_src);
        updateSize += length_src;

        //	fprintf(debug_SCD_file, "size = %d len = %d\n", size, length_src);

        Current_OUT_Pos += length_src;

        return updateSize;
    }

    /*
    void MP3_Test(FILE* f)
    {
    MP3_Reset();

    Current_MP3_File = f;

    if (Current_MP3_File == NULL) return;
    }
    */
}