#ifndef _ASM_RISCV_ATOMIC_H
#define _ASM_RISCV_ATOMIC_H

#include <linux/compiler.h>
#include <linux/irqflags.h>
#include <asm/cmpxchg.h>
#include <asm/barrier.h>

#define ATOMIC_INIT(i)	{ (i) }

/**
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically reads the value of @v.
 */
static inline int atomic_read(const atomic_t *v)
{
	return *((volatile int *)(&(v->counter)));
}

/**
 * atomic_set - set atomic variable
 * @v: pointer of type atomic_t
 * @i: required value
 *
 * Atomically sets the value of @v to @i.
 */
static inline void atomic_set(atomic_t *v, int i)
{
	__asm__ __volatile__ (
		"amoswap.w x0, %1, 0(%0)"
		:
		: "r" (&(v->counter)), "r" (i)
		: "memory");
}

/**
 * atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v.
 */
static inline void atomic_add(int i, atomic_t *v)
{
	__asm__ __volatile__ (
		"amoadd.w x0, %1, 0(%0)"
		:
		: "r" (&(v->counter)), "r" (i)
		: "memory");
}

/**
 * atomic_sub - subtract integer from atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v.
 */
static inline void atomic_sub(int i, atomic_t *v)
{
	return atomic_add(-i, v);
}

/**
 * atomic_add_return - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v and returns the result
 */
static inline int atomic_add_return(int i, atomic_t *v)
{
	register int c;
	__asm__ __volatile__ (
		"amoadd.w %0, %2, 0(%1)"
		: "=r" (c)
		: "r" (&(v->counter)), "r" (i)
		: "memory");
	return (c + i);
}

/**
 * atomic_sub_return - subtract integer from atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v and returns the result
 */
static inline int atomic_sub_return(int i, atomic_t *v)
{
	return atomic_add_return(-i, v);
}

/**
 * atomic_inc - increment atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1.
 */
static inline void atomic_inc(atomic_t *v)
{
	atomic_add(1, v);
}

/**
 * atomic_dec - decrement atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1.
 */
static inline void atomic_dec(atomic_t *v)
{
	atomic_add(-1, v);
}

static inline int atomic_inc_return(atomic_t *v)
{
	return atomic_add_return(1, v);
}

static inline int atomic_dec_return(atomic_t *v)
{
	return atomic_sub_return(1, v);
}

/**
 * atomic_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic_sub_and_test(int i, atomic_t *v)
{
	return (atomic_sub_return(i, v) == 0);
}

/**
 * atomic_inc_and_test - increment and test
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic_inc_and_test(atomic_t *v)
{
	return (atomic_inc_return(v) == 0);
}

/**
 * atomic_dec_and_test - decrement and test
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static inline int atomic_dec_and_test(atomic_t *v)
{
	return (atomic_dec_return(v) == 0);
}

/**
 * atomic_add_negative - add and test if negative
 * @i: integer value to add
 * @v: pointer of type atomic_t
 * 
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.
 */
static inline int atomic_add_negative(int i, atomic_t *v)
{
	return (atomic_add_return(i, v) < 0);
}


static inline int atomic_xchg(atomic_t *v, int n)
{
	register int c;
	__asm__ __volatile__ (
		"amoswap.w %0, %2, 0(%1)"
		: "=r" (c)
		: "r" (&(v->counter)), "r" (n)
		: "memory");
	return c;
}

#ifndef CONFIG_SMP

static inline int atomic_cmpxchg(atomic_t *v, int o, int n)
{
	int prev;
	unsigned long flags;

	local_irq_save(flags);
	if ((prev = v->counter) == o)
		v->counter = n;
	local_irq_restore(flags);
	return prev;
}

#define cmpxchg_local(ptr, o, n)                                \
	((__typeof__(*(ptr)))__cmpxchg_local_generic((ptr),     \
		(unsigned long)(o), (unsigned long)(n),         \
		sizeof(*(ptr))))

#define cmpxchg64_local(ptr, o, n) \
	__cmpxchg64_local_generic((ptr), (o), (n))

#else
#error "SMP not supported by current cmpxchg implementation"
#endif /* !CONFIG_SMP */

/**
 * __atomic_add_unless - add unless the number is already a given value
 * @v: pointer of type atomic_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so long as @v was not already @u.
 * Returns the old value of @v.
 */
static inline int __atomic_add_unless(atomic_t *v, int a, int u)
{
	int c, old;
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == u))
			break;
		old = atomic_cmpxchg(v, c, c + a);
		if (likely(old == c))
			break;
		c = old;
	}
	return c;
}

/**
 * atomic_clear_mask - Atomically clear bits in atomic variable
 * @mask: Mask of the bits to be cleared
 * @v: pointer of type atomic_t
 *
 * Atomically clears the bits set in @mask from @v
 */
static inline void atomic_clear_mask(unsigned int mask, atomic_t *v)
{
	__asm__ __volatile__ (
		"amoand.w x0, %1, 0(%0)"
		:
		: "r" (&(v->counter)), "r" (~mask)
		: "memory");
}

/**
 * atomic_set_mask - Atomically set bits in atomic variable
 * @mask: Mask of the bits to be set
 * @v: pointer of type atomic_t
 *
 * Atomically sets the bits set in @mask in @v
 */
static inline void atomic_set_mask(unsigned int mask, atomic_t *v)
{
	__asm__ __volatile__ (
		"amoor.w x0, %1, 0(%0)"
		:
		: "r" (&(v->counter)), "r" (mask)
		: "memory");
}

/* Assume that atomic operations are already serializing */
#define smp_mb__before_atomic_dec()	barrier()
#define smp_mb__after_atomic_dec()	barrier()
#define smp_mb__before_atomic_inc()	barrier()
#define smp_mb__after_atomic_inc()	barrier()

#include <asm/atomic64.h>

#endif /* _ASM_RISCV_ATOMIC_H */
