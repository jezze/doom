#ifndef __M_SWAP__
#define __M_SWAP__

#define doom_swap_l(x) ((long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | (((unsigned long int)(x) & 0x0000ff00U) <<  8) | (((unsigned long int)(x) & 0x00ff0000U) >>  8) | (((unsigned long int)(x) & 0xff000000U) >> 24)))
#define doom_swap_s(x) ((short int)((((unsigned short int)(x) & 0x00ff) << 8) | (((unsigned short int)(x) & 0xff00) >> 8))) 
#define doom_wtohl(x) (long int)(x)
#define doom_htowl(x) (long int)(x)
#define doom_wtohs(x) (short int)(x)
#define doom_htows(x) (short int)(x)
#define doom_ntohl(x) (long int)(x)
#define doom_htonl(x) (long int)(x)
#define doom_ntohs(x) (short int)(x)
#define doom_htons(x) (short int)(x)
#define LONG(x) doom_wtohl(x)
#define SHORT(x) doom_htows(x)

#endif
