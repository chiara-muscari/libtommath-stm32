#include "tommath_private.h"
#ifdef BN_MP_RAND_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

int (*s_mp_rand_source)(void *, size_t) = s_mp_rand_source_platform;

void mp_rand_source(int (*source)(void *out, size_t size))
{
   s_mp_rand_source = (source == NULL) ? s_mp_rand_source_platform : source;
}

/* makes a pseudo-random int of a given size */
int mp_rand_digit(mp_digit *r)
{
   int ret = s_mp_rand_source(r, sizeof(mp_digit));
   *r &= MP_MASK;
   return ret;
}

int mp_rand(mp_int *a, int digits)
{
   int ret, i;

   mp_zero(a);

   if (digits <= 0) {
      return MP_OKAY;
   }

   if ((ret = mp_grow(a, digits)) != MP_OKAY) {
      return ret;
   }

   if ((ret = s_mp_rand_source(a->dp, (size_t)digits * sizeof(mp_digit))) != MP_OKAY) {
      return ret;
   }

   /* TODO: We ensure that the highest digit is nonzero. Should this be removed? */
   while ((a->dp[digits - 1] & MP_MASK) == 0) {
      if ((ret = s_mp_rand_source(a->dp + digits - 1, sizeof(mp_digit))) != MP_OKAY) {
         return ret;
      }
   }

   a->used = digits;
   for (i = 0; i < digits; ++i) {
      a->dp[i] &= MP_MASK;
   }

   return MP_OKAY;
}
#endif
