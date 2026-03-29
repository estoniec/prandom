
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Denis Potapov

#ifndef Galois_Field_256
#define Galois_Field_256

#ifdef __KERNEL__
#include <linux/types.h>
typedef u8 GF256_t;
#else
#include <stdint.h>
typedef uint8_t GF256_t;
#endif

GF256_t GF256_Add(GF256_t a, GF256_t b);

GF256_t GF256_Sub(GF256_t a, GF256_t b);

GF256_t GF256_Mul(GF256_t a, GF256_t b);

GF256_t GF256_Div(GF256_t a, GF256_t b);

#endif