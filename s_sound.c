#include <stdlib.h>
#include <stdio.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_sound.h"

#define S_CLIPPING_DIST                 (1200 << FRACBITS)
#define S_CLOSE_DIST                    (160 << FRACBITS)
#define S_ATTENUATOR                    ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)
#define NORM_PITCH                      128
#define NORM_PRIORITY                   64
#define NORM_SEP                        128
#define S_STEREO_SWING                  (96 << FRACBITS)

struct musicinfo S_music[] = {
    {0},
    {"e1m1", 0},
    {"e1m2", 0},
    {"e1m3", 0},
    {"e1m4", 0},
    {"e1m5", 0},
    {"e1m6", 0},
    {"e1m7", 0},
    {"e1m8", 0},
    {"e1m9", 0},
    {"e2m1", 0},
    {"e2m2", 0},
    {"e2m3", 0},
    {"e2m4", 0},
    {"e2m5", 0},
    {"e2m6", 0},
    {"e2m7", 0},
    {"e2m8", 0},
    {"e2m9", 0},
    {"e3m1", 0},
    {"e3m2", 0},
    {"e3m3", 0},
    {"e3m4", 0},
    {"e3m5", 0},
    {"e3m6", 0},
    {"e3m7", 0},
    {"e3m8", 0},
    {"e3m9", 0},
    {"inter", 0},
    {"intro", 0},
    {"bunny", 0},
    {"victor", 0},
    {"introa", 0},
    {"runnin", 0},
    {"stalks", 0},
    {"countd", 0},
    {"betwee", 0},
    {"doom", 0},
    {"the_da", 0},
    {"shawn", 0},
    {"ddtblu", 0},
    {"in_cit", 0},
    {"dead", 0},
    {"stlks2", 0},
    {"theda2", 0},
    {"doom2", 0},
    {"ddtbl2", 0},
    {"runni2", 0},
    {"dead2", 0},
    {"stlks3", 0},
    {"romero", 0},
    {"shawn2", 0},
    {"messag", 0},
    {"count2", 0},
    {"ddtbl3", 0},
    {"ampie", 0},
    {"theda3", 0},
    {"adrian", 0},
    {"messg2", 0},
    {"romer2", 0},
    {"tense", 0},
    {"shawn3", 0},
    {"openin", 0},
    {"evil", 0},
    {"ultima", 0},
    {"read_m", 0},
    {"dm2ttl", 0},
    {"dm2int", 0}
};

struct sfxinfo S_sfx[] = {
    {"none", false,  0, 0, -1, -1, 0},
    {"pistol", false, 64, 0, -1, -1, 0},
    {"shotgn", false, 64, 0, -1, -1, 0},
    {"sgcock", false, 64, 0, -1, -1, 0},
    {"dshtgn", false, 64, 0, -1, -1, 0},
    {"dbopn", false, 64, 0, -1, -1, 0},
    {"dbcls", false, 64, 0, -1, -1, 0},
    {"dbload", false, 64, 0, -1, -1, 0},
    {"plasma", false, 64, 0, -1, -1, 0},
    {"bfg", false, 64, 0, -1, -1, 0},
    {"sawup", false, 64, 0, -1, -1, 0},
    {"sawidl", false, 118, 0, -1, -1, 0},
    {"sawful", false, 64, 0, -1, -1, 0},
    {"sawhit", false, 64, 0, -1, -1, 0},
    {"rlaunc", false, 64, 0, -1, -1, 0},
    {"rxplod", false, 70, 0, -1, -1, 0},
    {"firsht", false, 70, 0, -1, -1, 0},
    {"firxpl", false, 70, 0, -1, -1, 0},
    {"pstart", false, 100, 0, -1, -1, 0},
    {"pstop", false, 100, 0, -1, -1, 0},
    {"doropn", false, 100, 0, -1, -1, 0},
    {"dorcls", false, 100, 0, -1, -1, 0},
    {"stnmov", false, 119, 0, -1, -1, 0},
    {"swtchn", false, 78, 0, -1, -1, 0},
    {"swtchx", false, 78, 0, -1, -1, 0},
    {"plpain", false, 96, 0, -1, -1, 0},
    {"dmpain", false, 96, 0, -1, -1, 0},
    {"popain", false, 96, 0, -1, -1, 0},
    {"vipain", false, 96, 0, -1, -1, 0},
    {"mnpain", false, 96, 0, -1, -1, 0},
    {"pepain", false, 96, 0, -1, -1, 0},
    {"slop", false, 78, 0, -1, -1, 0},
    {"itemup", true, 78, 0, -1, -1, 0},
    {"wpnup", true, 78, 0, -1, -1, 0},
    {"oof", false, 96, 0, -1, -1, 0},
    {"telept", false, 32, 0, -1, -1, 0},
    {"posit1", true, 98, 0, -1, -1, 0},
    {"posit2", true, 98, 0, -1, -1, 0},
    {"posit3", true, 98, 0, -1, -1, 0},
    {"bgsit1", true, 98, 0, -1, -1, 0},
    {"bgsit2", true, 98, 0, -1, -1, 0},
    {"sgtsit", true, 98, 0, -1, -1, 0},
    {"cacsit", true, 98, 0, -1, -1, 0},
    {"brssit", true, 94, 0, -1, -1, 0},
    {"cybsit", true, 92, 0, -1, -1, 0},
    {"spisit", true, 90, 0, -1, -1, 0},
    {"bspsit", true, 90, 0, -1, -1, 0},
    {"kntsit", true, 90, 0, -1, -1, 0},
    {"vilsit", true, 90, 0, -1, -1, 0},
    {"mansit", true, 90, 0, -1, -1, 0},
    {"pesit", true, 90, 0, -1, -1, 0},
    {"sklatk", false, 70, 0, -1, -1, 0},
    {"sgtatk", false, 70, 0, -1, -1, 0},
    {"skepch", false, 70, 0, -1, -1, 0},
    {"vilatk", false, 70, 0, -1, -1, 0},
    {"claw", false, 70, 0, -1, -1, 0},
    {"skeswg", false, 70, 0, -1, -1, 0},
    {"pldeth", false, 32, 0, -1, -1, 0},
    {"pdiehi", false, 32, 0, -1, -1, 0},
    {"podth1", false, 70, 0, -1, -1, 0},
    {"podth2", false, 70, 0, -1, -1, 0},
    {"podth3", false, 70, 0, -1, -1, 0},
    {"bgdth1", false, 70, 0, -1, -1, 0},
    {"bgdth2", false, 70, 0, -1, -1, 0},
    {"sgtdth", false, 70, 0, -1, -1, 0},
    {"cacdth", false, 70, 0, -1, -1, 0},
    {"skldth", false, 70, 0, -1, -1, 0},
    {"brsdth", false, 32, 0, -1, -1, 0},
    {"cybdth", false, 32, 0, -1, -1, 0},
    {"spidth", false, 32, 0, -1, -1, 0},
    {"bspdth", false, 32, 0, -1, -1, 0},
    {"vildth", false, 32, 0, -1, -1, 0},
    {"kntdth", false, 32, 0, -1, -1, 0},
    {"pedth", false, 32, 0, -1, -1, 0},
    {"skedth", false, 32, 0, -1, -1, 0},
    {"posact", true, 120, 0, -1, -1, 0},
    {"bgact", true, 120, 0, -1, -1, 0},
    {"dmact", true, 120, 0, -1, -1, 0},
    {"bspact", true, 100, 0, -1, -1, 0},
    {"bspwlk", true, 100, 0, -1, -1, 0},
    {"vilact", true, 100, 0, -1, -1, 0},
    {"noway", false, 78, 0, -1, -1, 0},
    {"barexp", false, 60, 0, -1, -1, 0},
    {"punch", false, 64, 0, -1, -1, 0},
    {"hoof", false, 70, 0, -1, -1, 0},
    {"metal", false, 70, 0, -1, -1, 0},
    {"chgun", false, 64, &S_sfx[sfx_pistol], 150, 0, 0},
    {"tink", false, 60, 0, -1, -1, 0},
    {"bdopn", false, 100, 0, -1, -1, 0},
    {"bdcls", false, 100, 0, -1, -1, 0},
    {"itmbk", false, 100, 0, -1, -1, 0},
    {"flame", false, 32, 0, -1, -1, 0},
    {"flamst", false, 32, 0, -1, -1, 0},
    {"getpow", false, 60, 0, -1, -1, 0},
    {"bospit", false, 70, 0, -1, -1, 0},
    {"boscub", false, 70, 0, -1, -1, 0},
    {"bossit", false, 70, 0, -1, -1, 0},
    {"bospn", false, 70, 0, -1, -1, 0},
    {"bosdth", false, 70, 0, -1, -1, 0},
    {"manatk", false, 70, 0, -1, -1, 0},
    {"mandth", false, 70, 0, -1, -1, 0},
    {"sssit", false, 70, 0, -1, -1, 0},
    {"ssdth", false, 70, 0, -1, -1, 0},
    {"keenpn", false, 70, 0, -1, -1, 0},
    {"keendt", false, 70, 0, -1, -1, 0},
    {"skeact", false, 70, 0, -1, -1, 0},
    {"skesit", false, 70, 0, -1, -1, 0},
    {"skeatk", false, 70, 0, -1, -1, 0},
    {"radio", false, 60, 0, -1, -1, 0}
};

const char *S_music_files[NUMMUSIC];

typedef struct
{
  struct sfxinfo *sfxinfo;
  void *origin;
  int handle;
  int is_pickup;
} channel_t;

static channel_t *channels;
int snd_SfxVolume = 15;
int snd_MusicVolume = 15;
static boolean mus_paused;
static struct musicinfo *mus_playing;
int default_numChannels = 8;
int numChannels;
void S_StopChannel(int cnum);
int S_AdjustSoundParams(mobj_t *listener, mobj_t *source, int *vol, int *sep, int *pitch);
static int S_getChannel(void *origin, struct sfxinfo *sfxinfo, int is_pickup);

void S_Init(int sfxVolume, int musicVolume)
{

    numChannels = default_numChannels;

    if (snd_card)
    {

        int i;

        I_SetChannels();
        S_SetSfxVolume(sfxVolume);

        channels = (channel_t *)calloc(numChannels,sizeof(channel_t));

        for (i = 1; i < NUMSFX; i++)
            S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;

    }

    if (mus_card)
    {

        S_SetMusicVolume(musicVolume);

        mus_paused = 0;

    }

}

void S_Stop(void)
{

    int cnum;

    if (snd_card)
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

    if (!mus_card)
        return;

    mus_paused = 0;

    if (gamemode == commercial)
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

static int S_GetSfxLumpNum(struct sfxinfo *sfx)
{

    char namebuf[9];

    sprintf(namebuf, "ds%s", sfx->name);

    return W_GetNumForName(namebuf);

}

void S_StartSoundAtVolume(void *origin_p, int sfx_id, int volume)
{

    int sep, pitch, priority, cnum, is_pickup;
    struct sfxinfo *sfx;
    mobj_t *origin = (mobj_t *)origin_p;

    if (!snd_card)
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

    if (!origin || origin == players[consoleplayer].mo)
    {

        sep = NORM_SEP;
        volume *= 8;

    }
    
    else if (!S_AdjustSoundParams(players[consoleplayer].mo, origin, &volume, &sep, &pitch))
    {

        return;


    }

    else if (origin->x == players[consoleplayer].mo->x && origin->y == players[consoleplayer].mo->y)
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

        if (channels[cnum].sfxinfo && channels[cnum].origin == origin && (channels[cnum].is_pickup == is_pickup))
        {

            S_StopChannel(cnum);

            break;

        }

    }

    cnum = S_getChannel(origin, sfx, is_pickup);

    if (cnum < 0)
        return;

    if (sfx->lumpnum < 0 && (sfx->lumpnum = S_GetSfxLumpNum(sfx)) < 0)
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

    if (!snd_card)
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

    if (!mus_card)
        return;

    if (mus_playing && !mus_paused)
    {

        I_PauseSong(mus_playing->handle);

        mus_paused = true;

    }

}

void S_ResumeSound(void)
{

    if (!mus_card)
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

    if (!snd_card)
        return;

    for (cnum = 0; cnum < numChannels; cnum++)
    {

        struct sfxinfo *sfx;
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

    if (!mus_card)
        return;

    if (volume < 0 || volume > 15)
        I_Error("S_SetMusicVolume: Attempt to set music volume at %d", volume);

    I_SetMusicVolume(volume);

    snd_MusicVolume = volume;

}

void S_SetSfxVolume(int volume)
{

    if (!snd_card)
        return;

    if (volume < 0 || volume > 127)
        I_Error("S_SetSfxVolume: Attempt to set sfx volume at %d", volume);

    snd_SfxVolume = volume;

}

void S_StartMusic(int m_id)
{

    if (!mus_card)
        return;

    S_ChangeMusic(m_id, false);

}

void S_ChangeMusic(int musicnum, int looping)
{

    struct musicinfo *music;
    int music_file_failed;
    char* music_filename;

    if (!mus_card)
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

    if (!mus_card)
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

    if (!snd_card)
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

    if (!snd_card)
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

static int S_getChannel(void *origin, struct sfxinfo *sfxinfo, int is_pickup)
{

    int cnum;
    channel_t *c;

    if (!snd_card)
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

