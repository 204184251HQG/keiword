#ifndef _KEIATOMIC_H_
#define _KEIATOMIC_H_


/*
 * Atomic data structure, initialization, and access.
 */

typedef struct { volatile int counter; } atomic_t;

#define ATOMIC_INIT(i)  { (i) }

#define atomic_read(v) \
	__atomic_load_n(&(v)->counter, __ATOMIC_RELAXED)
#define atomic_set(v, i) \
	__atomic_store_n(&(v)->counter, (i), __ATOMIC_RELAXED)
#define smp_load_acquire(p) \
	__atomic_load_n(p, __ATOMIC_ACQUIRE)
#define smp_store_release(p, i) \
	__atomic_store_n(p, (i), __ATOMIC_RELEASE)

/*
 * Atomic operations.
 */

/**
 * atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v.
 */
static __inline__ void atomic_add(int i, atomic_t *v)
{
	__atomic_add_fetch(&v->counter, i, __ATOMIC_RELAXED);
}

/**
 * atomic_sub - subtract the atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v.
 */
static __inline__ void atomic_sub(int i, atomic_t *v)
{
	__atomic_sub_fetch(&v->counter, i, __ATOMIC_RELAXED);
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
static __inline__ int atomic_sub_and_test(int i, atomic_t *v)
{
	return __atomic_sub_fetch(&v->counter, i, __ATOMIC_SEQ_CST) == 0;
}

/**
 * atomic_inc - increment atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1.
 */
static __inline__ void atomic_inc(atomic_t *v)
{
	atomic_add(1, v);
}

/**
 * atomic_dec - decrement atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1.
 */
static __inline__ void atomic_dec(atomic_t *v)
{
	atomic_sub(1, v);
}

/**
 * atomic_dec_and_test - decrement and test
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static __inline__ int atomic_dec_and_test(atomic_t *v)
{
	return atomic_sub_and_test(1, v);
}

/**
 * atomic_inc_and_test - increment and test
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
static __inline__ int atomic_inc_and_test(atomic_t *v)
{
	return __atomic_add_fetch(&v->counter, 1, __ATOMIC_SEQ_CST) == 0;
}

/**
 * atomic_add_negative - add and test if negative
 * @v: pointer of type atomic_t
 * @i: integer value to add
 *
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.
 */
static __inline__ int atomic_add_negative(int i, atomic_t *v)
{
	return __atomic_add_fetch(&v->counter, i, __ATOMIC_SEQ_CST) < 0;
}

/**
 * atomic_add_return - add and return
 * @v: pointer of type atomic_t
 * @i: integer value to add
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static __inline__ int atomic_add_return(int i, atomic_t *v)
{
	return __atomic_add_fetch(&v->counter, i, __ATOMIC_SEQ_CST);
}

static __inline__ int atomic_sub_return(int i, atomic_t *v)
{
	return atomic_add_return(-i, v);
}

#define barrier()   ({ asm volatile("" ::: "memory"); (void)0; })


/* Manual memory barriers
 *
 *__atomic_thread_fence does not include a compiler barrier; instead,
 * the barrier is part of __atomic_load/__atomic_store's "volatile-like"
 * semantics. If smp_wmb() is a no-op, absence of the barrier means that
 * the compiler is free to reorder stores on each side of the barrier.
 * Add one here, and similarly in smp_rmb() and smp_read_barrier_depends().
 */

#define smp_mb()                     ({ barrier(); __atomic_thread_fence(__ATOMIC_SEQ_CST); })
#define smp_mb_release()             ({ barrier(); __atomic_thread_fence(__ATOMIC_RELEASE); })
#define smp_mb_acquire()             ({ barrier(); __atomic_thread_fence(__ATOMIC_ACQUIRE); })

/* The variable that receives the old value of an atomically-accessed
 * variable must be non-qualified, because atomic builtins return values
 * through a pointer-type argument as in __atomic_load(&var, &old, MODEL).
 *
 * This macro has to handle types smaller than int manually, because of
 * implicit promotion.  int and larger types, as well as pointers, can be
 * converted to a non-qualified type just by applying a binary operator.
 */
#define typeof_strip_qual(expr)                                                    \
  typeof(                                                                          \
    __builtin_choose_expr(                                                         \
      __builtin_types_compatible_p(typeof(expr), bool) ||                          \
        __builtin_types_compatible_p(typeof(expr), const bool) ||                  \
        __builtin_types_compatible_p(typeof(expr), volatile bool) ||               \
        __builtin_types_compatible_p(typeof(expr), const volatile bool),           \
        (bool)1,                                                                   \
    __builtin_choose_expr(                                                         \
      __builtin_types_compatible_p(typeof(expr), signed char) ||                   \
        __builtin_types_compatible_p(typeof(expr), const signed char) ||           \
        __builtin_types_compatible_p(typeof(expr), volatile signed char) ||        \
        __builtin_types_compatible_p(typeof(expr), const volatile signed char),    \
        (signed char)1,                                                            \
    __builtin_choose_expr(                                                         \
      __builtin_types_compatible_p(typeof(expr), unsigned char) ||                 \
        __builtin_types_compatible_p(typeof(expr), const unsigned char) ||         \
        __builtin_types_compatible_p(typeof(expr), volatile unsigned char) ||      \
        __builtin_types_compatible_p(typeof(expr), const volatile unsigned char),  \
        (unsigned char)1,                                                          \
    __builtin_choose_expr(                                                         \
      __builtin_types_compatible_p(typeof(expr), signed short) ||                  \
        __builtin_types_compatible_p(typeof(expr), const signed short) ||          \
        __builtin_types_compatible_p(typeof(expr), volatile signed short) ||       \
        __builtin_types_compatible_p(typeof(expr), const volatile signed short),   \
        (signed short)1,                                                           \
    __builtin_choose_expr(                                                         \
      __builtin_types_compatible_p(typeof(expr), unsigned short) ||                \
        __builtin_types_compatible_p(typeof(expr), const unsigned short) ||        \
        __builtin_types_compatible_p(typeof(expr), volatile unsigned short) ||     \
        __builtin_types_compatible_p(typeof(expr), const volatile unsigned short), \
        (unsigned short)1,                                                         \
      (expr)+0))))))


#if defined(CONFIG_STATIC_ASSERT)
#define QEMU_BUILD_BUG_MSG(x, msg) _Static_assert(!(x), msg)
#else
#define QEMU_BUILD_BUG_MSG(x, msg)
#endif
#define QEMU_BUILD_BUG_ON(x) QEMU_BUILD_BUG_MSG(x, "not expecting: " #x)
/* Sanity check that the size of an atomic operation isn't "overly large".
 * Despite the fact that e.g. i686 has 64-bit atomic operations, we do not
 * want to use them because we ought not need them, and this lets us do a
 * bit of sanity checking that other 32-bit hosts might build.
 *
 * That said, we have a problem on 64-bit ILP32 hosts in that in order to
 * sync with TCG_OVERSIZED_GUEST, this must match TCG_TARGET_REG_BITS.
 * We'd prefer not want to pull in everything else TCG related, so handle
 * those few cases by hand.
 *
 * Note that x32 is fully detected with __x86_64__ + _ILP32, and that for
 * Sparc we always force the use of sparcv9 in configure. MIPS n32 (ILP32) &
 * n64 (LP64) ABIs are both detected using __mips64.
 */
#if defined(__x86_64__) || defined(__sparc__) || defined(__mips64)
# define ATOMIC_REG_SIZE  8
#else
# define ATOMIC_REG_SIZE  sizeof(void *)
#endif

/* Returns the eventual value, failed or not */
#define atomic_cmpxchg__nocheck(ptr, old, new)    ({                    \
    typeof_strip_qual(*ptr) _old = (old);                               \
    (void)__atomic_compare_exchange_n(ptr, &_old, new, false,           \
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);      \
    _old;                                                               \
})

#define atomic_cmpxchg(ptr, old, new)    ({                             \
    KEI_BUILD_BUG_ON(sizeof(*ptr) > ATOMIC_REG_SIZE);                  \
    atomic_cmpxchg__nocheck(ptr, old, new);                             \
})


#endif // _KEIATOMIC_H_
