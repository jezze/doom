#ifndef __S_SOUND__
#define __S_SOUND__

#define PICKUP_SOUND                    0x8000

void S_Init(int sfxVolume, int musicVolume);
void S_Stop(void);
void S_Start(void);
void S_StartSound(void *origin, int sound_id);
void S_StartSoundAtVolume(void *origin, int sound_id, int volume);
void S_StopSound(void *origin);
void S_StartMusic(int music_id);
void S_ChangeMusic(int music_id, int looping);
void S_StopMusic(void);
void S_PauseSound(void);
void S_ResumeSound(void);
void S_UpdateSounds(void *listener);
void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);

extern int numChannels;

#endif
