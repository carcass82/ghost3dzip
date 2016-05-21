#ifndef port_h
#define port_h
#include <limits.h>

#ifdef GCC
#define Inline inline
#else
#define Inline __inline
#endif

#if INT_MAX > 0x7FFF
typedef unsigned short uint_2;  /* two-byte integer (large arrays)      */
typedef unsigned int   uint_4;  /* four-byte integers (range needed)    */
#else
typedef unsigned int   uint_2;
typedef unsigned long  uint_4;
#endif

typedef unsigned int uint;     /* fast unsigned integer, 2 or 4 bytes  */

#endif
