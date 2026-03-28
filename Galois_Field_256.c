
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Denis Potapov

#include "Galois_Field_256.h"

#ifdef __KERNEL__
#include <linux/types.h>
u16 Module        = 0x11B;
u8  ready_to_work = 0;
#else
#include <stdint.h>
uint16_t Module        = 0x11B;
uint8_t  ready_to_work = 0;
#endif

#define ORDER 256

GF256_t alogs[ORDER];
GF256_t logs[ORDER];

void GF256_init(void) {
    // порождающий элемент x + 1
    GF256_t elem = 1;  // (x + 1)**0

    for (int i = 0; i < ORDER - 1; i++) {
        alogs[i]   = elem;  // <=> (x + 1)**i = elem
        logs[elem] = i;

        uint16_t new_elem = (uint16_t)elem << 1;  // <=> new_elem = elem * x

        new_elem ^= elem;  // <=> new_elem = elem * x + elem <=> new_elem = elem * (x + 1)

        if (new_elem >= 0x100) {
            new_elem ^= Module;
        }
        elem = (GF256_t)(new_elem);
    }
    alogs[ORDER - 1] = alogs[0];

    logs[0] = 0;

    ready_to_work = 1;
}

GF256_t GF256_Add(GF256_t a, GF256_t b) {
    return a ^ b;
}

GF256_t GF256_Sub(GF256_t a, GF256_t b) {
    return a ^ b;
}

GF256_t GF256_Mul(GF256_t a, GF256_t b) {
    if (!ready_to_work) {
        GF256_init();
    }
    if (a == 0 || b == 0) {
        return 0;
    }

    uint8_t log_a = logs[a];
    uint8_t log_b = logs[b];

    uint16_t sum = (log_a + log_b) % (ORDER - 1);

    return alogs[sum];
}

GF256_t inverse_mul_element(GF256_t a) {
    if (a == 0) {
        return 0;
    }
    if (!ready_to_work) {
        GF256_init();
    }
    return alogs[255 - logs[a]];
}

GF256_t GF256_Div(GF256_t a, GF256_t b) {
    return GF256_Mul(a, inverse_mul_element(b));
}