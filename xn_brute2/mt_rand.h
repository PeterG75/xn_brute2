#pragma once

#define MT_N          624                /* length of state vector */
#define MT_M          397                /* a period parameter */
#define hiBit(u)      ((u) & 0x80000000U)  /* mask all but highest   bit of u */
#define loBit(u)      ((u) & 0x00000001U)  /* mask all but lowest    bit of u */
#define loBits(u)     ((u) & 0x7fffffffU)  /* mask     the highest   bit of u */
#define mixBits(u, v) (hiBit(u)|loBits(v)) /* move hi bit of u to hi bit of v */

#define twist(m, u, v) \
    (m ^ (mixBits(u, v) >> 1) ^ ((uint32_t)(-(int32_t)(loBit(u))) & 0x9908b0dfU))

#define RAND_RANGE(n, rmin, rmax, tmax) \
    (n) = (rmin) + (long)((double)((double)(rmax) - (rmin) + 1.0) * ((n) / ((tmax) + 1.0)))

#define PHP_MT_RAND_MAX 0x7fffffffL /* (1<<31) - 1 */ 

struct mt_s
{
    uint32_t state[MT_N];
    uint32_t left;
    uint32_t *next;
};

void php_mt_srand(struct mt_s *mt, uint32_t seed);

uint32_t php_mt_rand(struct mt_s *mt);

long mt_rand_range(struct mt_s *mt, long min, long max);

#define mt_rand(x) mt_rand_range((x), 0L, 0L)

long mt_getrandmax(void);
