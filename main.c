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
#include "randombytes.h"
#include "params.h"
#include "hal.h"

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

static void get_rand(size_t ncoeffs, uint32_t mod, uint32_t x[ncoeffs])
{
    for (size_t j = 0; j < ncoeffs; j++)
    {
        x[j] = random_uint32() % mod;
    }
}

static void mask(size_t nshares, size_t ncoeffs, uint32_t x_masked[ncoeffs][nshares], uint32_t x[ncoeffs])
{
    for (size_t i = 0; i < ncoeffs; i++)
    {
        x_masked[i][0] = x[i];

        for (size_t j = 1; j < nshares; j++)
        {
            uint32_t R = random_uint32() % Q;
            x_masked[i][0] = (x_masked[i][0] + (Q - R)) % Q;
            x_masked[i][j] = R;
        }
#ifdef DEBUG

        uint32_t x_unmasked = 0;

        for (size_t j = 0; j < nshares; j++)
        {
            x_unmasked += x_masked[i][j];
        }

        assert((x_unmasked % Q) == x[i]);

#endif
    }
}

#ifdef SABER
static void compress(size_t ncoeffs, size_t compressfrom, size_t compressto, uint32_t submitted_poly[ncoeffs])
{
    for (size_t i = 0; i < ncoeffs; i++)
    {
        submitted_poly[i] >>= (compressfrom - compressto);
    }
}
#endif

#ifdef KYBER
static void compress(size_t ncoeffs, __attribute__((unused)) size_t compressfrom, size_t compressto, uint32_t submitted_poly[ncoeffs])
{
    for (size_t i = 0; i < ncoeffs; i++)
    {
        submitted_poly[i] = (((submitted_poly[i] << compressto) + Q / 2) / Q) % (1 << compressto);
    }
}
#endif

static int test_MaskedComparison(void)
{
    uint64_t result;
    uint32_t public_B[NCOEFFS_B], public_C[NCOEFFS_C];
    uint32_t B[NCOEFFS_B][NSHARES], C[NCOEFFS_C][NSHARES];

    PROFILE_TOP_INIT();

    hal_send_str("=====Testing MaskedComparison====");

    for (size_t i = 0; i < NTESTS; i++)
    {
        get_rand(NCOEFFS_B, Q, public_B);
        get_rand(NCOEFFS_C, P, public_C);

        // Create reencrypted poly as a (correct) sharing of public poly
        mask(NSHARES, NCOEFFS_B, B, public_B);
        mask(NSHARES, NCOEFFS_C, C, public_C);

        // Now compress public poly
        compress(NCOEFFS_B, COMPRESSFROM_B, COMPRESSTO_B, public_B);
        compress(NCOEFFS_C, COMPRESSFROM_C, COMPRESSTO_C, public_C);

        // Send an unmodified ciphertext for comparison
        hal_send_str("===== Start (unmodified ct) ====");
        PROFILE_TOP_START();
        result = MaskedComparison(B, C, public_B, public_C);
        PROFILE_TOP_STOP();
        hal_send_str("===== End (unmodified ct) ====");

        // Unmodified ciphertext: result should be 1
        if (result != 1)
        {
            hal_send_str("[FAIL] result != 1 for unmodified ct");
        }

        // Modify a ciphertext coefficient
        uint32_t BorC, coeff, value;

        // Select either a coefficient from B or C
        BorC = random_uint32() & 1;

        if (BorC == 1)
        {
            // modify B[coeff] with value
            coeff = random_uint32() % NCOEFFS_B;
            value = random_uint32() % ((1 << COMPRESSTO_B) - 1);
            value++;
            public_B[coeff] = (public_B[coeff] - value) & ((1 << COMPRESSTO_B) - 1);
#ifdef DEBUG
            printf("[MAIN] : Ciphertext modification: D[%d] = %d.\n", coeff, value);
#endif
        }
        else
        {
            // modify C[coeff] with value
            coeff = random_uint32() % NCOEFFS_C;
            value = random_uint32() % ((1 << COMPRESSTO_C) - 1);
            value++;
            public_C[coeff] = (public_C[coeff] - value) & ((1 << COMPRESSTO_C) - 1);
#ifdef DEBUG
            printf("[MAIN] : Ciphertext modification: D[%d] = %d.\n", NCOEFFS_B + coeff, value);
#endif
        }

        // Send the modified ciphertext for comparison
        hal_send_str("===== Start (modified ct) ====");
        PROFILE_TOP_START();
        result = MaskedComparison(B, C, public_B, public_C);
        PROFILE_TOP_STOP();
        hal_send_str("===== End (modified ct) ====");

        // Unmodified ciphertext: result should be 0
        if (result == 1)
        {
            hal_send_str("[FAIL] result == 1 for modified ct");
        }
    }

    return 0;
}

int main(void)
{
    hal_setup();
    test_MaskedComparison();
    return 0;
}