/*
	This file is part of the ChipWhisperer Example Targets
	Copyright (C) 2012-2017 NewAE Technology Inc.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../hal/hal.h"
#include <stdint.h>
#include <stdlib.h>
#include "simpleserial.h"
#include <string.h>


#include "MaskedComparison.h"
#include "randombytes.h"
#include "bitmask.h"
#include "params.h"


#define NBITS 13

#define NSHARES 2

#undef DEBUG


uint16_t x[NSHARES];
uint16_t y[NSHARES];
uint16_t z[NSHARES];
uint64_t z64[NSHARES];

uint16_t x_fixed;

uint32_t public_B_fixed[NCOEFFS_B], public_C_fixed[NCOEFFS_C];
uint32_t public_B[NCOEFFS_B], public_C[NCOEFFS_C];

static void get_rand(size_t ncoeffs, uint32_t mod, uint32_t x[ncoeffs])
{
	for (size_t j = 0; j < ncoeffs; j++)
	{
		x[j] = random_uint32() % mod;
	}
}

//Helper Functions to test if the setup/comparison algorithm works
static void compress(size_t ncoeffs, size_t compressfrom, size_t compressto, uint32_t submitted_poly[ncoeffs])
{
	for (size_t i = 0; i < ncoeffs; i++)
	{
		submitted_poly[i] >>= (compressfrom - compressto);
	}
}

static int test_MaskedComparison(void)
{
	uint16_t R;
	uint64_t result;
	uint32_t B[NCOEFFS_B][NSHARES], C[NCOEFFS_C][NSHARES];


	get_rand(NCOEFFS_B, Q, public_B);
	get_rand(NCOEFFS_C, P, public_C);

	// create reencrypted poly as a (correct) sharing of public poly
	// first for B
	for (size_t i = 0; i < NCOEFFS_B; i++)
	{
		uint16_t sum = 0;

		for (size_t j = 0; j < NSHARES - 1; j++)
		{
			
			R = (uint16_t) random_uint32() & bit_mask(NBITS);
			sum += R;
			B[i][j] = R;
		}

		B[i][NSHARES - 1] = public_B[i] - sum;
	}

	// then for C
	for (size_t i = 0; i < NCOEFFS_C; i++)
	{
		uint16_t sum = 0;

		for (size_t j = 0; j < NSHARES - 1; j++)
		{
			R = (uint16_t)random_uint32() & bit_mask(NBITS);
			sum += R;
			C[i][j] = R;
		}

		C[i][NSHARES - 1] = public_C[i] - sum;
	}

	// Now compress public poly's
	compress(NCOEFFS_B, COMPRESSFROM_B, COMPRESSTO_B, public_B);
	compress(NCOEFFS_C, COMPRESSFROM_C, COMPRESSTO_C, public_C);

	result = MaskedComparison(B, C, public_B, public_C);

	if (result != 1)
	{
		led_error(1);
	}
	else
	{
		led_ok(1);
	}


	//Generate modified ciphertext, which should result in faulty comparison
	public_B[4] -= 1;
	public_C[31] -= 7;
	
	result = MaskedComparison(B, C, public_B, public_C);


	if (result == 1)
	{
		led_error(1);
	}
	else
	{
		led_ok(1);
	}

	return 0;
}

//sets the polynomials to be inputted to the comparison algorithms (fixed ones or random ones)
uint8_t get_key(uint8_t* k, uint8_t len)
{
	// Load key here
	//create re-encrypted poly (either the input fixed public poly or completely random
	if (k[0] % 2 == 0) {
		get_rand(COMPRESSFROM_B, NCOEFFS_B, public_B);
		get_rand(COMPRESSFROM_C, NCOEFFS_C, public_C);
	}
	else {
		memcpy(public_B, public_B_fixed, NCOEFFS_B * sizeof(uint32_t));
		memcpy(public_C, public_C_fixed, NCOEFFS_C * sizeof(uint32_t));
	}
	
	//get_rand(NBITS, y);

	return 0x00;
}

//captures a trace of the masked comparison
#if SS_VER == SS_VER_2_0
uint8_t get_pt(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t* pt)
#else
uint8_t get_pt(uint8_t* pt, uint8_t len)
#endif
{
	uint16_t R;
	uint64_t result;
	uint32_t B[NCOEFFS_B][NSHARES], C[NCOEFFS_C][NSHARES];

	// create sharing of re-encrypted poly
	// first for B
	for (size_t i = 0; i < NCOEFFS_B; i++)
	{
		uint16_t sum = 0;

		for (size_t j = 0; j < NSHARES - 1; j++)
		{
			R = (uint16_t)random_uint32() & bit_mask(NBITS);
			sum += R;
			B[i][j] = R;

		}

		B[i][NSHARES - 1] = (public_B[i] - sum) & bit_mask(NBITS);
	}

	// then for C
	for (size_t i = 0; i < NCOEFFS_C; i++)
	{
		uint16_t sum = 0;

		for (size_t j = 0; j < NSHARES - 1; j++)
		{
			R = (uint16_t)random_uint32() & bit_mask(NBITS);
			sum += R;
			C[i][j] = R;
		}

		C[i][NSHARES - 1] = (public_C[i] - sum) & bit_mask(NBITS);
	}

	//always fix the public poly (=> non-sensitive data)
	memcpy(public_B, public_B_fixed, NCOEFFS_B * sizeof(uint32_t));
	memcpy(public_C, public_C_fixed, NCOEFFS_C * sizeof(uint32_t));
	
	// Now compress public poly's
	compress(NCOEFFS_B, COMPRESSFROM_B, COMPRESSTO_B, public_B);
	compress(NCOEFFS_C, COMPRESSFROM_C, COMPRESSTO_C, public_C);

	//add exemplary noise for FNvR test on one coefficient of public poly
	public_C[0] += 1;


	trigger_high();
	result = MaskedComparison( B, C, public_B, public_C);
	trigger_low();


	simpleserial_put('r', 16, pt);

	return 0x00;
}

uint8_t reset(uint8_t* x, uint8_t len)
{
	// Reset key here if needed
	return 0x00;
}

int main(void)
{


	platform_init();
	init_uart();
	trigger_setup();

	//Sets the fixed polys for fixed vs random t-test
	get_rand(COMPRESSFROM_B, NCOEFFS_B, public_B_fixed);
	get_rand(COMPRESSFROM_C, NCOEFFS_C, public_C_fixed);

	simpleserial_init();
	simpleserial_addcmd('p', 16, get_pt);
#if SS_VER != SS_VER_2_0
	simpleserial_addcmd('k', 16, get_key);
	simpleserial_addcmd('x', 0, reset);
#endif

	test_MaskedComparison();

	while (1)
		simpleserial_get();
}
