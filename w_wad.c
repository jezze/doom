#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "m_fixed.h"
#include "z_zone.h"
#include "w_wad.h"
#include "i_system.h"

lumpinfo_t *lumpinfo;
int numlumps;
wadfile_info_t *wadfiles = NULL;
size_t numwadfiles = 0;

static struct
{

    void *cache;
    unsigned int locks;

} *cachelump;

const void *W_LockLumpNum(int lump)
{

    return W_CacheLumpNum(lump);

}

void W_UnlockLumpNum(int lump)
{

    const int unlocks = 1;

    cachelump[lump].locks -= unlocks;

    if (unlocks && !cachelump[lump].locks)
        Z_ChangeTag(cachelump[lump].cache, PU_CACHE);

}

const void *W_CacheLumpNum(int lump)
{

    const int locks = 1;

    if (!cachelump[lump].cache)
        W_ReadLump(lump, Z_Malloc(W_LumpLength(lump), PU_CACHE, &cachelump[lump].cache));

    if (!cachelump[lump].locks && locks)
        Z_ChangeTag(cachelump[lump].cache, PU_STATIC);

    cachelump[lump].locks += locks;

    return cachelump[lump].cache;

}

static void ExtractFileBase(const char *path, char *dest)
{

    const char *src = path + strlen(path) - 1;
    int length;

    while (src != path && src[-1] != ':' && *(src - 1) != '\\' && *(src - 1) != '/')
        src--;

    memset(dest, 0, 8);
    length = 0;

    while ((*src) && (*src != '.') && (++length < 9))
    {

        *dest++ = toupper(*src);
        *src++;

    }

}

static void W_AddFile(wadfile_info_t *wadfile) 
{

    wadinfo_t header;
    lumpinfo_t *lump_p;
    unsigned i;
    int length;
    int startlump;
    filelump_t *fileinfo, *fileinfo2free = NULL;
    filelump_t singleinfo;

    wadfile->handle = open(wadfile->name, O_RDONLY);

    if (wadfile->handle == -1) 
    {

        if (strlen(wadfile->name) <= 4 || (strcasecmp(wadfile->name + strlen(wadfile->name) - 4, ".lmp") && strcasecmp(wadfile->name + strlen(wadfile->name) - 4, ".gwa")))
            I_Error("W_AddFile: couldn't open %s", wadfile->name);

        return;

    }

    startlump = numlumps;

    if (strlen(wadfile->name) <= 4 || (strcasecmp(wadfile->name + strlen(wadfile->name) - 4, ".wad") && strcasecmp(wadfile->name + strlen(wadfile->name) - 4,".gwa")))
    {

        fileinfo = &singleinfo;
        singleinfo.filepos = 0;
        singleinfo.size = I_Filelength(wadfile->handle);

        ExtractFileBase(wadfile->name, singleinfo.name);

        numlumps++;

    }

    else
    {

        I_Read(wadfile->handle, &header, sizeof (header));

        if (strncmp(header.identification, "IWAD", 4) && strncmp(header.identification, "PWAD", 4))
            I_Error("W_AddFile: Wad file %s doesn't have IWAD or PWAD id", wadfile->name);

        header.numlumps = header.numlumps;
        header.infotableofs = header.infotableofs;
        length = header.numlumps * sizeof(filelump_t);
        fileinfo2free = fileinfo = malloc(length);

        lseek(wadfile->handle, header.infotableofs, SEEK_SET);
        I_Read(wadfile->handle, fileinfo, length);

        numlumps += header.numlumps;

    }

    lumpinfo = realloc(lumpinfo, numlumps * sizeof (lumpinfo_t));
    lump_p = &lumpinfo[startlump];

    for (i = startlump; (int)i < numlumps ; i++, lump_p++, fileinfo++)
    {

        lump_p->wadfile = wadfile;
        lump_p->position = fileinfo->filepos;
        lump_p->size = fileinfo->size;
        lump_p->li_namespace = ns_global;

        strncpy (lump_p->name, fileinfo->name, 8);

        lump_p->source = wadfile->src;

    }

    free(fileinfo2free);

}

static int IsMarker(const char *marker, const char *name)
{

    return !strncasecmp(name, marker, 8) || (*name == *marker && !strncasecmp(name + 1, marker, 7));

}

static void W_CoalesceMarkedResource(const char *start_marker, const char *end_marker, int li_namespace)
{

    lumpinfo_t *marked = malloc(sizeof(*marked) * numlumps);
    size_t i, num_marked = 0, num_unmarked = 0;
    int is_marked = 0, mark_end = 0;
    lumpinfo_t *lump = lumpinfo;

    for (i = numlumps; i--; lump++)
    {

        if (IsMarker(start_marker, lump->name))
        {

            if (!num_marked)
            {

                strncpy(marked->name, start_marker, 8);

                marked->size = 0;
                marked->li_namespace = ns_global;
                marked->wadfile = NULL;
                num_marked = 1;

            }

            is_marked = 1;

        }

        else if (IsMarker(end_marker, lump->name))
        {

            mark_end = 1;
            is_marked = 0;

        }

        else if (is_marked)
        {

            marked[num_marked] = *lump;
            marked[num_marked++].li_namespace = li_namespace;

        }

        else
        {

            lumpinfo[num_unmarked++] = *lump;

        }

    }

    memcpy(lumpinfo + num_unmarked, marked, num_marked * sizeof(*marked));
    free(marked);

    numlumps = num_unmarked + num_marked;

    if (mark_end)
    {

        lumpinfo[numlumps].size = 0;
        lumpinfo[numlumps].wadfile = NULL;
        lumpinfo[numlumps].li_namespace = ns_global;

        strncpy(lumpinfo[numlumps++].name, end_marker, 8);

    }

}

unsigned W_LumpNameHash(const char *s)
{

    unsigned hash;

    (void) ((hash = toupper(s[0]), s[1]) && (hash = hash * 3 + toupper(s[1]), s[2]) && (hash = hash * 2 + toupper(s[2]), s[3]) && (hash = hash * 2 + toupper(s[3]), s[4]) && (hash = hash * 2 + toupper(s[4]), s[5]) && (hash = hash * 2 + toupper(s[5]), s[6]) && (hash = hash * 2 + toupper(s[6]), hash = hash * 2 + toupper(s[7])));

    return hash;

}

int W_CheckNumForName(register const char *name, register int li_namespace)
{

    register int i = (numlumps == 0) ? (-1) : (lumpinfo[W_LumpNameHash(name) % (unsigned)numlumps].index);

    while (i >= 0 && (strncasecmp(lumpinfo[i].name, name, 8) || lumpinfo[i].li_namespace != li_namespace))
        i = lumpinfo[i].next;

    return i;

}

static void W_HashLumps(void)
{

    int i;

    for (i = 0; i < numlumps; i++)
        lumpinfo[i].index = -1;

    for (i = 0; i < numlumps; i++)
    {

        int j = W_LumpNameHash(lumpinfo[i].name) % (unsigned)numlumps;

        lumpinfo[i].next = lumpinfo[j].index;
        lumpinfo[j].index = i;

    }

}

int W_GetNumForName(const char *name)
{

    int i = W_CheckNumForName(name, ns_global);

    if (i == -1)
        I_Error("W_GetNumForName: %.8s not found", name);

    return i;

}

void W_Init(void)
{

    int i;

    numlumps = 0; lumpinfo = NULL;

    for (i = 0; (size_t)i < numwadfiles; i++)
        W_AddFile(&wadfiles[i]);

    if (!numlumps)
        I_Error ("W_Init: No files found");

    W_CoalesceMarkedResource("S_START", "S_END", ns_sprites);
    W_CoalesceMarkedResource("F_START", "F_END", ns_flats);
    W_CoalesceMarkedResource("B_START", "B_END", ns_prboom);
    W_HashLumps();

    cachelump = calloc(sizeof *cachelump, numlumps);

    if (!cachelump)
        I_Error ("W_Init: Couldn't allocate lumpcache");

}

int W_LumpLength(int lump)
{

    if (lump >= numlumps)
        I_Error("W_LumpLength: %i >= numlumps", lump);

    return lumpinfo[lump].size;

}

void W_ReadLump(int lump, void *dest)
{

    lumpinfo_t *l = lumpinfo + lump;

    if (l->wadfile)
    {

        lseek(l->wadfile->handle, l->position, SEEK_SET);
        I_Read(l->wadfile->handle, dest, l->size);

    }

}

