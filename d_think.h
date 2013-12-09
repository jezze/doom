#ifndef __D_THINK__
#define __D_THINK__

typedef void (*actionf_t)();
typedef actionf_t think_t;

typedef struct thinker_s
{

    struct thinker_s *prev;
    struct thinker_s *next;
    think_t function;
    struct thinker_s *cnext, *cprev;
    unsigned references;

} thinker_t;

#endif
