#include "doomstat.h"
#include "s_sound.h"
#include "i_sound.h"
#include "i_system.h"
#include "d_main.h"
#include "r_main.h"
#include "m_random.h"
#include "w_wad.h"

#define S_CLIPPING_DIST                 (1200 << FRACBITS)
#define S_CLOSE_DIST                    (160 << FRACBITS)
#define S_ATTENUATOR                    ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)
#define NORM_PITCH                      128
#define NORM_PRIORITY                   64
#define NORM_SEP                        128
#define S_STEREO_SWING                  (96 << FRACBITS)

const char *S_music_files[NUMMUSIC];

typedef struct
{
  sfxinfo_t *sfxinfo;
  void *origin;
  int handle;
  int is_pickup;
} channel_t;

static channel_t *channels;
int snd_SfxVolume = 15;
int snd_MusicVolume = 15;
static boolean mus_paused;
static musicinfo_t *mus_playing;
int default_numChannels = 8;
int numChannels;
int idmusnum;
void S_StopChannel(int cnum);
int S_AdjustSoundParams(mobj_t *listener, mobj_t *source, int *vol, int *sep, int *pitch);
static int S_getChannel(void *origin, sfxinfo_t *sfxinfo, int is_pickup);

void S_Init(int sfxVolume, int musicVolume)
{

    numChannels = default_numChannels;

    if (snd_card && !nosfxparm)
    {

        int i;

        I_Print("S_Init: default sfx volume %d\n", sfxVolume);
        I_SetChannels();
        S_SetSfxVolume(sfxVolume);

        channels = (channel_t *)calloc(numChannels,sizeof(channel_t));

        for (i = 1; i < NUMSFX; i++)
            S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;

    }

    if (mus_card && !nomusicparm)
    {

        S_SetMusicVolume(musicVolume);

        mus_paused = 0;

    }

}

void S_Stop(void)
{

    int cnum;

    if (snd_card && !nosfxparm)
    {

        for (cnum = 0; cnum < numChannels; cnum++)
        {

            if (channels[cnum].sfxinfo)
                S_StopChannel(cnum);

        }

    }

}

void S_Start(void)
{

    int mnum;

    S_Stop();

    if (!mus_card || nomusicparm)
        return;

    mus_paused = 0;

    if (idmusnum != -1)
    {

        mnum = idmusnum;

    }

    else if (gamemode == commercial)
    {

        mnum = mus_runnin + gamemap - 1;

    }

    else
    {

        static const int spmus[] = {
            mus_e3m4,
            mus_e3m2,
            mus_e3m3,
            mus_e1m5,
            mus_e2m7,
            mus_e2m4,
            mus_e2m6,
            mus_e2m5,
            mus_e1m9
        };

        if (gameepisode < 4)
            mnum = mus_e1m1 + (gameepisode - 1) * 9 + gamemap - 1;
        else
            mnum = spmus[gamemap - 1];

    }

    S_ChangeMusic(mnum, true);

}

void S_StartSoundAtVolume(void *origin_p, int sfx_id, int volume)
{

    int sep, pitch, priority, cnum, is_pickup;
    sfxinfo_t *sfx;
    mobj_t *origin = (mobj_t *)origin_p;

    if (!snd_card || nosfxparm)
        return;

    is_pickup = sfx_id & PICKUP_SOUND || sfx_id == sfx_oof || sfx_id == sfx_noway;
    sfx_id &= ~PICKUP_SOUND;

    if (sfx_id < 1 || sfx_id > NUMSFX)
        I_Error("S_StartSoundAtVolume: Bad sfx #: %d", sfx_id);

    sfx = &S_sfx[sfx_id];

    if (sfx->link)
    {

        pitch = sfx->pitch;
        priority = sfx->priority;
        volume += sfx->volume;

        if (volume < 1)
            return;

        if (volume > snd_SfxVolume)
            volume = snd_SfxVolume;

    }

    else
    {

        pitch = NORM_PITCH;
        priority = NORM_PRIORITY;

    }

    if (!origin || origin == players[displayplayer].mo)
    {

        sep = NORM_SEP;
        volume *= 8;

    }
    
    else if (!S_AdjustSoundParams(players[displayplayer].mo, origin, &volume, &sep, &pitch))
    {

        return;


    }

    else if (origin->x == players[displayplayer].mo->x && origin->y == players[displayplayer].mo->y)
    {

        sep = NORM_SEP;

    }

    if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
        pitch += 8 - (P_Random(pr_misc) & 15);
    else if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
        pitch += 16 - (P_Random(pr_misc) & 31);

    if (pitch < 0)
        pitch = 0;

    if (pitch > 255)
        pitch = 255;

    for (cnum = 0; cnum < numChannels; cnum++)
    {

        if (channels[cnum].sfxinfo && channels[cnum].origin == origin && (comp[comp_sound] || channels[cnum].is_pickup == is_pickup))
        {

            S_StopChannel(cnum);

            break;

        }

    }

    cnum = S_getChannel(origin, sfx, is_pickup);

    if (cnum < 0)
        return;

    if (sfx->lumpnum < 0 && (sfx->lumpnum = I_GetSfxLumpNum(sfx)) < 0)
        return;

    if (sfx->usefulness++ < 0)
        sfx->usefulness = 1;

    {

        int h = I_StartSound(sfx_id, cnum, volume, sep, pitch, priority);

        if (h != -1)
            channels[cnum].handle = h;

    }

}

void S_StartSound(void *origin, int sfx_id)
{

    S_StartSoundAtVolume(origin, sfx_id, snd_SfxVolume);

}

void S_StopSound(void *origin)
{

    int cnum;

    if (!snd_card || nosfxparm)
        return;

    for (cnum = 0; cnum < numChannels; cnum++)
    {

        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {

            S_StopChannel(cnum);

            break;
        }

    }

}

void S_PauseSound(void)
{

    if (!mus_card || nomusicparm)
        return;

    if (mus_playing && !mus_paused)
    {

        I_PauseSong(mus_playing->handle);

        mus_paused = true;

    }

}

void S_ResumeSound(void)
{

    if (!mus_card || nomusicparm)
        return;

    if (mus_playing && mus_paused)
    {

        I_ResumeSong(mus_playing->handle);

        mus_paused = false;

    }

}

void S_UpdateSounds(void *listener_p)
{

    mobj_t *listener = (mobj_t*)listener_p;
    int cnum;

    if (!snd_card || nosfxparm)
        return;

    for (cnum = 0; cnum < numChannels; cnum++)
    {

        sfxinfo_t *sfx;
        channel_t *c = &channels[cnum];

        if ((sfx = c->sfxinfo))
        {

            if (I_SoundIsPlaying(c->handle))
            {

                int volume = snd_SfxVolume;
                int pitch = NORM_PITCH;
                int sep = NORM_SEP;

                if (sfx->link)
                {

                    pitch = sfx->pitch;
                    volume += sfx->volume;

                    if (volume < 1)
                    {

                        S_StopChannel(cnum);

                        continue;

                    }

                    else if (volume > snd_SfxVolume)
                    {

                        volume = snd_SfxVolume;

                    }

                }

                if (c->origin && listener_p != c->origin)
                {

                    if (!S_AdjustSoundParams(listener, c->origin, &volume, &sep, &pitch))
                        S_StopChannel(cnum);
                    else
                        I_UpdateSoundParams(c->handle, volume, sep, pitch);

                }

            }

            else
            {

                S_StopChannel(cnum);

            }

        }
    }
}

void S_SetMusicVolume(int volume)
{

    if (!mus_card || nomusicparm)
        return;

    if (volume < 0 || volume > 15)
        I_Error("S_SetMusicVolume: Attempt to set music volume at %d", volume);

    I_SetMusicVolume(volume);

    snd_MusicVolume = volume;

}

void S_SetSfxVolume(int volume)
{

    if (!snd_card || nosfxparm)
        return;

    if (volume < 0 || volume > 127)
        I_Error("S_SetSfxVolume: Attempt to set sfx volume at %d", volume);

    snd_SfxVolume = volume;

}

void S_StartMusic(int m_id)
{

    if (!mus_card || nomusicparm)
        return;

    S_ChangeMusic(m_id, false);

}

void S_ChangeMusic(int musicnum, int looping)
{

    musicinfo_t *music;
    int music_file_failed;
    char* music_filename;

    if (!mus_card || nomusicparm)
        return;

    if (musicnum <= mus_None || musicnum >= NUMMUSIC)
        I_Error("S_ChangeMusic: Bad music number %d", musicnum);

    music = &S_music[musicnum];

    if (mus_playing == music)
        return;

    S_StopMusic();

    if (!music->lumpnum)
    {

        char namebuf[9];

        sprintf(namebuf, "d_%s", music->name);

        music->lumpnum = W_GetNumForName(namebuf);

    }

    music_file_failed = 1;

    if (lumpinfo[music->lumpnum].source == source_iwad)
    {

        music_filename = I_FindFile(S_music_files[musicnum], "");

        if (music_filename)
        {

            music_file_failed = I_RegisterMusic(music_filename, music);

            free(music_filename);

        }
    }

    if (music_file_failed)
    {

        music->data = W_CacheLumpNum(music->lumpnum);
        music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

    }

    I_PlaySong(music->handle, looping);

    mus_playing = music;

}

void S_StopMusic(void)
{

    if (!mus_card || nomusicparm)
        return;

    if (mus_playing)
    {

        if (mus_paused)
            I_ResumeSong(mus_playing->handle);

        I_StopSong(mus_playing->handle);
        I_UnRegisterSong(mus_playing->handle);

        if (mus_playing->lumpnum >= 0)
            W_UnlockLumpNum(mus_playing->lumpnum);

        mus_playing->data = 0;
        mus_playing = 0;

    }

}

void S_StopChannel(int cnum)
{

    int i;
    channel_t *c = &channels[cnum];

    if (!snd_card || nosfxparm)
        return;

    if (c->sfxinfo)
    {

        if (I_SoundIsPlaying(c->handle))
            I_StopSound(c->handle);

        for (i = 0; i < numChannels; i++)
        {

            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
                break;

        }

        c->sfxinfo->usefulness--;
        c->sfxinfo = 0;

    }

}

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source, int *vol, int *sep, int *pitch)
{

    fixed_t adx, ady,approx_dist;
    angle_t angle;

    if (!snd_card || nosfxparm)
        return 0;

    if (!listener)
        return 0;

    adx = D_abs(listener->x - source->x);
    ady = D_abs(listener->y - source->y);
    approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

    if (!approx_dist)
    {

        *sep = NORM_SEP;
        *vol = snd_SfxVolume;

        return *vol > 0;

    }

    if (approx_dist > S_CLIPPING_DIST)
        return 0;

    angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

    if (angle <= listener->angle)
        angle += 0xffffffff;

    angle -= listener->angle;
    angle >>= ANGLETOFINESHIFT;

    *sep = 128 - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);

    if (approx_dist < S_CLOSE_DIST)
        *vol = snd_SfxVolume*8;
    else
        *vol = (snd_SfxVolume * ((S_CLIPPING_DIST - approx_dist) >> FRACBITS) * 8) / S_ATTENUATOR;

    return (*vol > 0);

}

static int S_getChannel(void *origin, sfxinfo_t *sfxinfo, int is_pickup)
{

    int cnum;
    channel_t *c;

    if (!snd_card || nosfxparm)
        return -1;

    for (cnum = 0; cnum < numChannels && channels[cnum].sfxinfo; cnum++)
    {

        if (origin && channels[cnum].origin == origin && channels[cnum].is_pickup == is_pickup)
        {

            S_StopChannel(cnum);

            break;

        }

    }

    if (cnum == numChannels)
    {

        for (cnum = 0; cnum < numChannels; cnum++)
        {

            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
                break;

        }

        if (cnum == numChannels)
            return -1;
        else
            S_StopChannel(cnum);

    }

    c = &channels[cnum];
    c->sfxinfo = sfxinfo;
    c->origin = origin;
    c->is_pickup = is_pickup;

    return cnum;

}

