#ifndef R_PATCH_H
#define R_PATCH_H

typedef enum
{

    RDRAW_EDGESLOPE_TOP_UP              = (1 << 0),
    RDRAW_EDGESLOPE_TOP_DOWN            = (1 << 1),
    RDRAW_EDGESLOPE_BOT_UP              = (1 << 2),
    RDRAW_EDGESLOPE_BOT_DOWN            = (1 << 3),
    RDRAW_EDGESLOPE_TOP_MASK            = RDRAW_EDGESLOPE_TOP_UP | RDRAW_EDGESLOPE_TOP_DOWN,
    RDRAW_EDGESLOPE_BOT_MASK            = RDRAW_EDGESLOPE_BOT_UP | RDRAW_EDGESLOPE_BOT_DOWN

} edgeslope_t;

typedef struct
{

    int topdelta;
    int length;
    edgeslope_t slope;

} rpost_t;

typedef struct
{

    int numPosts;
    rpost_t *posts;
    unsigned char *pixels;

} rcolumn_t;

typedef struct
{

    int width;
    int height;
    unsigned widthmask;
    unsigned char isNotTileable;
    int leftoffset;
    int topoffset;
    unsigned char *data;
    unsigned char *pixels;
    rcolumn_t *columns;
    rpost_t *posts;
    unsigned int locks;

} rpatch_t;

const rpatch_t *R_CachePatchNum(int id);
void R_UnlockPatchNum(int id);
const rpatch_t *R_CacheTextureCompositePatchNum(int id);
void R_UnlockTextureCompositePatchNum(int id);
int R_NumPatchWidth(int lump);
int R_NumPatchHeight(int lump);
const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnIndex);
const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnIndex);
const rcolumn_t *R_GetPatchColumn(const rpatch_t *patch, int columnIndex);
void R_InitPatches();
void R_FlushAllPatches();

#define R_CachePatchName(name) R_CachePatchNum(W_GetNumForName(name))
#define R_UnlockPatchName(name) R_UnlockPatchNum(W_GetNumForName(name))
#define R_NamePatchWidth(name) R_NumPatchWidth(W_GetNumForName(name))
#define R_NamePatchHeight(name) R_NumPatchHeight(W_GetNumForName(name))

#endif
