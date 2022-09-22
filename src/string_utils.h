#ifndef cellox_string_utils_h
#define cellox_string_utils_h

#include "common.h"

// Checks if a string contains the specified character restricted the specified length
bool string_utils_contains_character_restricted(char const * text, char character, uint32_t length);

// Resolves all the escape sequences specified in a string literal
int string_utils_resolve_escape_sequence(char * text, uint32_t * length);

#endif