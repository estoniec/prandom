#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Galois_Field_256.h"
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

static int test1_simple(void)
{
    const char* coeff_file    = "assets/coeff_file.bin";
    const char* seed_file     = "assets/seed_file.bin";
    const char* expected_file = "assets/expected1.bin";

    GF256_t coeff[POLY_DEGREE];
    GF256_t seed[POLY_DEGREE];
    GF256_t generated[NUM_BYTES];
    GF256_t expected[NUM_BYTES];

    if (read_file(coeff_file, coeff, POLY_DEGREE) != 0)
    {
        printf("failed to read %s\n", coeff_file);
        return -1;
    }

    if (read_file(seed_file, seed, POLY_DEGREE) != 0)
    {
        printf("failed to read %s\n", seed_file);
        return -1;
    }

    struct gf256_gprn gprn;
    gf256_gprn_init_t(&gprn, coeff, seed);
    gf256_gprn_generate(&gprn, NUM_BYTES, generated);

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

static int test2_without_seed(void)
{
    const char* coeff_file    = "assets/coeff_file.bin";
    const char* expected_file = "assets/expected1.bin";

    GF256_t coeff[POLY_DEGREE];
    GF256_t generated[NUM_BYTES];
    GF256_t expected[NUM_BYTES];

    if (read_file(coeff_file, coeff, POLY_DEGREE) != 0)
    {
        printf("failed to read %s\n", coeff_file);
        return -1;
    }

    struct gf256_gprn gprn;
    gf256_gprn_init_t(&gprn, coeff, NULL);
    gf256_gprn_generate(&gprn, NUM_BYTES, generated);

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

    printf("✅ Test 2 passed\n");
    return 0;
}

// тест на равномерность чисел
static int test3_chi_squared(void)
{
    int      TEST_SIZE = 1024 * 1024;
    GF256_t  coeff[POLY_DEGREE];
    GF256_t* data = malloc(TEST_SIZE);
    if (!data)
    {
        printf("failed to allocate memory\n");
        return -1;
    }
    if (read_file("assets/coeff_file.bin", coeff, POLY_DEGREE) != 0)
    {
        printf("failed to read coeff_file\n");
        free(data);
        return -1;
    }
    struct gf256_gprn gprn;
    gf256_gprn_init_t(&gprn, coeff, NULL);
    gf256_gprn_generate(&gprn, TEST_SIZE, data);
    int observed[256] = {0};
    for (size_t i = 0; i < TEST_SIZE; i++)
    {
        observed[data[i]]++;
    }

    double expected = TEST_SIZE / 256.0;
    double chi_sq   = 0.0;
    for (int i = 0; i < 256; i++)
    {
        double diff = observed[i] - expected;
        chi_sq += (diff * diff) / expected;
    }
    free(data);
    double chi_critical = 293.0;
    if (chi_sq < chi_critical)
    {
        printf("✅ Chi-squared test passed (χ² = %.2f < %.2f)\n", chi_sq, chi_critical);
        return 0;
    }
    else
    {
        printf("❌ Chi-squared test failed (χ² = %.2f >= %.2f)\n", chi_sq, chi_critical);
        return -1;
    }
}

int main(void)
{
    int result = 0;

    result = test1_simple();
    if (result != 0)
    {
        printf("test1 failed\n");
        return 1;
    }

    result = test2_without_seed();
    if (result != 0)
    {
        printf("test2 failed\n");
        return 1;
    }

    result = test3_chi_squared();
    if (result != 0)
    {
        printf("test3 failed\n");
        return 1;
    }

    printf("All tests passed\n");
    return 0;
}
