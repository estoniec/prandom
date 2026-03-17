#include "prandom.h"

#include "lib.h"

void gf256_gprn_init_t(struct gf256_gprn* gprn, GF256_t* coeff_data, GF256_t* polynom_data)
{
    int i;

    for (i = 0; i < POLY_DEGREE; i++)
    {
        gprn->coeff[i] = coeff_data[i];
    }

    for (i = 0; i < POLY_DEGREE; i++)
    {
        gprn->t[i] = polynom_data[i];
    }

    gprn->index = 0;
}

GF256_t gf256_gprn_next(struct gf256_gprn* gprn)
{
    GF256_t new_byte = 0;
    int     idx;
    int     i;

    for (i = 0; i < POLY_DEGREE; i++)
    {
        if (gprn->t[i] != 0)
        {
            idx      = (gprn->index + i) % POLY_DEGREE;
            new_byte = GF256_Add(new_byte, GF256_Mul(gprn->t[i], gprn->coeff[idx]));
        }
    }

    // "circular" array
    gprn->coeff[gprn->index] = new_byte;
    gprn->index              = (gprn->index + 1) % POLY_DEGREE;

    return new_byte;
}

void gf256_gprn_generate(struct gf256_gprn* gprn, size_t count, GF256_t* output)
{
    size_t i;
    for (i = 0; i < count; i++)
    {
        output[i] = gf256_gprn_next(gprn);
    }
}