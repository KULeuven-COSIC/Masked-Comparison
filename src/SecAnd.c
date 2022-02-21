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
#include "SecAnd.h"
#include "randombytes.h"

#ifdef CW
extern uint32_t SecAND32_asm(uint32_t x, uint32_t y, uint32_t r);
extern void clear_regs(void);
#endif // CW



// [http://www.crypto-uni.lu/jscoron/publications/secconvorder.pdf, Algorithm 1]
void SecAND32(size_t nshares, uint32_t z[nshares], const uint32_t x[nshares], const uint32_t y[nshares])
{
	uint32_t r[nshares][nshares];

	for (size_t i = 0; i < nshares; i++)
	{
		for (size_t j = (i + 1); j < nshares; j++)
		{
#ifdef  CW
			r[i][j] = random_uint32();
			r[j][i] = SecAND32_asm(x[i], y[j], r[i][j]);
			r[j][i] = SecAND32_asm(x[j], y[i], r[j][i]);

#else

			r[i][j] = random_uint32();
			r[j][i] = r[i][j] ^ (x[i] & y[j]);
			r[j][i] = r[j][i] ^ (x[j] & y[i]);
#endif //  CW
		}
	}

	for (size_t i = 0; i < nshares; i++)
	{
		z[i] = x[i] & y[i];
		for (size_t j = 0; j < nshares; j++)
		{
			if (i != j)
			{
				z[i] ^= r[i][j];
			}
		}
#ifdef CW
		clear_regs()
#endif
	}

#ifdef DEBUG
	uint32_t x_unmasked = 0;
	uint32_t y_unmasked = 0;
	uint32_t z_unmasked = 0;

	for (size_t j = 0; j < nshares; j++)
	{
		x_unmasked ^= x[j];
		y_unmasked ^= y[j];
		z_unmasked ^= z[j];
	}

	assert(z_unmasked == (x_unmasked & y_unmasked));
#endif
}

// [http://www.crypto-uni.lu/jscoron/publications/secconvorder.pdf, Algorithm 1]
void SecAND64(size_t nshares, uint64_t z[nshares], const uint64_t x[nshares], const uint64_t y[nshares])
{
	uint64_t r[nshares][nshares];

	for (size_t i = 0; i < nshares; i++)
	{
		for (size_t j = (i + 1); j < nshares; j++)
		{
			r[i][j] = random_uint64();
			r[j][i] = r[i][j] ^ (x[i] & y[j]);
			r[j][i] = r[j][i] ^ (x[j] & y[i]);
		}
	}

	for (size_t i = 0; i < nshares; i++)
	{
		z[i] = x[i] & y[i];
		for (size_t j = 0; j < nshares; j++)
		{
			if (i != j)
			{
				z[i] ^= r[i][j];
			}
		}
	}

#ifdef DEBUG
	uint64_t x_unmasked = 0;
	uint64_t y_unmasked = 0;
	uint64_t z_unmasked = 0;

	for (size_t j = 0; j < nshares; j++)
	{
		x_unmasked ^= x[j];
		y_unmasked ^= y[j];
		z_unmasked ^= z[j];
	}

	assert(z_unmasked == (x_unmasked & y_unmasked));
#endif
}