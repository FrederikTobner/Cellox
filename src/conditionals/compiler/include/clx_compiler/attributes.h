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
 * @brief Public compiler-attributes API.
 *
 * The actual implementation is selected by CMake include paths and must be
 * provided at clx_compiler/impl/attributes.h.
 */

#ifndef CELLOX_COMPILER_ATTRIBUTES_H_
#define CELLOX_COMPILER_ATTRIBUTES_H_

#include "clx_compiler/impl/attributes.h"

/* Enforce a complete compiler attributes contract at compile time. */
#ifndef CLX_ALWAYS_INLINE
#error "Missing CLX_ALWAYS_INLINE in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_NEVER_INLINE
#error "Missing CLX_NEVER_INLINE in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_HOT
#error "Missing CLX_HOT in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_COLD
#error "Missing CLX_COLD in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_NORETURN
#error "Missing CLX_NORETURN in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_PURE
#error "Missing CLX_PURE in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_CONST
#error "Missing CLX_CONST in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_NODISCARD
#error "Missing CLX_NODISCARD in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_PRINTF_FORMAT
#error "Missing CLX_PRINTF_FORMAT in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_NONNULL
#error "Missing CLX_NONNULL in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_LIKELY
#error "Missing CLX_LIKELY in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_UNLIKELY
#error "Missing CLX_UNLIKELY in clx_compiler/impl/attributes.h"
#endif
#ifndef CLX_UNREACHABLE
#error "Missing CLX_UNREACHABLE in clx_compiler/impl/attributes.h"
#endif

#endif /* CELLOX_COMPILER_ATTRIBUTES_H_ */