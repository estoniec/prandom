#include "prandom.h"

#include "Galois_Field_256.h"

void gf256_gprn_init_t(struct gf256_gprn* gprn, GF256_t* coeff_data, GF256_t* seed_data)
{
    if (!gprn)
        return;

    int i;

    if (coeff_data)
    {
        for (i = 0; i < POLY_DEGREE; i++)
        {
            gprn->coeff[i] = coeff_data[i];
        }
    }
    else
    {
        for (i = 0; i < POLY_DEGREE; i++)
        {
            gprn->coeff[i] = (GF256_t)(i % 256);
        }
    }

    if (seed_data)
    {
        for (i = 0; i < POLY_DEGREE; i++)
        {
            gprn->t[i] = seed_data[i];
        }
    }
    else
    {
        memset(gprn->t, 0, POLY_DEGREE);
        for (i = 0; i < ARRAY_SIZE(GF256_DEGREES_AUTO); i++)
        {
            gprn->t[GF256_DEGREES_AUTO[i]] = GF256_ONE;
        }
    }

    gprn->index = 0;
}

GF256_t gf256_gprn_next(struct gf256_gprn* gprn)
{
    if (!gprn)
        return 0;

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
    if (!gprn || !output || count == 0)
        return;

    size_t i;
    for (i = 0; i < count; i++)
    {
        output[i] = gf256_gprn_next(gprn);
    }
}
