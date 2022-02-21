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
#include "ReduceComparisons.h"
#include "randombytes.h"

#ifdef CW
extern void clear_regs(void);
#endif // CW

void ReduceComparisons(uint64_t E[NSHARES], const uint64_t D[NCOEFFS_B + NCOEFFS_C][NSHARES])
{
    for (size_t j = 0; j < NSHARES; j++)
    {
        E[j] = 0;
    }

    for (size_t i = 0; i < (NCOEFFS_B + NCOEFFS_C); i++)
    {
        uint64_t R = random_uint64();

#ifdef DEBUG
        uint64_t D_unmasked = 0;
#endif

        for (size_t j = 0; j < NSHARES; j++)
        {
            E[j] += R * D[i][j];
#ifdef CW
			clear_regs();
#endif
#ifdef DEBUG
            D_unmasked += D[i][j];
#endif
        }

#ifdef DEBUG
        if (D_unmasked != 0)
        {
            printf("[ReduceComparisons] : Ciphertext modification: D[%ld] = %ld.\n", i, D_unmasked);
        }
#endif
    }
}