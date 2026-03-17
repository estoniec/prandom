#ifndef LIB_H
#define LIB_H

#ifdef __KERNEL__
#include <linux/types.h>
typedef u8 GF256_t;
#else
#include <stdint.h>
typedef uint8_t GF256_t;
#endif

void    GF256_init(void);
GF256_t GF256_Add(GF256_t a, GF256_t b);
GF256_t GF256_Sub(GF256_t a, GF256_t b);
GF256_t GF256_Mul(GF256_t a, GF256_t b);
GF256_t GF256_Div(GF256_t a, GF256_t b);

#endif