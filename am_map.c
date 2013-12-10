#include "d_englsh.h"
#include "doomstat.h"
#include "st_stuff.h"
#include "r_main.h"
#include "p_setup.h"
#include "p_maputl.h"
#include "w_wad.h"
#include "v_video.h"
#include "p_spec.h"
#include "am_map.h"
#include "g_game.h"

int mapcolor_back;
int mapcolor_grid;
int mapcolor_wall;
int mapcolor_fchg;
int mapcolor_cchg;
int mapcolor_clsd;
int mapcolor_rkey;
int mapcolor_bkey;
int mapcolor_ykey;
int mapcolor_rdor;
int mapcolor_bdor;
int mapcolor_ydor;
int mapcolor_tele;
int mapcolor_secr;
int mapcolor_exit;
int mapcolor_unsn;
int mapcolor_flat;
int mapcolor_sprt;
int mapcolor_item;
int mapcolor_frnd;
int mapcolor_enemy;
int mapcolor_hair;
int mapcolor_sngl;
int mapcolor_plyr[4] = { 112, 88, 64, 32 };

int map_secret_after;

#define NC 0
#define BC 247
#define FB    0

#define INITSCALEMTOF (.2*FRACUNIT)
#define F_PANINC  4
#define M_ZOOMIN        ((int) (1.02*FRACUNIT))
#define M_ZOOMOUT       ((int) (FRACUNIT/1.02))
#define PLAYERRADIUS    (16*(1<<MAPBITS))
#define FTOM(x) FixedMul(((x)<<16),scale_ftom)
#define MTOF(x) (FixedMul((x),scale_mtof)>>16)
#define CXMTOF(x)  (f_x + MTOF((x)-m_x))
#define CYMTOF(y)  (f_y + (f_h - MTOF((y)-m_y)))

typedef struct
{
    mpoint_t a, b;
} mline_t;

#define R ((8*PLAYERRADIUS)/7)
mline_t player_arrow[] =
{
  { { -R+R/8, 0 }, { R, 0 } },
  { { R, 0 }, { R-R/2, R/4 } },
  { { R, 0 }, { R-R/2, -R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R
#define NUMPLYRLINES (sizeof(player_arrow)/sizeof(mline_t))

#define R ((8*PLAYERRADIUS)/7)
mline_t cheat_player_arrow[] =
{
  { { -R+R/8, 0 }, { R, 0 } },
  { { R, 0 }, { R-R/2, R/4 } },
  { { R, 0 }, { R-R/2, -R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } },
  { { -R/10-R/6, R/4}, {-R/10-R/6, -R/4} },
  { { -R/10-R/6, -R/4}, {-R/10-R/6-R/8, -R/4} },
  { { -R/10-R/6-R/8, -R/4}, {-R/10-R/6-R/8, -R/8} },
  { { -R/10, R/4}, {-R/10, -R/4}},
  { { -R/10, R/4}, {-R/10+R/8, R/4}},
  { { -R/10+R/4, R/4}, {-R/10+R/4, -R/4}},
  { { -R/10+R/4, R/4}, {-R/10+R/4+R/8, R/4}},
};
#undef R
#define NUMCHEATPLYRLINES (sizeof(cheat_player_arrow)/sizeof(mline_t))

#define R (FRACUNIT)
mline_t triangle_guy[] =
{
{ { (fixed_t)(-.867*R), (fixed_t)(-.5*R) }, { (fixed_t)( .867*R), (fixed_t)(-.5*R) } },
{ { (fixed_t)( .867*R), (fixed_t)(-.5*R) }, { (fixed_t)(0      ), (fixed_t)(    R) } },
{ { (fixed_t)(0      ), (fixed_t)(    R) }, { (fixed_t)(-.867*R), (fixed_t)(-.5*R) } }
};
#undef R
#define NUMTRIANGLEGUYLINES (sizeof(triangle_guy)/sizeof(mline_t))


#define R (FRACUNIT)
mline_t cross_mark[] =
{
  { { -R, 0 }, { R, 0} },
  { { 0, -R }, { 0, R } },
};
#undef R
#define NUMCROSSMARKLINES (sizeof(cross_mark)/sizeof(mline_t))


#define R (FRACUNIT)
mline_t thintriangle_guy[] =
{
{ { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(    R), (fixed_t)(    0) } },
{ { (fixed_t)(    R), (fixed_t)(    0) }, { (fixed_t)(-.5*R), (fixed_t)( .7*R) } },
{ { (fixed_t)(-.5*R), (fixed_t)( .7*R) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
};
#undef R
#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy)/sizeof(mline_t))

static int leveljuststarted = 1;

enum automapmode_e automapmode;


static int  f_x;
static int  f_y;


static int  f_w;
static int  f_h;

static mpoint_t m_paninc;
static fixed_t mtof_zoommul;
static fixed_t ftom_zoommul;

static fixed_t m_x, m_y;
static fixed_t m_x2, m_y2;




static fixed_t  m_w;
static fixed_t  m_h;


static fixed_t  min_x;
static fixed_t  min_y;
static fixed_t  max_x;
static fixed_t  max_y;

static fixed_t  max_w;
static fixed_t  max_h;


static fixed_t  min_w;
static fixed_t  min_h;


static fixed_t  min_scale_mtof;
static fixed_t  max_scale_mtof;


static fixed_t old_m_w, old_m_h;
static fixed_t old_m_x, old_m_y;


static mpoint_t f_oldloc;


static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;

static fixed_t scale_ftom;

static player_t *plr;




mpoint_t *markpoints = NULL;
int markpointnum = 0;
int markpointnum_max = 0;

static boolean stopped = true;








static void AM_activateNewScale(void)
{
  m_x += m_w/2;
  m_y += m_h/2;
  m_w = FTOM(f_w);
  m_h = FTOM(f_h);
  m_x -= m_w/2;
  m_y -= m_h/2;
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}









static void AM_saveScaleAndLoc(void)
{
  old_m_x = m_x;
  old_m_y = m_y;
  old_m_w = m_w;
  old_m_h = m_h;
}









static void AM_restoreScaleAndLoc(void)
{
  m_w = old_m_w;
  m_h = old_m_h;
  if (!(automapmode & am_follow))
  {
    m_x = old_m_x;
    m_y = old_m_y;
  }
  else
  {
    m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
    m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;
  }
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;

  scale_mtof = FixedDiv(f_w<<FRACBITS, m_w);
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

static void AM_addMark(void)
{

  if (markpointnum >= markpointnum_max)
    markpoints = realloc(markpoints,
                        (markpointnum_max = markpointnum_max ?
                         markpointnum_max*2 : 16) * sizeof(*markpoints));

  markpoints[markpointnum].x = m_x + m_w/2;
  markpoints[markpointnum].y = m_y + m_h/2;
  markpointnum++;
}

static void AM_findMinMaxBoundaries(void)
{
  int i;
  fixed_t a;
  fixed_t b;

  min_x = min_y =  INT_MAX;
  max_x = max_y = -INT_MAX;

  for (i=0;i<numvertexes;i++)
  {
    if (vertexes[i].x < min_x)
      min_x = vertexes[i].x;
    else if (vertexes[i].x > max_x)
      max_x = vertexes[i].x;

    if (vertexes[i].y < min_y)
      min_y = vertexes[i].y;
    else if (vertexes[i].y > max_y)
      max_y = vertexes[i].y;
  }

  max_w = (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS);
  max_h = (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS);

  min_w = 2*PLAYERRADIUS;
  min_h = 2*PLAYERRADIUS;

  a = FixedDiv(f_w<<FRACBITS, max_w);
  b = FixedDiv(f_h<<FRACBITS, max_h);

  min_scale_mtof = a < b ? a : b;
  max_scale_mtof = FixedDiv(f_h<<FRACBITS, 2*PLAYERRADIUS);
}

static void AM_changeWindowLoc(void)
{
  if (m_paninc.x || m_paninc.y)
  {
    automapmode &= ~am_follow;
    f_oldloc.x = INT_MAX;
  }

  m_x += m_paninc.x;
  m_y += m_paninc.y;

  if (m_x + m_w/2 > max_x)
    m_x = max_x - m_w/2;
  else if (m_x + m_w/2 < min_x)
    m_x = min_x - m_w/2;

  if (m_y + m_h/2 > max_y)
    m_y = max_y - m_h/2;
  else if (m_y + m_h/2 < min_y)
    m_y = min_y - m_h/2;

  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}

static void AM_initVariables(void)
{
  int pnum;
  static event_t st_notify = { ev_keyup, AM_MSGENTERED, 0, 0 };

  automapmode |= am_active;

  f_oldloc.x = INT_MAX;

  m_paninc.x = m_paninc.y = 0;
  ftom_zoommul = FRACUNIT;
  mtof_zoommul = FRACUNIT;

  m_w = FTOM(f_w);
  m_h = FTOM(f_h);


  if (!playeringame[pnum = consoleplayer])
  for (pnum=0;pnum<MAXPLAYERS;pnum++)
    if (playeringame[pnum])
  break;

  plr = &players[pnum];
  m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
  m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;
  AM_changeWindowLoc();


  old_m_x = m_x;
  old_m_y = m_y;
  old_m_w = m_w;
  old_m_h = m_h;


  ST_Responder(&st_notify);
}

void AM_ClearMarks(void)
{
  markpointnum = 0;
}

static void AM_LevelInit(void)
{
  leveljuststarted = 0;

  f_x = f_y = 0;
  f_w = SCREENWIDTH;
  f_h = SCREENHEIGHT-ST_SCALED_HEIGHT;

  AM_findMinMaxBoundaries();
  scale_mtof = FixedDiv(min_scale_mtof, (int) (0.7*FRACUNIT));
  if (scale_mtof > max_scale_mtof)
    scale_mtof = min_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

void AM_Stop (void)
{
  static event_t st_notify = { 0, ev_keyup, AM_MSGEXITED, 0 };

  automapmode &= ~am_active;
  ST_Responder(&st_notify);
  stopped = true;
}

void AM_Start(void)
{
  static int lastlevel = -1, lastepisode = -1;

  if (!stopped)
    AM_Stop();
  stopped = false;
  if (lastlevel != gamemap || lastepisode != gameepisode)
  {
    AM_LevelInit();
    lastlevel = gamemap;
    lastepisode = gameepisode;
  }
  AM_initVariables();
}

static void AM_minOutWindowScale(void)
{
  scale_mtof = min_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  AM_activateNewScale();
}

static void AM_maxOutWindowScale(void)
{
  scale_mtof = max_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  AM_activateNewScale();
}

boolean AM_Responder
( event_t*  ev )
{
  int rc;
  static int bigstate=0;
  int ch;

  rc = false;

  if (!(automapmode & am_active))
  {
    if (ev->type == ev_keydown && ev->data1 == key_map)
    {
      AM_Start();
      rc = true;
    }
  }
  else if (ev->type == ev_keydown)
  {
    rc = true;
    ch = ev->data1;
    if (ch == key_map_right)
      if (!(automapmode & am_follow))
        m_paninc.x = FTOM(F_PANINC);
      else
        rc = false;
    else if (ch == key_map_left)
      if (!(automapmode & am_follow))
          m_paninc.x = -FTOM(F_PANINC);
      else
          rc = false;
    else if (ch == key_map_up)
      if (!(automapmode & am_follow))
          m_paninc.y = FTOM(F_PANINC);
      else
          rc = false;
    else if (ch == key_map_down)
      if (!(automapmode & am_follow))
          m_paninc.y = -FTOM(F_PANINC);
      else
          rc = false;
    else if (ch == key_map_zoomout)
    {
      mtof_zoommul = M_ZOOMOUT;
      ftom_zoommul = M_ZOOMIN;
    }
    else if (ch == key_map_zoomin)
    {
      mtof_zoommul = M_ZOOMIN;
      ftom_zoommul = M_ZOOMOUT;
    }
    else if (ch == key_map)
    {
      bigstate = 0;
      AM_Stop ();
    }
    else if (ch == key_map_gobig)
    {
      bigstate = !bigstate;
      if (bigstate)
      {
        AM_saveScaleAndLoc();
        AM_minOutWindowScale();
      }
      else
        AM_restoreScaleAndLoc();
    }
    else if (ch == key_map_follow)
    {
      automapmode ^= am_follow;
      f_oldloc.x = INT_MAX;

      plr->message = (automapmode & am_follow) ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF;
    }
    else if (ch == key_map_grid)
    {
      automapmode ^= am_grid;

      plr->message = (automapmode & am_grid) ? AMSTR_GRIDON : AMSTR_GRIDOFF;
    }
    else if (ch == key_map_mark)
    {
      /* Ty 03/27/98 - *not* externalized     
       * cph 2001/11/20 - use doom_printf so we don't have our own buffer */
      doom_printf("%s %d", AMSTR_MARKEDSPOT, markpointnum);
      AM_addMark();
    }
    else if (ch == key_map_clear)
    {
      AM_ClearMarks();
      plr->message = AMSTR_MARKSCLEARED;
    }
    else if (ch == key_map_rotate) {
      automapmode ^= am_rotate;
      plr->message = (automapmode & am_rotate) ? AMSTR_ROTATEON : AMSTR_ROTATEOFF;
    }
    else if (ch == key_map_overlay) {
      automapmode ^= am_overlay;
      plr->message = (automapmode & am_overlay) ? AMSTR_OVERLAYON : AMSTR_OVERLAYOFF;
    }
    else
    {
      rc = false;
    }
  }
  else if (ev->type == ev_keyup)
  {
    rc = false;
    ch = ev->data1;
    if (ch == key_map_right)
    {
      if (!(automapmode & am_follow))
          m_paninc.x = 0;
    }
    else if (ch == key_map_left)
    {
      if (!(automapmode & am_follow))
          m_paninc.x = 0;
    }
    else if (ch == key_map_up)
    {
      if (!(automapmode & am_follow))
          m_paninc.y = 0;
    }
    else if (ch == key_map_down)
    {
      if (!(automapmode & am_follow))
          m_paninc.y = 0;
    }
    else if ((ch == key_map_zoomout) || (ch == key_map_zoomin))
    {
      mtof_zoommul = FRACUNIT;
      ftom_zoommul = FRACUNIT;
    }
  }
  return rc;
}

static void AM_rotate(fixed_t* x,  fixed_t* y, angle_t a, fixed_t xorig, fixed_t yorig)
{
  fixed_t tmpx;


  xorig>>=FRACTOMAPBITS;
  yorig>>=FRACTOMAPBITS;

  tmpx =
    FixedMul(*x - xorig,finecosine[a>>ANGLETOFINESHIFT])
      - FixedMul(*y - yorig,finesine[a>>ANGLETOFINESHIFT]);

  *y   = yorig +
    FixedMul(*x - xorig,finesine[a>>ANGLETOFINESHIFT])
      + FixedMul(*y - yorig,finecosine[a>>ANGLETOFINESHIFT]);

  *x = tmpx + xorig;
}

static void AM_changeWindowScale(void)
{

  scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

  if (scale_mtof < min_scale_mtof)
    AM_minOutWindowScale();
  else if (scale_mtof > max_scale_mtof)
    AM_maxOutWindowScale();
  else
    AM_activateNewScale();
}

static void AM_doFollowPlayer(void)
{
  if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
  {
    m_x = FTOM(MTOF(plr->mo->x >> FRACTOMAPBITS)) - m_w/2;
    m_y = FTOM(MTOF(plr->mo->y >> FRACTOMAPBITS)) - m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
    f_oldloc.x = plr->mo->x;
    f_oldloc.y = plr->mo->y;
  }
}

void AM_Ticker(void)
{
  if (!(automapmode & am_active))
    return;

  if (automapmode & am_follow)
    AM_doFollowPlayer();


  if (ftom_zoommul != FRACUNIT)
    AM_changeWindowScale();


  if (m_paninc.x || m_paninc.y)
    AM_changeWindowLoc();
}

static boolean AM_clipMline
( mline_t*  ml,
  fline_t*  fl )
{
  enum
  {
    LEFT    =1,
    RIGHT   =2,
    BOTTOM  =4,
    TOP     =8
  };

  register int outcode1 = 0;
  register int outcode2 = 0;
  register int outside;

  fpoint_t  tmp;
  int   dx;
  int   dy;


#define DOOUTCODE(oc, mx, my) \
  (oc) = 0; \
  if ((my) < 0) (oc) |= TOP; \
  else if ((my) >= f_h) (oc) |= BOTTOM; \
  if ((mx) < 0) (oc) |= LEFT; \
  else if ((mx) >= f_w) (oc) |= RIGHT;



  if (ml->a.y > m_y2)
  outcode1 = TOP;
  else if (ml->a.y < m_y)
  outcode1 = BOTTOM;

  if (ml->b.y > m_y2)
  outcode2 = TOP;
  else if (ml->b.y < m_y)
  outcode2 = BOTTOM;

  if (outcode1 & outcode2)
  return false;

  if (ml->a.x < m_x)
  outcode1 |= LEFT;
  else if (ml->a.x > m_x2)
  outcode1 |= RIGHT;

  if (ml->b.x < m_x)
  outcode2 |= LEFT;
  else if (ml->b.x > m_x2)
  outcode2 |= RIGHT;

  if (outcode1 & outcode2)
  return false;


  fl->a.x = CXMTOF(ml->a.x);
  fl->a.y = CYMTOF(ml->a.y);
  fl->b.x = CXMTOF(ml->b.x);
  fl->b.y = CYMTOF(ml->b.y);

  DOOUTCODE(outcode1, fl->a.x, fl->a.y);
  DOOUTCODE(outcode2, fl->b.x, fl->b.y);

  if (outcode1 & outcode2)
  return false;

  while (outcode1 | outcode2)
  {


    if (outcode1)
      outside = outcode1;
    else
      outside = outcode2;


    if (outside & TOP)
    {
      dy = fl->a.y - fl->b.y;
      dx = fl->b.x - fl->a.x;
      tmp.x = fl->a.x + (dx*(fl->a.y))/dy;
      tmp.y = 0;
    }
    else if (outside & BOTTOM)
    {
      dy = fl->a.y - fl->b.y;
      dx = fl->b.x - fl->a.x;
      tmp.x = fl->a.x + (dx*(fl->a.y-f_h))/dy;
      tmp.y = f_h-1;
    }
    else if (outside & RIGHT)
    {
      dy = fl->b.y - fl->a.y;
      dx = fl->b.x - fl->a.x;
      tmp.y = fl->a.y + (dy*(f_w-1 - fl->a.x))/dx;
      tmp.x = f_w-1;
    }
    else if (outside & LEFT)
    {
      dy = fl->b.y - fl->a.y;
      dx = fl->b.x - fl->a.x;
      tmp.y = fl->a.y + (dy*(-fl->a.x))/dx;
      tmp.x = 0;
    }

    if (outside == outcode1)
    {
      fl->a = tmp;
      DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    }
    else
    {
      fl->b = tmp;
      DOOUTCODE(outcode2, fl->b.x, fl->b.y);
    }

    if (outcode1 & outcode2)
      return false;
  }

  return true;
}
#undef DOOUTCODE

static void AM_drawMline
( mline_t*  ml,
  int   color )
{
  static fline_t fl;

  if (color==-1)
    return;
  if (color==247)
    color=0;

  if (AM_clipMline(ml, &fl))
    V_DrawLine(&fl, color);
}

static void AM_drawGrid(int color)
{
  fixed_t x, y;
  fixed_t start, end;
  mline_t ml;


  start = m_x;
  if ((start-bmaporgx)%(MAPBLOCKUNITS<<MAPBITS))
    start += (MAPBLOCKUNITS<<MAPBITS)
      - ((start-bmaporgx)%(MAPBLOCKUNITS<<MAPBITS));
  end = m_x + m_w;


  ml.a.y = m_y;
  ml.b.y = m_y+m_h;
  for (x=start; x<end; x+=(MAPBLOCKUNITS<<MAPBITS))
  {
    ml.a.x = x;
    ml.b.x = x;
    AM_drawMline(&ml, color);
  }


  start = m_y;
  if ((start-bmaporgy)%(MAPBLOCKUNITS<<MAPBITS))
    start += (MAPBLOCKUNITS<<MAPBITS)
      - ((start-bmaporgy)%(MAPBLOCKUNITS<<MAPBITS));
  end = m_y + m_h;


  ml.a.x = m_x;
  ml.b.x = m_x + m_w;
  for (y=start; y<end; y+=(MAPBLOCKUNITS<<MAPBITS))
  {
    ml.a.y = y;
    ml.b.y = y;
    AM_drawMline(&ml, color);
  }
}

static int AM_DoorColor(int type)
{
  if (GenLockedBase <= type && type< GenDoorBase)
  {
    type -= GenLockedBase;
    type = (type & LockedKey) >> LockedKeyShift;
    if (!type || type==7)
      return 3;
    else return (type-1)%3;
  }
  switch (type)
  {
    case 26: case 32: case 99: case 133:
      /*bluekey*/
      return 1;
    case 27: case 34: case 136: case 137:
      /*yellowkey*/
      return 2;
    case 28: case 33: case 134: case 135:
      /*redkey*/
      return 0;
    default:
      return -1;
  }
}

static void AM_drawWalls(void)
{
  int i;
  static mline_t l;


  for (i=0;i<numlines;i++)
  {
    l.a.x = lines[i].v1->x >> FRACTOMAPBITS;
    l.a.y = lines[i].v1->y >> FRACTOMAPBITS;
    l.b.x = lines[i].v2->x >> FRACTOMAPBITS;
    l.b.y = lines[i].v2->y >> FRACTOMAPBITS;

    if (automapmode & am_rotate) {
      AM_rotate(&l.a.x, &l.a.y, ANG90-plr->mo->angle, plr->mo->x, plr->mo->y);
      AM_rotate(&l.b.x, &l.b.y, ANG90-plr->mo->angle, plr->mo->x, plr->mo->y);
    }


    if ((lines[i].flags & ML_MAPPED))
    {
      if ((lines[i].flags & ML_DONTDRAW))
        continue;
      {
        /* cph - show keyed doors and lines */
        int amd;
        if ((mapcolor_bdor || mapcolor_ydor || mapcolor_rdor) &&
            !(lines[i].flags & ML_SECRET) &&    /* non-secret */
          (amd = AM_DoorColor(lines[i].special)) != -1
        )
        {
          {
            switch (amd) /* closed keyed door */
            {
              case 1:
                /*bluekey*/
                AM_drawMline(&l,
                  mapcolor_bdor? mapcolor_bdor : mapcolor_cchg);
                continue;
              case 2:
                /*yellowkey*/
                AM_drawMline(&l,
                  mapcolor_ydor? mapcolor_ydor : mapcolor_cchg);
                continue;
              case 0:
                /*redkey*/
                AM_drawMline(&l,
                  mapcolor_rdor? mapcolor_rdor : mapcolor_cchg);
                continue;
              case 3:
                /*any or all*/
                AM_drawMline(&l,
                  mapcolor_clsd? mapcolor_clsd : mapcolor_cchg);
                continue;
            }
          }
        }
      }
      if /* jff 4/23/98 add exit lines to automap */
        (
          mapcolor_exit &&
          (
            lines[i].special==11 ||
            lines[i].special==52 ||
            lines[i].special==197 ||
            lines[i].special==51  ||
            lines[i].special==124 ||
            lines[i].special==198
          )
        ) {
          AM_drawMline(&l, mapcolor_exit); /* exit line */
          continue;
        }

      if (!lines[i].backsector)
      {

        if (mapcolor_secr &&
            (
             (
              map_secret_after &&
              P_WasSecret(lines[i].frontsector) &&
              !P_IsSecret(lines[i].frontsector)
             )
             ||
             (
              !map_secret_after &&
              P_WasSecret(lines[i].frontsector)
             )
            )
          )
          AM_drawMline(&l, mapcolor_secr);
        else
          AM_drawMline(&l, mapcolor_wall);
      }
      else /* now for 2S lines */
      {

        if
        (
            mapcolor_tele && !(lines[i].flags & ML_SECRET) &&
            (lines[i].special == 39 || lines[i].special == 97 ||
            lines[i].special == 125 || lines[i].special == 126)
        )
        {
          AM_drawMline(&l, mapcolor_tele);
        }
        else if (lines[i].flags & ML_SECRET)
        {
          AM_drawMline(&l, mapcolor_wall);
        }
        else if
        (
            mapcolor_clsd &&
            !(lines[i].flags & ML_SECRET) &&
            ((lines[i].backsector->floorheight==lines[i].backsector->ceilingheight) ||
            (lines[i].frontsector->floorheight==lines[i].frontsector->ceilingheight))
        )
        {
          AM_drawMline(&l, mapcolor_clsd);
        }
        else if
        (
            mapcolor_secr &&
            (
              (map_secret_after &&
               (
                (P_WasSecret(lines[i].frontsector)
                 && !P_IsSecret(lines[i].frontsector)) ||
                (P_WasSecret(lines[i].backsector)
                 && !P_IsSecret(lines[i].backsector))
               )
              )
              ||
              (
                !map_secret_after &&
                 (P_WasSecret(lines[i].frontsector) ||
                  P_WasSecret(lines[i].backsector))
              )
            )
        )
        {
          AM_drawMline(&l, mapcolor_secr);
        }
        else if (lines[i].backsector->floorheight !=
                  lines[i].frontsector->floorheight)
        {
          AM_drawMline(&l, mapcolor_fchg);
        }
        else if (lines[i].backsector->ceilingheight !=
                  lines[i].frontsector->ceilingheight)
        {
          AM_drawMline(&l, mapcolor_cchg);
        }
        else if (mapcolor_flat)
        {
          AM_drawMline(&l, mapcolor_flat);
        }
      }
    }
    else if (plr->powers[pw_allmap])
    {
      if (!(lines[i].flags & ML_DONTDRAW))
      {
        if
        (
          mapcolor_flat
          ||
          !lines[i].backsector
          ||
          lines[i].backsector->floorheight
          != lines[i].frontsector->floorheight
          ||
          lines[i].backsector->ceilingheight
          != lines[i].frontsector->ceilingheight
        )
          AM_drawMline(&l, mapcolor_unsn);
      }
    }
  }
}

static void AM_drawLineCharacter
( mline_t*  lineguy,
  int   lineguylines,
  fixed_t scale,
  angle_t angle,
  int   color,
  fixed_t x,
  fixed_t y )
{
  int   i;
  mline_t l;

  if (automapmode & am_rotate) angle -= plr->mo->angle - ANG90;

  for (i=0;i<lineguylines;i++)
  {
    l.a.x = lineguy[i].a.x;
    l.a.y = lineguy[i].a.y;

    if (scale)
    {
      l.a.x = FixedMul(scale, l.a.x);
      l.a.y = FixedMul(scale, l.a.y);
    }

    if (angle)
      AM_rotate(&l.a.x, &l.a.y, angle, 0, 0);

    l.a.x += x;
    l.a.y += y;

    l.b.x = lineguy[i].b.x;
    l.b.y = lineguy[i].b.y;

    if (scale)
    {
      l.b.x = FixedMul(scale, l.b.x);
      l.b.y = FixedMul(scale, l.b.y);
    }

    if (angle)
      AM_rotate(&l.b.x, &l.b.y, angle, 0, 0);

    l.b.x += x;
    l.b.y += y;

    AM_drawMline(&l, color);
  }
}

static void AM_drawPlayers(void)
{
  int   i;

  AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, plr->mo->angle, mapcolor_sngl, plr->mo->x >> FRACTOMAPBITS, plr->mo->y >> FRACTOMAPBITS);

}

static void AM_drawThings(void)
{
  int   i;
  mobj_t* t;


  for (i=0;i<numsectors;i++)
  {
    t = sectors[i].thinglist;
    while (t)
    {
      fixed_t x = t->x >> FRACTOMAPBITS, y = t->y >> FRACTOMAPBITS;

      if (automapmode & am_rotate)
  AM_rotate(&x, &y, ANG90-plr->mo->angle, plr->mo->x, plr->mo->y);


      if (mapcolor_rkey || mapcolor_ykey || mapcolor_bkey)
      {
        switch(t->info->doomednum)
        {

          case 38: case 13:
            AM_drawLineCharacter
            (
              cross_mark,
              NUMCROSSMARKLINES,
              16<<MAPBITS,
              t->angle,
              mapcolor_rkey!=-1? mapcolor_rkey : mapcolor_sprt,
              x, y
            );
            t = t->snext;
            continue;
          case 39: case 6:
            AM_drawLineCharacter
            (
              cross_mark,
              NUMCROSSMARKLINES,
              16<<MAPBITS,
              t->angle,
              mapcolor_ykey!=-1? mapcolor_ykey : mapcolor_sprt,
              x, y
            );
            t = t->snext;
            continue;
          case 40: case 5:
            AM_drawLineCharacter
            (
              cross_mark,
              NUMCROSSMARKLINES,
              16<<MAPBITS,
              t->angle,
              mapcolor_bkey!=-1? mapcolor_bkey : mapcolor_sprt,
              x, y
            );
            t = t->snext;
            continue;
          default:
            break;
        }
      }
      AM_drawLineCharacter
      (
        thintriangle_guy,
        NUMTHINTRIANGLEGUYLINES,
        16<<MAPBITS,
        t->angle,
        t->flags & MF_FRIEND && !t->player ? mapcolor_frnd : ((t->flags & (MF_COUNTKILL | MF_CORPSE)) == MF_COUNTKILL) ? mapcolor_enemy : t->flags & MF_COUNTITEM ? mapcolor_item : mapcolor_sprt, x, y);
        t = t->snext;
    }
  }
}

static void AM_drawMarks(void)
{

    int i;

    for (i = 0; i < markpointnum; i++)
    if (markpoints[i].x != -1)
    {
      int w = 5;
      int h = 6;
      int fx = markpoints[i].x;
      int fy = markpoints[i].y;
      int j = i;

      if (automapmode & am_rotate)
        AM_rotate(&fx, &fy, ANG90-plr->mo->angle, plr->mo->x, plr->mo->y);

      fx = CXMTOF(fx); fy = CYMTOF(fy);

      do
      {
        int d = j % 10;
        if (d==1)
          fx++;

        if (fx >= f_x && fx < f_w - w && fy >= f_y && fy < f_h - h) {

    char namebuf[] = { 'A', 'M', 'M', 'N', 'U', 'M', '0'+d, 0 };

          V_DrawNamePatch(fx, fy, FB, namebuf, CR_DEFAULT, VPT_NONE);
  }
        fx -= w-1;
        j /= 10;
      }
      while (j>0);
    }
}

inline static void AM_drawCrosshair(int color)
{

    fline_t line;

    line.a.x = (f_w / 2) - 1;
    line.a.y = (f_h / 2);
    line.b.x = (f_w / 2) + 1;
    line.b.y = (f_h / 2);

    V_DrawLine(&line, color);

    line.a.x = (f_w / 2);
    line.a.y = (f_h / 2) - 1;
    line.b.x = (f_w / 2);
    line.b.y = (f_h / 2) + 1;

    V_DrawLine(&line, color);

}

void AM_Drawer (void)
{

    if (!(automapmode & am_active))
        return;

    if (!(automapmode & am_overlay))
        V_FillRect(FB, f_x, f_y, f_w, f_h, (byte)mapcolor_back);

    if (automapmode & am_grid)
        AM_drawGrid(mapcolor_grid);

    AM_drawWalls();
    AM_drawPlayers();
    AM_drawThings();
        
    AM_drawCrosshair(mapcolor_hair);
    AM_drawMarks();

}

