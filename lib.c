
#include "lib.h"
#ifdef __KERNEL__
#include <linux/types.h>
u16 Module = 0x11B;
u8 ready_to_work = 0;
#else
#include <stdint.h>
uint16_t Module = 0x11B;
uint8_t ready_to_work = 0;
#endif

#define ORDER 256

GF256_t alogs[ORDER];
GF256_t logs[ORDER];

void GF256_init(void)
{
    GF256_t g = 0x03;
    GF256_t x = 1;

    for (int i = 0; i < ORDER - 1; i++)
    {
        alogs[i] = x;
        logs[x]  = i;

        uint16_t prod = x << 1;
        if (prod & 0x100)
        {
            prod ^= Module;
        }
        prod ^= x;
        x = (GF256_t)(prod & 0xFF);
    }
    alogs[ORDER - 1] = alogs[0];
    logs[0]          = 0;

    ready_to_work = 1;
}

GF256_t GF256_Add(GF256_t a, GF256_t b)
{
    return a ^ b;
}

GF256_t GF256_Sub(GF256_t a, GF256_t b)
{
    return a ^ b;
}

GF256_t GF256_Mul(GF256_t a, GF256_t b)
{
    if (!ready_to_work)
    {
        GF256_init();
    }

    if (a == 0 || b == 0)
    {
        return 0;
    }

    uint8_t log_a = logs[a];
    uint8_t log_b = logs[b];

    uint8_t sum = 0;
    if ((uint8_t)(ORDER - 1) - log_a < log_b)
    {
        sum = log_a - (ORDER - 1 - log_b);
    }
    else
    {
        sum = log_a + log_b;
    }

    return alogs[sum];
}

static GF256_t inverse_mul_element(GF256_t a)
{
    if (!ready_to_work)
    {
        GF256_init();
    }
    if (a == 0)
    {
        return 0;
    }
    return alogs[255 - logs[a]];
}

GF256_t GF256_Div(GF256_t a, GF256_t b)
{
    return GF256_Mul(a, inverse_mul_element(b));
}
