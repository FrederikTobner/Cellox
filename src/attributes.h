/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file attributes.h
 * @brief Portable compiler-attribute macros.
 *
 * @details Provides a single, centralized layer of portability wrappers over
 * GCC / Clang / MSVC extension attributes. Every macro falls back to a safe
 * no-op when the compiler does not support the underlying intrinsic.
 *
 * Relies on the compile-time definitions injected by CMake:
 *   COMPILER_GCC, COMPILER_CLANG, COMPILER_MSVC, COMPILER_INTEL
 *
 * Include indirectly through common.h — do not depend on include order.
 */

#ifndef CELLOX_ATTRIBUTES_H_
#define CELLOX_ATTRIBUTES_H_

// ── always_inline ─────────────────────────────────────────────────────────────
/// Forces the compiler to inline a function, even when inlining is disabled
/// globally.  Replaces the `static inline` qualifier — do not write both.
/// Usage:  CLX_ALWAYS_INLINE int foo(void) { return 42; }
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_ALWAYS_INLINE __attribute__((always_inline)) static inline
#elif defined(COMPILER_MSVC)
#    define CLX_ALWAYS_INLINE __forceinline static
#else
#    define CLX_ALWAYS_INLINE static inline
#endif

// ── never_inline ──────────────────────────────────────────────────────────────
/// Prevents the compiler from inlining a function.
/// Useful for cold paths that bloat hot inlined code.
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_NEVER_INLINE __attribute__((noinline))
#elif defined(COMPILER_MSVC)
#    define CLX_NEVER_INLINE __declspec(noinline)
#else
#    define CLX_NEVER_INLINE
#endif

// ── hot / cold ────────────────────────────────────────────────────────────────
/// CLX_HOT  — the function is on a critical hot path; the compiler should
///            optimise it more aggressively and give it preferential placement.
/// CLX_COLD — the function is rarely executed (error paths, init code, etc.);
///            the compiler can de-prioritise it and use size over speed.
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_HOT  __attribute__((hot))
#    define CLX_COLD __attribute__((cold))
#else
#    define CLX_HOT
#    define CLX_COLD
#endif

// ── noreturn ──────────────────────────────────────────────────────────────────
/// Declares that a function never returns to its caller (calls exit / longjmp /
/// throws unconditionally).  Allows the compiler to elide dead-code after a
/// call to such a function and to omit the return-value boilerplate.
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_NORETURN __attribute__((noreturn))
#elif defined(COMPILER_MSVC)
#    define CLX_NORETURN __declspec(noreturn)
#else
/// C99 / C11 standard keyword (falls back to _Noreturn if available).
#    if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#        include <stdnoreturn.h>
#        define CLX_NORETURN noreturn
#    else
#        define CLX_NORETURN
#    endif
#endif

// ── pure / const ──────────────────────────────────────────────────────────────
/// CLX_PURE  — the function has no side effects and its return value depends
///             only on parameters and global (read-only) memory.
///             The compiler may avoid repeated calls with the same arguments.
/// CLX_CONST — stronger form: the return value depends only on the parameters;
///             no reads of global/heap memory are performed.  Suitable for
///             arithmetic or type-pun helpers.
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_PURE  __attribute__((pure))
#    define CLX_CONST __attribute__((const))
#else
#    define CLX_PURE
#    define CLX_CONST
#endif

// ── nodiscard ─────────────────────────────────────────────────────────────────
/// Emit a warning if the return value of the annotated function is discarded.
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_NODISCARD __attribute__((warn_unused_result))
#elif defined(COMPILER_MSVC) && _MSC_VER >= 1700
#    define CLX_NODISCARD _Check_return_
#else
#    define CLX_NODISCARD
#endif

// ── printf format ─────────────────────────────────────────────────────────────
/// Declare that a variadic function accepts a printf-style format string and
/// enable format-string checking at compile time.
///
/// @param fmt_idx  1-based index of the format-string parameter.
/// @param va_idx   1-based index of the first variadic argument.
///                 Use 0 for vprintf-style wrappers (va_list, not ...).
///
/// Example:  void log(char const * fmt, ...) CLX_PRINTF_FORMAT(1, 2);
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_PRINTF_FORMAT(fmt_idx, va_idx) __attribute__((format(printf, fmt_idx, va_idx)))
#else
#    define CLX_PRINTF_FORMAT(fmt_idx, va_idx)
#endif

// ── nonnull ───────────────────────────────────────────────────────────────────
/// Declare that the listed (1-based, comma-separated) pointer parameters must
/// not be NULL.  The compiler emits a warning or error if a NULL is passed.
/// Leave the argument list empty to mark all pointer parameters.
///
/// Example:  void process(char * buf, size_t len) CLX_NONNULL(1);
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#else
#    define CLX_NONNULL(...)
#endif

// ── branch-prediction hints ───────────────────────────────────────────────────
/// CLX_LIKELY(x)   — hint that the Boolean expression `x` is usually true.
/// CLX_UNLIKELY(x) — hint that the Boolean expression `x` is usually false.
///
/// Both macros expand to `x` on compilers that lack __builtin_expect so code
/// is always correct; they only influence instruction scheduling / branch
/// prediction on GCC and Clang.
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_LIKELY(x)   __builtin_expect(!!(x), 1)
#    define CLX_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#    define CLX_LIKELY(x)   (x)
#    define CLX_UNLIKELY(x) (x)
#endif

// ── unreachable ───────────────────────────────────────────────────────────────
/// Mark a code path as unreachable.
///
/// In release builds this tells the optimizer it may assume the path is never
/// taken, enabling dead-code elimination and better register allocation.
/// In debug builds (BUILD_TYPE_DEBUG defined) it calls abort() so that an
/// unexpected execution of the "unreachable" path is immediately visible.
///
/// Usage:
///   switch (op) {
///     case A: ...; break;
///     default: CLX_UNREACHABLE();
///   }
#if defined(BUILD_TYPE_DEBUG)
#    include <stdlib.h>
#    define CLX_UNREACHABLE() abort()
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    define CLX_UNREACHABLE() __builtin_unreachable()
#elif defined(COMPILER_MSVC)
#    define CLX_UNREACHABLE() __assume(0)
#else
#    include <stdlib.h>
#    define CLX_UNREACHABLE() abort()
#endif

#endif /* CELLOX_ATTRIBUTES_H_ */
