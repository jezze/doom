#ifndef __W_WAD__
#define __W_WAD__

typedef struct
{

    char identification[4];
    int numlumps;
    int infotableofs;

} wadinfo_t;

typedef struct
{

    int filepos;
    int size;
    char name[8];

} filelump_t;

typedef enum
{

  source_iwad = 0,
  source_pre,
  source_auto_load,
  source_pwad,
  source_lmp,
  source_net

} wad_source_t;

typedef struct
{

  const char *name;
  wad_source_t src;
  int handle;

} wadfile_info_t;

typedef struct
{

    char name[9];
    int size;
    int index;
    int next;

    enum
    {

        ns_global = 0,
        ns_sprites,
        ns_flats,
        ns_prboom

    } li_namespace;

    wadfile_info_t *wadfile;
    int position;
    wad_source_t source;

} lumpinfo_t;

extern wadfile_info_t *wadfiles;
extern size_t numwadfiles;
extern lumpinfo_t *lumpinfo;
extern int numlumps;

int W_CheckNumForName(const char *name, int);
int W_GetNumForName(const char *name);
int W_LumpLength(int lump);
void W_ReadLump(int lump, void *dest);
const void *W_CacheLumpNum(int lump);
const void *W_LockLumpNum(int lump);
void W_UnlockLumpNum(int lump);
unsigned W_LumpNameHash(const char *s);
void W_Init(void);

#define W_CacheLumpName(name) W_CacheLumpNum(W_GetNumForName(name))
#define W_UnlockLumpName(name) W_UnlockLumpNum(W_GetNumForName(name))

#endif
