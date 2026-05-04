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
 * @file module_parser.c
 * @brief Parser for Cellox module source text.
 */

#include <module-loading/internal/module_parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/string_utils.h"

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

/// Returns true if `text` starts with `keyword` and the next character is not
/// an identifier character (i.e. the keyword is not a prefix of a longer word).
static bool module_parser_starts_with_keyword(char const * text, char const * keyword) {
    size_t keywordLength = strlen(keyword);
    if (strncmp(text, keyword, keywordLength)) {
        return false;
    }
    char c = text[keywordLength];
    bool identifierChar =
        (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    return !identifierChar;
}

/// Tries to parse an export declaration from `line`.
/// On success, sets `*exportName` to a malloc'd identifier string and returns true.
/// On failure returns false without allocating.
static bool module_parser_parse_export(char const * line, char ** exportName) {
    char const * cursor = line;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (!module_parser_starts_with_keyword(cursor, "export")) {
        return false;
    }
    cursor += strlen("export");
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (module_parser_starts_with_keyword(cursor, "var")) {
        cursor += strlen("var");
    } else if (module_parser_starts_with_keyword(cursor, "fun")) {
        cursor += strlen("fun");
    } else if (module_parser_starts_with_keyword(cursor, "class")) {
        cursor += strlen("class");
    } else {
        return false;
    }

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    char const * identifierStart = cursor;
    if (!((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') || *cursor == '_')) {
        return false;
    }
    cursor++;
    while ((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') ||
           (*cursor >= '0' && *cursor <= '9') || *cursor == '_') {
        cursor++;
    }

    *exportName = string_utils_substr(identifierStart, (size_t)(cursor - identifierStart));
    return *exportName != NULL;
}

/// Tries to parse an import declaration from `line`.
/// On success, populates `*importSpec` and returns true (caller owns the allocation).
/// On failure returns false; any partial allocation is freed before returning.
static bool module_parser_parse_import(char const * line, import_spec_t * importSpec) {
    memset(importSpec, 0, sizeof(*importSpec));

    char const * cursor = line;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (!module_parser_starts_with_keyword(cursor, "import")) {
        return false;
    }
    cursor += strlen("import");
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    // Bare-path import: import "path/to/module";
    if (*cursor == '"') {
        cursor++;
        char const * pathStart = cursor;
        while (*cursor && *cursor != '"') {
            cursor++;
        }
        if (*cursor != '"') {
            return false;
        }
        importSpec->path = string_utils_substr(pathStart, (size_t)(cursor - pathStart));
        if (!importSpec->path) {
            return false;
        }
        cursor++;
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }
        return *cursor == ';';
    }

    // Named import: import { A, B } from "path/to/module";
    if (*cursor != '{') {
        return false;
    }
    cursor++;

    for (;;) {
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }

        char const * nameStart = cursor;
        if (!((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') || *cursor == '_')) {
            module_parser_cleanup_imports(importSpec, 1);
            return false;
        }
        cursor++;
        while ((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') ||
               (*cursor >= '0' && *cursor <= '9') || *cursor == '_') {
            cursor++;
        }

        char * name = string_utils_substr(nameStart, (size_t)(cursor - nameStart));
        if (!name) {
            module_parser_cleanup_imports(importSpec, 1);
            return false;
        }

        char ** grownNames = realloc(importSpec->importedNames, (importSpec->importedNameCount + 1) * sizeof(char *));
        if (!grownNames) {
            free(name);
            module_parser_cleanup_imports(importSpec, 1);
            return false;
        }
        importSpec->importedNames = grownNames;
        importSpec->importedNames[importSpec->importedNameCount++] = name;

        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }

        if (*cursor == ',') {
            cursor++;
            continue;
        }
        if (*cursor == '}') {
            cursor++;
            break;
        }

        module_parser_cleanup_imports(importSpec, 1);
        return false;
    }

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (!module_parser_starts_with_keyword(cursor, "from")) {
        module_parser_cleanup_imports(importSpec, 1);
        return false;
    }
    cursor += strlen("from");

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (*cursor != '"') {
        module_parser_cleanup_imports(importSpec, 1);
        return false;
    }

    cursor++;
    char const * pathStart = cursor;
    while (*cursor && *cursor != '"') {
        cursor++;
    }
    if (*cursor != '"') {
        module_parser_cleanup_imports(importSpec, 1);
        return false;
    }

    importSpec->path = string_utils_substr(pathStart, (size_t)(cursor - pathStart));
    if (!importSpec->path) {
        module_parser_cleanup_imports(importSpec, 1);
        return false;
    }

    cursor++;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (*cursor != ';') {
        module_parser_cleanup_imports(importSpec, 1);
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void module_parser_append_source(source_buffer_t * buffer, char const * text, size_t length) {
    if (!length) {
        return;
    }

    size_t required = buffer->length + length;
    if (required > buffer->capacity) {
        size_t newCapacity = buffer->capacity ? buffer->capacity : 128;
        while (newCapacity < required) {
            newCapacity *= 2;
        }
        char * grown = realloc(buffer->chars, newCapacity);
        if (!grown) {
            fprintf(stderr, "Module error: Failed to allocate module source buffer\n");
            exit(EXIT_CODE_SYSTEM_ERROR);
        }
        buffer->chars = grown;
        buffer->capacity = newCapacity;
    }

    memcpy(buffer->chars + buffer->length, text, length);
    buffer->length += length;
}

bool module_parser_parse(char const * source, export_list_t * exports, import_spec_t ** imports, size_t * importCount,
                         char ** transformedSource) {
    *imports = NULL;
    *importCount = 0;
    *transformedSource = NULL;

    source_buffer_t transformed = {NULL, 0, 0};

    char const * lineStart = source;
    while (*lineStart) {
        char const * lineEnd = lineStart;
        while (*lineEnd && *lineEnd != '\n') {
            lineEnd++;
        }

        size_t lineLength = (size_t)(lineEnd - lineStart);
        bool hasLineBreak = *lineEnd == '\n';
        char * line = string_utils_substr(lineStart, lineLength);
        if (!line) {
            free(transformed.chars);
            return false;
        }

        import_spec_t importSpec;
        if (module_parser_parse_import(line, &importSpec)) {
            import_spec_t * grownImports = realloc(*imports, ((*importCount) + 1) * sizeof(import_spec_t));
            if (!grownImports) {
                free(line);
                module_parser_cleanup_imports(&importSpec, 1);
                module_parser_cleanup_imports(*imports, *importCount);
                *imports = NULL;
                *importCount = 0;
                free(transformed.chars);
                return false;
            }
            *imports = grownImports;
            (*imports)[(*importCount)++] = importSpec;
            if (hasLineBreak) {
                module_parser_append_source(&transformed, "\n", 1);
            }
        } else {
            char * exportName = NULL;
            if (module_parser_parse_export(line, &exportName)) {
                bool duplicateExport = false;
                for (size_t i = 0; i < exports->count; i++) {
                    if (!strcmp(exports->names[i], exportName)) {
                        duplicateExport = true;
                        break;
                    }
                }
                if (duplicateExport) {
                    free(exportName);
                    free(line);
                    module_parser_cleanup_imports(*imports, *importCount);
                    *imports = NULL;
                    *importCount = 0;
                    free(transformed.chars);
                    return false;
                }

                if (exports->count + 1 > exports->capacity) {
                    size_t newCapacity = exports->capacity ? exports->capacity * 2 : 8;
                    char ** grownExports = realloc(exports->names, newCapacity * sizeof(char *));
                    if (!grownExports) {
                        free(exportName);
                        free(line);
                        module_parser_cleanup_imports(*imports, *importCount);
                        *imports = NULL;
                        *importCount = 0;
                        free(transformed.chars);
                        return false;
                    }
                    exports->names = grownExports;
                    exports->capacity = newCapacity;
                }
                exports->names[exports->count++] = exportName;

                char const * trimmed = line;
                while (*trimmed == ' ' || *trimmed == '\t') {
                    trimmed++;
                }
                size_t indentLength = (size_t)(trimmed - line);
                module_parser_append_source(&transformed, line, indentLength);
                module_parser_append_source(&transformed, trimmed + strlen("export"),
                                            strlen(trimmed) - strlen("export"));
            } else {
                module_parser_append_source(&transformed, line, lineLength);
            }
            if (hasLineBreak) {
                module_parser_append_source(&transformed, "\n", 1);
            }
        }

        free(line);
        lineStart = *lineEnd ? lineEnd + 1 : lineEnd;
    }

    module_parser_append_source(&transformed, "\0", 1);
    *transformedSource = transformed.chars;
    return true;
}

void module_parser_cleanup_imports(import_spec_t * imports, size_t importCount) {
    for (size_t i = 0; i < importCount; i++) {
        free(imports[i].path);
        for (size_t j = 0; j < imports[i].importedNameCount; j++) {
            free(imports[i].importedNames[j]);
        }
        free(imports[i].importedNames);
    }
    free(imports);
}

void module_parser_export_list_free(export_list_t * list) {
    for (size_t i = 0; i < list->count; i++) {
        free(list->names[i]);
    }
    free(list->names);
    list->names = NULL;
    list->count = 0;
    list->capacity = 0;
}
