/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@php.net>                             |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Pedro Melo <melo@ip.pt>                                     |
   |          Sterling Hughes <sterling@php.net>                          |
   |                                                                      |
   | Based on code from: Richard J. Wagner <rjwagner@writeme.com>         |
   |                     Makoto Matsumoto <matumoto@math.keio.ac.jp>      |
   |                     Takuji Nishimura                                 |
   |                     Shawn Cokus <Cokus@math.washington.edu>          |
   +----------------------------------------------------------------------+
 */

#include "pch.h"
#include "mt_rand.h"

/* MT RAND FUNCTIONS */

/*
    The following php_mt_...() functions are based on a C++ class MTRand by
    Richard J. Wagner. For more information see the web page at
    http://www-personal.engin.umich.edu/~wagnerr/MersenneTwister.html

    Mersenne Twister random number generator -- a C++ class MTRand
    Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
    Richard J. Wagner  v1.0  15 May 2003  rjwagner@writeme.com

    The Mersenne Twister is an algorithm for generating random numbers.  It
    was designed with consideration of the flaws in various other generators.
    The period, 2^19937-1, and the order of equidistribution, 623 dimensions,
    are far greater.  The generator is also fast; it avoids multiplication and
    division, and it benefits from caches and pipelines.  For more information
    see the inventors' web page at http://www.math.keio.ac.jp/~matumoto/emt.html

    Reference
    M. Matsumoto and T. Nishimura, "Mersenne Twister: A 623-Dimensionally
    Equidistributed Uniform Pseudo-Random Number Generator", ACM Transactions on
    Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.

    Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
    Copyright (C) 2000 - 2003, Richard J. Wagner
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    3. The names of its contributors may not be used to endorse or promote
       products derived from this software without specific prior written
       permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

static inline void php_mt_initialize(uint32_t seed, uint32_t *state)
{
    /* Initialize generator state with seed
       See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
       In previous versions, most significant bits (MSBs) of the seed affect
       only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto. */

    register uint32_t *s = state;
    register uint32_t *r = state;
    register int i = 1;

    *s++ = seed & 0xffffffffU;
    for ( ; i < MT_N; ++i ) {
        *s++ = (1812433253U * (*r ^ (*r >> 30)) + i) & 0xffffffffU;
        r++;
    }
}

static inline void php_mt_reload(struct mt_s *mt)
{
    /* Generate N new values in state
       Made clearer and faster by Matthew Bellew (matthew.bellew@home.com) */

    register uint32_t *state = mt->state;
    register uint32_t *p = mt->state;
    register int i;

    for ( i = MT_N - MT_M; i--; ++p )
        *p = twist(p[MT_M], p[0], p[1]);
    for ( i = MT_M; --i; ++p )
        *p = twist(p[MT_M - MT_N], p[0], p[1]);
    *p = twist(p[MT_M - MT_N], p[0], state[0]);
    mt->left = MT_N;
    mt->next= state;
}

void php_mt_srand(struct mt_s *mt, uint32_t seed)
{
    /* Seed the generator with a simple uint32 */

    php_mt_initialize(seed, mt->state);
    php_mt_reload(mt);
}

uint32_t php_mt_rand(struct mt_s *mt)
{
    /* Pull a 32-bit integer from the generator state
       Every other access function simply transforms the numbers extracted here */

    register uint32_t s1;

    if ( mt->left == 0 ) {
        php_mt_reload(mt);
    }
    --mt->left;

    s1 = *mt->next++;
    s1 ^= (s1 >> 11);
    s1 ^= (s1 << 7) & 0x9d2c5680U;
    s1 ^= (s1 << 15) & 0xefc60000U;
    return (s1 ^ (s1 >> 18));
}

/*
 * A bit of tricky math here.  We want to avoid using a modulus because
 * that simply tosses the high-order bits and might skew the distribution
 * of random values over the range.  Instead we map the range directly.
 *
 * We need to map the range from 0...M evenly to the range a...b
 * Let n = the random number and n' = the mapped random number
 *
 * Then we have: n' = a + n(b-a)/M
 *
 * We have a problem here in that only n==M will get mapped to b which
 # means the chances of getting b is much much less than getting any of
 # the other values in the range.  We can fix this by increasing our range
 # artifically and using:
 #
 #               n' = a + n(b-a+1)/M
 *
 # Now we only have a problem if n==M which would cause us to produce a
 # number of b+1 which would be bad.  So we bump M up by one to make sure
 # this will never happen, and the final algorithm looks like this:
 #
 #               n' = a + n(b-a+1)/(M+1)
 *
 * -RL
 */

long mt_rand_range(struct mt_s *mt, long min, long max)
{
    long number;

    /*
     * Melo: hmms.. randomMT() returns 32 random bits...
     * Yet, the previous php_rand only returns 31 at most.
     * So I put a right shift to loose the lsb. It *seems*
     * better than clearing the msb.
     * Update:
     * I talked with Cokus via email and it won't ruin the algorithm
     */
    number = (long)(php_mt_rand(mt) >> 1);
    if ( max ) {
        RAND_RANGE(number, min, max, PHP_MT_RAND_MAX);
    }

    return number;
}

long mt_getrandmax(void)
{
    /*
     * Melo: it could be 2^^32 but we only use 2^^31 to maintain
     * compatibility with the previous php_rand
     */
    return PHP_MT_RAND_MAX; /* 2^^31 */
}
