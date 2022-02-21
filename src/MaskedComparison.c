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
#include "MaskedComparison.h"
#include "ReduceComparisons.h"
#include "BooleanEqualityTest.h"
#include "randombytes.h"
#include "bitmask.h"
#include "A2B.h"
#include "B2A.h"
#include "hal.h"
#include <string.h>

#ifdef KYBER
static void shared_compress(size_t ncoeffs, size_t compressto, uint32_t B[ncoeffs][NSHARES])
{
    for (size_t i = 0; i < ncoeffs; i++)
    {
#ifdef DEBUG

        uint32_t B_unmasked = 0, BA_unmasked = 0;

        for (size_t j = 0; j < NSHARES; j++)
        {
            B_unmasked = (B_unmasked + B[i][j]) % Q;
        }

        B_unmasked = (((B_unmasked << compressto) + Q / 2) / Q) & bit_mask(compressto);

#endif

        for (size_t j = 0; j < NSHARES; j++)
        {
            uint64_t tmp = (uint64_t)B[i][j] << (compressto + KYBER_FRAC_BITS);

            if (j == 0)
            {
                tmp += (Q << KYBER_FRAC_BITS) / 2;
            }

            B[i][j] = (tmp / Q); // ! make sure (uint64_t/constant) is constant-time on your platform
        }

#ifdef DEBUG

        for (size_t j = 0; j < NSHARES; j++)
        {
            BA_unmasked = (BA_unmasked + B[i][j]) & bit_mask(compressto + KYBER_FRAC_BITS);
        }

        BA_unmasked >>= KYBER_FRAC_BITS;

        assert(B_unmasked == BA_unmasked);

#endif
    }
}
#endif

uint64_t MaskedComparison(const uint32_t B[NCOEFFS_B][NSHARES], const uint32_t C[NCOEFFS_C][NSHARES],
                          const uint32_t public_B[NCOEFFS_B], const uint32_t public_C[NCOEFFS_C])
{
    uint32_t B_compressed[NCOEFFS_B][NSHARES];
    uint32_t C_compressed[NCOEFFS_C][NSHARES];
    uint64_t BC_reshared[NCOEFFS_B + NCOEFFS_C][NSHARES];
    uint64_t E[NSHARES];
    uint32_t Bp[NCOEFFS_B][NSHARES], Cp[NCOEFFS_B][NSHARES];

    PROFILE_STEP_INIT();

    ////////////////////////////////////////////////////////////
    ///                Step 0 : Preprocessing                ///
    ////////////////////////////////////////////////////////////

    PROFILE_STEP_START();

    memcpy(Bp, B, NCOEFFS_B * NSHARES * sizeof(uint32_t));
    memcpy(Cp, C, NCOEFFS_C * NSHARES * sizeof(uint32_t));

#if defined(KYBER)
    shared_compress(NCOEFFS_B, COMPRESSTO_B, Bp);
    shared_compress(NCOEFFS_C, COMPRESSTO_C, Cp);
#endif

    for (size_t i = 0; i < NCOEFFS_B; i++)
    {
        Bp[i][0] = (Bp[i][0] - (public_B[i] << (COMPRESSFROM_B - COMPRESSTO_B))) & bit_mask(COMPRESSFROM_B);
    }

    for (size_t i = 0; i < NCOEFFS_C; i++)
    {
        Cp[i][0] = (Cp[i][0] - (public_C[i] << (COMPRESSFROM_C - COMPRESSTO_C))) & bit_mask(COMPRESSFROM_C);
    }

    PROFILE_STEP_STOP(0);

    ////////////////////////////////////////////////////////////
    ///                    Step 1 : A2B                      ///
    ////////////////////////////////////////////////////////////

    PROFILE_STEP_START();

    for (size_t i = 0; i < NCOEFFS_B; i += 32)
    {
        A2B_bitsliced(NSHARES, COMPRESSFROM_B, B_compressed + i, Bp + i);
    }

    for (size_t i = 0; i < NCOEFFS_B; i++)
    {
        for (size_t j = 0; j < NSHARES; j++)
        {
            B_compressed[i][j] = (B_compressed[i][j] >> (COMPRESSFROM_B - COMPRESSTO_B)) & bit_mask(COMPRESSTO_B);
        }
    }

    for (size_t i = 0; i < NCOEFFS_C; i += 32)
    {
        A2B_bitsliced(NSHARES, COMPRESSFROM_C, C_compressed + i, Cp + i);
    }

    for (size_t i = 0; i < NCOEFFS_C; i++)
    {
        for (size_t j = 0; j < NSHARES; j++)
        {
            C_compressed[i][j] = (C_compressed[i][j] >> (COMPRESSFROM_C - COMPRESSTO_C)) & bit_mask(COMPRESSTO_C);
        }
    }

    PROFILE_STEP_STOP(1);

    ////////////////////////////////////////////////////////////
    ///                    Step 2 : B2A                      ///
    ////////////////////////////////////////////////////////////

    PROFILE_STEP_START();

    for (size_t i = 0; i < NCOEFFS_B; i++)
    {
        B2A(BC_reshared[i], B_compressed[i]);
    }

    for (size_t i = 0; i < NCOEFFS_C; i++)
    {
        B2A(BC_reshared[NCOEFFS_B + i], C_compressed[i]);
    }

    PROFILE_STEP_STOP(2);

    ////////////////////////////////////////////////////////////
    ///              Step 3 : ReduceComparisons              ///
    ////////////////////////////////////////////////////////////

    PROFILE_STEP_START();

    ReduceComparisons(E, BC_reshared);

    PROFILE_STEP_STOP(3);

    ////////////////////////////////////////////////////////////
    ///            Step 4 : BooleanEqualityTest              ///
    ////////////////////////////////////////////////////////////

    PROFILE_STEP_START();

    uint64_t result = BooleanEqualityTest(E);

    PROFILE_STEP_STOP(4);

    return result;
}