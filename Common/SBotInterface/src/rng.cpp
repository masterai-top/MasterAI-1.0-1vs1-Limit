
#include "../include/rng.h"

/* NOTE changes made on 2005/9/7 by Neil Burch - if you have problems
   with this code, DON'T complain to Makoto Matsumoto... */


/* initializes mt[RNG_N] with a seed */
void init_genrand( rng_state_t *state, uint32_t s )
{
  state->mt[0]= s & 0xffffffffUL;
  for (state->mti=1; state->mti<RNG_N; state->mti++) {
    state->mt[state->mti] = 
      (1812433253UL * (state->mt[state->mti-1]
		       ^ (state->mt[state->mti-1] >> 30))
       + state->mti); 
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
  }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array( rng_state_t *state, uint32_t init_key[], int key_length )
{
  int i, j, k;

  init_genrand(state, 19650218UL);
  i=1; j=0;
  k = (RNG_N>key_length ? RNG_N : key_length);
  for (; k; k--) {
    state->mt[i] = ( state->mt[i]
		     ^ ( ( state->mt[i-1] ^ ( state->mt[i-1] >> 30 ) )
			 * 1664525UL ) )
      + init_key[j] + j; /* non linear */
    i++; j++;
    if (i>=RNG_N) { state->mt[0] = state->mt[RNG_N-1]; i=1; }
    if (j>=key_length) j=0;
  }
  for (k=RNG_N-1; k; k--) {
    state->mt[i] = ( state->mt[i]
		     ^ ( ( state->mt[i-1] ^ ( state->mt[i-1] >> 30 ) )
			 * 1566083941UL ) )
      - i; /* non linear */
    state->mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
    i++;
    if (i>=RNG_N) { state->mt[0] = state->mt[RNG_N-1]; i=1; }
  }

  state->mt[0]|= 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

/* generates a random number on [0,0xffffffff]-interval */
uint32_t genrand_int32( rng_state_t *state )
{
    uint32_t y;
    static uint32_t mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (state->mti == RNG_N) { /* generate RNG_N words at one time */
        int kk;

        for (kk=0;kk<RNG_N-RNG_M;kk++) {
            y = (state->mt[kk]&UPPER_MASK)|(state->mt[kk+1]&LOWER_MASK);
            state->mt[kk] = state->mt[kk+RNG_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<RNG_N-1;kk++) {
            y = (state->mt[kk]&UPPER_MASK)|(state->mt[kk+1]&LOWER_MASK);
            state->mt[kk] =
	      state->mt[kk+(RNG_M-RNG_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (state->mt[RNG_N-1]&UPPER_MASK)|(state->mt[0]&LOWER_MASK);
        state->mt[RNG_N-1] = state->mt[RNG_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        state->mti = 0;
    }
  
    y = state->mt[state->mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}
