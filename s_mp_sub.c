#include "tommath_private.h"
#ifdef S_MP_SUB_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

/* low level subtraction (assumes |a| > |b|), HAC pp.595 Algorithm 14.9 */
mp_err s_mp_sub(const mp_int *a, const mp_int *b, mp_int *c)
{
   int oldused = c->used, min = b->used, max = a->used, i;
#ifdef MP_32BIT
#ifndef STM32
   mp_digit tmp, tmp0;
#endif
#endif
   mp_digit u;
   mp_err err;

#ifdef STM32
	volatile uint32_t base_address_op1, base_address_op2, base_address_res, limit_1, limit_2;
#endif

   /* init result */
   if ((err = mp_grow(c, max)) != MP_OKAY) {
      return err;
   }

   c->used = max;

#ifdef STM32

	base_address_op1 = &(a->dp[0]);
	base_address_op2 = &(b->dp[0]);
	base_address_res = &(c->dp[0]);
	limit_1 = min;
	limit_2 = max-min+1;

	// Note that in ARM asm the carry is set whenever there is no borrow and clear whenever there is
	__asm__ volatile (
		//Load the variables in cpu registers
		"MOV %%r3, %0;" // base_address_op1
		"MOV %%r4, %1;" // base_address_op2
		"MOV %%r5, %2;" // base_address_res
		"MOV %%r6, %3;" // limit_1

		"LDR %%r0, [%%r3];" // Load 1st operand
		"LDR %%r1, [%%r4];" // Load 2nd operand
		"SUBS %%r2, %%r0, %%r1;" // Perform the subtraction of the least significant digit
		"STR %%r2, [%%r5];" // Store the result

		"LOOP_1:" // Check for first loop exit condition
		"SUB %%r6, %%r6, $1;"
		"CBZ %%r6, EXIT_1;"

		"ADD %%r3, %%r3, $4;" // Increase the addresses
		"ADD %%r4, %%r4, $4;"
		"ADD %%r5, %%r5, $4;"

		"LDR %%r0, [%%r3];"
		"LDR %%r1, [%%r4];"
		"SBCS %%r2, %%r0, %%r1 ;" // Perform the subtraction, considering the carry from the previous one
		"STR %%r2, [%%r5];"
		"B LOOP_1;"

		"EXIT_1:"
		"MOV %%r6, %4;" // limit_2

		"LOOP_2:" // Now just the carry propagation has to be computed

		"SUB %%r6, %%r6, $1;" // Check for end of loop
		"CBZ %%r6, EXIT_2;"
		"ADD %%r3, %%r3, $4;" // Increase the addresses
		"ADD %%r5, %%r5, $4;"
		"LDR %%r2, [%%r3];"   // Manage the borrow
		"SBCS %%r2, $0 ;"
		"STR %%r2, [%%r5];"
		"B LOOP_2;"

		"EXIT_2:"

		:
		:"r" (base_address_op1), "r" (base_address_op2),
			 "r" (base_address_res), "r" (limit_1), "r" (limit_2)
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "memory", "cc");
#else

   /* set carry to zero */
   u = 0;
   for (i = 0; i < min; i++) {
#ifdef MP_32BIT
		tmp0 = a->dp[i] - b->dp[i];
		tmp = tmp0 - u;

		// Check if there has been a borrow
		u = 0;
		if(tmp0 > a->dp[i]) {
			u++;
		}
		else if(tmp > tmp0) {
			u++;
		}

		c->dp[i] = tmp;
#else
      /* T[i] = A[i] - B[i] - U */
      c->dp[i] = (a->dp[i] - b->dp[i]) - u;

      /* U = carry bit of T[i]
       * Note this saves performing an AND operation since
       * if a carry does occur it will propagate all the way to the
       * MSB.  As a result a single shift is enough to get the carry
       */
      u = c->dp[i] >> (MP_SIZEOF_BITS(mp_digit) - 1u);

      /* Clear carry from T[i] */
      c->dp[i] &= MP_MASK;
#endif
   }

   /* now copy higher words if any, e.g. if A has more digits than B  */
   for (; i < max; i++) {
#ifdef MP_32BIT
		tmp0 = a->dp[i] - u;
		if(tmp0 > a->dp[i]) {
			u = 1;
		}
		else {
			u = 0;
		}
		c->dp[i] = tmp0;
#else
      /* T[i] = A[i] - U */
      c->dp[i] = a->dp[i] - u;

      /* U = carry bit of T[i] */
      u = c->dp[i] >> (MP_SIZEOF_BITS(mp_digit) - 1u);

      /* Clear carry from T[i] */
      c->dp[i] &= MP_MASK;
#endif
   }


#endif
   /* clear digits above used (since we may not have grown result above) */
   s_mp_zero_digs(c->dp + c->used, oldused - c->used);

   mp_clamp(c);
   return MP_OKAY;
}

#endif
