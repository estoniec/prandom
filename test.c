#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "prandom.h"

#define NUM_BYTES 10

static int read_file(const char* path, unsigned char* buf, size_t size)
{
    FILE* f = fopen(path, "rb");
    if (!f)
        return -1;

    size_t read = fread(buf, 1, size, f);
    fclose(f);
    return (read == size) ? 0 : -1;
}

static int test1(void)
{
    const char* coeff_file    = "test-files/coeff_file.bin";
    const char* poly_file     = "test-files/polynom_file.bin";
    const char* expected_file = "test-files/expected1.bin";

    unsigned char coeff[POLY_DEGREE];
    unsigned char poly[POLY_DEGREE];
    unsigned char generated[NUM_BYTES];
    unsigned char expected[NUM_BYTES];

    if (read_file(coeff_file, coeff, POLY_DEGREE) != 0)
    {
        printf("failed to read %s\n", coeff_file);
        return -1;
    }

    if (read_file(poly_file, poly, POLY_DEGREE) != 0)
    {
        printf("failed to read %s\n", poly_file);
        return -1;
    }

    struct gf256_gprn gprn;
    gf256_gprn_init_t(&gprn, (GF256_t*)coeff, (GF256_t*)poly);
    gf256_gprn_generate(&gprn, NUM_BYTES, (GF256_t*)generated);

    if (read_file(expected_file, expected, NUM_BYTES) != 0)
    {
        printf("failed to read %s\n", expected_file);
        return -1;
    }

    for (int i = 0; i < NUM_BYTES; i++)
    {
        if (generated[i] != expected[i])
        {
            printf("❌ Byte %d: 0x%02x != 0x%02x\n", i, generated[i], expected[i]);
            return -1;
        }
    }

    printf("✅ Test 1 passed\n");
    return 0;
}

int main(void)
{
    int result = 0;

    result = test1();
    if (result != 0)
    {
        printf("test1 failed\n");
        return 1;
    }

    printf("All tests passed\n");
    return 0;
}
