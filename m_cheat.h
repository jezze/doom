#ifndef __M_CHEAT__
#define __M_CHEAT__

extern struct cheat_s
{

    const char *cheat;
    void (*const func)();
    const int arg;
    uint_64_t code, mask;

} cheat[];

boolean M_FindCheats(int key);

#endif
