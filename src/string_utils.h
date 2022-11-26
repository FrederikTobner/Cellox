#ifndef CELLOX_STRING_UTILS_H_
#define CELLOX_STRING_UTILS_H_

#include "common.h"

/// @brief Checks if a string contains the specified character restricted the specified length
/// @param text The character sequence where the character is looked up
/// @param character The character that is looked up
/// @param length The length of the character sequence that is used for matching
/// @return True if the character was found, false if not
bool string_utils_contains_character_restricted(char const * text, char character, uint32_t length);

/// @brief Resolves all the escape sequences specified in a string literal and returns an integer, that indicates whether an excape sequence in the string were valid
/// @param text The character sequence where all the escape sequences are resolved
/// @param length The lenghth of the character sequence where escape sequences are resolved
/// @return 0 if all escape sequences where resoved successful, -1 if a unknown escape sequence was found in the character sequence
int string_utils_resolve_escape_sequence(char * text, uint32_t * length);

#endif
