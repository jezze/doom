#include "doomstat.h"
#include "d_event.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_saveg.h"
#include "m_random.h"
#include "am_map.h"
#include "p_enemy.h"
#include "lprintf.h"
#include "z_zone.h"

byte *save_p;

#define PADSAVEP()    do { save_p += (4 - ((long) save_p & 3)) & 3; } while (0)

void P_ArchivePlayers (void)
{
  int i;

  CheckSaveGame(sizeof(player_t) * MAXPLAYERS);
  for (i=0 ; i<MAXPLAYERS ; i++)
    if (playeringame[i])
      {
        int      j;
        player_t *dest;

        PADSAVEP();
        dest = (player_t *) save_p;
        memcpy(dest, &players[i], sizeof(player_t));
        save_p += sizeof(player_t);
        for (j=0; j<NUMPSPRITES; j++)
          if (dest->psprites[j].state)
            dest->psprites[j].state =
              (state_t *)(dest->psprites[j].state-states);
      }
}

void P_UnArchivePlayers (void)
{
  int i;

  for (i=0 ; i<MAXPLAYERS ; i++)
    if (playeringame[i])
      {
        int j;

        PADSAVEP();

        memcpy(&players[i], save_p, sizeof(player_t));
        save_p += sizeof(player_t);


        players[i].mo = NULL;
        players[i].message = NULL;
        players[i].attacker = NULL;

        for (j=0 ; j<NUMPSPRITES ; j++)
          if (players[i]. psprites[j].state)
            players[i]. psprites[j].state = &states[(long)players[i].psprites[j].state];
      }
}

void P_ArchiveWorld (void)
{
  int            i;
  const sector_t *sec;
  const line_t   *li;
  const side_t   *si;
  short          *put;

  size_t size =
    (sizeof(short)*5 + sizeof sec->floorheight + sizeof sec->ceilingheight)
    * numsectors + sizeof(short)*3*numlines + 4;

  for (i=0; i<numlines; i++)
    {
      if (lines[i].sidenum[0] != NO_INDEX)
        size +=
    sizeof(short)*3 + sizeof si->textureoffset + sizeof si->rowoffset;
      if (lines[i].sidenum[1] != NO_INDEX)
  size +=
    sizeof(short)*3 + sizeof si->textureoffset + sizeof si->rowoffset;
    }

  CheckSaveGame(size);

  PADSAVEP();

  put = (short *)save_p;


  for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
    {

      memcpy(put, &sec->floorheight, sizeof sec->floorheight);
      put = (void *)((char *) put + sizeof sec->floorheight);
      memcpy(put, &sec->ceilingheight, sizeof sec->ceilingheight);
      put = (void *)((char *) put + sizeof sec->ceilingheight);

      *put++ = sec->floorpic;
      *put++ = sec->ceilingpic;
      *put++ = sec->lightlevel;
      *put++ = sec->special;
      *put++ = sec->tag;
    }


  for (i=0, li = lines ; i<numlines ; i++,li++)
    {
      int j;

      *put++ = li->flags;
      *put++ = li->special;
      *put++ = li->tag;

      for (j=0; j<2; j++)
        if (li->sidenum[j] != NO_INDEX)
          {
      si = &sides[li->sidenum[j]];




      memcpy(put, &si->textureoffset, sizeof si->textureoffset);
      put = (void *)((char *) put + sizeof si->textureoffset);
      memcpy(put, &si->rowoffset, sizeof si->rowoffset);
      put = (void *)((char *) put + sizeof si->rowoffset);

            *put++ = si->toptexture;
            *put++ = si->bottomtexture;
            *put++ = si->midtexture;
          }
    }
  save_p = (byte *) put;
}

void P_UnArchiveWorld (void)
{
  int          i;
  sector_t     *sec;
  line_t       *li;
  const short  *get;

  PADSAVEP();

  get = (short *) save_p;

  for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
    {


      memcpy(&sec->floorheight, get, sizeof sec->floorheight);
      get = (void *)((char *) get + sizeof sec->floorheight);
      memcpy(&sec->ceilingheight, get, sizeof sec->ceilingheight);
      get = (void *)((char *) get + sizeof sec->ceilingheight);

      sec->floorpic = *get++;
      sec->ceilingpic = *get++;
      sec->lightlevel = *get++;
      sec->special = *get++;
      sec->tag = *get++;
      sec->ceilingdata = 0;
      sec->floordata = 0;
      sec->lightingdata = 0;
      sec->soundtarget = 0;
    }


  for (i=0, li = lines ; i<numlines ; i++,li++)
    {
      int j;

      li->flags = *get++;
      li->special = *get++;
      li->tag = *get++;
      for (j=0 ; j<2 ; j++)
        if (li->sidenum[j] != NO_INDEX)
          {
            side_t *si = &sides[li->sidenum[j]];



      memcpy(&si->textureoffset, get, sizeof si->textureoffset);
      get = (void *)((char *) get + sizeof si->textureoffset);
      memcpy(&si->rowoffset, get, sizeof si->rowoffset);
      get = (void *)((char *) get + sizeof si->rowoffset);

            si->toptexture = *get++;
            si->bottomtexture = *get++;
            si->midtexture = *get++;
          }
    }
  save_p = (byte *) get;
}





typedef enum {
  tc_end,
  tc_mobj
} thinkerclass_t;




static long number_of_thinkers;

void P_ThinkerToIndex(void)
  {
  thinker_t *th;





  number_of_thinkers = 0;
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function == P_MobjThinker)
      th->prev = (thinker_t *) ++number_of_thinkers;
  }




void P_IndexToThinker(void)
  {

  thinker_t *th;
  thinker_t *prev = &thinkercap;

  for (th = thinkercap.next ; th != &thinkercap ; prev=th, th=th->next)
    th->prev = prev;
  }


void P_ArchiveThinkers (void)
{
  thinker_t *th;

  CheckSaveGame(sizeof brain);
  memcpy(save_p, &brain, sizeof brain);
  save_p += sizeof brain;

  CheckSaveGame(number_of_thinkers*(sizeof(mobj_t)-3*sizeof(fixed_t)+4+3*sizeof(void*)) +1);


  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function == P_MobjThinker)
      {
        mobj_t *mobj;

        *save_p++ = tc_mobj;
        PADSAVEP();
        mobj = (mobj_t *)save_p;
        memcpy (mobj, th, sizeof(*mobj) - 2*sizeof(void*));
        save_p += sizeof(*mobj) - 2*sizeof(void*) - 4*sizeof(fixed_t);
        memset (save_p, 0, 5*sizeof(void*));
        mobj->state = (state_t *)(mobj->state - states);

        if (mobj->target)
          mobj->target = mobj->target->thinker.function ==
            P_MobjThinker ?
            (mobj_t *) mobj->target->thinker.prev : NULL;

        if (mobj->tracer)
          mobj->tracer = mobj->tracer->thinker.function ==
            P_MobjThinker ?
            (mobj_t *) mobj->tracer->thinker.prev : NULL;

        if (((mobj_t*)th)->lastenemy && ((mobj_t*)th)->lastenemy->thinker.function == P_MobjThinker) {
          memcpy (save_p + sizeof(void*), &(((mobj_t*)th)->lastenemy->thinker.prev), sizeof(void*));
    }

        save_p += 5*sizeof(void*);

        if (mobj->player)
          mobj->player = (player_t *)((mobj->player-players) + 1);
      }


  *save_p++ = tc_end;


  {
    int i;
    CheckSaveGame(numsectors * sizeof(mobj_t *));
    for (i = 0; i < numsectors; i++)
    {
      mobj_t *target = sectors[i].soundtarget;


      if (target && target->thinker.function == P_MobjThinker)
        target = (mobj_t *) target->thinker.prev;
      else
        target = NULL;
      memcpy(save_p, &target, sizeof target);
      save_p += sizeof target;
    }
  }
}

static void P_SetNewTarget(mobj_t **mop, mobj_t *targ)
{
  *mop = NULL;
  P_SetTarget(mop, targ);
}

static int P_GetMobj(mobj_t* mi, size_t s)
{
  size_t i = (size_t)mi;
  if (i >= s) I_Error("Corrupt savegame");
  return i;
}

void P_UnArchiveThinkers (void)
{
  thinker_t *th;
  mobj_t    **mobj_p;
  size_t    size;

  totallive = 0;

  memcpy(&brain, save_p, sizeof brain);
  save_p += sizeof brain;


  for (th = thinkercap.next; th != &thinkercap; )
    {
      thinker_t *next = th->next;
      if (th->function == P_MobjThinker)
        P_RemoveMobj ((mobj_t *) th);
      else
        Z_Free (th);
      th = next;
    }
  P_InitThinkers ();


  {
    byte *sp = save_p;
    for (size = 1; *save_p++ == tc_mobj; size++)
      {
        PADSAVEP();
        save_p += sizeof(mobj_t)+3*sizeof(void*)-4*sizeof(fixed_t);
      }

    if (*--save_p != tc_end)
      I_Error ("P_UnArchiveThinkers: Unknown tclass %i in savegame", *save_p);


    *(mobj_p = malloc(size * sizeof *mobj_p)) = 0;
    save_p = sp;
  }

  for (size = 1; *save_p++ == tc_mobj; size++)
    {
      mobj_t *mobj = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);
      mobj_p[size] = mobj;

      PADSAVEP();
      memcpy (mobj, save_p, sizeof(mobj_t)-2*sizeof(void*)-4*sizeof(fixed_t));
      save_p += sizeof(mobj_t)-sizeof(void*)-4*sizeof(fixed_t);
      memcpy (&(mobj->lastenemy), save_p, sizeof(void*));
      save_p += 4*sizeof(void*);
      mobj->state = states + (long)mobj->state;

      if (mobj->player)
        (mobj->player = &players[(long)mobj->player - 1]) -> mo = mobj;

      P_SetThingPosition (mobj);
      mobj->info = &mobjinfo[mobj->type];
      mobj->thinker.function = P_MobjThinker;
      P_AddThinker (&mobj->thinker);

      if (!((mobj->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL | MF_CORPSE)))
        totallive++;
    }







  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
      P_SetNewTarget(&((mobj_t *) th)->target,
        mobj_p[P_GetMobj(((mobj_t *)th)->target,size)]);

      P_SetNewTarget(&((mobj_t *) th)->tracer,
        mobj_p[P_GetMobj(((mobj_t *)th)->tracer,size)]);

      P_SetNewTarget(&((mobj_t *) th)->lastenemy,
        mobj_p[P_GetMobj(((mobj_t *)th)->lastenemy,size)]);
    }

  {
    int i;
    for (i = 0; i < numsectors; i++)
    {
      mobj_t *target;
      memcpy(&target, save_p, sizeof target);
      save_p += sizeof target;

      P_SetNewTarget(&sectors[i].soundtarget, mobj_p[P_GetMobj(target,size)]);
    }
  }

  free(mobj_p);


  if (gamemode == commercial)
    P_SpawnBrainTargets();
}




enum {
  tc_ceiling,
  tc_door,
  tc_floor,
  tc_plat,
  tc_flash,
  tc_strobe,
  tc_glow,
  tc_elevator,
  tc_scroll,
  tc_pusher,
  tc_flicker,
  tc_endspecials
} specials_e;

void P_ArchiveSpecials (void)
{
  thinker_t *th;
  size_t    size = 0;

  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (!th->function)
      {
        platlist_t *pl;
        ceilinglist_t *cl;
        for (pl=activeplats; pl; pl=pl->next)
          if (pl->plat == (plat_t *) th)
            {
              size += 4+sizeof(plat_t);
              goto end;
            }
        for (cl=activeceilings; cl; cl=cl->next)
          if (cl->ceiling == (ceiling_t *) th)
            {
              size += 4+sizeof(ceiling_t);
              goto end;
            }
      end:;
      }
    else
      size +=
        th->function==T_MoveCeiling  ? 4+sizeof(ceiling_t) :
        th->function==T_VerticalDoor ? 4+sizeof(vldoor_t)  :
        th->function==T_MoveFloor    ? 4+sizeof(floormove_t):
        th->function==T_PlatRaise    ? 4+sizeof(plat_t)    :
        th->function==T_LightFlash   ? 4+sizeof(lightflash_t):
        th->function==T_StrobeFlash  ? 4+sizeof(strobe_t)  :
        th->function==T_Glow         ? 4+sizeof(glow_t)    :
        th->function==T_MoveElevator ? 4+sizeof(elevator_t):
        th->function==T_Scroll       ? 4+sizeof(scroll_t)  :
        th->function==T_Pusher       ? 4+sizeof(pusher_t)  :
        th->function==T_FireFlicker? 4+sizeof(fireflicker_t) :
      0;

  CheckSaveGame(size + 1);

  for (th=thinkercap.next; th!=&thinkercap; th=th->next)
    {
      if (!th->function)
        {
          platlist_t *pl;
          ceilinglist_t *cl;

          for (pl=activeplats; pl; pl=pl->next)
            if (pl->plat == (plat_t *) th)
              goto plat;

          for (cl=activeceilings; cl; cl=cl->next)
            if (cl->ceiling == (ceiling_t *) th)
              goto ceiling;

          continue;
        }

      if (th->function == T_MoveCeiling)
        {
          ceiling_t *ceiling;
        ceiling:
          *save_p++ = tc_ceiling;
          PADSAVEP();
          ceiling = (ceiling_t *)save_p;
          memcpy (ceiling, th, sizeof(*ceiling));
          save_p += sizeof(*ceiling);
          ceiling->sector = (sector_t *)(ceiling->sector - sectors);
          continue;
        }

      if (th->function == T_VerticalDoor)
        {
          vldoor_t *door;
          *save_p++ = tc_door;
          PADSAVEP();
          door = (vldoor_t *) save_p;
          memcpy (door, th, sizeof *door);
          save_p += sizeof(*door);
          door->sector = (sector_t *)(door->sector - sectors);

          door->line = (line_t *) (door->line ? door->line-lines : -1);
          continue;
        }

      if (th->function == T_MoveFloor)
        {
          floormove_t *floor;
          *save_p++ = tc_floor;
          PADSAVEP();
          floor = (floormove_t *)save_p;
          memcpy (floor, th, sizeof(*floor));
          save_p += sizeof(*floor);
          floor->sector = (sector_t *)(floor->sector - sectors);
          continue;
        }

      if (th->function == T_PlatRaise)
        {
          plat_t *plat;
        plat:
          *save_p++ = tc_plat;
          PADSAVEP();
          plat = (plat_t *)save_p;
          memcpy (plat, th, sizeof(*plat));
          save_p += sizeof(*plat);
          plat->sector = (sector_t *)(plat->sector - sectors);
          continue;
        }

      if (th->function == T_LightFlash)
        {
          lightflash_t *flash;
          *save_p++ = tc_flash;
          PADSAVEP();
          flash = (lightflash_t *)save_p;
          memcpy (flash, th, sizeof(*flash));
          save_p += sizeof(*flash);
          flash->sector = (sector_t *)(flash->sector - sectors);
          continue;
        }

      if (th->function == T_StrobeFlash)
        {
          strobe_t *strobe;
          *save_p++ = tc_strobe;
          PADSAVEP();
          strobe = (strobe_t *)save_p;
          memcpy (strobe, th, sizeof(*strobe));
          save_p += sizeof(*strobe);
          strobe->sector = (sector_t *)(strobe->sector - sectors);
          continue;
        }

      if (th->function == T_Glow)
        {
          glow_t *glow;
          *save_p++ = tc_glow;
          PADSAVEP();
          glow = (glow_t *)save_p;
          memcpy (glow, th, sizeof(*glow));
          save_p += sizeof(*glow);
          glow->sector = (sector_t *)(glow->sector - sectors);
          continue;
        }


      if (th->function == T_FireFlicker)
        {
          fireflicker_t *flicker;
          *save_p++ = tc_flicker;
          PADSAVEP();
          flicker = (fireflicker_t *)save_p;
          memcpy (flicker, th, sizeof(*flicker));
          save_p += sizeof(*flicker);
          flicker->sector = (sector_t *)(flicker->sector - sectors);
          continue;
        }


      if (th->function == T_MoveElevator)
        {
          elevator_t *elevator;
          *save_p++ = tc_elevator;
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
          continue;
        }


      if (th->function == T_Scroll)
        {
          *save_p++ = tc_scroll;
          memcpy (save_p, th, sizeof(scroll_t));
          save_p += sizeof(scroll_t);
          continue;
        }



      if (th->function == T_Pusher)
        {
          *save_p++ = tc_pusher;
          memcpy (save_p, th, sizeof(pusher_t));
          save_p += sizeof(pusher_t);
          continue;
        }
    }


  *save_p++ = tc_endspecials;
}

void P_UnArchiveSpecials (void)
{
  byte tclass;


  while ((tclass = *save_p++) != tc_endspecials)
    switch (tclass)
      {
      case tc_ceiling:
        PADSAVEP();
        {
          ceiling_t *ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVEL, NULL);
          memcpy (ceiling, save_p, sizeof(*ceiling));
          save_p += sizeof(*ceiling);
          ceiling->sector = &sectors[(long)ceiling->sector];
          ceiling->sector->ceilingdata = ceiling;

          if (ceiling->thinker.function)
            ceiling->thinker.function = T_MoveCeiling;

          P_AddThinker (&ceiling->thinker);
          P_AddActiveCeiling(ceiling);
          break;
        }

      case tc_door:
        PADSAVEP();
        {
          vldoor_t *door = Z_Malloc (sizeof(*door), PU_LEVEL, NULL);
          memcpy (door, save_p, sizeof(*door));
          save_p += sizeof(*door);
          door->sector = &sectors[(long)door->sector];


          door->line = (long)door->line!=-1? &lines[(long)door->line] : NULL;

          door->sector->ceilingdata = door;
          door->thinker.function = T_VerticalDoor;
          P_AddThinker (&door->thinker);
          break;
        }

      case tc_floor:
        PADSAVEP();
        {
          floormove_t *floor = Z_Malloc (sizeof(*floor), PU_LEVEL, NULL);
          memcpy (floor, save_p, sizeof(*floor));
          save_p += sizeof(*floor);
          floor->sector = &sectors[(long)floor->sector];
          floor->sector->floordata = floor;
          floor->thinker.function = T_MoveFloor;
          P_AddThinker (&floor->thinker);
          break;
        }

      case tc_plat:
        PADSAVEP();
        {
          plat_t *plat = Z_Malloc (sizeof(*plat), PU_LEVEL, NULL);
          memcpy (plat, save_p, sizeof(*plat));
          save_p += sizeof(*plat);
          plat->sector = &sectors[(long)plat->sector];
          plat->sector->floordata = plat;

          if (plat->thinker.function)
            plat->thinker.function = T_PlatRaise;

          P_AddThinker (&plat->thinker);
          P_AddActivePlat(plat);
          break;
        }

      case tc_flash:
        PADSAVEP();
        {
          lightflash_t *flash = Z_Malloc (sizeof(*flash), PU_LEVEL, NULL);
          memcpy (flash, save_p, sizeof(*flash));
          save_p += sizeof(*flash);
          flash->sector = &sectors[(long)flash->sector];
          flash->thinker.function = T_LightFlash;
          P_AddThinker (&flash->thinker);
          break;
        }

      case tc_strobe:
        PADSAVEP();
        {
          strobe_t *strobe = Z_Malloc (sizeof(*strobe), PU_LEVEL, NULL);
          memcpy (strobe, save_p, sizeof(*strobe));
          save_p += sizeof(*strobe);
          strobe->sector = &sectors[(long)strobe->sector];
          strobe->thinker.function = T_StrobeFlash;
          P_AddThinker (&strobe->thinker);
          break;
        }

      case tc_glow:
        PADSAVEP();
        {
          glow_t *glow = Z_Malloc (sizeof(*glow), PU_LEVEL, NULL);
          memcpy (glow, save_p, sizeof(*glow));
          save_p += sizeof(*glow);
          glow->sector = &sectors[(long)glow->sector];
          glow->thinker.function = T_Glow;
          P_AddThinker (&glow->thinker);
          break;
        }

      case tc_flicker:
        PADSAVEP();
        {
          fireflicker_t *flicker = Z_Malloc (sizeof(*flicker), PU_LEVEL, NULL);
          memcpy (flicker, save_p, sizeof(*flicker));
          save_p += sizeof(*flicker);
          flicker->sector = &sectors[(long)flicker->sector];
          flicker->thinker.function = T_FireFlicker;
          P_AddThinker (&flicker->thinker);
          break;
        }


      case tc_elevator:
        PADSAVEP();
        {
          elevator_t *elevator = Z_Malloc (sizeof(*elevator), PU_LEVEL, NULL);
          memcpy (elevator, save_p, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = &sectors[(long)elevator->sector];
          elevator->sector->floordata = elevator;
          elevator->sector->ceilingdata = elevator;
          elevator->thinker.function = T_MoveElevator;
          P_AddThinker (&elevator->thinker);
          break;
        }

      case tc_scroll:
        {
          scroll_t *scroll = Z_Malloc (sizeof(scroll_t), PU_LEVEL, NULL);
          memcpy (scroll, save_p, sizeof(scroll_t));
          save_p += sizeof(scroll_t);
          scroll->thinker.function = T_Scroll;
          P_AddThinker(&scroll->thinker);
          break;
        }

      case tc_pusher:
        {
          pusher_t *pusher = Z_Malloc (sizeof(pusher_t), PU_LEVEL, NULL);
          memcpy (pusher, save_p, sizeof(pusher_t));
          save_p += sizeof(pusher_t);
          pusher->thinker.function = T_Pusher;
          pusher->source = P_GetPushThing(pusher->affectee);
          P_AddThinker(&pusher->thinker);
          break;
        }

      default:
        I_Error("P_UnarchiveSpecials: Unknown tclass %i in savegame", tclass);
      }
}



void P_ArchiveRNG(void)
{
  CheckSaveGame(sizeof rng);
  memcpy(save_p, &rng, sizeof rng);
  save_p += sizeof rng;
}

void P_UnArchiveRNG(void)
{
  memcpy(&rng, save_p, sizeof rng);
  save_p += sizeof rng;
}



void P_ArchiveMap(void)
{
  int zero = 0, one = 1;
  CheckSaveGame(2 * sizeof zero + sizeof markpointnum +
                markpointnum * sizeof *markpoints +
                sizeof automapmode + sizeof one);

  memcpy(save_p, &automapmode, sizeof automapmode);
  save_p += sizeof automapmode;
  memcpy(save_p, &one, sizeof one);
  save_p += sizeof one;
  memcpy(save_p, &zero, sizeof zero);
  save_p += sizeof zero;
  memcpy(save_p, &zero, sizeof zero);
  save_p += sizeof zero;
  memcpy(save_p, &markpointnum, sizeof markpointnum);
  save_p += sizeof markpointnum;

  if (markpointnum)
    {
      memcpy(save_p, markpoints, sizeof *markpoints * markpointnum);
      save_p += markpointnum * sizeof *markpoints;
    }
}

void P_UnArchiveMap(void)
{
  int unused;
  memcpy(&automapmode, save_p, sizeof automapmode);
  save_p += sizeof automapmode;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;

  if (automapmode & am_active)
    AM_Start();

  memcpy(&markpointnum, save_p, sizeof markpointnum);
  save_p += sizeof markpointnum;

  if (markpointnum)
    {
      while (markpointnum >= markpointnum_max)
        markpoints = realloc(markpoints, sizeof *markpoints *
         (markpointnum_max = markpointnum_max ? markpointnum_max*2 : 16));
      memcpy(markpoints, save_p, markpointnum * sizeof *markpoints);
      save_p += markpointnum * sizeof *markpoints;
    }
}

