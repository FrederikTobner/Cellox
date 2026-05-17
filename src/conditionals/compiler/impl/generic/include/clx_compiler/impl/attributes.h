#ifndef CELLOX_COMPILER_IMPL_GENERIC_ATTRIBUTES_H_
#define CELLOX_COMPILER_IMPL_GENERIC_ATTRIBUTES_H_

#define CLX_ALWAYS_INLINE static inline
#define CLX_NEVER_INLINE

#define CLX_HOT
#define CLX_COLD

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <stdnoreturn.h>
#define CLX_NORETURN noreturn
#else
#define CLX_NORETURN
#endif

#define CLX_PURE
#define CLX_CONST

#define CLX_NODISCARD

#define CLX_PRINTF_FORMAT(fmt_idx, va_idx)

#define CLX_NONNULL(...)

#define CLX_LIKELY(x)   (x)
#define CLX_UNLIKELY(x) (x)

#include <stdlib.h>
#define CLX_UNREACHABLE() abort()

#endif /* CELLOX_COMPILER_IMPL_GENERIC_ATTRIBUTES_H_ */