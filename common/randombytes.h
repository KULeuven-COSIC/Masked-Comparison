/*
 * MIT License
 *
 * Copyright (c) 2021-2022: imec-COSIC KU Leuven, 3001 Leuven, Belgium 
 * Author: Michiel Van Beirendonck <michiel.vanbeirendonck@esat.kuleuven.be>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include <stdint.h>
#include <stddef.h>

#ifdef DEBUG
uint32_t rng_get_random_blocking(void);
#else
#include <libopencm3/stm32/rng.h>
#endif

// fully deterministic randomness
int randombytes(uint8_t *obuf, size_t len);

// trng
#if defined(PROFILE_STEP_RAND) || defined(PROFILE_TOP_RAND)
extern uint64_t nb_randombytes;
uint32_t rng_count_get_random_blocking(void);
#define random_uint32() (rng_count_get_random_blocking())
#define random_uint64() (((uint64_t)rng_count_get_random_blocking()) | ((uint64_t)rng_count_get_random_blocking()) << 32)
#else
#define random_uint32() (rng_get_random_blocking())
#define random_uint64() (((uint64_t)rng_get_random_blocking()) | ((uint64_t)rng_get_random_blocking()) << 32)
#endif

#endif /* RANDOMBYTES_H */