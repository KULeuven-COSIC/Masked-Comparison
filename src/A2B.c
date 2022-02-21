/*
 * MIT License
 *
 * Copyright (c) 2021-2022: imec-COSIC KU Leuven, 3001 Leuven, Belgium
 * Copyright (c) 2021-2022: Infineon Technologies AG, 85579 Neubiberg, Germany
 * Copyright (c) 2021-2022: Universität der Bundeswehr, 85577 Neubiberg, Germany
 * Authors: Michiel Van Beirendonck <michiel.vanbeirendonck@esat.kuleuven.be>
 *			Daniel Heinz <daniel.heinz@unibw.de>
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
#include "A2B.h"
#include "SecAdd.h"
#include "randombytes.h"

#ifdef DEBUG
#include "bitmask.h"
#endif

#ifdef CW
extern void clear_regs(void);
#endif // CW




// [https://eprint.iacr.org/2018/381.pdf, Algorithm 8]
static void RefreshXOR(size_t from, size_t to, uint64_t x[to])
{
    for (size_t i = from; i < to; i++)
    {
        x[i] = 0;
    }

    for (size_t i = 0; i < to - 1; i++)
    {
        for (size_t j = i + 1; j < to; j++)
        {
            uint64_t R = random_uint64();
            x[i] ^= R;
            x[j] ^= R;
        }
    }
}

static void RefreshXOR_bitsliced(size_t from, size_t to, size_t nbits, uint32_t x[to][nbits])
{
    for (size_t i = from; i < to; i++)
    {
        for (size_t k = 0; k < nbits; k++)
        {
            x[i][k] = 0;
        }
    }

    for (size_t i = 0; i < to - 1; i++)
    {
        for (size_t j = i + 1; j < to; j++)
        {
            for (size_t k = 0; k < nbits; k++)
            {
                uint32_t R = random_uint32();
                x[i][k] ^= R;
                x[j][k] ^= R;
            }
        }
    }
}

// [http://www.crypto-uni.lu/jscoron/publications/secconvorder.pdf, Algorithm 4]
void A2B(size_t nshares, uint64_t B[nshares], const uint64_t A[nshares])
{
    if (nshares == 1)
    {
        B[0] = A[0];
        return;
    }

    uint64_t x[nshares], y[nshares];

    uint32_t half_nshares = nshares / 2;
    A2B(half_nshares, &x[0], &A[0]);
    RefreshXOR(half_nshares, nshares, x);
    A2B(nshares - half_nshares, &y[0], &A[half_nshares]);
    RefreshXOR(nshares - half_nshares, nshares, y);
    SecAdd(nshares, B, x, y);

#ifdef DEBUG
    uint64_t A_unmasked = 0;
    uint64_t B_unmasked = 0;

    for (size_t j = 0; j < nshares; j++)
    {
        A_unmasked = A_unmasked + A[j];
        B_unmasked = B_unmasked ^ B[j];
    }

    assert(A_unmasked == B_unmasked);
#endif
}

static void A2B_bitsliced_inner(size_t nshares, size_t nbits, uint32_t B_bitsliced[nshares][nbits], const uint32_t A_bitsliced[nshares][nbits])
{
    if (nshares == 1)
    {
        for (size_t i = 0; i < nbits; i++)
        {
            B_bitsliced[0][i] = A_bitsliced[0][i];
        }
        return;
    }

    uint32_t x[nshares][nbits], y[nshares][nbits];

    uint32_t half_nshares = nshares / 2;
    A2B_bitsliced_inner(half_nshares, nbits, &x[0], &A_bitsliced[0]);
    RefreshXOR_bitsliced(half_nshares, nshares, nbits, x);
    A2B_bitsliced_inner(nshares - half_nshares, nbits, &y[0], &A_bitsliced[half_nshares]);
    RefreshXOR_bitsliced(nshares - half_nshares, nshares, nbits, y);
    SecAdd_bitsliced(nshares, nbits, B_bitsliced, x, y);
}

static void pack_bitslice(size_t nshares, size_t nbits, uint32_t x_bitsliced[nshares][nbits], const uint32_t x[32][nshares])
{
    for (size_t j = 0; j < nshares; j++)
    {
        for (size_t k = 0; k < nbits; k++)
        {
            x_bitsliced[j][k] = 0;
        }
    }

    for (size_t i = 0; i < 32; i++)
    {
        for (size_t j = 0; j < nshares; j++)
        {
            for (size_t k = 0; k < nbits; k++)
            {
                x_bitsliced[j][k] = x_bitsliced[j][k] | (((x[i][j] >> k) & 1) << i);
            }
#ifdef CW
			clear_regs();
#endif // CW

			
        }
    }
}

static void unpack_bitslice(size_t nshares, size_t nbits, uint32_t x[32][nshares], uint32_t x_bitsliced[nshares][nbits])
{
    for (size_t i = 0; i < 32; i++)
    {
        for (size_t j = 0; j < nshares; j++)
        {
            uint32_t tmp = 0;

            for (size_t k = 0; k < nbits; k++)
            {
                tmp |= ((x_bitsliced[j][k] & (1 << i)) >> i) << k;
            }

            x[i][j] = tmp;
        }
    }
}

void A2B_bitsliced(size_t nshares, size_t nbits, uint32_t B[32][nshares], const uint32_t A[32][nshares])
{
    uint32_t A_bitsliced[nshares][nbits];
    uint32_t B_bitsliced[nshares][nbits];

    pack_bitslice(nshares, nbits, A_bitsliced, A);
    A2B_bitsliced_inner(nshares, nbits, B_bitsliced, A_bitsliced);
    unpack_bitslice(nshares, nbits, B, B_bitsliced);

#ifdef DEBUG
    for (size_t i = 0; i < 32; i++)
    {
        uint32_t A_unmasked = 0;
        uint32_t B_unmasked = 0;

        for (size_t j = 0; j < nshares; j++)
        {
            A_unmasked = (A_unmasked + A[i][j]) & bit_mask(nbits);
            B_unmasked = (B_unmasked ^ B[i][j]) & bit_mask(nbits);
        }

        assert(A_unmasked == B_unmasked);
    }
#endif
}
