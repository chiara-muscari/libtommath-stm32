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

/* find the n'th root of an integer 
 *
 * Result found such that (c)^b <= a and (c+1)^b > a 
 */
int
mp_n_root (mp_int * a, mp_digit b, mp_int * c)
{
  mp_int    t1, t2, t3;
  int       res, neg;

  /* input must be positive if b is even */
  if ((b & 1) == 0 && a->sign == MP_NEG) {
    return MP_VAL;
  }

  if ((res = mp_init (&t1)) != MP_OKAY) {
    return res;
  }

  if ((res = mp_init (&t2)) != MP_OKAY) {
    goto __T1;
  }

  if ((res = mp_init (&t3)) != MP_OKAY) {
    goto __T2;
  }

  /* if a is negative fudge the sign but keep track */
  neg = a->sign;
  a->sign = MP_ZPOS;

  /* t2 = 2 */
  mp_set (&t2, 2);

  do {
    /* t1 = t2 */
    if ((res = mp_copy (&t2, &t1)) != MP_OKAY) {
      goto __T3;
    }

    /* t2 = t1 - ((t1^b - a) / (b * t1^(b-1))) */
    if ((res = mp_expt_d (&t1, b - 1, &t3)) != MP_OKAY) {	/* t3 = t1^(b-1) */
      goto __T3;
    }

    /* numerator */
    if ((res = mp_mul (&t3, &t1, &t2)) != MP_OKAY) {	/* t2 = t1^b */
      goto __T3;
    }

    if ((res = mp_sub (&t2, a, &t2)) != MP_OKAY) {	/* t2 = t1^b - a */
      goto __T3;
    }

    if ((res = mp_mul_d (&t3, b, &t3)) != MP_OKAY) {	/* t3 = t1^(b-1) * b  */
      goto __T3;
    }

    if ((res = mp_div (&t2, &t3, &t3, NULL)) != MP_OKAY) {	/* t3 = (t1^b - a)/(b * t1^(b-1)) */
      goto __T3;
    }

    if ((res = mp_sub (&t1, &t3, &t2)) != MP_OKAY) {
      goto __T3;
    }
  }
  while (mp_cmp (&t1, &t2) != MP_EQ);

  /* result can be off by a few so check */
  for (;;) {
    if ((res = mp_expt_d (&t1, b, &t2)) != MP_OKAY) {
      goto __T3;
    }

    if (mp_cmp (&t2, a) == MP_GT) {
      if ((res = mp_sub_d (&t1, 1, &t1)) != MP_OKAY) {
	goto __T3;
      }
    } else {
      break;
    }
  }

  /* reset the sign of a first */
  a->sign = neg;

  /* set the result */
  mp_exch (&t1, c);

  /* set the sign of the result */
  c->sign = neg;

  res = MP_OKAY;

__T3:mp_clear (&t3);
__T2:mp_clear (&t2);
__T1:mp_clear (&t1);
  return res;
}