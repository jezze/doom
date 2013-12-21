#include <math.h>
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_mutex.h>
#include <SDL_byteorder.h>
#include <SDL_version.h>
#include "m_fixed.h"
#include "z_zone.h"
#include "i_sound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "i_system.h"
#include "s_sound.h"
#include "doomdef.h"
#include "doomstat.h"
#include "doomtype.h"
#include "d_main.h"

#define MAX_CHANNELS                    32

int snd_card = 1;
int mus_card = 1;
int detect_voices = 0;
static boolean sound_inited = false;
static boolean first_sound_init = true;
static int SAMPLECOUNT = 512;
int snd_samplerate = 11025;
int audio_fd;

typedef struct
{

    int id;
    unsigned int step;
    unsigned int stepremainder;
    unsigned int samplerate;
    const unsigned char *data;
    const unsigned char *enddata;
    int starttime;
    int *leftvol_lookup;
    int *rightvol_lookup;

} channel_info_t;

channel_info_t channelinfo[MAX_CHANNELS];

int steptable[256];
int vol_lookup[128 * 256];

static void stopchan(int i)
{

    if (channelinfo[i].data)
    {

        channelinfo[i].data = NULL;
        W_UnlockLumpNum(S_sfx[channelinfo[i].id].lumpnum);

    }

}

static int addsfx(int sfxid, int channel, const unsigned char* data, size_t len)
{

    stopchan(channel);

    channelinfo[channel].data = data;
    channelinfo[channel].enddata = channelinfo[channel].data + len - 1;
    channelinfo[channel].samplerate = (channelinfo[channel].data[3] << 8) + channelinfo[channel].data[2];
    channelinfo[channel].data += 8;
    channelinfo[channel].stepremainder = 0;
    channelinfo[channel].starttime = gametic;
    channelinfo[channel].id = sfxid;

    return channel;

}

static void updateSoundParams(int handle, int volume, int seperation, int pitch)
{

    int slot = handle;
    int rightvol;
    int leftvol;
    int step = steptable[pitch];

    channelinfo[slot].step = ((channelinfo[slot].samplerate << 16) / snd_samplerate);

    seperation += 1;
    leftvol = volume - ((volume * seperation*seperation) >> 16);
    seperation = seperation - 257;
    rightvol= volume - ((volume * seperation*seperation) >> 16);

    if (rightvol < 0 || rightvol > 127)
        I_Error("rightvol out of bounds");

    if (leftvol < 0 || leftvol > 127)
        I_Error("leftvol out of bounds");

    channelinfo[slot].leftvol_lookup = &vol_lookup[leftvol * 256];
    channelinfo[slot].rightvol_lookup = &vol_lookup[rightvol * 256];

}

void I_UpdateSoundParams(int handle, int volume, int seperation, int pitch)
{

    SDL_LockAudio();
    updateSoundParams(handle, volume, seperation, pitch);
    SDL_UnlockAudio();

}

void I_SetChannels(void)
{

    int i;
    int j;
    int *steptablemid = steptable + 128;

    for (i = 0; i < MAX_CHANNELS; i++)
        memset(&channelinfo[i], 0, sizeof (channel_info_t));

    for (i = -128; i < 128; i++)
        steptablemid[i] = (int)(pow(1.2, ((double)i / (64.0 * snd_samplerate / 11025))) * 65536.0);

    for (i = 0; i < 128; i++)
    {

        for (j = 0; j < 256; j++)
            vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 191;

    }

}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{

    char namebuf[9];

    sprintf(namebuf, "ds%s", sfx->name);

    return W_GetNumForName(namebuf);

}

int I_StartSound(int id, int channel, int vol, int sep, int pitch, int priority)
{

    const unsigned char* data;
    int lump;
    size_t len;

    if ((channel < 0) || (channel >= MAX_CHANNELS))
        return -1;

    lump = S_sfx[id].lumpnum;
    len = W_LumpLength(lump);

    if (len <= 8)
        return -1;

    len -= 8;
    data = W_LockLumpNum(lump);

    SDL_LockAudio();
    addsfx(id, channel, data, len);
    updateSoundParams(channel, vol, sep, pitch);
    SDL_UnlockAudio();

    return channel;

}

void I_StopSound(int handle)
{

    SDL_LockAudio();
    stopchan(handle);
    SDL_UnlockAudio();

}

boolean I_SoundIsPlaying(int handle)
{

    return channelinfo[handle].data != NULL;

}

boolean I_AnySoundStillPlaying(void)
{

    boolean result = false;
    int i;

    for (i = 0; i < MAX_CHANNELS; i++)
        result |= channelinfo[i].data != NULL;

    return result;

}

static void I_UpdateSound(void *unused, Uint8 *stream, int len)
{

    short *leftout = (short *)stream;
    short *rightout = ((short *)stream)+1;
    int step = 2;
    short *leftend = leftout + (len / 4) * step;
    int chan;

    while (leftout != leftend)
    {

        register int dl = *leftout;
        register int dr = *rightout;

        for (chan = 0; chan < numChannels; chan++)
        {

            register unsigned char sample;

            if (!channelinfo[chan].data)
                continue;

            sample = (((unsigned int)channelinfo[chan].data[0] * (0x10000 - channelinfo[chan].stepremainder)) + ((unsigned int)channelinfo[chan].data[1] * (channelinfo[chan].stepremainder))) >> 16;
            dl += channelinfo[chan].leftvol_lookup[sample];
            dr += channelinfo[chan].rightvol_lookup[sample];
            channelinfo[chan].stepremainder += channelinfo[chan].step;
            channelinfo[chan].data += channelinfo[chan].stepremainder >> 16;
            channelinfo[chan].stepremainder &= 0xffff;

            if (channelinfo[chan].data >= channelinfo[chan].enddata)
                stopchan(chan);

        }

        if (dl > SHRT_MAX)
            *leftout = SHRT_MAX;
        else if (dl < SHRT_MIN)
            *leftout = SHRT_MIN;
        else
            *leftout = (short)dl;

        if (dr > SHRT_MAX)
            *rightout = SHRT_MAX;
        else if (dr < SHRT_MIN)
            *rightout = SHRT_MIN;
        else
            *rightout = (short)dr;

        leftout += step;
        rightout += step;

    }

}

void I_ShutdownSound(void)
{

    if (!sound_inited)
        return;

    I_Print("I_ShutdownSound: ");
    SDL_CloseAudio();
    I_Print("\n");

    sound_inited = false;

}

void I_InitSound(void)
{

    SDL_AudioSpec audio;

    I_Print("I_InitSound: ");

    audio.freq = snd_samplerate;
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    audio.format = AUDIO_S16MSB;
#else
    audio.format = AUDIO_S16LSB;
#endif
    audio.channels = 2;
    audio.samples = SAMPLECOUNT*snd_samplerate / 11025;
    audio.callback = I_UpdateSound;

    if (SDL_OpenAudio(&audio, NULL) < 0 )
    {

        I_Print("couldn't open audio with desired format\n");

        return;
    }

    SAMPLECOUNT = audio.samples;

    I_Print(" configured audio device with %d samples/slice\n", SAMPLECOUNT);

    if (first_sound_init)
    {

        atexit(I_ShutdownSound);

        first_sound_init = false;

    }

    if (!nomusicparm)
        I_InitMusic();

    I_Print("I_InitSound: sound module ready\n");
    SDL_PauseAudio(0);

}

void I_ShutdownMusic(void)
{

}

void I_InitMusic(void)
{

}

void I_PlaySong(int handle, int looping)
{

}

void I_PauseSong(int handle)
{

}

void I_ResumeSong(int handle)
{

}

void I_StopSong(int handle)
{

}

void I_UnRegisterSong(int handle)
{

}

int I_RegisterSong(const void *data, size_t len)
{

    return 0;

}

int I_RegisterMusic(const char *filename, musicinfo_t *song )
{

    return 1;

}

void I_SetMusicVolume(int volume)
{

}

