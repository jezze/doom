#include "z_zone.h"
#include "i_sound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "s_sound.h"
#include "doomdef.h"
#include "doomstat.h"
#include "doomtype.h"
#include "d_main.h"

int snd_card = 1;
int mus_card = 1;
int snd_samplerate = 11025;

void I_UpdateSoundParams(int handle, int volume, int seperation, int pitch)
{

}

void I_SetChannels(void)
{

}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{

    return 0;

}

int I_StartSound(int id, int channel, int vol, int sep, int pitch, int priority)
{

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

int I_RegisterMusic(const char *filename, musicinfo_t *song )
{

    return 1;

}

void I_SetMusicVolume(int volume)
{

}

