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
#include "SecAdd.h"
#include "SecAnd.h"
#include "randombytes.h"

#ifdef DEBUG
#include "bitmask.h"
#endif

#ifdef CW
extern void secure_assign(uint32_t* dest, uint32_t* src);
extern uint32_t SecXOR32_asm(uint32_t x, uint32_t y);
extern void clear_regs(void);
#endif

static void SecXOR32(size_t nshares, uint32_t z[nshares], const uint32_t x[nshares], const uint32_t y[nshares])
{
	for (size_t i = 0; i < nshares; i++)
	{
#ifdef CW
		z[i] = SecXOR32_asm(x[i], y[i]);
		//z[i] = x[i] ^ y[i];
		clear_regs();
#else
		z[i] = x[i] ^ y[i];
#endif
		

	}
}

static void SecXOR64(size_t nshares, uint64_t z[nshares], const uint64_t x[nshares], const uint64_t y[nshares])
{
	for (size_t i = 0; i < nshares; i++)
	{
		z[i] = x[i] ^ y[i];
	}
}

static void get_bit(size_t nshares, size_t nbits, uint32_t x_bit[nshares], const uint32_t x[nshares][nbits], size_t bit)
{
	for (size_t i = 0; i < nshares; i++)
	{
#ifdef CW
		secure_assign(&x_bit[i], &x[i][bit]);
#else
		x_bit[i] = x[i][bit];
#endif
	}
}

static void write_bit(size_t nshares, size_t nbits, uint32_t x[nshares][nbits], const uint32_t x_bit[nshares], size_t bit)
{
	for (size_t i = 0; i < nshares; i++)
	{
#ifdef CW
		//prevent shares to be overwritten by each other in registers
		secure_assign(&x[i][bit], &x_bit[i]);
#else

		x[i][bit] = x_bit[i];
#endif // CW
	}
}

void SecAdd_bitsliced(size_t nshares, size_t nbits, uint32_t z[nshares][nbits], const uint32_t x[nshares][nbits], const uint32_t y[nshares][nbits])
{
	uint32_t xANDy[nshares], xXORy[nshares], cANDxXORy[nshares];
	uint32_t carry[nshares], sum[nshares];
	uint32_t x_bit[nshares], y_bit[nshares];

	get_bit(nshares, nbits, x_bit, x, 0);
	get_bit(nshares, nbits, y_bit, y, 0);

	SecAND32(nshares, carry, x_bit, y_bit);
	SecXOR32(nshares, sum, x_bit, y_bit);

	write_bit(nshares, nbits, z, sum, 0);

	for (size_t i = 1; i < nbits; i++)
	{
		get_bit(nshares, nbits, x_bit, x, i);
		get_bit(nshares, nbits, y_bit, y, i);

		// sum
		SecXOR32(nshares, xXORy, x_bit, y_bit);
		SecXOR32(nshares, sum, xXORy, carry);

		/* carry out : implemented with (2) to reduce SecAND's
		*  	(1) c_out = (c_in AND x) XOR (c_in AND y) XOR (x AND y) [http://www.crypto-uni.lu/jscoron/publications/secconvorder.pdf, Algorithm 2]
		* 	(2) c_out = (c_in AND (x XOR y)) XOR (x AND y) *
		*/
		if (i != nbits - 1) //* nbits - 1 because we don't need final carry
		{
			SecAND32(nshares, xANDy, x_bit, y_bit);
			SecAND32(nshares, cANDxXORy, xXORy, carry);
			SecXOR32(nshares, carry, cANDxXORy, xANDy);
		}

		write_bit(nshares, nbits, z, sum, i);
	}

#ifdef DEBUG
	for (size_t i = 0; i < 32; i++)
	{
		uint32_t x_unmasked = 0;
		uint32_t y_unmasked = 0;
		uint32_t z_unmasked = 0;

		for (size_t j = 0; j < nshares; j++)
		{
			for (size_t k = 0; k < nbits; k++)
			{
				x_unmasked = (x_unmasked ^ (((x[j][k] & (1 << i)) >> i) << k)) & bit_mask(nbits);
				y_unmasked = (y_unmasked ^ (((y[j][k] & (1 << i)) >> i) << k)) & bit_mask(nbits);
				z_unmasked = (z_unmasked ^ (((z[j][k] & (1 << i)) >> i) << k)) & bit_mask(nbits);
			}
		}

		assert(z_unmasked == ((x_unmasked + y_unmasked) & bit_mask(nbits)));
	}
#endif
}

/*
* [http://www.crypto-uni.lu/jscoron/publications/secconvorder.pdf]
*
* This is roughly Algorithm 3: we make the same optimisation (1 SecAND in the loop), but keep everything bit-wise
*/
void SecAdd(size_t nshares, uint64_t z[nshares], const uint64_t x[nshares], const uint64_t y[nshares])
{
	uint64_t xXORy[nshares], xANDy[nshares];
	uint32_t c_bit[nshares], xANDy_bit[nshares], cANDxXORy_bit[nshares], xXORy_bit[nshares];

	for (size_t i = 0; i < nshares; i++)
	{
		z[i] = 0;
		c_bit[i] = 0;
	}

	SecAND64(nshares, xANDy, x, y);
	// sum
	SecXOR64(nshares, xXORy, x, y);

	// carry out
	for (size_t j = 0; j < 64 - 1; j++) //* (nbits=64) - 1 because we don't need final carry
	{
		for (size_t i = 0; i < nshares; i++)
		{
			xANDy_bit[i] = (xANDy[i] >> j) & 0x1;
			xXORy_bit[i] = (xXORy[i] >> j) & 0x1;
		}

		SecAND32(nshares, cANDxXORy_bit, c_bit, xXORy_bit);

		for (size_t i = 0; i < nshares; i++)
		{
			c_bit[i] = xANDy_bit[i] ^ cANDxXORy_bit[i];
			z[i] |= (uint64_t)(c_bit[i] & 0x1) << (j + 1);
		}
	}

	SecXOR64(nshares, z, xXORy, z);

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

	assert(z_unmasked == (x_unmasked + y_unmasked));
#endif
}