// Randombytes imlpementation. 
// Copyright unclear.
#include "randombytes.h"

// Use fully deterministic randomness, using an insecure PRNG from a fixed
// seed.  This is only meant for debugging purposes.
struct {
    uint32_t a, b, c, d;
} xorshift128_state = {
    .a = 0x12345678,
    .b = 0xAAAAAAA1,
    .c = 0xAAAAAAA2,
    .d = 0xAAAAAAA3,
};

static int xorshift128(void)
{
	// Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs"
    // This version is taken from Wikipedia.
	uint32_t t = xorshift128_state.d;

	uint32_t const s = xorshift128_state.a;
	xorshift128_state.d = xorshift128_state.c;
	xorshift128_state.c = xorshift128_state.b;
	xorshift128_state.b = s;

	t ^= t << 11;
	t ^= t >> 8;
    xorshift128_state.a = t ^ s ^ (s >> 19);
    return (int)xorshift128_state.a;
}

int randombytes(uint8_t *obuf, size_t len)
{
    union
    {
        unsigned char aschar[4];
        uint32_t asint;
    } random;

    while (len > 4)
    {
        random.asint = xorshift128();
        *obuf++ = random.aschar[0];
        *obuf++ = random.aschar[1];
        *obuf++ = random.aschar[2];
        *obuf++ = random.aschar[3];
        len -= 4;
    }
    if (len > 0)
    {
        for (random.asint = xorshift128(); len > 0; --len)
        {
            *obuf++ = random.aschar[len - 1];
        }
    }

    return 0;
}

#ifdef DEBUG

uint32_t rng_get_random_blocking()
{
	uint32_t R;

    randombytes((uint8_t*)&R, 4);

    return R;
}

#endif // DEBUG

#if defined(PROFILE_STEP_RAND) || defined(PROFILE_TOP_RAND)

uint64_t nb_randombytes;

uint32_t rng_count_get_random_blocking()
{
	nb_randombytes += 4; 

    return rng_get_random_blocking();
}

#endif 