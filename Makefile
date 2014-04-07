BACKEND=sdl

OBJ_sdl=sdl/i_sound.o sdl/i_system.o sdl/i_video.o
LIB_sdl=sdl/libsdldoom.a
CFLAGS_sdl=-I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -std=c99
LDFLAGS_sdl=-L/usr/lib -lSDL -lpthread

OBJ_fudge=fudge/i_sound.o fudge/i_system.o fudge/i_video.o
LIB_fudge=fudge/libfudgedoom.a
CFLAGS_fudge=
LDFLAGS_fudge=

OBJ=doomstat.o g_game.o hu_stuff.o info.o s_sound.o st_stuff.o tables.o v_video.o wi_stuff.o f_finale.o d_main.o m_cheat.o m_menu.o m_misc.o m_random.o p_ceilng.o p_doors.o p_enemy.o p_floor.o p_genlin.o p_inter.o p_lights.o p_map.o p_maputl.o p_mobj.o p_plats.o p_pspr.o p_setup.o p_sight.o p_spec.o p_switch.o p_telept.o p_tick.o p_user.o r_bsp.o r_data.o r_draw.o r_filter.o r_main.o r_patch.o r_plane.o r_segs.o r_things.o w_wad.o z_bmalloc.o z_zone.o
CFLAGS=-c -I. -Wall $(CFLAGS_$(BACKEND))
LDFLAGS=-lm $(LDFLAGS_$(BACKEND))
ARFLAGS=crv

all: doom

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

$(LIB_sdl): $(OBJ_sdl)
	$(AR) $(ARFLAGS) $@ $^

$(LIB_fudge): $(OBJ_fudge)
	$(AR) $(ARFLAGS) $@ $^

doom: $(OBJ) $(LIB_$(BACKEND))
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ_sdl)
	rm -f $(LIB_sdl)
	rm -f $(OBJ_fudge)
	rm -f $(LIB_fudge)
	rm -f $(OBJ)
	rm -f doom

