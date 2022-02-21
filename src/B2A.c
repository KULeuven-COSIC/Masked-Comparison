// [https://tches.iacr.org/index.php/TCHES/article/view/873/825]
// [https://pastebin.com/WKnNyEU8]

// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published
// by the Free Software Foundation.

#include "B2A.h"
#include "randombytes.h"

#ifdef CW
extern uint64_t combine1_asm(uint64_t rnd1, uint64_t rnd2, uint64_t x1, uint64_t x2);
extern uint64_t combine2_asm(uint64_t rnd1, uint64_t rnd2, uint64_t x0, uint64_t x1, uint64_t x2);
extern void clear_regs(void);
#endif // DEBUG



#ifdef DEBUG

static uint64_t xorop(uint64_t a[], size_t n)
{
	uint64_t r = 0;
	for (size_t i = 0; i < n; i++)
		r ^= a[i];
	return r;
}

static uint64_t addop(uint64_t a[], size_t n)
{
	uint64_t r = 0;
	for (size_t i = 0; i < n; i++)
		r += a[i];
	return r;
}

#endif

static void refresh(uint64_t a[], size_t n)
{
	for (size_t i = 1; i < n; i++)
	{
		uint64_t tmp = random_uint64();
		a[0] = a[0] ^ tmp;
		a[i] = a[i] ^ tmp;
	}
}

static uint64_t Psi(uint64_t x, uint64_t y)
{
	return (x ^ y) - y;
}

static uint64_t Psi0(uint64_t x, uint64_t y, size_t n)
{
	return Psi(x, y) ^ ((~n & 1) * x);
}

static void copy(uint64_t *x, uint64_t *y, size_t n)
{
	for (size_t i = 0; i < n; i++)
		x[i] = y[i];
}

// here, x contains n+1 shares
static void impconvBA_rec(uint64_t *D, uint64_t *x, size_t n)
{
	if (n == 2)
	{
		#ifdef CW
		uint64_t r1 = random_uint64();
		uint64_t r2 = random_uint64();
		D[0] = combine1_asm(r1, r2, x[1], x[2]);
		clear_regs();
		D[1] = combine2_asm(r1, r2, x[1], x[2], x[0]);
		clear_regs();
		#else
		uint64_t r1 = random_uint64();
		uint64_t r2 = random_uint64();
		uint64_t y0 = (x[0] ^ r1) ^ r2;
		uint64_t y1 = x[1] ^ r1;
		uint64_t y2 = x[2] ^ r2;

		uint64_t z0 = y0 ^ Psi(y0, y1);
		uint64_t z1 = Psi(y0, y2);

		D[0] = y1 ^ y2;
		D[1] = z0 ^ z1;
		#endif // CW

		

#ifdef DEBUG
		assert((x[0] ^ x[1] ^ x[2]) == (D[0] + D[1]));
#endif

		return;
	}

	uint64_t y[n + 1];
	copy(y, x, n + 1);

	refresh(y, n + 1);

	uint64_t z[n];

	z[0] = Psi0(y[0], y[1], n);
	for (size_t i = 1; i < n; i++)
		z[i] = Psi(y[0], y[i + 1]);

#ifdef DEBUG
	assert(xorop(x, n + 1) == (xorop(y + 1, n) + xorop(z, n)));
#endif

	uint64_t A[n - 1], B[n - 1];
	impconvBA_rec(A, y + 1, n - 1);
	impconvBA_rec(B, z, n - 1);

	for (size_t i = 0; i < n - 2; i++)
		D[i] = A[i] + B[i];

	D[n - 2] = A[n - 2];
	D[n - 1] = B[n - 2];

#ifdef DEBUG
	assert(xorop(x, n + 1) == addop(D, n));
#endif
}

void B2A(uint64_t A[NSHARES], uint32_t B[NSHARES])
{
	uint64_t B_ext[NSHARES + 1];
	for (size_t i = 0; i < NSHARES; i++)
	{
		B_ext[i] = B[i];
#ifdef CW
		clear_regs();
#endif // CW

	}
	B_ext[NSHARES] = 0;
	impconvBA_rec(A, B_ext, NSHARES);
}