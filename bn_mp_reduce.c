/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is library that provides for multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library is designed directly after the MPI library by
 * Michael Fromberger but has been written from scratch with
 * additional optimizations in place.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@iahu.ca, http://libtommath.iahu.ca
 */
#include <tommath.h>

/* pre-calculate the value required for Barrett reduction
 * For a given modulus "b" it calulates the value required in "a"
 */
int
mp_reduce_setup (mp_int * a, mp_int * b)
{
  int       res;


  if ((res = mp_2expt (a, b->used * 2 * DIGIT_BIT)) != MP_OKAY) {
    return res;
  }
  res = mp_div (a, b, a, NULL);
  return res;
}

/* reduces x mod m, assumes 0 < x < m^2, mu is precomputed via mp_reduce_setup 
 * From HAC pp.604 Algorithm 14.42 
 */
int
mp_reduce (mp_int * x, mp_int * m, mp_int * mu)
{
  mp_int    q;
  int       res, um = m->used;


  if ((res = mp_init_copy (&q, x)) != MP_OKAY) {
    return res;
  }

  mp_rshd (&q, um - 1);		/* q1 = x / b^(k-1)  */

  /* according to HAC this is optimization is ok */
  if (((unsigned long) m->used) > (1UL << (unsigned long) (DIGIT_BIT - 1UL))) {
    if ((res = mp_mul (&q, mu, &q)) != MP_OKAY) {
      goto CLEANUP;
    }
  } else {
    if ((res = s_mp_mul_high_digs (&q, mu, &q, um - 1)) != MP_OKAY) {
      goto CLEANUP;
    }
  }

  mp_rshd (&q, um + 1);		/* q3 = q2 / b^(k+1) */

  /* x = x mod b^(k+1), quick (no division) */
  if ((res = mp_mod_2d (x, DIGIT_BIT * (um + 1), x)) != MP_OKAY) {
    goto CLEANUP;
  }

  /* q = q * m mod b^(k+1), quick (no division) */
  if ((res = s_mp_mul_digs (&q, m, &q, um + 1)) != MP_OKAY) {
    goto CLEANUP;
  }

  /* x = x - q */
  if ((res = mp_sub (x, &q, x)) != MP_OKAY)
    goto CLEANUP;

  /* If x < 0, add b^(k+1) to it */
  if (mp_cmp_d (x, 0) == MP_LT) {
    mp_set (&q, 1);
    if ((res = mp_lshd (&q, um + 1)) != MP_OKAY)
      goto CLEANUP;
    if ((res = mp_add (x, &q, x)) != MP_OKAY)
      goto CLEANUP;
  }

  /* Back off if it's too big */
  while (mp_cmp (x, m) != MP_LT) {
    if ((res = s_mp_sub (x, m, x)) != MP_OKAY)
      break;
  }

CLEANUP:
  mp_clear (&q);

  return res;
}