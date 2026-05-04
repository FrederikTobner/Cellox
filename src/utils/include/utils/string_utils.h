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
 * @file string_utils.h
 * @brief Header file containing the declarations of functionality regarding character sequences.
 */

#ifndef CELLOX_STRING_UTILS_H_
#define CELLOX_STRING_UTILS_H_

#include "base/common.h"

/// @brief Checks if a string contains the specified character restricted by the specified length
/// @param text The character sequence where the character is looked up
/// @param character The character that is looked up
/// @param length The length of the character sequence that is used for matching
/// @return True if the character was found, false if not
/// @details Checks for the specified character only until the specified lenth is reached.
bool string_utils_contains_character_restricted(char const * text, char character, uint32_t length);

/// @brief Resolves all the escape sequences specified in a string literal
/// @param text The character sequence where all the escape sequences are resolved
/// @param length Pointer to the lenghth of the character sequence where escape sequences are resolved
/// @return 0 if all escape sequences where resoved successful, -1 if a unknown escape sequence was found in the
/// character sequence
int string_utils_resolve_escape_sequence(char * text, uint32_t * length);

/// @brief <a href=https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash>FNV-1a</a>
/// hash function
/// @details Fownler-Noll-Vo is a non-cryptographic hash function, that comes in three different version FNV-0, FNV-1
/// and FNV-1a. There are 32-, 64-, 128-, 256-, 512-, and 1024-bit variants of the function. We use the 32-bit variant
/// to hash all the strings in cellox.
/// @param key The key that is hashed
/// @param length The length of the key
/// @return The hashvalue of the key
CLX_PURE uint32_t string_utils_hash_string(char const * key, uint32_t length);

/// @brief Error codes for string_utils_read_file
typedef enum {
    /// File was read successfully
    STRING_UTILS_READ_FILE_OK = 0,
    /// fopen failed (file not found or permission denied)
    STRING_UTILS_READ_FILE_OPEN_FAILED,
    /// malloc failed
    STRING_UTILS_READ_FILE_ALLOC_FAILED,
    /// fread returned fewer bytes than expected
    STRING_UTILS_READ_FILE_READ_FAILED,
} string_utils_read_file_error_t;

/// @brief Reads an entire file into a heap-allocated, NUL-terminated buffer
/// @param path Path to the file to read
/// @param outSize If non-NULL, receives the number of bytes read (not counting the NUL terminator)
/// @param outError If non-NULL, receives the error code on failure (STRING_UTILS_READ_FILE_OK on success)
/// @return A malloc'd buffer that the caller must free, or NULL on any failure
char * string_utils_read_file(char const * path, size_t * outSize, string_utils_read_file_error_t * outError);

/// @brief Duplicates a NUL-terminated string into a new heap allocation
/// @param text The string to duplicate
/// @return A malloc'd copy that the caller must free, or NULL on allocation failure
char * string_utils_strdup(char const * text);

/// @brief Copies the first `length` characters of `text` into a new NUL-terminated heap buffer
/// @param text The source character sequence
/// @param length Number of characters to copy (not counting the NUL terminator)
/// @return A malloc'd, NUL-terminated buffer the caller must free, or NULL on failure
char * string_utils_substr(char const * text, size_t length);

#endif
