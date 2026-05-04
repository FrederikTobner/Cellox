#ifndef CELLOX_COMPILER_IMPL_GCC_ATTRIBUTES_H_
#define CELLOX_COMPILER_IMPL_GCC_ATTRIBUTES_H_

#define CLX_ALWAYS_INLINE __attribute__((always_inline)) static inline
#define CLX_NEVER_INLINE __attribute__((noinline))

#define CLX_HOT  __attribute__((hot))
#define CLX_COLD __attribute__((cold))

#define CLX_NORETURN __attribute__((noreturn))

#define CLX_PURE  __attribute__((pure))
#define CLX_CONST __attribute__((const))

#define CLX_NODISCARD __attribute__((warn_unused_result))

#define CLX_PRINTF_FORMAT(fmt_idx, va_idx) __attribute__((format(printf, fmt_idx, va_idx)))

#define CLX_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

#define CLX_LIKELY(x)   __builtin_expect(!!(x), 1)
#define CLX_UNLIKELY(x) __builtin_expect(!!(x), 0)

#if defined(BUILD_TYPE_DEBUG)
#    include <stdlib.h>
#    define CLX_UNREACHABLE() abort()
#else
#    define CLX_UNREACHABLE() __builtin_unreachable()
#endif

#endif /* CELLOX_COMPILER_IMPL_GCC_ATTRIBUTES_H_ */