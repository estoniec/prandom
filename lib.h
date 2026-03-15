#ifndef LIB_H
#define LIB_H

#include <linux/types.h>

#define GF256_t u8

void    GF256_init(void);
GF256_t GF256_Add(GF256_t a, GF256_t b);
GF256_t GF256_Sub(GF256_t a, GF256_t b);
GF256_t GF256_Mul(GF256_t a, GF256_t b);
GF256_t GF256_Div(GF256_t a, GF256_t b);

#endif