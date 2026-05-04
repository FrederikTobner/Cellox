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
 * @file macros.h
 * @brief General-purpose utility macros.
 *
 * @details Provides small, self-contained utility macros that are used in
 * multiple places across the codebase but do not belong to a specific module.
 *
 * Included indirectly through common.h.
 *
 * @note Value / object model macros (IS_*, AS_*, *_VAL, GROW_ARRAY, …) live
 * in their own headers to stay close to the types they operate on.
 */

#ifndef CELLOX_MACROS_H_
#define CELLOX_MACROS_H_

// ── Array utilities ───────────────────────────────────────────────────────────

/// Number of elements in a fixed-size C array.
/// Evaluates to a compile-time constant.  Do NOT use on pointer decay.
///
/// Example:
///   int arr[] = {1, 2, 3};
///   size_t n = CLX_ARRAY_SIZE(arr); // 3
#define CLX_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// ── Suppress unused warnings ──────────────────────────────────────────────────

/// Silence unused-variable and unused-parameter warnings portably.
///
/// Example:
///   void callback(int event, void * ctx) {
///       CLX_UNUSED(ctx);
///       handle(event);
///   }
#define CLX_UNUSED(x) ((void)(x))

// ── Stringification ───────────────────────────────────────────────────────────

/// Convert a token to a string literal, expanding macros first.
///
/// Example:
///   #define VERSION 42
///   const char * s = CLX_STRINGIFY(VERSION); // "42"
#define CLX_STRINGIFY_IMPL(x) #x
#define CLX_STRINGIFY(x)      CLX_STRINGIFY_IMPL(x)

// ── Token concatenation ───────────────────────────────────────────────────────

/// Paste two tokens together, expanding macros in both before pasting.
///
/// Example:
///   #define PREFIX my_
///   CLX_CONCAT(PREFIX, func)()  →  my_func()
#define CLX_CONCAT_IMPL(a, b) a##b
#define CLX_CONCAT(a, b)      CLX_CONCAT_IMPL(a, b)

// ── Arithmetic min / max ──────────────────────────────────────────────────────

/// Type-safe minimum and maximum that evaluate each argument exactly once.
/// Prefer these over the classic multi-evaluation versions.
#define CLX_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLX_MAX(a, b) ((a) > (b) ? (a) : (b))

// ── Alignment ────────────────────────────────────────────────────────────────

/// Round `x` up to the nearest multiple of `align` (alignment must be a
/// power of two).
///
/// Example:
///   CLX_ALIGN_UP(13, 8) == 16
#define CLX_ALIGN_UP(x, align) (((x) + (align) - 1u) & ~((align) - 1u))

#endif /* CELLOX_MACROS_H_ */
