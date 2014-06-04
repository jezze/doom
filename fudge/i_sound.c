#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_sound.h"

int snd_card = 1;
int mus_card = 1;
int snd_samplerate = 11025;

void I_UpdateSoundParams(int handle, int volume, int seperation, int pitch)
{

}

void I_SetChannels(void)
{

}

int I_StartSound(int id, int channel, int vol, int sep, int pitch, int priority)
{

    return 0;

}

void I_StopSound(int handle)
{

}

boolean I_SoundIsPlaying(int handle)
{

    return 0;

}

boolean I_AnySoundStillPlaying(void)
{

    return 0;

}

void I_InitSound(void)
{

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

int I_RegisterMusic(const char *filename, struct musicinfo *song)
{

    return 1;

}

void I_SetMusicVolume(int volume)
{

}

