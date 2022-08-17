#ifndef CELLOX_STRING_UTILS_H
#define CELLOX_STRING_UTILS_H

#include "common.h"

// Checks if a string contains the specified character restricted the specified length
bool containsCharacterRestricted(const char *text, char character, int length);

void resolveEscapeSequence(char *text, int *length);

#endif