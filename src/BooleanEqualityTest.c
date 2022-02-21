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
#include "BooleanEqualityTest.h"
#include "SecAnd.h"
#include "A2B.h"

uint32_t BooleanEqualityTest(uint64_t E[NSHARES])
{
    uint64_t B[NSHARES];
    uint32_t Bh[NSHARES], Bl[NSHARES];
    uint32_t out[NSHARES];
    uint32_t out_unmasked = 0;

    A2B(NSHARES, B, E);

    // Boolean equality circuit: B = 0 -> ~B[63] AND ~B[62] AND ... ~B[0];

    // ~B
    B[0] ^= 0xffffffffffffffff;

    for (size_t i = 0; i < NSHARES; i++)
    {
        Bl[i] = B[i];
        Bh[i] = B[i] >> 32;
    }

    SecAND32(NSHARES, out, Bh, Bl);

    for (size_t j = 16; j > 0; j >>= 1)
    {
        for (size_t i = 0; i < NSHARES; i++)
        {
            Bl[i] = (out[i]) & ((1 << j) - 1);
            Bh[i] = (out[i] >> j) & ((1 << j) - 1);
        }

        SecAND32(NSHARES, out, Bh, Bl);
    }

    for (size_t i = 0; i < NSHARES; i++)
    {
        out_unmasked ^= out[i];
    }

    return out_unmasked;
}
