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
 * See the <"https://www.gnu.org/licenses/gpl-3.0.html">GNU General Public  *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file parse_rules.h
 * @brief Parse rule and precedence declarations.
 */

#ifndef CELLOX_PARSE_RULES_H_
#define CELLOX_PARSE_RULES_H_

/// @brief Precedences that corresponds to a single or a group of tokens
typedef enum {
    /// Lowest precedence (literals)
    PREC_NONE,
    ///  Precedence of &quot;= += -= *= /= %= **=&quot;
    PREC_ASSIGNMENT,
    ///  Precedence of &quot;or ||&quot;
    PREC_OR,
    /// Precedence of "and  &&"
    PREC_AND,
    /// Precedence of &quot;== !=&quot;
    PREC_EQUALITY,
    /// Precedence of &quot;< > <= >=&quot;
    PREC_COMPARISON,
    /// Precedence of &quot;+ -&quot;
    PREC_TERM,
    /// Precedence of &quot;* / % **&quot;
    PREC_FACTOR,
    /// Precedence of &quot;! -&quot;
    PREC_UNARY,
    ///  &quot;. () []&quot;
    PREC_CALL,
    /// Primary precedence (unused)
    PREC_PRIMARY
} precedence;

/// @brief A parse function
/// @details This provides a common pattern for all parsing function
typedef void (*parse_function_t)(bool canAssign);

/// @brief A parsing rule that applies to a special token
typedef struct {
    /// @brief The prefix rule of the parsing rule
    parse_function_t prefix;
    /// @brief The infix rule of the parsing rule
    parse_function_t infix;
    /// @brief The precedence of a token
    precedence precedence;
} parse_rule_t;

#endif