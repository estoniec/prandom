#ifndef PRANDOM_H
#define PRANDOM_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stddef.h>  // для size_t в userspace
#endif
#include "lib.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif
#define POLY_DEGREE 256
#define GF256_ONE (GF256_t)0x01
#define GF256_DEGREES_AUTO {0, 2, 5, 10, 255}

struct gf256_gprn
{
    GF256_t coeff[POLY_DEGREE];
    GF256_t t[POLY_DEGREE];
    int     index;
};

void    gf256_gprn_init_t(struct gf256_gprn* gprn, GF256_t* coeff_data, GF256_t* polynom_data);
GF256_t gf256_gprn_next(struct gf256_gprn* gprn);
void    gf256_gprn_generate(struct gf256_gprn* gprn, size_t count, GF256_t* output);

#endif