#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "z_zone.h"
#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_bsp.h"
#include "r_things.h"
#include "p_tick.h"
#include "i_system.h"
#include "r_draw.h"
#include "r_patch.h"

typedef struct
{
  byte topdelta;
  byte length;
} post_t;

typedef post_t column_t;

typedef struct
{
  short width, height;
  short leftoffset;
  short topoffset;
  int columnofs[8];
} patch_t;

static rpatch_t *patches = 0;
static rpatch_t *texture_composites = 0;

void R_InitPatches(void) {
  if (!patches)
  {
    patches = (rpatch_t*)malloc(numlumps * sizeof(rpatch_t));

    memset(patches, 0, sizeof(rpatch_t)*numlumps);
  }
  if (!texture_composites)
  {
    texture_composites = (rpatch_t*)malloc(numtextures * sizeof(rpatch_t));

    memset(texture_composites, 0, sizeof(rpatch_t)*numtextures);
  }
}


void R_FlushAllPatches(void) {
  int i;

  if (patches)
  {
    for (i=0; i < numlumps; i++)
      if (patches[i].locks > 0)
        I_Error("R_FlushAllPatches: patch number %i still locked",i);
    free(patches);
    patches = NULL;
  }
  if (texture_composites)
  {
    for (i=0; i<numtextures; i++)
      if (texture_composites[i].data)
        free(texture_composites[i].data);
    free(texture_composites);
    texture_composites = NULL;
  }
}


int R_NumPatchWidth(int lump)
{
  const rpatch_t *patch = R_CachePatchNum(lump);
  int width = patch->width;
  R_UnlockPatchNum(lump);
  return width;
}


int R_NumPatchHeight(int lump)
{
  const rpatch_t *patch = R_CachePatchNum(lump);
  int height = patch->height;
  R_UnlockPatchNum(lump);
  return height;
}


static int getPatchIsNotTileable(const patch_t *patch) {
  int x=0, numPosts, lastColumnDelta = 0;
  const column_t *column;
  int cornerCount = 0;
  int hasAHole = 0;

  for (x=0; x<patch->width; x++) {
    column = (const column_t *)((const byte *)patch + patch->columnofs[x]);
    if (!x) lastColumnDelta = column->topdelta;
    else if (lastColumnDelta != column->topdelta) hasAHole = 1;

    numPosts = 0;
    while (column->topdelta != 0xff) {

      if (x == 0 && column->topdelta == 0) cornerCount++;
      else if (x == 0 && column->topdelta + column->length >= patch->height) cornerCount++;
      else if (x == patch->width-1 && column->topdelta == 0) cornerCount++;
      else if (x == patch->width-1 && column->topdelta + column->length >= patch->height) cornerCount++;

      if (numPosts++) hasAHole = 1;
      column = (const column_t *)((const byte *)column + column->length + 4);
    }
  }

  if (cornerCount == 4) return 0;
  return hasAHole;
}


static int getIsSolidAtSpot(const column_t *column, int spot) {
  if (!column) return 0;
  while (column->topdelta != 0xff) {
    if (spot < column->topdelta) return 0;
    if ((spot >= column->topdelta) && (spot <= column->topdelta + column->length)) return 1;
    column = (const column_t*)((const byte*)column + 3 + column->length + 1);
  }
  return 0;
}

static int getColumnEdgeSlope(const column_t *prevcolumn, const column_t *nextcolumn, int spot) {
  int holeToLeft = !getIsSolidAtSpot(prevcolumn, spot);
  int holeToRight = !getIsSolidAtSpot(nextcolumn, spot);

  if (holeToLeft && !holeToRight) return 1;
  if (!holeToLeft && holeToRight) return -1;
  return 0;
}

static void createPatch(int id) {
  rpatch_t *patch;
  const int patchNum = id;
  const patch_t *oldPatch = (const patch_t*)W_CacheLumpNum(patchNum);
  const column_t *oldColumn, *oldPrevColumn, *oldNextColumn;
  int x, y;
  int pixelDataSize;
  int columnsDataSize;
  int postsDataSize;
  int dataSize;
  int *numPostsInColumn;
  int numPostsTotal;
  const unsigned char *oldColumnPixelData;
  int numPostsUsedSoFar;
  int edgeSlope;

  patch = &patches[id];

  patch->width = oldPatch->width;
  patch->widthmask = 0;
  patch->height = oldPatch->height;
  patch->leftoffset = oldPatch->leftoffset;
  patch->topoffset = oldPatch->topoffset;
  patch->isNotTileable = getPatchIsNotTileable(oldPatch);


  pixelDataSize = (patch->width * patch->height + 4) & ~3;
  columnsDataSize = sizeof(rcolumn_t) * patch->width;


  numPostsInColumn = (int*)malloc(sizeof(int) * patch->width);
  numPostsTotal = 0;

  for (x=0; x<patch->width; x++) {
    oldColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[x]);
    numPostsInColumn[x] = 0;
    while (oldColumn->topdelta != 0xff) {
      numPostsInColumn[x]++;
      numPostsTotal++;
      oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
    }
  }

  postsDataSize = numPostsTotal * sizeof(rpost_t);


  dataSize = pixelDataSize + columnsDataSize + postsDataSize;
  patch->data = (unsigned char*)Z_Malloc(dataSize, PU_CACHE, (void **)&patch->data);
  memset(patch->data, 0, dataSize);


  patch->pixels = patch->data;
  patch->columns = (rcolumn_t*)((unsigned char*)patch->pixels + pixelDataSize);
  patch->posts = (rpost_t*)((unsigned char*)patch->columns + columnsDataSize);


  assert((((byte*)patch->posts  + numPostsTotal*sizeof(rpost_t)) - (byte*)patch->data) == dataSize);

  memset(patch->pixels, 0xff, (patch->width*patch->height));


  numPostsUsedSoFar = 0;
  for (x=0; x<patch->width; x++) {

    oldColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[x]);

    if (patch->isNotTileable) {

      if (x == 0) oldPrevColumn = 0;
      else oldPrevColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[x-1]);
      if (x == patch->width-1) oldNextColumn = 0;
      else oldNextColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[x+1]);
    }
    else {

      int prevColumnIndex = x-1;
      int nextColumnIndex = x+1;
      while (prevColumnIndex < 0) prevColumnIndex += patch->width;
      while (nextColumnIndex >= patch->width) nextColumnIndex -= patch->width;
      oldPrevColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[prevColumnIndex]);
      oldNextColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[nextColumnIndex]);
    }


    patch->columns[x].pixels = patch->pixels + (x*patch->height) + 0;
    patch->columns[x].numPosts = numPostsInColumn[x];
    patch->columns[x].posts = patch->posts + numPostsUsedSoFar;

    while (oldColumn->topdelta != 0xff) {

      patch->posts[numPostsUsedSoFar].topdelta = oldColumn->topdelta;
      patch->posts[numPostsUsedSoFar].length = oldColumn->length;
      patch->posts[numPostsUsedSoFar].slope = 0;

      edgeSlope = getColumnEdgeSlope(oldPrevColumn, oldNextColumn, oldColumn->topdelta);
      if (edgeSlope == 1) patch->posts[numPostsUsedSoFar].slope |= RDRAW_EDGESLOPE_TOP_UP;
      else if (edgeSlope == -1) patch->posts[numPostsUsedSoFar].slope |= RDRAW_EDGESLOPE_TOP_DOWN;

      edgeSlope = getColumnEdgeSlope(oldPrevColumn, oldNextColumn, oldColumn->topdelta+oldColumn->length);
      if (edgeSlope == 1) patch->posts[numPostsUsedSoFar].slope |= RDRAW_EDGESLOPE_BOT_UP;
      else if (edgeSlope == -1) patch->posts[numPostsUsedSoFar].slope |= RDRAW_EDGESLOPE_BOT_DOWN;


      oldColumnPixelData = (const byte *)oldColumn + 3;
      for (y=0; y<oldColumn->length; y++) {
        patch->pixels[x * patch->height + oldColumn->topdelta + y] = oldColumnPixelData[y];
      }

      oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
      numPostsUsedSoFar++;
    }
  }

  if (1 || patch->isNotTileable) {
    const rcolumn_t *column, *prevColumn;



    for (x=0; x<patch->width; x++) {


      column = R_GetPatchColumnClamped(patch, x);
      prevColumn = R_GetPatchColumnClamped(patch, x-1);

      if (column->pixels[0] == 0xff) {


        for (y=0; y<patch->height; y++) {
          if (column->pixels[y] != 0xff) {
            column->pixels[0] = column->pixels[y];
            break;
          }
        }
      }


      for (y=1; y<patch->height; y++) {

        if (column->pixels[y] != 0xff) continue;



        if (x && prevColumn->pixels[y-1] != 0xff) {

          column->pixels[y] = prevColumn->pixels[y];
        }
        else {

          column->pixels[y] = column->pixels[y-1];
        }
      }
    }



  }

  W_UnlockLumpNum(patchNum);
  free(numPostsInColumn);
}

typedef struct {
  unsigned short patches;
  unsigned short posts;
  unsigned short posts_used;
} count_t;

static void switchPosts(rpost_t *post1, rpost_t *post2) {
  rpost_t dummy;

  dummy.topdelta = post1->topdelta;
  dummy.length = post1->length;
  dummy.slope = post1->slope;
  post1->topdelta = post2->topdelta;
  post1->length = post2->length;
  post1->slope = post2->slope;
  post2->topdelta = dummy.topdelta;
  post2->length = dummy.length;
  post2->slope = dummy.slope;
}

static void removePostFromColumn(rcolumn_t *column, int post) {
  int i;
  if (post < column->numPosts)
    for (i=post; i<(column->numPosts-1); i++) {
      rpost_t *post1 = &column->posts[i];
      rpost_t *post2 = &column->posts[i+1];
      post1->topdelta = post2->topdelta;
      post1->length = post2->length;
      post1->slope = post2->slope;
    }
  column->numPosts--;
}


static void createTextureCompositePatch(int id) {
  rpatch_t *composite_patch;
  texture_t *texture;
  texpatch_t *texpatch;
  int patchNum;
  const patch_t *oldPatch;
  const column_t *oldColumn, *oldPrevColumn, *oldNextColumn;
  int i, x, y;
  int oy, count;
  int pixelDataSize;
  int columnsDataSize;
  int postsDataSize;
  int dataSize;
  int numPostsTotal;
  const unsigned char *oldColumnPixelData;
  int numPostsUsedSoFar;
  int edgeSlope;
  count_t *countsInColumn;

  composite_patch = &texture_composites[id];

  texture = textures[id];

  composite_patch->width = texture->width;
  composite_patch->height = texture->height;
  composite_patch->widthmask = texture->widthmask;
  composite_patch->leftoffset = 0;
  composite_patch->topoffset = 0;
  composite_patch->isNotTileable = 0;


  pixelDataSize = (composite_patch->width * composite_patch->height + 4) & ~3;
  columnsDataSize = sizeof(rcolumn_t) * composite_patch->width;


  countsInColumn = (count_t *)calloc(sizeof(count_t), composite_patch->width);
  numPostsTotal = 0;

  for (i=0; i<texture->patchcount; i++) {
    texpatch = &texture->patches[i];
    patchNum = texpatch->patch;
    oldPatch = (const patch_t*)W_CacheLumpNum(patchNum);

    for (x=0; x<oldPatch->width; x++) {
      int tx = texpatch->originx + x;

      if (tx < 0)
        continue;
      if (tx >= composite_patch->width)
        break;

      countsInColumn[tx].patches++;

      oldColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[x]);
      while (oldColumn->topdelta != 0xff) {
        countsInColumn[tx].posts++;
        numPostsTotal++;
        oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
      }
    }

    W_UnlockLumpNum(patchNum);
  }

  postsDataSize = numPostsTotal * sizeof(rpost_t);


  dataSize = pixelDataSize + columnsDataSize + postsDataSize;
  composite_patch->data = (unsigned char*)Z_Malloc(dataSize, PU_STATIC, (void **)&composite_patch->data);
  memset(composite_patch->data, 0, dataSize);


  composite_patch->pixels = composite_patch->data;
  composite_patch->columns = (rcolumn_t*)((unsigned char*)composite_patch->pixels + pixelDataSize);
  composite_patch->posts = (rpost_t*)((unsigned char*)composite_patch->columns + columnsDataSize);


  assert((((byte*)composite_patch->posts + numPostsTotal*sizeof(rpost_t)) - (byte*)composite_patch->data) == dataSize);

  memset(composite_patch->pixels, 0xff, (composite_patch->width*composite_patch->height));

  numPostsUsedSoFar = 0;

  for (x=0; x<texture->width; x++) {

      composite_patch->columns[x].pixels = composite_patch->pixels + (x*composite_patch->height);
      composite_patch->columns[x].numPosts = countsInColumn[x].posts;
      composite_patch->columns[x].posts = composite_patch->posts + numPostsUsedSoFar;
      numPostsUsedSoFar += countsInColumn[x].posts;
  }


  for (i=0; i<texture->patchcount; i++) {
    texpatch = &texture->patches[i];
    patchNum = texpatch->patch;
    oldPatch = (const patch_t*)W_CacheLumpNum(patchNum);

    for (x=0; x<oldPatch->width; x++) {
      int tx = texpatch->originx + x;

      if (tx < 0)
        continue;
      if (tx >= composite_patch->width)
        break;

      oldColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[x]);

      {

        int prevColumnIndex = x-1;
        int nextColumnIndex = x+1;
        while (prevColumnIndex < 0) prevColumnIndex += oldPatch->width;
        while (nextColumnIndex >= oldPatch->width) nextColumnIndex -= oldPatch->width;
        oldPrevColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[prevColumnIndex]);
        oldNextColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[nextColumnIndex]);
      }

      while (oldColumn->topdelta != 0xff) {
        rpost_t *post = &composite_patch->columns[tx].posts[countsInColumn[tx].posts_used];
        oldColumnPixelData = (const byte *)oldColumn + 3;
        oy = texpatch->originy;
        count = oldColumn->length;

        if (countsInColumn[tx].patches > 1) {


          if (i == 0) {


            for (y=0; y<count; y++) {
              int ty = oy + oldColumn->topdelta + y;
              if (ty < 0)
                continue;
              if (ty >= composite_patch->height)
                break;
              composite_patch->pixels[tx * composite_patch->height + ty] = oldColumnPixelData[y];
            }
          }

          if ((oy + oldColumn->topdelta) < 0) {
            count += oy;
            oy = 0;
          }
        } else {

          oy = 0;
        }

        post->topdelta = oldColumn->topdelta + oy;
        post->length = count;
        if ((post->topdelta + post->length) > composite_patch->height) {
          if (post->topdelta > composite_patch->height)
            post->length = 0;
          else
            post->length = composite_patch->height - post->topdelta;
        }
        if (post->topdelta < 0) {
          if ((post->topdelta + post->length) <= 0)
            post->length = 0;
          else
            post->length -= post->topdelta;
          post->topdelta = 0;
        }
        post->slope = 0;

        edgeSlope = getColumnEdgeSlope(oldPrevColumn, oldNextColumn, oldColumn->topdelta);
        if (edgeSlope == 1) post->slope |= RDRAW_EDGESLOPE_TOP_UP;
        else if (edgeSlope == -1) post->slope |= RDRAW_EDGESLOPE_TOP_DOWN;

        edgeSlope = getColumnEdgeSlope(oldPrevColumn, oldNextColumn, oldColumn->topdelta+count);
        if (edgeSlope == 1) post->slope |= RDRAW_EDGESLOPE_BOT_UP;
        else if (edgeSlope == -1) post->slope |= RDRAW_EDGESLOPE_BOT_DOWN;


        for (y=0; y<count; y++) {
          int ty = oy + oldColumn->topdelta + y;
          if (ty < 0)
            continue;
          if (ty >= composite_patch->height)
            break;
          composite_patch->pixels[tx * composite_patch->height + ty] = oldColumnPixelData[y];
        }

        oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
        countsInColumn[tx].posts_used++;
        assert(countsInColumn[tx].posts_used <= countsInColumn[tx].posts);
      }
    }

    W_UnlockLumpNum(patchNum);
  }

  for (x=0; x<texture->width; x++) {
    rcolumn_t *column;

    if (countsInColumn[x].patches <= 1)
      continue;


    column = &composite_patch->columns[x];

    i = 0;
    while (i<(column->numPosts-1)) {
      rpost_t *post1 = &column->posts[i];
      rpost_t *post2 = &column->posts[i+1];
      int length;

      if ((post2->topdelta - post1->topdelta) < 0)
        switchPosts(post1, post2);

      if ((post1->topdelta + post1->length) >= post2->topdelta) {
        length = (post1->length + post2->length) - ((post1->topdelta + post1->length) - post2->topdelta);
        if (post1->length < length) {
          post1->slope = post2->slope;
          post1->length = length;
        }
        removePostFromColumn(column, i+1);
        i = 0;
        continue;
      }
      i++;
    }
  }

  if (1 || composite_patch->isNotTileable) {
    const rcolumn_t *column, *prevColumn;



    for (x=0; x<composite_patch->width; x++) {


      column = R_GetPatchColumnClamped(composite_patch, x);
      prevColumn = R_GetPatchColumnClamped(composite_patch, x-1);

      if (column->pixels[0] == 0xff) {


        for (y=0; y<composite_patch->height; y++) {
          if (column->pixels[y] != 0xff) {
            column->pixels[0] = column->pixels[y];
            break;
          }
        }
      }


      for (y=1; y<composite_patch->height; y++) {

        if (column->pixels[y] != 0xff) continue;



        if (x && prevColumn->pixels[y-1] != 0xff) {

          column->pixels[y] = prevColumn->pixels[y];
        }
        else {

          column->pixels[y] = column->pixels[y-1];
        }
      }
    }

  }

  free(countsInColumn);
}


const rpatch_t *R_CachePatchNum(int id) {
  const int locks = 1;

  if (!patches)
    I_Error("R_CachePatchNum: Patches not initialized");

  if (!patches[id].data)
    createPatch(id);

  if (!patches[id].locks && locks) {
    Z_ChangeTag(patches[id].data,PU_STATIC);
  }
  patches[id].locks += locks;

  return &patches[id];
}

void R_UnlockPatchNum(int id)
{
  const int unlocks = 1;
  patches[id].locks -= unlocks;
  if (unlocks && !patches[id].locks)
    Z_ChangeTag(patches[id].data, PU_CACHE);
}


const rpatch_t *R_CacheTextureCompositePatchNum(int id) {
  const int locks = 1;

  if (!texture_composites)
    I_Error("R_CacheTextureCompositePatchNum: Composite patches not initialized");

  if (!texture_composites[id].data)
    createTextureCompositePatch(id);

  if (!texture_composites[id].locks && locks) {
    Z_ChangeTag(texture_composites[id].data,PU_STATIC);
  }
  texture_composites[id].locks += locks;

  return &texture_composites[id];

}

void R_UnlockTextureCompositePatchNum(int id)
{
  const int unlocks = 1;
  texture_composites[id].locks -= unlocks;
  if (unlocks && !texture_composites[id].locks)
    Z_ChangeTag(texture_composites[id].data, PU_CACHE);
}


const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnIndex) {
  while (columnIndex < 0) columnIndex += patch->width;
  columnIndex %= patch->width;
  return &patch->columns[columnIndex];
}


const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnIndex) {
  if (columnIndex < 0) columnIndex = 0;
  if (columnIndex >= patch->width) columnIndex = patch->width-1;
  return &patch->columns[columnIndex];
}


const rcolumn_t *R_GetPatchColumn(const rpatch_t *patch, int columnIndex) {
  if (patch->isNotTileable) return R_GetPatchColumnClamped(patch, columnIndex);
  else return R_GetPatchColumnWrapped(patch, columnIndex);
}

