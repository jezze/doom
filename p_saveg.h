#ifndef __P_SAVEG__
#define __P_SAVEG__

void P_ArchivePlayers(void);
void P_UnArchivePlayers(void);
void P_ArchiveWorld(void);
void P_UnArchiveWorld(void);
void P_ArchiveThinkers(void);
void P_UnArchiveThinkers(void);
void P_ArchiveSpecials(void);
void P_UnArchiveSpecials(void);
void P_ThinkerToIndex(void);
void P_IndexToThinker(void);
void P_ArchiveRNG(void);
void P_UnArchiveRNG(void);
void P_ArchiveMap(void);
void P_UnArchiveMap(void);
extern byte *save_p;
void CheckSaveGame(size_t,const char*, int);

#define CheckSaveGame(a) (CheckSaveGame)(a, __FILE__, __LINE__)

#endif
