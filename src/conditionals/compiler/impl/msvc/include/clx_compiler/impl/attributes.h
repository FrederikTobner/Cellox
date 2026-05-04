#ifndef CELLOX_COMPILER_IMPL_MSVC_ATTRIBUTES_H_
#define CELLOX_COMPILER_IMPL_MSVC_ATTRIBUTES_H_

#define CLX_ALWAYS_INLINE __forceinline static
#define CLX_NEVER_INLINE  __declspec(noinline)

#define CLX_HOT
#define CLX_COLD

#define CLX_NORETURN __declspec(noreturn)

#define CLX_PURE
#define CLX_CONST

#if _MSC_VER >= 1700
#    define CLX_NODISCARD _Check_return_
#else
#    define CLX_NODISCARD
#endif

#define CLX_PRINTF_FORMAT(fmt_idx, va_idx)

#define CLX_NONNULL(...)

#define CLX_LIKELY(x)   (x)
#define CLX_UNLIKELY(x) (x)

#if defined(BUILD_TYPE_DEBUG)
#    include <stdlib.h>
#    define CLX_UNREACHABLE() abort()
#else
#    define CLX_UNREACHABLE() __assume(0)
#endif

#endif /* CELLOX_COMPILER_IMPL_MSVC_ATTRIBUTES_H_ */