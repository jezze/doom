#ifndef __I_SOUND__
#define __I_SOUND__

void I_InitSound(void);
void I_SetChannels(void);
int I_StartSound(int id, int channel, int vol, int sep, int pitch, int priority);
void I_StopSound(int handle);
boolean I_SoundIsPlaying(int handle);
boolean I_AnySoundStillPlaying(void);
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch);
void I_InitMusic(void);
void I_ShutdownMusic(void);
void I_UpdateMusic(void);
void I_SetMusicVolume(int volume);
void I_PauseSong(int handle);
void I_ResumeSong(int handle);
int I_RegisterSong(const void *data, size_t len);
int I_RegisterMusic(const char *filename, struct musicinfo *music);
void I_PlaySong(int handle, int looping);
void I_StopSong(int handle);
void I_UnRegisterSong(int handle);

extern int snd_card;
extern int mus_card;
extern int snd_samplerate;

#endif
