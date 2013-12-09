BACKEND=sdl

OBJ=am_map.o doomdef.o doomstat.o g_game.o hu_lib.o hu_stuff.o info.o lprintf.o sounds.o s_sound.o st_lib.o st_stuff.o tables.o v_video.o wi_stuff.o f_finale.o f_wipe.o d_client.o d_items.o d_main.o m_bbox.o m_cheat.o m_menu.o m_misc.o m_random.o p_ceilng.o p_doors.o p_enemy.o p_floor.o p_genlin.o p_inter.o p_lights.o p_map.o p_maputl.o p_mobj.o p_plats.o p_pspr.o p_saveg.o p_setup.o p_sight.o p_spec.o p_switch.o p_telept.o p_tick.o p_user.o r_bsp.o r_data.o r_demo.o r_draw.o r_filter.o r_fps.o r_main.o r_patch.o r_plane.o r_segs.o r_sky.o r_things.o w_mmap.o w_wad.o z_bmalloc.o z_zone.o
#OBJ+=w_memcache.o

OBJ_sdl=sdl/i_main.o sdl/i_sound.o sdl/i_system.o sdl/i_video.o
LIB_sdl=sdl/libsdldoom.a
CFLAGS_sdl=-I../src -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -std=c99
LDFLAGS_sdl=-lm -L/usr/lib -lSDL -lpthread

OBJ_fudge=fudge/i_main.o fudge/i_sound.o fudge/i_system.o fudge/i_video.o
LIB_fudge=fudge/libfudgedoom.a
CFLAGS_fudge=-I../src 
LDFLAGS_fudge=-lm

all: doom

.c.o:
	$(CC) $(CFLAGS_$(BACKEND)) -c -o $@ $<

$(LIB_sdl): $(OBJ_sdl)
	$(AR) crv $@ $^

$(LIB_fudge): $(OBJ_fudge)
	$(AR) crv $@ $^

doom: $(OBJ) $(LIB_$(BACKEND))
	$(CC) -o $@ $(OBJ) $(LDFLAGS_$(BACKEND)) $(LIB_$(BACKEND))

clean:
	rm -f $(OBJ_sdl)
	rm -f $(LIB_sdl)
	rm -f $(OBJ_fudge)
	rm -f $(LIB_fudge)
	rm -f $(OBJ)
	rm -f doom

